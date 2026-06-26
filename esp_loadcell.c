#include <stdio.h>
#include "esp_loadcell.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_rom_sys.h"

typedef struct esp_loadcell_t esp_loadcell_t;

static const char *tag = "ESP-loadcell";

struct esp_loadcell_t
{
    SemaphoreHandle_t ready;
    gpio_num_t dout;
    gpio_num_t pd_sck;
    loadcell_adc_type_t type;
    uint32_t gain;
};

static void IRAM_ATTR wait_loadcell(void *arg)
{
    esp_loadcell_handle_t handle = (esp_loadcell_handle_t)arg;
    gpio_intr_disable(handle->dout);
    xSemaphoreGiveFromISR(handle->ready, NULL);
}

esp_err_t init_esp_loadcell(esp_loadcell_config_t *config, esp_loadcell_handle_t *handle)
{
    esp_err_t ret = ESP_OK;
    esp_loadcell_t *esp_loadcell = NULL;
    ESP_GOTO_ON_FALSE(config && handle, ESP_ERR_INVALID_ARG, err, tag, "Invalid arguments");
    esp_loadcell = calloc(1, sizeof(esp_loadcell_t));

    // PASS configuration parameters to the handle
    esp_loadcell->dout = config->dout;
    esp_loadcell->pd_sck = config->pd_sck;
    esp_loadcell->gain = config->gain;
    esp_loadcell->type = config->type;

    gpio_config_t cfg_clk = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << esp_loadcell->pd_sck)};
    ESP_ERROR_CHECK(gpio_config(&cfg_clk));

    gpio_config_t cfg_dout = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << esp_loadcell->dout),
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&cfg_dout));

    esp_loadcell->ready = xSemaphoreCreateBinary();
    // install gpio isr service
    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(esp_loadcell->dout, wait_loadcell, (void *)esp_loadcell);

    *handle = esp_loadcell;
    ESP_LOGI(tag, "ESP-loadcell initialized successfully");
    ret = ESP_OK;
    return ret;
err:

    if (esp_loadcell)
    {
        free(esp_loadcell);
        esp_loadcell = NULL;
    }

    ESP_LOGE(tag, "Failed to initialize ESP-loadcell : %s", esp_err_to_name(ret));
    return ret;
}

static portMUX_TYPE s_loadcell_mux  = portMUX_INITIALIZER_UNLOCKED;

esp_err_t esp_loadcell_read(esp_loadcell_handle_t handle, int32_t *data_read)
{
    // Implementation for reading from ESP-loadcell
    gpio_set_intr_type(handle->dout, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(handle->dout);

    if (xSemaphoreTake(handle->ready, 500 / portTICK_PERIOD_MS))
    {
        gpio_intr_disable(handle->dout);
        // Semaphore taken successfully
        portENTER_CRITICAL(&s_loadcell_mux );
        uint32_t data = 0;
        for (size_t i = 0; i < 24; i++)
        {
            gpio_set_level(handle->pd_sck, 1);
            esp_rom_delay_us(1);
            data |= gpio_get_level(handle->dout) << (23 - i);
            gpio_set_level(handle->pd_sck, 0);
            esp_rom_delay_us(1);
        }

        if (handle->type == HX711)
        {
            for (size_t i = 0; i <= handle->gain; i++)
            {
                gpio_set_level(handle->pd_sck, 1);
                esp_rom_delay_us(1);
                gpio_set_level(handle->pd_sck, 0);
                esp_rom_delay_us(1);
            }
        }
        portEXIT_CRITICAL(&s_loadcell_mux );

        if (data & 0x800000)
        {
            data |= 0xff000000;
        }

        *data_read = data;
        ESP_LOGI(tag, "Read data: %d", *data_read);
        return ESP_OK;
    }
    else
    {
        // Handle the case where the semaphore could not be taken
        ESP_LOGE(tag, "Failed to read sensor");
        return -ESP_FAIL; // or handle as appropriate
    }

    return ESP_FAIL;
}