#include "lvgl.h"
#include "Styles.h"
#include "ImportData.h"
#include "Screens.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"


lv_obj_t *listContainer;

static int last_scroll_y = 0;

static SemaphoreHandle_t lvgl_mutex;

void lvgl_port_init_locking(void)
{
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();
}

void lvgl_port_lock(void)
{
    xSemaphoreTakeRecursive(lvgl_mutex, portMAX_DELAY);
}

void lvgl_port_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_mutex);
}

static void scroll_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_coord_t y = lv_obj_get_scroll_y(obj);

    int current_scroll_y = lv_obj_get_scroll_y(obj) / 120;
    if (current_scroll_y != last_scroll_y) {
        printf("Scroll changed! Y = %d\n", current_scroll_y);
       

        lv_obj_t *SingleRecord = lv_obj_get_child(listContainer, current_scroll_y +1);
        lv_obj_t *label = lv_obj_get_child(SingleRecord, 0);
        lvgl_port_lock();
        lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);
        lvgl_port_unlock();


        last_scroll_y = current_scroll_y;
    }





    //printf("Scrolled to: y=%ld\n", y / 120);
}

lv_obj_t *get_next_sibling(lv_obj_t *obj)
{
    lv_obj_t *parent = lv_obj_get_parent(obj);
    if (!parent) return NULL;

    uint32_t count = lv_obj_get_child_cnt(parent);
    bool found = false;

    for (uint32_t i = 0; i < count; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i );
        if (found) return child;     // Return the one after `obj`
        if (child == obj) found = true;
    }
    return NULL;  // No next sibling found
}

void PrintButtonEvent(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *SingleRecord = lv_obj_get_parent(btn); // assumes label is only child
    lv_obj_t *label = lv_obj_get_child(SingleRecord, 0);
    lv_coord_t scroll_y = lv_obj_get_scroll_y(listContainer);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_10, LV_PART_MAIN);
    lv_obj_set_size(SingleRecord, 300, 80);

    lv_obj_t *next = get_next_sibling(SingleRecord);
    lv_obj_t *NextLabel = lv_obj_get_child(next, 0);
    lv_obj_set_style_text_font(NextLabel, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_size(next, 300, 120);


    lv_obj_t *PrintButton = lv_btn_create(next);
    lv_obj_add_style(PrintButton, &GreenButtonsStyle, 0);
    lv_obj_t *PrintLabel = lv_label_create(PrintButton);
    lv_label_set_text_fmt(PrintLabel, "Print Label");
    lv_obj_center(PrintLabel);
    lv_obj_align(PrintButton, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_add_event_cb(PrintButton, PrintButtonEvent, LV_EVENT_PRESSED, NULL);
    
    lv_obj_del(btn); 
    lv_obj_scroll_to(listContainer, 0, scroll_y + 80, LV_ANIM_ON);

}

void LabelClickEvent(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);  // assumes label is only child
    lv_obj_set_size(btn, 300, 120);
    // Apply the new font style


    // lv_obj_t *next = get_next_sibling(btn);
    // lv_obj_t *NextLabel = lv_obj_get_child(next, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);

    lv_obj_t *PrintButton = lv_btn_create(btn);
    lv_obj_add_style(PrintButton, &GreenButtonsStyle, 0);
    lv_obj_t *PrintLabel = lv_label_create(PrintButton);
    lv_label_set_text_fmt(PrintLabel, "Print Label");
    lv_obj_center(PrintLabel);
    lv_obj_align(PrintButton, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
   // lv_obj_add_event_cb(PrintButton, PrintButtonEvent, LV_EVENT_PRESSED, NULL);
}

void CreateImportData()
{
    lvgl_port_init_locking();
    lv_obj_t *Label = lv_label_create(MainContainer);
    lv_label_set_text(Label, "Import Data");
    //lv_obj_scroll_to(listContainer, 0,  110, LV_ANIM_ON); 

    listContainer = lv_obj_create(MainContainer);
    lv_obj_set_size(listContainer, 320, 400);                 // Set container size
    lv_obj_align(listContainer, LV_ALIGN_TOP_MID, 0, 70);     // Align at the top center
    lv_obj_set_flex_flow(listContainer, LV_FLEX_FLOW_COLUMN); // Set flexbox direction (vertical)
    lv_obj_set_style_pad_row(listContainer, 10, 0);   
    lv_obj_add_event_cb(listContainer, scroll_event_cb, LV_EVENT_SCROLL, NULL);


   
    lv_obj_t *PaddingRecord = lv_btn_create(listContainer);
    lv_obj_set_size(PaddingRecord, 300, 150);
    lv_obj_add_style(PaddingRecord, &style, 0);
    

    // Add whatever you want
    for (int i = 0; i < 25; i++)
    {
        lv_obj_t *SingleRecord = lv_obj_create(listContainer);
        lv_obj_set_size(SingleRecord, 300, 110);
        lv_obj_t *WifiLabel = lv_label_create(SingleRecord);
        //lv_obj_add_style(SingleRecord, &style, 0);
        // lv_obj_set_style_bg_color(SingleRecord, lv_color_hex(0xFFFFFF), 0);
        lv_label_set_text_fmt(WifiLabel, "Label %d", i);
        lv_obj_align(WifiLabel, LV_ALIGN_TOP_MID, 0, 0);
       // lv_obj_add_event_cb(SingleRecord, LabelClickEvent, LV_EVENT_CLICKED, NULL);
    }

    for (int i = 0; i < 2; i++)
    {
    lv_obj_t *PaddingRecord = lv_btn_create(listContainer);
    lv_obj_set_size(PaddingRecord, 300, 110);
    lv_obj_add_style(PaddingRecord, &style, 0);
    }
}