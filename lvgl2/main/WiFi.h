#ifndef WIFI_H
#define WIFI_H
#include "esp_wifi.h"
#include "esp_log.h"
void InitWifi(void);
void wifi_scan(wifi_ap_record_t *ap_info, uint16_t *ap_count);
;

#endif