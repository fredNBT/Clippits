#include "lvgl.h"
#include "Styles.h"
#include "ImportData.h"
#include "Screens.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "LVGLPortLock.h"
#include "esp_log.h"

#define SingleRecordSize 110


lv_obj_t *listContainer;
static lv_obj_t *arc;  
static int last_scroll_y = 0;

static void arc_anim_ready_cb(lv_anim_t *a)
{
    lv_obj_add_flag(a->var, LV_OBJ_FLAG_HIDDEN);  // Hide the arc
}

static void Print_btn_event_cb(lv_event_t *e)
{
    lv_coord_t scroll_y = lv_obj_get_scroll_y(listContainer);
    scroll_y += 120;
    lv_obj_scroll_to(listContainer, 0, scroll_y, LV_ANIM_ON);

    lv_obj_clear_flag(arc, LV_OBJ_FLAG_HIDDEN);
    lv_arc_set_value(arc, 0);  // Reset to 0
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_arc_set_value);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 3000);  // 3 seconds
    lv_anim_set_ready_cb(&a, arc_anim_ready_cb);  // 🔥 Here's the magic
    lv_anim_start(&a);
}


static void scroll_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_coord_t y = lv_obj_get_scroll_y(obj);

    int current_scroll_y = lv_obj_get_scroll_y(obj) / 120;
    if (current_scroll_y != last_scroll_y) {
        lvgl_port_lock();
        printf("Scroll changed! Y = %d\n", current_scroll_y);
        lv_obj_t *SingleRecord = lv_obj_get_child(listContainer, current_scroll_y +1);
        lv_obj_t *label = lv_obj_get_child(SingleRecord, 0);

        lv_obj_t *Print_btn = lv_btn_create(SingleRecord);
        lv_obj_add_style(Print_btn, &GreenButtonsStyle,0);
        lv_obj_align(Print_btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_add_event_cb(Print_btn, Print_btn_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_t *PrintLabel = lv_label_create(Print_btn);
        lv_label_set_text_fmt(PrintLabel, "Print Label");
        lv_obj_center(PrintLabel);
        lv_obj_set_height(SingleRecord,150); // increase the size of the current record
        
        if (current_scroll_y != 0)// Set the previous object back to its normal state.
        {
            lv_obj_t *LastSingleRecord = lv_obj_get_child(listContainer, last_scroll_y +1);
            lv_obj_t *Lastlabel = lv_obj_get_child(LastSingleRecord, 0);
            lv_obj_set_style_text_font(Lastlabel, &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_t *child = lv_obj_get_child(LastSingleRecord, 1);  // Get child at index 1
            lv_obj_set_height(LastSingleRecord,SingleRecordSize);
            if (child) {
                lv_obj_del(child);  
            }
        }
        lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);
        lvgl_port_unlock();
        last_scroll_y = current_scroll_y;
    }
}


void CreateImportData()
{
    ESP_LOGI("MEMORY", "Create Import Data");

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
        lv_obj_set_size(SingleRecord, 300, SingleRecordSize);
        lv_obj_t *WifiLabel = lv_label_create(SingleRecord);
        lv_label_set_text_fmt(WifiLabel, "Label %d", i);
        lv_obj_align(WifiLabel, LV_ALIGN_TOP_MID, 0, 0);
    }

    for (int i = 0; i < 2; i++)
    {
    lv_obj_t *PaddingRecord = lv_btn_create(listContainer);
    lv_obj_set_size(PaddingRecord, 300, 110);
    lv_obj_add_style(PaddingRecord, &style, 0);
    }

    arc = lv_arc_create(MainContainer);
    lv_obj_set_size(arc, 300, 300);
    lv_obj_center(arc);

// Style it (optional)
lv_obj_set_style_arc_width(arc, 12, LV_PART_INDICATOR);
lv_obj_set_style_arc_color(arc, lv_palette_main(LV_PALETTE_GREEN), LV_PART_INDICATOR);
lv_arc_set_bg_angles(arc, 0, 360);
lv_arc_set_rotation(arc, 270);
lv_obj_remove_style(arc, NULL, LV_PART_KNOB);  // Hide the knob for a cleaner look
lv_obj_set_style_arc_opa(arc, LV_OPA_50, LV_PART_INDICATOR);
lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);

// Start value at 0, hide the arc
lv_arc_set_value(arc, 0);
lv_obj_add_flag(arc, LV_OBJ_FLAG_HIDDEN);
}