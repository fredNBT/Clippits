#ifndef LVGLPORTLOCK_H
#define LVGLPORTLOCK_H

void lvgl_port_init_locking(void);
void lvgl_port_lock(void);
void lvgl_port_unlock(void);

#endif 