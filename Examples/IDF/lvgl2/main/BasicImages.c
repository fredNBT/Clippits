#include "lvgl.h"
#include "BasicImage.h"

void draw_image(void)
{
    lv_obj_t* screen = lv_screen_active();
    lv_obj_t* img = lv_image_create(screen);
    lv_img_set_src(img, &clippits);
}