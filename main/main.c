/**
 ******************************************************************************
 * @file      device_shadow.c
 * @author    Dean Prince Agbodjan
 * @brief     Main Application implementation
 *
 ******************************************************************************
 */
/* Header Files */
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi-connect.h"
#include "device_shadow.h"
#include "output_driver.h"
#include "sub_pub_ota.h"


#define TAG         "main app"
#define SSID_1      "drover_ap"
#define PASS        "dje83ke3"

/**
 * @brief Setting up and connecting WiFi STA
 */
void wifi_sta_setup(void) {
  wifi_drivers();

  esp_err_t wifi_sta_callback = wifi_sta_connect(SSID_1, PASS);

  if (wifi_sta_callback == 0) {
    ESP_LOGI(TAG, "CONNECTED");
  }
}

void app_main(void) {

  /* Initializing GPIOs connecting the relay */
  gpio_init();

  /* Initializing lcd 20x04 screen */
  lcd2004();

  /* Initialize the flash */
  esp_err_t nvs_results = nvs_flash_init();
  if (nvs_results == ESP_ERR_NVS_NO_FREE_PAGES ||
      ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_results = nvs_flash_erase();
    nvs_results |= nvs_flash_init();
  }

  if (nvs_results != ESP_OK) {
    ESP_LOGE(TAG, "NVS init error");
  }

  /* Initializing Wifi driver and connecting WIFI STA */
  wifi_sta_setup();

  /* Begin task that connect to AWS Device Shadow */
  shadow_start();

  /* Begin task responsible for ota */
  ota_start();
}
