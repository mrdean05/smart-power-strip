/**
 ******************************************************************************
 * @file      device_shadow.c
 * @author    Dean Prince Agbodjan
 * @brief     New Firmware OTA and upgrade implementation
 *
 ******************************************************************************
 */
/* Header Files */
#include "esp_https_ota.h"
#include <esp_idf_version.h>
#include <esp_log.h>
#include <sdkconfig.h>

#include "sub_pub_ota.h"

/**
 * @brief Github Server Certificate
 */
extern const uint8_t
    upgrade_server_cert_pem_start[] asm("_binary_github_server_cert_start");
extern const uint8_t
    upgrade_server_cert_pem_end[] asm("_binary_github_server_cert_end");

/**
 * @brief This function handles firmware download in HTTPS and upgrade
 */
esp_err_t do_firmware_upgrade(const char *url) {
  if (!url) {
    return ESP_FAIL;
  }
  esp_http_client_config_t config = {
      .url = url,
      .cert_pem = (char *)upgrade_server_cert_pem_start,
  };
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  esp_https_ota_config_t ota_config = {
      .http_config = &config,
  };

  /* HTTP firrmware upgrade */
  return esp_https_ota(&ota_config);
#else
  return esp_https_ota(&config);
#endif
}