#include <stdio.h>
#include "WiFi.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include <string.h>
#include "Screens/Screens.h" // this need to be changed to a call back so we can decouple the wifi from screen
#include "freertos/semphr.h"
#include <time.h>
#include "esp_sntp.h"
#include "mqtt_client.h"

const char *ntpServer = "pool.ntp.org";
SemaphoreHandle_t got_time_semaphore;

#define MAX_APs 20 // Max APs to scan
static const char *TAG = "WiFi";
static EventGroupHandle_t wifi_events;
static int CONNECTED = BIT0;
static int DISCONNECTED = BIT1;
static esp_mqtt_client_handle_t client;
static void (*ui_update_callback)(const char *) = NULL; // Function pointer for UI callback

void register_time_callback(void (*callback)(const char *))
{
    ui_update_callback = callback; // Store the callback function
}

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
        UpdateWifiSymbol(false);
        // xEventGroupSetBits(wifi_events, DISCONNECTED);
        break;
    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        UpdateWifiSymbol(true); // calls the UpdateWifisymbol in the screens file - this needs to be changed to a call back
        xEventGroupSetBits(wifi_events, CONNECTED);
        GetTime();
        MQTTInit();
        break;
    default:
        break;
    }
}

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
    wifi_events = xEventGroupCreate(); // we need to init the xevent group before the first event
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t Wifi_Connect_STA(const char *ssid, const char *password)
{
    esp_wifi_disconnect(); // disconnects to any previous wifi
    wifi_config_t wifi_config = {};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    xEventGroupClearBits(wifi_events, CONNECTED);
    esp_wifi_connect();
    EventBits_t result = xEventGroupWaitBits(wifi_events, CONNECTED, true, false, pdMS_TO_TICKS(5000)); // if not connected in 5 seconds it will fail
    if (result == CONNECTED)
    {
        return ESP_OK;
    }
    else
    {
        printf("returning ESP_FAIL");
        return ESP_FAIL;
    }
}
// UTC+5

void on_got_time(struct timeval *tv)
{
    xSemaphoreGive(got_time_semaphore);
}

void print_time()
{
    time_t now = 0;
    time(&now);
    struct tm *time_info = localtime(&now);

    char time_buffer[50];
    strftime(time_buffer, sizeof(time_buffer), "%H:%M", time_info);
    ESP_LOGI(TAG, "************ %s ***********", time_buffer);

    if (ui_update_callback)
    {
        ui_update_callback(time_buffer); // Call the UI callback
    }
}

void GetTime()
{
    got_time_semaphore = xSemaphoreCreateBinary();

    setenv("TZ", "UTC+5", 1);
    tzset();
    time_t now = 0;
    time(&now);
    struct tm *time_info = localtime(&now);

    esp_sntp_init();
    esp_sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_set_time_sync_notification_cb(on_got_time);

    xSemaphoreTake(got_time_semaphore, portMAX_DELAY);
    print_time();
}



void mqtt_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client,"clippits/connect1",1);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        break;
    case MQTT_EVENT_UNSUBSCRIBED: /*!< unsubscribed event, additional context:  msg_id */
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
        break;
    case MQTT_EVENT_PUBLISHED: /*!< published event, additional context:  msg_id */
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        break;
    default:
    break;
    }
}
void MQTTInit()
{
    ESP_LOGI(TAG, "MQTT INIT");
    esp_mqtt_client_config_t esp_client_config = {
        .broker.address.uri = "mqtt://test.mosquitto.org:1883"};
    client = esp_mqtt_client_init(&esp_client_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

int mqtt_send(char* topic, char* payload)
{
   return esp_mqtt_client_publish(client,topic,payload,strlen(payload),1,0);
}