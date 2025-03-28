#ifndef SCREENS_H
#define SCREENS_H
#include "lvgl.h"  // Include LVGL header for lv_obj_t
#ifdef __cplusplus
extern "C" {
#endif  
typedef struct {
    lv_obj_t *home;
    lv_obj_t *settings;
    lv_obj_t *splash;
} screens_t;
extern screens_t screens;

typedef struct {
    lv_obj_t *label;
    lv_obj_t* ProgressBar;
} wigits_t;
extern wigits_t wigits;
extern lv_obj_t *MainContainer;

    void CreateScreens(void);
    void switch_screen(lv_obj_t *screen);  // Function prototype
    
    void HomeScreen(void);
    void SplashScreen(void);
    void CreateJustPrint(void);
    void CreateWifiSettings(void);
    void CreateCalibration(void);
    void CreateExportSettings();
    static void buildPWMsgBox(char *SSIDText);
    static void mboxCloseBtn_event_cb(lv_event_t * e);
    void UpdateWifiSymbol(bool status);
    void show_warning_box(void);
#ifdef __cplusplus
}
#endif  

#endif 