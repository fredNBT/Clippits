 #include "lvgl.h"
 #include "Screens.h" 
 #include "BasicImage.h"
 #include <stdio.h>

screens_t screens;

static void btn_event_cb(lv_event_t * e) {
    printf("Hello\n");
}

void CreateScreens() {
    screens.home = lv_obj_create(NULL);
    lv_obj_t* img = lv_image_create(screens.home);
    lv_img_set_src(img, &clippits);

    wigits.label = lv_label_create(screens.home);
    lv_obj_align(wigits.label, LV_ALIGN_TOP_MID, 0, 100);
    lv_label_set_text(wigits.label, "");

    wigits.ProgressBar = lv_bar_create(screens.home);
    lv_obj_align(wigits.ProgressBar, LV_ALIGN_TOP_MID, 0, 400);

    lv_obj_t *btn = lv_btn_create(screens.home);  // Create button
    lv_obj_set_pos(btn, 100, 100);                // Set position
    lv_obj_set_size(btn, 120, 50);                // Set size
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL); // Attach event

    // Add a label to the button
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Print Label");
    lv_obj_center(label); // Center the label



    screens.splash = lv_obj_create(NULL);
    lv_obj_t *bg_obj = lv_obj_create(screens.splash);
    lv_obj_set_size(bg_obj, LV_HOR_RES, LV_VER_RES);  // Full screen
    lv_obj_set_style_bg_color(bg_obj, lv_color_hex(0xFF00FF), 0);
    lv_obj_set_style_bg_opa(bg_obj, LV_OPA_COVER, 0);

    lv_obj_t *label2 = lv_label_create(screens.splash);
    lv_label_set_text(label2, "Clippits is cool");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
}

// Function to switch screens dynamically
void switch_screen(lv_obj_t *screen) {
    lv_scr_load(screen);
}

