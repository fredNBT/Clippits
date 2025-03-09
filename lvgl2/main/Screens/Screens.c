 #include "lvgl.h"
 #include "Screens.h" 
// #include "BasicImage.h"
 #include "Thermal.h"
 #include <stdio.h>
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "StepperMotor.h"
 #include <stdbool.h>
 #include "WiFi.h"
 #include "esp_wifi.h"
#include "esp_log.h"
static const char *TAG = "Main";
 //#define LV_FONT_MONTSERRAT_32 1
screens_t screens;
static lv_obj_t * menu_panel;  // Sidebar menu
static bool menu_open = false; // Menu state
lv_obj_t* MainContainer;
lv_obj_t* text_input;
static lv_style_t style;
static void toggle_menu(lv_event_t * e) {
    if (menu_open) {
        lv_obj_add_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); // Hide menu
    } else {
        lv_obj_clear_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); // Show menu
    }
    menu_open = !menu_open;
}

static const char * custom_kb_map[] = {
    "1","2" ,"3", "4","5","6" ,"7", "8", "9","0",  "\n",
    "q","w" ,"e", "r","t","y" ,"u", "i", "o","p",  "\n",
    "a","s" ,"d", "f","g","h" ,"j", "k", "l",  "\n",
    LV_SYMBOL_UP,"z" ,"x", "c","v","b" ,"n", "m", LV_SYMBOL_BACKSPACE,  "\n",
    ","," " ,".", "print", NULL
};

static const lv_btnmatrix_ctrl_t custom_kb_ctrl_map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // First row
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1,  
    1, 5, 1, 3
};
void update_bar_task(void *pvParameter) {
    lv_obj_t *bar = (lv_obj_t *)pvParameter;
    for (int i = 0; i <= 10; i += 1) {
        lv_bar_set_value(bar, i*10, LV_ANIM_OFF);  // Update bar
        lv_task_handler();  // Refresh UI
        vTaskDelay(pdMS_TO_TICKS(500));  // Delay 500ms
    }
    lv_bar_set_value(bar, 0, LV_ANIM_OFF); 
    vTaskDelete(NULL);  // Delete task when done
}
static void PrintLabel_event_cb(lv_event_t * e) {
    printf("Printing Label\n");
    print_text(lv_textarea_get_text(text_input));
    lv_textarea_set_text(text_input, "");  // Set an empty string
    //MakeLabel();
    xTaskCreate(update_bar_task, "UpdateBar", 4096, (void *)wigits.ProgressBar, 1, NULL);
    xTaskCreate(MakeLabel, "MakeLabel", 2048, NULL, 5, NULL);
}

static void ConnectWifi_event_cb(lv_event_t * e) {
    printf("connecting to wifi\n");
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    lv_obj_t * label = lv_obj_get_child(obj, 0); // Get first child (label)

    if (label != NULL) {
        const char * text = lv_label_get_text(label);  // Get label text
        printf("Button clicked: %s\n", text);
        buildPWMsgBox();
    }
}

static void buildPWMsgBox() {

    lv_obj_t* mboxConnect = lv_obj_create(MainContainer);
    //lv_obj_add_style(mboxConnect, &border_style, 0);
    lv_obj_set_size(mboxConnect, 300,150);
    lv_obj_center(mboxConnect);
  
    lv_obj_t* mboxTitle = lv_label_create(mboxConnect);
    lv_label_set_text(mboxTitle, "Selected WiFi SSID: ThatProject");
    lv_obj_align(mboxTitle, LV_ALIGN_TOP_LEFT, 0, 0);
  
    lv_obj_t* mboxPassword = lv_textarea_create(mboxConnect);
    lv_obj_set_size(mboxPassword, 280, 40);
    lv_obj_align_to(mboxPassword, mboxTitle, LV_ALIGN_TOP_LEFT, 0, 30);
    lv_textarea_set_placeholder_text(mboxPassword, "Password?");
    //lv_obj_add_event_cb(mboxPassword, text_input_event_cb, LV_EVENT_ALL, keyboard);
  
    lv_obj_t* mboxConnectBtn = lv_btn_create(mboxConnect);
    //lv_obj_add_event_cb(mboxConnectBtn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(mboxConnectBtn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *btnLabel = lv_label_create(mboxConnectBtn);
    lv_label_set_text(btnLabel, "Connect");
    lv_obj_center(btnLabel);
  
    lv_obj_t* mboxCloseBtn = lv_btn_create(mboxConnect);
    //lv_obj_add_event_cb(mboxCloseBtn, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_align(mboxCloseBtn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_t *btnLabel2 = lv_label_create(mboxCloseBtn);
    lv_label_set_text(btnLabel2, "Cancel");
    lv_obj_center(btnLabel2);
  }

static void Scan_event_cb(lv_event_t * e) {
   
    wifi_ap_record_t ap_info[20];
    uint16_t ap_count = 0;
    wifi_scan(ap_info, &ap_count);
    
    ESP_LOGI(TAG, "Found %d access points", ap_count);
   lv_obj_t *listContainer = lv_obj_create(MainContainer);
   lv_obj_set_size(listContainer, 320, 400);  // Set container size
   lv_obj_align(listContainer, LV_ALIGN_TOP_MID, 0, 70);  // Align at the top center
   lv_obj_set_flex_flow(listContainer, LV_FLEX_FLOW_COLUMN);  // Set flexbox direction (vertical)
   lv_obj_set_style_pad_row(listContainer, 10, 0);  // Add spacing between items

    for (int i = 0; i < ap_count; i++) {
        lv_obj_t *SingleRecord = lv_btn_create(listContainer);
        lv_obj_set_size(SingleRecord, 270,70);
        lv_obj_t *WifiLabel = lv_label_create(SingleRecord);
        lv_obj_add_style(SingleRecord, &style, 0);
       // lv_obj_set_style_bg_color(SingleRecord, lv_color_hex(0xFFFFFF), 0); 
        lv_label_set_text_fmt(WifiLabel, "%s", ap_info[i].ssid); 
        lv_obj_align(WifiLabel, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_t *WifiLabelStrength = lv_label_create(SingleRecord);
        //lv_label_set_text_fmt(WifiLabelStrength, "%d", ap_info[i].rssi);  LV_SYMBOL_WIFI
        lv_label_set_text(WifiLabelStrength, LV_SYMBOL_WIFI);
        lv_obj_align(WifiLabelStrength, LV_ALIGN_RIGHT_MID, 0, 0);

        lv_obj_add_event_cb(SingleRecord, ConnectWifi_event_cb, LV_EVENT_CLICKED, NULL); 

        
       // ESP_LOGI(TAG, "SSID: %s, RSSI: %d, Channel: %d", ap_info[i].ssid, ap_info[i].rssi, ap_info[i].primary);
    }   
}

static void Nudge(lv_event_t * e) {
    bool *nudge_value = (bool *)lv_event_get_user_data(e); // Retrieve the passed bool
    //this is stupid but But wont workother wise
    if (nudge_value && *nudge_value) {
        printf("Nudge Down\n");
        NudgeBarrel(1);
    } else {
        printf("Nudge Up\n");
        NudgeBarrel(0);
    }
   }

static void ChangeToSettings(lv_event_t * e) {
 lv_obj_clean(MainContainer);
 CreateWifiSettings();
 lv_obj_add_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); 
}
static void ChangeToJustPrint(lv_event_t * e) {
    lv_obj_clean(MainContainer);
    CreateJustPrint();
    lv_obj_add_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); 
   }

   static void ChangeToCalibration(lv_event_t * e) {
    lv_obj_clean(MainContainer);
    CreateCalibration();
    lv_obj_add_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); 
   }

void CreateScreens() {
    HomeScreen();
    SplashScreen();
}

void HomeScreen(){
    
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_24); 
    lv_style_set_text_color(&style, lv_color_hex(0x303030)); 
    lv_style_set_bg_color(&style, lv_color_hex(0xFFFFFF));

    screens.home = lv_obj_create(NULL);
    MainContainer = lv_obj_create(screens.home); 
    lv_obj_t* TopContainer = lv_obj_create(screens.home); 
    lv_obj_set_style_pad_all(TopContainer, 0, 0); 
    lv_obj_clear_flag(TopContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(TopContainer, 320, 80);              
    lv_obj_set_pos(TopContainer, 0, 0); 
    lv_obj_set_style_bg_color(TopContainer, lv_color_hex(0xFFFFFF), 0);  // Set white background

    lv_obj_t *Battery_label = lv_label_create(screens.home);
    lv_label_set_text(Battery_label, LV_SYMBOL_BATTERY_FULL);
    lv_obj_align(Battery_label, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_style(Battery_label, &style, 0);
    
    lv_obj_t *wifi_label = lv_label_create(screens.home);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_align(wifi_label, LV_ALIGN_TOP_RIGHT, -60, 10);
    lv_obj_add_style(wifi_label, &style, 0);

    lv_obj_t *GSM_label = lv_label_create(screens.home);
    lv_label_set_text(GSM_label, "2G");
    lv_obj_align(GSM_label, LV_ALIGN_TOP_RIGHT, -110, 10);
    lv_obj_add_style(GSM_label, &style, 0);

    lv_obj_set_style_pad_all(MainContainer, 0, 0); 
    lv_obj_clear_flag(MainContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(MainContainer, 320, 420);              
    lv_obj_set_pos(MainContainer, 0, 60); 
    lv_obj_set_style_bg_color(MainContainer, lv_color_hex(0xFFFFFF), 0);  // Set white background
    CreateJustPrint();

    // 📌 Create the menu panel (sidebar)
    menu_panel = lv_obj_create(screens.home);
    lv_obj_set_size(menu_panel, 150, 200); // Width x Height
    lv_obj_align(menu_panel, LV_ALIGN_LEFT_MID, 0, 0); // Position at left
    lv_obj_set_style_bg_color(menu_panel, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN);
    lv_obj_add_flag(menu_panel, LV_OBJ_FLAG_HIDDEN); // Start hidden

    // 📌 Add menu buttons
    lv_obj_t * btn1 = lv_btn_create(menu_panel);
    lv_obj_set_size(btn1, 150, 50);
    lv_obj_align(btn1, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t * label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "JustPrint");
    lv_obj_add_event_cb(btn1, ChangeToJustPrint, LV_EVENT_CLICKED, NULL); 

    lv_obj_t * btn2 = lv_btn_create(menu_panel);
    lv_obj_align_to(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_size(btn2, 150, 50);
    lv_obj_set_pos(btn2, 0, 50); 
    lv_obj_t * label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Wifi Settings");
    lv_obj_add_event_cb(btn2, ChangeToSettings, LV_EVENT_CLICKED, NULL); 

    lv_obj_t * Menubtn3 = lv_btn_create(menu_panel);
    lv_obj_set_size(Menubtn3, 150, 50);
    lv_obj_align(Menubtn3, LV_ALIGN_TOP_LEFT, 0, 100);
    lv_obj_t * Menulabel3 = lv_label_create(Menubtn3);
    lv_label_set_text(Menulabel3, "Calibration");
    lv_obj_add_event_cb(Menubtn3, ChangeToCalibration, LV_EVENT_CLICKED, NULL); 
    // 📌 Create hamburger button
    lv_obj_t * menu_btn = lv_btn_create(screens.home);
    lv_obj_set_size(menu_btn, 60, 60);
    lv_obj_align(menu_btn, LV_ALIGN_TOP_LEFT, 0, 0); // Top left corner
    lv_obj_add_event_cb(menu_btn, toggle_menu, LV_EVENT_CLICKED, NULL);

    // 📌 Add ☰ icon (hamburger)
    lv_obj_t * menu_icon = lv_label_create(menu_btn);
    lv_label_set_text(menu_icon, LV_SYMBOL_LIST); // ☰ icon
    lv_obj_center(menu_icon);
    lv_obj_add_style(menu_btn, &style, 0);
    //lv_obj_set_style_bg_color(menu_btn, lv_color_hex(0xFFFFFF), 0);  // Set white background
}
void SplashScreen(){
    screens.splash = lv_obj_create(NULL);
    // lv_obj_t* img = lv_image_create(screens.splash);
    // lv_img_set_src(img, &clippits);

}
// Function to switch screens dynamically
void switch_screen(lv_obj_t *screen) {
    lv_scr_load(screen);
}

void CreateJustPrint(){

    lv_obj_t* kb = lv_keyboard_create(MainContainer);
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_LOWER, custom_kb_map, custom_kb_ctrl_map);

    text_input = lv_textarea_create(MainContainer);
    lv_keyboard_set_textarea(kb, text_input);
    lv_obj_set_pos(text_input, 0, 20);                // Set position
    lv_obj_set_size(text_input, 320, 80); 

    wigits.ProgressBar = lv_bar_create(MainContainer);
    lv_obj_align(wigits.ProgressBar, LV_ALIGN_TOP_MID, 0, 120);

    lv_obj_t *PrintButton = lv_btn_create(MainContainer);  // Create button
    lv_obj_set_pos(PrintButton, 200, 370);                // Set position
    lv_obj_set_size(PrintButton, 120, 50);                // Set size
    lv_obj_add_event_cb(PrintButton, PrintLabel_event_cb, LV_EVENT_CLICKED, NULL); // Attach event

    // Add a label to the button
    lv_obj_t *label = lv_label_create(PrintButton);
    lv_label_set_text(label, "Print Label");
    lv_obj_center(label); // Center the label
}

void CreateWifiSettings(){
    lv_obj_t *WifiLabel = lv_label_create(MainContainer);
    lv_label_set_text(WifiLabel, "Wifi Settings");
    lv_obj_center(WifiLabel); // Center the label

    lv_obj_t *ScanWifiButton = lv_btn_create(MainContainer);  // Create button
    lv_obj_set_pos(ScanWifiButton, 220, 10);                // Set position
    lv_obj_set_size(ScanWifiButton, 70, 70);                // Set size
   
    lv_obj_add_event_cb(ScanWifiButton, Scan_event_cb, LV_EVENT_CLICKED, NULL); // Attach event

    
    lv_obj_t *ScanWifiButtonLabel = lv_label_create(ScanWifiButton);
    lv_label_set_text(ScanWifiButtonLabel, "Scan");
    lv_obj_center(ScanWifiButtonLabel); // Center the label
}

void CreateCalibration(){
    lv_obj_t *Calibrationlabel = lv_label_create(MainContainer);
    lv_label_set_text(Calibrationlabel, "Calibration Settings");
    lv_obj_align(Calibrationlabel, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *MoveBarrellabel = lv_label_create(MainContainer);
    lv_label_set_text(MoveBarrellabel, "Jog Barrel");
    lv_obj_set_pos(MoveBarrellabel,170,150);

    lv_obj_t *MovePaperlabel = lv_label_create(MainContainer);
    lv_label_set_text(MovePaperlabel, "Jog paper");
    lv_obj_set_pos(MovePaperlabel,170,280);
    
    lv_obj_t *NudgeButton = lv_btn_create(MainContainer);  // Create button
    lv_obj_set_pos(NudgeButton, 20, 70);                // Set position
    lv_obj_set_size(NudgeButton, 70, 70);                // Set size
    static bool nudge_up = true;
    lv_obj_add_event_cb(NudgeButton, Nudge, LV_EVENT_CLICKED, &nudge_up); // Attach event
    
    lv_obj_t *Nudgelabel = lv_label_create(NudgeButton);
    lv_label_set_text(Nudgelabel, LV_SYMBOL_UP);
    lv_obj_center(Nudgelabel); // Center the label

    lv_obj_t *NudgeButtonDown = lv_btn_create(MainContainer);  // Create button
    lv_obj_set_pos(NudgeButtonDown, 20, 160);                // Set position
    lv_obj_set_size(NudgeButtonDown, 70, 70);                // Set size
    static bool nudge_down = false; // Define the boolean variable
    lv_obj_add_event_cb(NudgeButtonDown, Nudge, LV_EVENT_CLICKED, &nudge_down); // Attach event
    
    lv_obj_t *NudgeDownlabel = lv_label_create(NudgeButtonDown);
    lv_label_set_text(NudgeDownlabel, LV_SYMBOL_DOWN);
    lv_obj_center(NudgeDownlabel); // Center the label

    lv_obj_t *NudgePaperButton = lv_btn_create(MainContainer);  // Create button
    lv_obj_set_pos(NudgePaperButton, 20, 280);                // Set position
    lv_obj_set_size(NudgePaperButton, 70, 70);                // Set size
    
    lv_obj_t *NudgePaperlabel = lv_label_create(NudgePaperButton);
    lv_label_set_text(NudgePaperlabel, LV_SYMBOL_UP);
    lv_obj_center(NudgePaperlabel); // Center the label
}
