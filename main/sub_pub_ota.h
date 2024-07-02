#pragma once

int ota_start(void);
esp_err_t do_firmware_upgrade(const char *url);