#include <stdio.h>
#include "esp_loadcell.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

const char *TAG = "ESP-Loadcell Example";

void app_main(void)
{
    esp_loadcell_handle_t esp_loadcell_handle;

    esp_loadcell_config_t esp_loadcell_config = 
    {
        .dout = 19,      // GPIO_NUM_19
        .pd_sck = 18,    // GPIO_NUM_18
        .speed_io = 4,   // GPIO_NUM_4
        .gain0_io = 21,  // GPIO_NUM_21
        .gain1_io = 22,  // GPIO_NUM_22
        .pdwn_io = 25,   // GPIO_NUM_25
        .a0_io = 23,     // GPIO_NUM_23
        .type = ESP_LOADCELL_ADS1232, // ADS1232 or Hx711
        .channel = ESP_LOADCELL_CHANNEL_A, // A or B
        .speed = ESP_LOADCELL_SPEED_80HZ,      // 0 --> 10Hz, 1 --> 80Hz
        .gain = ESP_LOADCELL_GAIN_2        // 1, 2, 64 ou 128
    };

    ESP_ERROR_CHECK(esp_loadcell_init(&esp_loadcell_config, &esp_loadcell_handle));

    esp_loadcell_set_channel(esp_loadcell_handle, ESP_LOADCELL_CHANNEL_B);

    esp_loadcell_set_speed(esp_loadcell_handle, ESP_LOADCELL_SPEED_10HZ);

    esp_loadcell_set_gain(esp_loadcell_handle, ESP_LOADCELL_GAIN_128);

    
    esp_loadcell_power_down(esp_loadcell_handle, false);

    int32_t value;
    esp_err_t r;
    while (1)
    {
        r = esp_loadcell_read(esp_loadcell_handle, &value);
        // next text must be in english
        if(r == ESP_OK)
        {
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read sensor: %s", esp_err_to_name(r));
        }

    }

   

}