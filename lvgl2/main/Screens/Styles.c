
#include "lvgl.h"
#include "Styles.h"
lv_style_t style;
lv_style_t SideMenuButtons;



static void InitSideMenuButton(){
    lv_style_init(&SideMenuButtons);
    lv_style_set_text_font(&SideMenuButtons, &lv_font_montserrat_14);
    lv_style_set_text_color(&SideMenuButtons, lv_color_hex(0x303030));
    lv_style_set_bg_color(&SideMenuButtons, lv_color_hex(0xFFFFFF));
    lv_style_set_bg_opa(&SideMenuButtons, LV_OPA_TRANSP);
    lv_style_set_border_width(&SideMenuButtons,0);
    lv_style_set_shadow_width(&SideMenuButtons,0);
}

static void MakeUnderline(){
    // lv_obj_set_size(underline, 100, 2);  // Width, Height
    // lv_obj_set_style_border_width(underline,0,LV_PART_MAIN);
    // lv_obj_set_style_bg_color(underline, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);
    // lv_obj_set_size(underline, 320, 2);
    // lv_obj_set_pos(underline,100,100);
    // lv_obj_set_style_bg_color(underline, lv_palette_main(LV_PALETTE_RED), 0);
    // lv_obj_set_height(underline,200);
    // lv_obj_set_style_bg_opa(underline, LV_OPA_COVER, 0);
    // lv_obj_clear_flag(underline, LV_OBJ_FLAG_SCROLLABLE);
    //lv_obj_align_to(underline, label, LV_ALIGN_OUT_BOTTOM_MID, 0, -2);
}

void StylesInit(){
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_24);
    lv_style_set_text_color(&style, lv_color_hex(0x303030));
    lv_style_set_bg_color(&style, lv_color_hex(0xFFFFFF));
    InitSideMenuButton();
    MakeUnderline();
}