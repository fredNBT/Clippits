#include "lvgl.h"
#include "Styles.h"
#include "RFID.h"
#include "Screens.h"
#include "esp_log.h"

void CreateRFIDScreen(){
    ESP_LOGI("MEMORY", "RFID");
    lv_obj_t *Label = lv_label_create(MainContainer);
    lv_label_set_text(Label, "RFID");
    lv_obj_add_style(Label, &style,0);
    lv_obj_align(Label, LV_ALIGN_TOP_MID, 0, 10);  // Align to top center with Y offset

    lv_obj_t *UserLabel = lv_label_create(MainContainer);
    lv_label_set_text(UserLabel, "User: Fred");
    //lv_obj_add_style(UserLabel, &style,0);
    lv_obj_align(UserLabel, LV_ALIGN_TOP_MID, -50, 50);  // Align to top center with Y offset

    lv_obj_t *TimeLabel = lv_label_create(MainContainer);
    lv_label_set_text(TimeLabel, "Time:     08:02");
    lv_obj_align(TimeLabel, LV_ALIGN_TOP_MID, -50, 80);  // Align to top center with Y offset

    lv_obj_t *DateLabel = lv_label_create(MainContainer);
    lv_label_set_text(DateLabel, "Date:     24 April 2025");
    lv_obj_align(DateLabel, LV_ALIGN_TOP_MID, -50, 100);  // Align to top center with Y offset
}
