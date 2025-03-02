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

    void CreateScreens(void);
    void switch_screen(lv_obj_t *screen);  // Function prototype
    static void btn_event_cb(lv_event_t * e);
    void HomeScreen(void);
    void SplashScreen(void);
#ifdef __cplusplus
}
#endif  

#endif 