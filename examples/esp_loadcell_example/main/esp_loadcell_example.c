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

    //esp_err_t r;
/*    while (1)
    {
        r = esp_loadcell_read(esp_loadcell_handle, &value);
        if(r == ESP_OK)
        {
            counter++;

            if(counter >= 5)
            {
                counter = 0;

                gain_index = (gain_index + 1) % 4;
                uint32_t new_gain = gains_to_test[gain_index];
                ESP_LOGI(TAG, "==> Teste: Alterando ganho para %lu", new_gain);
                esp_loadcell_set_gain(esp_loadcell_handle, new_gain);

                //current_channel = (current_channel == ESP_LOADCELL_CHANNEL_A) ? ESP_LOADCELL_CHANNEL_B : ESP_LOADCELL_CHANNEL_A;
                //ESP_LOGI(TAG, "==> Teste: Alternando para %s", (current_channel == ESP_LOADCELL_CHANNEL_B) ? "Canal B" : "Canal A");
                esp_loadcell_set_channel(esp_loadcell_handle, current_channel);

                ESP_LOGI(TAG, "==> Teste: Entrando em modo Power-Down (standby)...");
                esp_loadcell_power_down(esp_loadcell_handle, true);
                
                esp_err_t err_pd = esp_loadcell_read(esp_loadcell_handle, &value);
                ESP_LOGW(TAG, "Status da leitura em Power-Down: %s (esperado ESP_ERR_INVALID_STATE)", esp_err_to_name(err_pd));

                vTaskDelay(pdMS_TO_TICKS(1000));

                ESP_LOGI(TAG, "==> Teste: Reativando o ADS1232 (Power-Up)...");
                esp_loadcell_power_down(esp_loadcell_handle, false);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
        else
        {
            ESP_LOGE(TAG, "Falha na leitura do sensor: %s", esp_err_to_name(r));
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }*/

}