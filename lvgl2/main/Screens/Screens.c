#include "lvgl.h"
#include "Screens.h"
#include "Styles.h"
#include "ImportData.h"
#include "Thermal.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "StepperMotor.h"
#include <stdbool.h>
#include "WiFi.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <string.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include <time.h>


static const char *TAG = "Main";
screens_t screens;
static lv_obj_t *menu_panel;   // Sidebar menu
static bool menu_open = false; // Menu state
lv_obj_t *MainContainer;
lv_obj_t *text_input;
lv_obj_t *mboxConnect;
static lv_obj_t *keyboard;
lv_obj_t *Time_label;
lv_anim_t BarAnimation;
lv_obj_t *URL_text_input;
lv_obj_t *UserName_text_input;
lv_obj_t *Password_text_input;
lv_obj_t *underline; // the Hambguer Menu Underline object 
lv_obj_t *underlineFont; // the font Underline object 
// top bar icons
lv_obj_t *wifi_label;

typedef struct
{
    lv_obj_t *SSID_Label;
    lv_obj_t *password_box;
} Wifi_credentials_t;
void update_ui_time(const char *time_str)
{
    lv_label_set_text(Time_label, time_str); // Update LVGL label
}
static void time_timer_cb(lv_timer_t *timer)
{
    print_time(); // This will trigger the UI update callback
}

static void toggle_menu(lv_event_t *e)
{
    if (menu_open)
    {
        lv_obj_add_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); // Hide menu
    }
    else
    {
        lv_obj_clear_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); // Show menu
    }
    menu_open = !menu_open;
}

static const char *custom_kb_map[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
    "ABC", "z", "x", "c", "v", "b", "n", "m", LV_SYMBOL_BACKSPACE, "\n",
    "1#", " ", ".", "⇧", NULL};

static const lv_btnmatrix_ctrl_t custom_kb_ctrl_map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // First row
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 5, 1, 3};

static const char *custom_kb_map_upper[] = {
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "\n",
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "\n",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", "\n",
    "abc", "Z", "X", "C", "V", "B", "N", "M", LV_SYMBOL_BACKSPACE, "\n",
    "1#", " ", ".", "⇧", NULL};

static const lv_btnmatrix_ctrl_t custom_kb_ctrl_map_upper[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // First row
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 5, 1, 3};

      
    char* MakeJSON(const char* LabelText) {
        cJSON *root = cJSON_CreateObject();  // Create a JSON object
        if (root == NULL) {
            return NULL;
        }
        static char USER[50];
        ReadStringsFromNVS("User", USER, sizeof(USER));
        cJSON_AddStringToObject(root, "LabelText", LabelText);
        time_t now = 0;
        cJSON_AddNumberToObject(root, "Time", time(&now));
        cJSON_AddStringToObject(root, "User", USER);
        char *json_string = cJSON_PrintUnformatted(root);  // Returns a dynamically allocated string
        cJSON_Delete(root);
        return json_string; 
    }

static void PrintLabel_event_cb(lv_event_t *e)
{
    print_text(lv_textarea_get_text(text_input));                     // sends text to the thermal printer
    char* json_str = MakeJSON(lv_textarea_get_text(text_input));
    mqtt_send("clippits/connect1", json_str); // sends text to the mqtt
    free(json_str);
    lv_textarea_set_text(text_input, "");                            
    lv_anim_start(&BarAnimation);
    xTaskCreate(MakeLabel, "MakeLabel", 2048, NULL, 5, NULL);
}

static void ConnectWifi_event_cb(lv_event_t *e)
{
    printf("connecting to wifi\n");
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    lv_obj_t *label = lv_obj_get_child(obj, 0); // Get first child (label)

    if (label != NULL)
    {
        const char *text = lv_label_get_text(label); // Get label text
        buildPWMsgBox(text);
    }
}

static void ConnectWifiSTA_event_cb(lv_event_t *e)
{
    // Wifi_credentials
    Wifi_credentials_t *credentials = (Wifi_credentials_t *)lv_event_get_user_data(e);
    if (credentials && credentials->password_box)
    {
        const char *password = lv_textarea_get_text(credentials->password_box);
        const char *SSID = lv_label_get_text(credentials->SSID_Label);
        esp_err_t err = Wifi_Connect_STA(SSID, password);

        if (err == ESP_OK)
        {
            lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
            lv_obj_t *btn = lv_event_get_target(e);
            lv_obj_t *parent = lv_obj_get_parent(btn);
            if (parent)
            {
                lv_async_call((lv_async_cb_t)lv_obj_del, parent);
            }
        }
        else
        {
            printf("WiFi connection failed! Error code: %d\n", err);
            show_warning_box();
        }
    }
}
void show_warning_box()
{
    // Create a container for the warning box
    lv_obj_t *mbox = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mbox, 250, 200);
    lv_obj_center(mbox);
    lv_obj_set_style_border_width(mbox, 2, 0);

    // Create a label for the warning message
    lv_obj_t *label = lv_label_create(mbox);
    lv_label_set_text(label, "Password incorrect!");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    // Create a "Close" button
    lv_obj_t *close_btn = lv_btn_create(mbox);
    lv_obj_set_size(close_btn, 160, 60);
    lv_obj_align(close_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(close_btn, mboxCloseBtn_event_cb, LV_EVENT_CLICKED, NULL);

    // Add label to the button
    lv_obj_t *btn_label = lv_label_create(close_btn);
    lv_label_set_text(btn_label, "Close");
    lv_obj_center(btn_label);
}

void UpdateWifiSymbol(bool status)
{
    if (status)
    {
        lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    }
    else
    {
        lv_label_set_text(wifi_label, "");
    }
}
static void text_input_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_move_foreground(keyboard);
        lv_keyboard_set_textarea(keyboard, ta);
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    if (code == LV_EVENT_DEFOCUSED)
    {
        lv_keyboard_set_textarea(keyboard, NULL);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

static void buildPWMsgBox(char *SSIDText)
{
    static Wifi_credentials_t wifi_credentials; // SSID_Label  wifi_credentials.SSID_Label
    printf("SSIDText: %s\n", SSIDText);
    mboxConnect = lv_obj_create(MainContainer);
    lv_obj_set_size(mboxConnect, 300, 170);
    lv_obj_align(mboxConnect, LV_ALIGN_CENTER, 0, -100); // Offset +10 X, -20 Y

    wifi_credentials.SSID_Label = lv_label_create(mboxConnect);
    lv_label_set_text(wifi_credentials.SSID_Label, SSIDText);
    lv_obj_align(wifi_credentials.SSID_Label, LV_ALIGN_TOP_LEFT, 0, 0);

    wifi_credentials.password_box = lv_textarea_create(mboxConnect);
    lv_obj_set_size(wifi_credentials.password_box, 280, 40);
    lv_obj_align_to(wifi_credentials.password_box, wifi_credentials.SSID_Label, LV_ALIGN_TOP_LEFT, 0, 30);
    lv_textarea_set_placeholder_text(wifi_credentials.password_box, "Password?");
    lv_obj_add_event_cb(wifi_credentials.password_box, text_input_event_cb, LV_EVENT_ALL, keyboard);

    lv_obj_t *mboxConnectBtn = lv_btn_create(mboxConnect);
    lv_obj_align(mboxConnectBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(mboxConnectBtn, 100, 60);
    lv_obj_t *btnLabel = lv_label_create(mboxConnectBtn);
    lv_label_set_text(btnLabel, "Connect");
    // const char * password = lv_textarea_get_text(mboxPassword);

    lv_obj_add_event_cb(mboxConnectBtn, ConnectWifiSTA_event_cb, LV_EVENT_PRESSED, &wifi_credentials);
    lv_obj_center(btnLabel);

    lv_obj_t *mboxCloseBtn = lv_btn_create(mboxConnect);
    lv_obj_add_event_cb(mboxCloseBtn, mboxCloseBtn_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_align(mboxCloseBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_size(mboxCloseBtn, 100, 60);
    lv_obj_t *btnLabel2 = lv_label_create(mboxCloseBtn);
    lv_label_set_text(btnLabel2, "Cancel");
    lv_obj_center(btnLabel2);
}

static void mboxCloseBtn_event_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(btn);
    if (parent)
    {
        lv_async_call((lv_async_cb_t)lv_obj_del, parent); // Defer deletion
    }
}

static void Scan_event_cb(lv_event_t *e)
{
    wifi_ap_record_t ap_info[20];
    uint16_t ap_count = 0;
    wifi_scan(ap_info, &ap_count);

    ESP_LOGI(TAG, "Found %d access points", ap_count);
    lv_obj_t *listContainer = lv_obj_create(MainContainer);
    lv_obj_set_size(listContainer, 320, 400);                 // Set container size
    lv_obj_align(listContainer, LV_ALIGN_TOP_MID, 0, 70);     // Align at the top center
    lv_obj_set_flex_flow(listContainer, LV_FLEX_FLOW_COLUMN); // Set flexbox direction (vertical)
    lv_obj_set_style_pad_row(listContainer, 10, 0);           // Add spacing between items
    for (int i = 0; i < ap_count; i++)
    {
        lv_obj_t *SingleRecord = lv_btn_create(listContainer);
        lv_obj_set_size(SingleRecord, 270, 70);
        lv_obj_t *WifiLabel = lv_label_create(SingleRecord);
        lv_obj_add_style(SingleRecord, &style, 0);
        // lv_obj_set_style_bg_color(SingleRecord, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text_fmt(WifiLabel, "%s", ap_info[i].ssid);
        lv_obj_align(WifiLabel, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_t *WifiLabelStrength = lv_label_create(SingleRecord);
        // lv_label_set_text_fmt(WifiLabelStrength, "%d", ap_info[i].rssi);  LV_SYMBOL_WIFI
        lv_label_set_text(WifiLabelStrength, LV_SYMBOL_WIFI);
        lv_obj_align(WifiLabelStrength, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_add_event_cb(SingleRecord, ConnectWifi_event_cb, LV_EVENT_CLICKED, NULL);
    }
}

static void Nudge(lv_event_t *e)
{
    bool *nudge_value = (bool *)lv_event_get_user_data(e); // Retrieve the passed bool
    // this is stupid but But wont workother wise
    if (nudge_value && *nudge_value)
    {
        printf("Nudge Down\n");
        NudgeBarrel(1);
    }
    else
    {
        printf("Nudge Up\n");
        NudgeBarrel(0);
    }
}
static void MovePaper(lv_event_t *e)
{
    JogPrinter();
}

static void ChangeDifferentScreen(lv_event_t *e){
    lv_obj_clean(MainContainer);
    lv_obj_add_flag(menu_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_obj_set_size(underline, lv_obj_get_width(label), 2); 
    lv_obj_align_to(underline, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    const char *text = lv_label_get_text(label);

    if (strcmp(text, "Just Print") == 0) {
        CreateJustPrint();
    } else if (strcmp(text, "Wifi Settings") == 0) {
        CreateWifiSettings();
    } else if (strcmp(text, "Calibration") == 0) {
        CreateCalibration();
    } else if (strcmp(text, "Export Settings") == 0) {
        CreateExportSettings();
    } else if (strcmp(text, "Import Data") == 0) {
        CreateImportData();
        
    }
}

void CreateScreens()
{
    HomeScreen();
    SplashScreen();
}

void HomeScreen()
{
    StylesInit();
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);
    screens.home = lv_obj_create(NULL);
    
    lv_obj_set_style_bg_color(screens.home, lv_color_white(), 0);
    
    lv_obj_t *TopContainer = lv_obj_create(screens.home);
    lv_obj_set_style_pad_all(TopContainer, 0, 0);
    lv_obj_clear_flag(TopContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(TopContainer, 320, 70);
    lv_obj_set_pos(TopContainer, 0, 0);
    //lv_obj_set_style_bg_color(TopContainer, lv_color_hex(0xFFFFFF), 0); // Set white background
    lv_obj_set_style_bg_opa(TopContainer, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_obj_set_style_border_width(TopContainer, 0, LV_PART_MAIN);

    lv_obj_t *Battery_label = lv_label_create(screens.home);
    lv_label_set_text(Battery_label, LV_SYMBOL_BATTERY_FULL);
    lv_obj_align(Battery_label, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_style(Battery_label, &TopIconStyle, 0);

    wifi_label = lv_label_create(screens.home);
    lv_label_set_text(wifi_label, "");
    // lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_align(wifi_label, LV_ALIGN_TOP_RIGHT, -40, 10);
    lv_obj_add_style(wifi_label, &TopIconStyle, 0);

    lv_obj_t *GSM_label = lv_label_create(screens.home);
    lv_label_set_text(GSM_label, "2G");
    lv_obj_align(GSM_label, LV_ALIGN_TOP_RIGHT, -70, 10);
    lv_obj_add_style(GSM_label, &TopIconStyle, 0);

    Time_label = lv_label_create(screens.home);
    lv_label_set_text(Time_label, "");
    lv_obj_align(Time_label, LV_ALIGN_TOP_RIGHT, -100, 10);
    lv_obj_add_style(Time_label, &TopIconStyle, 0);
    register_time_callback(update_ui_time);      // Register callback
    lv_timer_create(time_timer_cb, 60000, NULL); // Update every 60 seconds

    MainContainer = lv_obj_create(screens.home);
    lv_obj_set_style_pad_all(MainContainer, 0, 0);
    lv_obj_clear_flag(MainContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(MainContainer, 320, 450);
    lv_obj_set_pos(MainContainer, 0, 30);
    lv_obj_set_style_bg_color(MainContainer, lv_color_hex(0xFFFFFF), 0); // Set white background
    lv_obj_set_style_border_width(MainContainer, 0, LV_PART_MAIN);


    // 📌 Create the menu panel (sidebar)
    menu_panel = lv_obj_create(screens.home);
    lv_obj_set_size(menu_panel, 200, 330);             // Width x Height
    lv_obj_align(menu_panel, LV_ALIGN_LEFT_MID, 0, 0); // Position at left
    lv_obj_set_style_bg_color(menu_panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_add_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); // Start hidden

    // 📌 Add menu buttons
    lv_obj_t *btn1 = lv_btn_create(menu_panel);
    lv_obj_add_style(btn1, &SideMenuButtons, 0);
    lv_obj_set_size(btn1, 150, 50);
    lv_obj_align(btn1, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "Just Print");

    // create the underline and assign it to the first screen i.e the Just print screen
    underline = lv_obj_create(menu_panel);
    lv_obj_refr_size(label1);  // Force size recalculation as the label has just been drawn getting the size will retun 0 otheriwse
    lv_obj_set_size(underline, lv_obj_get_width(label1), 2);  // Width, Height
    lv_obj_add_style(underline,&UnderlineStyle,0);
    lv_obj_align_to(underline, label1, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);

    lv_obj_add_event_cb(btn1, ChangeDifferentScreen, LV_EVENT_PRESSED, NULL);

    lv_obj_t *btn2 = lv_btn_create(menu_panel);
    lv_obj_add_style(btn2, &SideMenuButtons, 0);
    lv_obj_align_to(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_size(btn2, 150, 50);
    lv_obj_set_pos(btn2, 0, 50);
    lv_obj_t *label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Wifi Settings");
    lv_obj_add_event_cb(btn2, ChangeDifferentScreen, LV_EVENT_PRESSED, NULL);

    lv_obj_t *Menubtn3 = lv_btn_create(menu_panel);
    lv_obj_add_style(Menubtn3, &SideMenuButtons, 0);
    lv_obj_set_size(Menubtn3, 150, 50);
    lv_obj_align(Menubtn3, LV_ALIGN_TOP_LEFT, 0, 100);
    lv_obj_t *Menulabel3 = lv_label_create(Menubtn3);
    lv_label_set_text(Menulabel3, "Calibration");
    lv_obj_add_event_cb(Menubtn3, ChangeDifferentScreen, LV_EVENT_PRESSED, NULL);

    lv_obj_t *Menubtn4 = lv_btn_create(menu_panel);
    lv_obj_add_style(Menubtn4, &SideMenuButtons, 0);
    lv_obj_set_size(Menubtn4, 150, 50);
    lv_obj_align(Menubtn4, LV_ALIGN_TOP_LEFT, 0, 150);
    lv_obj_t *Menulabel4 = lv_label_create(Menubtn4);
    lv_label_set_text(Menulabel4, "Export Settings");
    lv_obj_add_event_cb(Menubtn4, ChangeDifferentScreen, LV_EVENT_PRESSED, NULL);

    lv_obj_t *Menubtn5 = lv_btn_create(menu_panel);
    lv_obj_add_style(Menubtn5, &SideMenuButtons, 0);
    lv_obj_set_size(Menubtn5, 150, 50);
    lv_obj_align(Menubtn5, LV_ALIGN_TOP_LEFT, 0, 200);
    lv_obj_t *Menulabel5 = lv_label_create(Menubtn5);
    lv_label_set_text(Menulabel5, "Import Data");
    lv_obj_add_event_cb(Menubtn5, ChangeDifferentScreen, LV_EVENT_PRESSED, NULL);

    lv_obj_t *Menubtn6 = lv_btn_create(menu_panel);
    lv_obj_add_style(Menubtn6, &SideMenuButtons, 0);
    lv_obj_set_size(Menubtn6, 150, 50);
    lv_obj_align(Menubtn6, LV_ALIGN_TOP_LEFT, 0, 250);
    lv_obj_t *Menulabel6 = lv_label_create(Menubtn6);
    lv_label_set_text(Menulabel6, "Read RFID");
    lv_obj_add_event_cb(Menubtn6, ChangeDifferentScreen, LV_EVENT_PRESSED, NULL);

    // 📌 Create hamburger button  Import Data
    lv_obj_t *menu_btn = lv_btn_create(screens.home);
    lv_obj_set_size(menu_btn, 60, 60);
    lv_obj_align(menu_btn, LV_ALIGN_TOP_LEFT, 0, 0); // Top left corner
    lv_obj_add_event_cb(menu_btn, toggle_menu, LV_EVENT_PRESSED, NULL);

    // 📌 Add ☰ icon (hamburger)
    lv_obj_t *menu_icon = lv_label_create(menu_btn);
    lv_label_set_text(menu_icon, LV_SYMBOL_LIST); // ☰ icon
    lv_obj_center(menu_icon);
    lv_obj_add_style(menu_btn, &style, 0);
    // Create the first screen 
    CreateJustPrint();

}
void SplashScreen()
{
    screens.splash = lv_obj_create(NULL);
    // lv_obj_t* img = lv_image_create(screens.splash);
     //lv_img_set_src(img, &clippits);
}
// Function to switch screens dynamically
void switch_screen(lv_obj_t *screen)
{
    lv_scr_load(screen);
}
void arc_anim_ready_cb(lv_anim_t *a)
{
    lv_arc_set_value(a->var, 0);  // Reset arc value to 0
}

void ChangeFont(lv_event_t *e){
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    
    lv_obj_set_size(underlineFont, lv_obj_get_width(label), 2); 
    lv_obj_align_to(underlineFont, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 4);
    const char *text = lv_label_get_text(label);

    if (strcmp(text, "10") == 0) {
        lv_obj_set_style_text_font(text_input, &lv_font_montserrat_10, LV_PART_MAIN);
    } else if (strcmp(text, "14") == 0) {
        lv_obj_set_style_text_font(text_input, &lv_font_montserrat_14, LV_PART_MAIN);
    } else if (strcmp(text, "18") == 0) {
        lv_obj_set_style_text_font(text_input, &lv_font_montserrat_18, LV_PART_MAIN);
    } else if (strcmp(text, "24") == 0) {
        lv_obj_set_style_text_font(text_input, &lv_font_montserrat_24, LV_PART_MAIN);
    } else if (strcmp(text, "28") == 0) {
        lv_obj_set_style_text_font(text_input, &lv_font_montserrat_28, LV_PART_MAIN);
    }
    
}

void CreateJustPrint()
{
    lv_obj_t *kb = lv_keyboard_create(MainContainer);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_LOWER, custom_kb_map, custom_kb_ctrl_map);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_UPPER, custom_kb_map_upper, custom_kb_ctrl_map_upper);
    lv_obj_set_size(kb, 320, 250);

    lv_obj_t *TenFontButton = lv_btn_create(MainContainer);                          // Create button
    lv_obj_set_pos(TenFontButton, 0, 150);                                         // Set position
    lv_obj_set_size(TenFontButton, 62, 50);
    lv_obj_add_style(TenFontButton, &SideMenuButtons, 0);
    lv_obj_t *TenFontButtonLabel = lv_label_create(TenFontButton);
    lv_label_set_text(TenFontButtonLabel, "10");
    lv_obj_set_style_text_font(TenFontButtonLabel, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_align_to(TenFontButtonLabel, TenFontButton, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_event_cb(TenFontButton, ChangeFont, LV_EVENT_PRESSED, NULL);


    lv_obj_t *FourteenFontButton = lv_btn_create(MainContainer);                          // Create button
    lv_obj_set_pos(FourteenFontButton, 62, 150);                                         // Set position
    lv_obj_set_size(FourteenFontButton, 62, 50);
    lv_obj_add_style(FourteenFontButton, &SideMenuButtons, 0);
    lv_obj_t *FourteenFontButtonLabel = lv_label_create(FourteenFontButton);
    lv_label_set_text(FourteenFontButtonLabel, "14");
    lv_obj_set_style_text_font(FourteenFontButtonLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align_to(FourteenFontButtonLabel, FourteenFontButton, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_event_cb(FourteenFontButton, ChangeFont, LV_EVENT_PRESSED, NULL);

    lv_obj_t *EightteenFontButton = lv_btn_create(MainContainer);                          // Create button
    lv_obj_set_pos(EightteenFontButton, 124, 150);                                         // Set position
    lv_obj_set_size(EightteenFontButton, 62, 50);
    lv_obj_add_style(EightteenFontButton, &SideMenuButtons, 0);
    lv_obj_t *EightteenFontButtonLabel = lv_label_create(EightteenFontButton);
    lv_label_set_text(EightteenFontButtonLabel, "18");
    lv_obj_set_style_text_font(EightteenFontButtonLabel, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align_to(EightteenFontButtonLabel, EightteenFontButton, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_event_cb(EightteenFontButton, ChangeFont, LV_EVENT_PRESSED, NULL);

    lv_obj_t *TwentyfourFontButton = lv_btn_create(MainContainer);                          // Create button
    lv_obj_set_pos(TwentyfourFontButton, 186, 150);                                         // Set position
    lv_obj_set_size(TwentyfourFontButton, 62, 50);
    lv_obj_add_style(TwentyfourFontButton, &SideMenuButtons, 0);
    lv_obj_t *TwentyfourFontButtonLabel = lv_label_create(TwentyfourFontButton);
    lv_label_set_text(TwentyfourFontButtonLabel, "24");
    lv_obj_set_style_text_font(TwentyfourFontButtonLabel, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align_to(TwentyfourFontButtonLabel, TwentyfourFontButton, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_event_cb(TwentyfourFontButton, ChangeFont, LV_EVENT_PRESSED, NULL);

    underlineFont = lv_obj_create(MainContainer);
    lv_obj_refr_size(TwentyfourFontButtonLabel);  // Force size recalculation as the label has just been drawn getting the size will retun 0 otheriwse
    lv_obj_set_size(underlineFont, lv_obj_get_width(TwentyfourFontButtonLabel), 2);  // Width, Height
    lv_obj_add_style(underlineFont,&UnderlineStyle,0);
    lv_obj_align_to(underlineFont, TwentyfourFontButtonLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    lv_obj_t *TwentyEightFontButton = lv_btn_create(MainContainer);                          // Create button
    lv_obj_set_pos(TwentyEightFontButton, 248, 150);                                         // Set position
    lv_obj_set_size(TwentyEightFontButton, 62, 50);
    lv_obj_add_style(TwentyEightFontButton, &SideMenuButtons, 0);
    lv_obj_t *TwentyEightFontButtonLabel = lv_label_create(TwentyEightFontButton);
    lv_label_set_text(TwentyEightFontButtonLabel, "28");
    lv_obj_set_style_text_font(TwentyEightFontButtonLabel, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_align_to(TwentyEightFontButtonLabel, TwentyEightFontButton, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_event_cb(TwentyEightFontButton, ChangeFont, LV_EVENT_PRESSED, NULL);


    text_input = lv_textarea_create(MainContainer);
    lv_keyboard_set_textarea(kb, text_input);
    lv_obj_set_pos(text_input, 0, 70); // Set position
    lv_obj_set_size(text_input, 320, 80);
    lv_obj_set_style_border_width(text_input, 0, LV_PART_MAIN);
    lv_obj_set_style_text_align(text_input, LV_TEXT_ALIGN_CENTER, 0);  // 0 = default part
    lv_textarea_set_cursor_click_pos(text_input, true); // Optional: tap to move cursor
    lv_obj_add_state(text_input, LV_STATE_FOCUSED);  // Fake the focus state to allow the cusor to always blink
    lv_obj_set_style_bg_opa(text_input, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_add_style(text_input, &style, 0);

    lv_obj_clear_flag(MainContainer, LV_OBJ_FLAG_SCROLLABLE);  // Prevent scroll-based clipping

    lv_obj_t *arc = lv_arc_create(MainContainer);
    lv_obj_align(arc , LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_arc_opa(arc, LV_OPA_50, LV_PART_INDICATOR);
    lv_obj_set_size(arc, 250, 250);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_arc_set_value(arc, 0);
    lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN); // Make it fully transparent
    lv_obj_move_background(arc);

    lv_anim_init(&BarAnimation);
    lv_anim_set_var(&BarAnimation, arc); 
    lv_anim_set_exec_cb(&BarAnimation, (lv_anim_exec_xcb_t)lv_arc_set_value);
    lv_anim_set_time(&BarAnimation, 10000);                  // Animation time: 3000ms (3 sec)
    lv_anim_set_values(&BarAnimation, 0, 100);               // Start from 0, go to 100
    lv_anim_set_path_cb(&BarAnimation, lv_anim_path_linear); // Linear animation
    lv_anim_set_ready_cb(&BarAnimation, arc_anim_ready_cb);


    lv_obj_t *PrintButton = lv_btn_create(MainContainer);                          // Create button
    lv_obj_set_pos(PrintButton, 195, 400);                                         // Set position
    lv_obj_set_size(PrintButton, 125, 50);                                         // Set size
    lv_obj_set_style_bg_color(PrintButton, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_radius(PrintButton, 0, 0);
    lv_obj_add_event_cb(PrintButton, PrintLabel_event_cb, LV_EVENT_CLICKED, NULL); // Attach event

    // Add a label to the button
    lv_obj_t *label = lv_label_create(PrintButton);
    lv_label_set_text(label, "Print Label");
    lv_obj_center(label); // Center the label
}

void CreateWifiSettings()
{
    lv_obj_t *WifiLabel = lv_label_create(MainContainer);
    lv_label_set_text(WifiLabel, "Wifi Settings");
    lv_obj_center(WifiLabel); // Center the label

    lv_obj_t *ScanWifiButton = lv_btn_create(MainContainer); // Create button
    lv_obj_set_pos(ScanWifiButton, 220, 10);                 // Set position
    lv_obj_set_size(ScanWifiButton, 70, 70);                 // Set size
    lv_obj_add_event_cb(ScanWifiButton, Scan_event_cb, LV_EVENT_PRESSED, NULL); // Attach event

    lv_obj_t *ScanWifiButtonLabel = lv_label_create(ScanWifiButton);
    lv_label_set_text(ScanWifiButtonLabel, "Scan");
    lv_obj_center(ScanWifiButtonLabel); // Center the label
}

static esp_err_t save_string_to_nvs(const char *key, const char *value)
{
    ESP_LOGI("Save String", "URL: %s", value);
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        return err;

    err = nvs_set_str(my_handle, key, value);
    if (err == ESP_OK)
    {
        err = nvs_commit(my_handle);
    }

    nvs_close(my_handle);
    return err;
}

esp_err_t read_string_from_nvs(const char *key, char *out_value, size_t max_size)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);

    if (err != ESP_OK){
        ESP_LOGI("error", "failed to reteive");
        return err;
    }

    size_t required_size;
    err = nvs_get_str(my_handle, key, NULL, &required_size);
    if (err == ESP_OK && required_size <= max_size)
    {
        err = nvs_get_str(my_handle, key, out_value, &required_size);
    }
    nvs_close(my_handle);
    return err;
}

static void MQTT_input_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_move_foreground(keyboard);
        lv_keyboard_set_textarea(keyboard, ta);
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    if (code == LV_EVENT_DEFOCUSED)
    {
        lv_keyboard_set_textarea(keyboard, NULL);
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        ESP_LOGI("", "defocused");
        const char *URLEnteredtext = lv_textarea_get_text(URL_text_input); // Get the current text
        const char *UserNameText = lv_textarea_get_text(UserName_text_input); 
        const char *PassText = lv_textarea_get_text(Password_text_input); 
        esp_err_t errrr = save_string_to_nvs("URL", URLEnteredtext);
        esp_err_t err = save_string_to_nvs("User", UserNameText);
        esp_err_t errb = save_string_to_nvs("Pass", PassText);

        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "String saved successfully!");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to save string: %s", esp_err_to_name(err));
        }
    }
}

void CreateExportSettings()
{
    lv_obj_t *URLLabel = lv_label_create(MainContainer);
    lv_label_set_text(URLLabel, "URL");
    lv_obj_set_pos(URLLabel, 10, 40);

    URL_text_input = lv_textarea_create(MainContainer);
    lv_obj_set_size(URL_text_input, 250, 40);
    lv_obj_set_pos(URL_text_input, 50, 30);
    lv_textarea_set_placeholder_text(URL_text_input, "mqqt://myServer.com");
    lv_obj_add_event_cb(URL_text_input, MQTT_input_event_cb, LV_EVENT_ALL, keyboard);

    lv_obj_t *UserNameLabel = lv_label_create(MainContainer);
    lv_label_set_text(UserNameLabel, "User Name");
    lv_obj_set_pos(UserNameLabel, 10, 90);

    UserName_text_input = lv_textarea_create(MainContainer);
    lv_obj_set_size(UserName_text_input, 200, 40);
    lv_obj_set_pos(UserName_text_input, 100, 80);
    lv_textarea_set_placeholder_text(UserName_text_input, "Username");
    lv_obj_add_event_cb(UserName_text_input, MQTT_input_event_cb, LV_EVENT_ALL, keyboard);

    lv_obj_t *PasswordLabel = lv_label_create(MainContainer);
    lv_label_set_text(PasswordLabel, "Password");
    lv_obj_set_pos(PasswordLabel, 10, 140);

    Password_text_input = lv_textarea_create(MainContainer);
    lv_obj_set_size(Password_text_input, 200, 40);
    lv_obj_set_pos(Password_text_input, 100, 130);
    lv_textarea_set_placeholder_text(Password_text_input, "Password");
    lv_obj_add_event_cb(Password_text_input, MQTT_input_event_cb, LV_EVENT_ALL, keyboard);

    char URL[50];
    if (read_string_from_nvs("URL", URL, sizeof(URL)) == ESP_OK)
    {
        lv_textarea_set_text(URL_text_input, URL);
    }
    char USER[50];
    if (read_string_from_nvs("User", USER, sizeof(USER)) == ESP_OK)
    {
        lv_textarea_set_text(UserName_text_input, USER);
    }
}

void CreateCalibration()
{
    lv_obj_t *Calibrationlabel = lv_label_create(MainContainer);
    lv_label_set_text(Calibrationlabel, "Calibration Settings");
    lv_obj_align(Calibrationlabel, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *MoveBarrellabel = lv_label_create(MainContainer);
    lv_label_set_text(MoveBarrellabel, "Jog Barrel");
    lv_obj_set_pos(MoveBarrellabel, 170, 150);

    lv_obj_t *MovePaperlabel = lv_label_create(MainContainer);
    lv_label_set_text(MovePaperlabel, "Jog paper");
    lv_obj_set_pos(MovePaperlabel, 170, 280);

    lv_obj_t *NudgeButton = lv_btn_create(MainContainer); // Create button
    lv_obj_set_pos(NudgeButton, 20, 70);                  // Set position
    lv_obj_set_size(NudgeButton, 70, 70);                 // Set size
    static bool nudge_up = true;
    lv_obj_add_event_cb(NudgeButton, Nudge, LV_EVENT_CLICKED, &nudge_up); // Attach event

    lv_obj_t *Nudgelabel = lv_label_create(NudgeButton);
    lv_label_set_text(Nudgelabel, LV_SYMBOL_UP);
    lv_obj_center(Nudgelabel); // Center the label

    lv_obj_t *NudgeButtonDown = lv_btn_create(MainContainer);                   // Create button
    lv_obj_set_pos(NudgeButtonDown, 20, 160);                                   // Set position
    lv_obj_set_size(NudgeButtonDown, 70, 70);                                   // Set size
    static bool nudge_down = false;                                             // Define the boolean variable
    lv_obj_add_event_cb(NudgeButtonDown, Nudge, LV_EVENT_CLICKED, &nudge_down); // Attach event

    lv_obj_t *NudgeDownlabel = lv_label_create(NudgeButtonDown);
    lv_label_set_text(NudgeDownlabel, LV_SYMBOL_DOWN);
    lv_obj_center(NudgeDownlabel); // Center the label

    lv_obj_t *NudgePaperButton = lv_btn_create(MainContainer); // Create button
    lv_obj_set_pos(NudgePaperButton, 20, 280);                 // Set position
    lv_obj_set_size(NudgePaperButton, 70, 70);                 // Set size
    lv_obj_add_event_cb(NudgePaperButton, MovePaper, LV_EVENT_PRESSED, NULL);
    lv_obj_t *NudgePaperlabel = lv_label_create(NudgePaperButton);
    lv_label_set_text(NudgePaperlabel, LV_SYMBOL_UP);
    lv_obj_center(NudgePaperlabel); // Center the label
}
