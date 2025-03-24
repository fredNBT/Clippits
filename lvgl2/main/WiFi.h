#ifndef WIFI_H
#define WIFI_H
#include "esp_wifi.h"
#include "esp_log.h"

void InitWifi(void);
void wifi_scan(wifi_ap_record_t *ap_info, uint16_t *ap_count);
esp_err_t  Wifi_Connect_STA(const char *ssid, const char *password);
static void GetTime();
void MQTTInit();
void print_time();
void register_time_callback(void (*callback)(const char *));
int mqtt_send(char* topic, char* payload);
esp_err_t ReadStringsFromNVS(const char *key, char *out_value, size_t max_size);

#endif