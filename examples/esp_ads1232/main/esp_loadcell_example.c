#include <stdio.h>
#include "esp_loadcell.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void app_main(void)
{
    esp_loadcell_handle_t esp_loadcell_handle;

    esp_loadcell_config_t esp_loadcell_config = {
        .dout = GPIO_NUM_19,
        .pd_sck = GPIO_NUM_18,
    };
    
    init_esp_loadcell(&esp_loadcell_config, &esp_loadcell_handle);

    uint32_t value = 0;
    
    while(1)
    {
        value = esp_loadcell_read (esp_loadcell_handle);
        //printf("Read value: %ld\n", value);
    }


}
