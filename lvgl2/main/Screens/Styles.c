
#include "lvgl.h"
#include "Styles.h"
lv_style_t style;
lv_style_t SideMenuButtons;
lv_style_t UnderlineStyle;
lv_style_t TopIconStyle;
lv_style_t GreenButtonsStyle;

static void InitSideMenuButton(){
    lv_style_init(&SideMenuButtons);
    lv_style_set_text_font(&SideMenuButtons, &lv_font_montserrat_14);
    lv_style_set_text_color(&SideMenuButtons, lv_color_hex(0x303030));
    lv_style_set_bg_color(&SideMenuButtons, lv_color_hex(0xFFFFFF));
    lv_style_set_bg_opa(&SideMenuButtons, LV_OPA_TRANSP);
    lv_style_set_border_width(&SideMenuButtons,0);
    lv_style_set_shadow_width(&SideMenuButtons,0);
}
static void InitTopIcons(){
    lv_style_init(&TopIconStyle);
    lv_style_set_text_font(&TopIconStyle, &lv_font_montserrat_14);
    lv_style_set_text_color(&TopIconStyle, lv_color_hex(0x303030));
}

static void InitGreenButtons(){

    lv_style_set_size(&GreenButtonsStyle, 125, 50);
    lv_style_set_radius(&GreenButtonsStyle,0);
    lv_style_set_bg_color(&GreenButtonsStyle, lv_palette_main(LV_PALETTE_GREEN));
}

static void MakeUnderline(){
    lv_style_init(&UnderlineStyle);
    lv_style_set_bg_color(&UnderlineStyle, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_border_width(&UnderlineStyle,0);
}

void StylesInit(){
    lv_style_init(&style);
    lv_style_set_text_font(&style, &lv_font_montserrat_24);
    lv_style_set_text_color(&style, lv_color_hex(0x303030));
    lv_style_set_bg_color(&style, lv_color_hex(0xFFFFFF));

    InitSideMenuButton();
    MakeUnderline();
    InitTopIcons();
    InitGreenButtons();
}