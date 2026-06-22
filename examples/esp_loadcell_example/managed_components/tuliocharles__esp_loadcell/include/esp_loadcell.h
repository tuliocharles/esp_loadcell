#ifndef _esp_loadcell_H_
#define _esp_loadcell_H_

#include <stdio.h>
#include "esp_err.h"
#include "esp_check.h"
#include "driver/gpio.h"

typedef struct esp_loadcell_t *esp_loadcell_handle_t;

void func(void);

// fazer um enum para ads1232 e hx711
typedef enum {
    ADS1232 = 0,
    HX711
}  loadcell_adc_type_t;

typedef struct{
    gpio_num_t dout;
    gpio_num_t pd_sck;
    loadcell_adc_type_t type;
    uint32_t gain;
    
} esp_loadcell_config_t;

esp_err_t init_esp_loadcell(esp_loadcell_config_t *config, esp_loadcell_handle_t *handle);

uint32_t esp_loadcell_read(esp_loadcell_handle_t handle);

#endif
