/**
 ******************************************************************************
 * @file      wifi-connect.c
 * @author    Dean Prince Agbodjan
 * @brief     WiFi mode and connection as a Station Implementation
 *
 ******************************************************************************
 */
/* Header Files */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/event_groups.h"

#include "output_driver.h"

#define TAG "WIFI"

esp_netif_t *esp_netif;
static EventGroupHandle_t wifi_events;
const uint32_t gotIP = BIT0;

/**
 * @brief This event handles various WiFi and IP events.
 * @param [IN] event_handler_arg: pointer to user defined argument.
 * @param [IN] event_base: Specifies the base  'WIFI_EVENT' and 'IP_EVENT'
 * @param [IN] event_id: Specifies the specific event.
 * @param [IN] event_data: Additional data
 */
void event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "connecting...");
        esp_wifi_connect();
        break;

    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "CONNECTED");
        wifi_status(1);
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "DISCONNECTED");
        wifi_status(0);
        esp_wifi_connect();
        break;

    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "IP OBTAINED");
 
        xEventGroupSetBits(wifi_events, gotIP);
        break;

    default:
        break;
    }
}
/** 
 * @brief WiFi initialization process, setting the WiFi mode and starting the WiFi driver. 
*/
void wifi_drivers(void)
{
    /* Initialize the network interface */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Create default event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Initialize WiFI with default configuration */
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    /* Handles WiFi event handler */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));
    
    /* Set the WiFi API configuration storage type */
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
}

/** 
 * @brief Setting and connecting up WiFi mode as a station and connecting using ssid and passwd.
 * @retval ESP_OK or ESP_FAIL 
*/
esp_err_t wifi_sta_connect(const char *ssid, const char *password)
{
    /* Create an event group to manage WiFi events*/
    wifi_events = xEventGroupCreate();
    if (wifi_events == NULL){
        return ESP_FAIL;
    }

    /* Create default WiFi station */
    esp_netif = esp_netif_create_default_wifi_sta();
    if (esp_netif == NULL){
        vEventGroupDelete(wifi_events);
        return ESP_FAIL;
    }

    /* Initialize WiFi configuration structure */
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    
    /* Set WiFi mode to a station */
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

    /* Start WiFi */
    esp_wifi_start();

    /* Wait for connection event */
    EventBits_t event_results = xEventGroupWaitBits(wifi_events, gotIP, pdFALSE, pdTRUE, 2000 / portTICK_PERIOD_MS);
    if (event_results == 0)
    {
        return ESP_OK;
    } else {
        return ESP_FAIL;
    }
}