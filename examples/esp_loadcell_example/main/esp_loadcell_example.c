#include <stdio.h>
#include "esp_loadcell.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    esp_loadcell_handle_t esp_loadcell_handle;

    esp_loadcell_config_t esp_loadcell_config = {
        .dout = 18,//GPIO_NUM_19,
        .pd_sck = 4,//GPIO_NUM_18,
        .type = HX711,
        .gain = 2};

    init_esp_loadcell(&esp_loadcell_config, &esp_loadcell_handle);

    int32_t value = 0;

    esp_err_t r;
    while (1)
    {
        r = esp_loadcell_read(esp_loadcell_handle, &value);
    }
}
