#include <stdio.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include <string.h>

#define MAX_APs 20 // Max APs to scan
static const char *TAG = "WiFi";
static EventGroupHandle_t wifi_events;
static int CONNECTED = BIT0;
static int DISCONNECTED = BIT1;

void wifi_scan(wifi_ap_record_t *ap_info, uint16_t *ap_count)
{
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,        // Scan all channels
        .show_hidden = false // Show hidden networks
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true)); // Blocking scan

    esp_wifi_scan_get_ap_num(ap_count);
    ESP_LOGI(TAG, "Found %d access points", *ap_count);
    uint16_t ap_num = (*ap_count > MAX_APs) ? MAX_APs : *ap_count;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_info));
}

void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
        xEventGroupSetBits(wifi_events, DISCONNECTED);
        break;
    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        xEventGroupSetBits(wifi_events, CONNECTED);
        break;
    default:
        break;
    }
}
// void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
//         ESP_LOGI(TAG, "WiFi started, connecting...");
//         esp_wifi_connect();  // Attempt to connect
//     } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
//         ESP_LOGI(TAG, "\nConnected\n");
//     } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
//         ESP_LOGW(TAG, "Disconnected! Reconnecting...");
//         esp_wifi_connect();  // Reconnect on disconnect
//     } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
//         ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
//         ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
//     }
// }

void InitWifi()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // Wifi_Connect_STA();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t  Wifi_Connect_STA(const char *ssid, const char *password)
{
    wifi_events = xEventGroupCreate();

    esp_wifi_disconnect();

    wifi_config_t wifi_config = {};
    //  printf("started wifi\n");
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();
    EventBits_t result = xEventGroupWaitBits(wifi_events, CONNECTED | DISCONNECTED, true, false, pdMS_TO_TICKS(5000));
    if (result == CONNECTED){
        return ESP_OK;
    }
    return ESP_FAIL;
    //if (result == DISCONNECTED){ESP_LOGI(TAG, "Failed");} 

    // //ESP_ERROR_CHECK(esp_wifi_start());

    //ESP_LOGI(TAG, "WiFi Connected!");
}
