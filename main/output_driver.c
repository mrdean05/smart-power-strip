/**
 ******************************************************************************
 * @file      device_shadow.c
 * @author    Dean Prince Agbodjan
 * @brief     Device Drivers Firmware Implementation
 *
 ******************************************************************************
 */
/* Header Files */
#include "esp_system.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"
#include "output_driver.h"

/* Relay GPIOs */
unsigned int relay[] = {23, 22, 21, 5};

static bool r_output_state[4];

/* I2C variables */
smbus_info_t *smbus_info;
i2c_lcd1602_info_t *lcd_info;

/**
 * @brief Initializing I2C bus 
 */
static void i2c_master_init(void) {
  int i2c_master_port = I2C_MASTER_NUM;
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = I2C_MASTER_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_DISABLE; // GY-2561 provides 10kΩ pullups
  conf.scl_io_num = I2C_MASTER_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_DISABLE; // GY-2561 provides 10kΩ pullups
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
  i2c_param_config(i2c_master_port, &conf);
  i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_LEN,
                     I2C_MASTER_TX_BUF_LEN, 0);
}

/**
 * @brief Initializes I2C and SMBus and display info on lcd screen
 */
void lcd2004(void) {

  /* Set up and initalize the I2C */
  i2c_master_init();
  i2c_port_t i2c_num = I2C_MASTER_NUM;
  uint8_t address = CONFIG_LCD1602_I2C_ADDRESS;

  /* Allocate memory for SMBus */
  smbus_info = smbus_malloc();

  /* Initialize SMBus with I2C port abd device address */
  ESP_ERROR_CHECK(smbus_init(smbus_info, i2c_num, address));

  /*Set a second timeout for SMbus */
  ESP_ERROR_CHECK(smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS));

  /* Set up the LCD1602 device with backlight off */
  lcd_info = i2c_lcd1602_malloc();
  ESP_ERROR_CHECK(i2c_lcd1602_init(lcd_info, smbus_info, true, LCD_NUM_ROWS,
                                   LCD_NUM_COLUMNS, LCD_NUM_VISIBLE_COLUMNS));

  /* lcd reset */
  ESP_ERROR_CHECK(i2c_lcd1602_reset(lcd_info));

  /* Write Info on LCD */
  i2c_lcd1602_move_cursor(lcd_info, 3, 0);
  i2c_lcd1602_write_string(lcd_info, "LOAD STATUS");

  i2c_lcd1602_move_cursor(lcd_info, 0, 1);
  i2c_lcd1602_write_string(lcd_info, "LOAD 1: ");
  i2c_lcd1602_move_cursor(lcd_info, 8, 1);
  i2c_lcd1602_write_string(lcd_info, "0");
  i2c_lcd1602_move_cursor(lcd_info, 0, 2);
  i2c_lcd1602_write_string(lcd_info, "LOAD 2: ");
  i2c_lcd1602_move_cursor(lcd_info, 8, 2);
  i2c_lcd1602_write_string(lcd_info, "0");
  i2c_lcd1602_move_cursor(lcd_info, 10, 1);
  i2c_lcd1602_write_string(lcd_info, "LOAD 3: ");
  i2c_lcd1602_move_cursor(lcd_info, 18, 1);
  i2c_lcd1602_write_string(lcd_info, "0");
  i2c_lcd1602_move_cursor(lcd_info, 10, 2);
  i2c_lcd1602_write_string(lcd_info, "LOAD 4: ");
  i2c_lcd1602_move_cursor(lcd_info, 18, 2);
  i2c_lcd1602_write_string(lcd_info, "0");
}

/**
 * @brief Displays WiFi Status on LCD 
 * @param [IN] WiFi status
 *  - 1: connected
 *  - 0: not connected
 */
void wifi_status(int status) {

  if (status == 1) {
    i2c_lcd1602_move_cursor(lcd_info, 19, 0);
    i2c_lcd1602_write_string(lcd_info, "C");
  }

  else if (status == 0) {
    i2c_lcd1602_move_cursor(lcd_info, 19, 0);
    i2c_lcd1602_write_string(lcd_info, " ");
  }
}

/**
 *@brief This functions handle change in relay state.
 *@param [IN] pointer to an unsigned int
 *@param [IN] pointer to a bool
 */
static void change_output_state(unsigned int *relay_pin, bool *set) {
  gpio_set_level(*relay_pin, (*set));
}

/**
 *@brief Configures and set as output GPIOs connected to relays 
 */
void gpio_init() {

  gpio_config_t io_config_1 = {
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = 0,
      .pull_down_en = 1,
      .pin_bit_mask = (((uint64_t)1 << relay[0]) | ((uint64_t)1 << relay[1]) |
                       ((uint64_t)1 << relay[2]) | ((uint64_t)1 << relay[3])),
  };

  gpio_config(&io_config_1);
}

/** 
 * @brief Update Relay status on LCD scren and changes output state. 
 * @param [IN] state in bool
 * @param [IN] GPIO number
 * @retval Returns ESP_OK if successful
 */
int app_driver_set_state(bool state, unsigned short relay_no) {
  switch (relay_no) {
  case 1:
    if (r_output_state[0] != state) {
      r_output_state[0] = state;
      int display = state;
      /* Change relay state */
      change_output_state(&relay[0], &r_output_state[0]);

      if (display == 0) {
        /* Update data on the lcd screen */
        i2c_lcd1602_move_cursor(lcd_info, 8, 1);
        i2c_lcd1602_write_string(lcd_info, "0");
      }

      else if (display == 1) {
        i2c_lcd1602_move_cursor(lcd_info, 8, 1);
        i2c_lcd1602_write_string(lcd_info, "1");
      }
    }

    break;

  case 2:

    if (r_output_state[1] != state) {
      r_output_state[1] = state;
      int display = state;
      change_output_state(&relay[1], &r_output_state[1]);
      if (display == 0) {
        i2c_lcd1602_move_cursor(lcd_info, 8, 2);
        i2c_lcd1602_write_string(lcd_info, "0");
      }

      else if (display == 1) {
        i2c_lcd1602_move_cursor(lcd_info, 8, 2);
        i2c_lcd1602_write_string(lcd_info, "1");
      }
    }
    break;

  case 3:

    if (r_output_state[2] != state) {
      r_output_state[2] = state;
      int display = state;
      change_output_state(&relay[2], &r_output_state[2]);
      if (display == 0) {
        i2c_lcd1602_move_cursor(lcd_info, 18, 1);
        i2c_lcd1602_write_string(lcd_info, "0");
      }

      else if (display == 1) {
        i2c_lcd1602_move_cursor(lcd_info, 18, 1);
        i2c_lcd1602_write_string(lcd_info, "1");
      }
    }
    break;

  case 4:

    if (r_output_state[3] != state) {
      r_output_state[3] = state;
      int display = state;
      change_output_state(&relay[3], &r_output_state[3]);
      if (display == 0) {
        i2c_lcd1602_move_cursor(lcd_info, 18, 2);
        i2c_lcd1602_write_string(lcd_info, "0");
      }

      else if (display == 1) {
        i2c_lcd1602_move_cursor(lcd_info, 18, 2);
        i2c_lcd1602_write_string(lcd_info, "1");
      }
    }
    break;
  default:
    break;
  }

  return ESP_OK;
}

/**
 * @brief Get the current state of the GPIO/relay.
 * @param [IN] Relay(GPIO) number
 */
bool app_driver_get_state(unsigned short relay_pin) {
  relay_pin -= 1;
  return r_output_state[relay_pin];
}
