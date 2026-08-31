#ifndef _esp_loadcell_H_
#define _esp_loadcell_H_

#include <stdio.h>
#include "esp_err.h"
#include "esp_check.h"
#include "driver/gpio.h"

typedef struct esp_loadcell_t *esp_loadcell_handle_t;

typedef enum {
    ADS1232 = 0,
    HX711
}  loadcell_adc_type_t;

typedef struct{
    gpio_num_t dout;
    gpio_num_t pd_sck;
    gpio_num_t speed_io;
    gpio_num_t gain0_io;
    gpio_num_t gain1_io;
    gpio_num_t pdwn_io;
    gpio_num_t a0_io;

    uint8_t speed;       //1 --> 80Hz, 0 --> 10Hz
    loadcell_adc_type_t type;
    uint32_t gain;
    
} esp_loadcell_config_t;

esp_err_t init_esp_loadcell(esp_loadcell_config_t *config, esp_loadcell_handle_t *handle);

esp_err_t esp_loadcell_read(esp_loadcell_handle_t handle, int32_t *data_read);

esp_err_t esp_loadcell_set_speed(esp_loadcell_handle_t handle, uint8_t speed);

#endif
