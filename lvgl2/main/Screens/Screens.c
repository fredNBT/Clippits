 #include "lvgl.h"
 #include "Screens.h" 
 #include "BasicImage.h"
 #include "Thermal.h"
 #include <stdio.h>
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 #include "StepperMotor.h"
screens_t screens;
static lv_obj_t * menu_panel;  // Sidebar menu
static bool menu_open = false; // Menu state
lv_obj_t* MainContainer;
lv_obj_t* text_input;
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
    xTaskCreate(MakeLabel, "MakeLabel", 4096, NULL, 5, NULL);
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

void CreateScreens() {
    HomeScreen();
    SplashScreen();
}

void HomeScreen(){
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_14); 
    lv_style_set_text_color(&style, lv_color_hex(0x0000FF)); // Green color

   

    screens.home = lv_obj_create(NULL);
    MainContainer = lv_obj_create(screens.home); 
    lv_obj_t* TopContainer = lv_obj_create(screens.home); 
    lv_obj_set_style_pad_all(TopContainer, 0, 0); 
    lv_obj_clear_flag(TopContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(TopContainer, 320, 60);              
    lv_obj_set_pos(TopContainer, 0, 0); 
    lv_obj_set_style_bg_color(TopContainer, lv_color_hex(0xFFFFFF), 0);  // Set white background


    lv_obj_t *wifi_label = lv_label_create(screens.home);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_align(wifi_label, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_add_style(wifi_label, &style, 0);

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
    // 📌 Create hamburger button
    lv_obj_t * menu_btn = lv_btn_create(screens.home);
    lv_obj_set_size(menu_btn, 60, 60);
    lv_obj_align(menu_btn, LV_ALIGN_TOP_LEFT, 0, 0); // Top left corner
    lv_obj_add_event_cb(menu_btn, toggle_menu, LV_EVENT_CLICKED, NULL);

    // 📌 Add ☰ icon (hamburger)
    lv_obj_t * menu_icon = lv_label_create(menu_btn);
    lv_label_set_text(menu_icon, LV_SYMBOL_LIST); // ☰ icon
    lv_obj_center(menu_icon);
}
void SplashScreen(){
    screens.splash = lv_obj_create(NULL);
    lv_obj_t *bg_obj = lv_obj_create(screens.splash);


    lv_obj_t* img = lv_image_create(screens.splash);
    lv_img_set_src(img, &clippits);
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
}