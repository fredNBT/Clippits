#include <stdio.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define MAX_APs 20  // Max APs to scan
static const char *TAG = "WiFi_Scan";

void wifi_scan(wifi_ap_record_t *ap_info, uint16_t *ap_count) {
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,          // Scan all channels
        .show_hidden = false    // Show hidden networks
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true)); // Blocking scan

    //uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(ap_count);
    ESP_LOGI(TAG, "Found %d access points", *ap_count);


    //wifi_ap_record_t ap_info[MAX_APs];
    uint16_t ap_num = (*ap_count > MAX_APs) ? MAX_APs : *ap_count;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_info));
    //ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(ap_count, ap_info));
    // for (int i = 0; i < ap_num; i++) {
    //     ESP_LOGI(TAG, "SSID: %s, RSSI: %d, Channel: %d",
    //              ap_info[i].ssid, ap_info[i].rssi, ap_info[i].primary);
    // }
}

void InitWifi() {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));  // Station mode
    ESP_ERROR_CHECK(esp_wifi_start());

    //wifi_scan();  // Perform Wi-Fi scan
}
