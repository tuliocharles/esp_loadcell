#include <stdio.h>
#include "esp_loadcell.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_rom_sys.h"

typedef struct esp_loadcell_t esp_loadcell_t;

static const char *tag = "ESP-loadcell";

struct esp_loadcell_t
{
    SemaphoreHandle_t   ready;
    gpio_num_t          dout;
    gpio_num_t          pd_sck;
    gpio_num_t          speed_io;
    gpio_num_t          gain0_io;
    gpio_num_t          gain1_io;
    gpio_num_t          pdwn_io;
    gpio_num_t          a0_io;
    esp_loadcell_adc_type_t type;
    esp_loadcell_channel_t  channel;
    esp_loadcell_speed_t speed; // 1 --> 80Hz, 0 --> 10Hz
    esp_loadcell_gain_t  gain;
    bool                 is_power_down;
};

static void IRAM_ATTR wait_loadcell(void *arg)
{
    esp_loadcell_handle_t handle = (esp_loadcell_handle_t)arg;
    gpio_intr_disable(handle->dout);
    xSemaphoreGiveFromISR(handle->ready, NULL);
}

esp_err_t esp_loadcell_set_speed(esp_loadcell_handle_t handle, esp_loadcell_speed_t speed)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (handle->type ==  ESP_LOADCELL_ADS1232 && handle->speed_io != GPIO_NUM_NC)
    {
        handle->speed = speed;
        gpio_set_level(handle->speed_io, handle->speed);
    }

    return ESP_OK;
}

esp_err_t esp_loadcell_set_gain(esp_loadcell_handle_t handle, esp_loadcell_gain_t gain)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->type != ESP_LOADCELL_ADS1232)
    {
        return ESP_ERR_NOT_SUPPORTED;
    }

    int g0_level = 0;
    int g1_level = 0;

    switch(gain)
    {
        case ESP_LOADCELL_GAIN_1:
            g1_level = 0;
            g0_level = 0;
            
            break;
        case ESP_LOADCELL_GAIN_2:
            g1_level = 0;
            g0_level = 1;
            
            break;
        case ESP_LOADCELL_GAIN_64:
            g1_level = 1;
            g0_level = 0;
            break;
        case ESP_LOADCELL_GAIN_128:
            g1_level = 1;
            g0_level = 1;
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }

    handle->gain = gain;

    gpio_set_level(handle->gain0_io, g0_level);
    gpio_set_level(handle->gain1_io, g1_level);

    return ESP_OK;
}

esp_err_t esp_loadcell_set_channel(esp_loadcell_handle_t handle, esp_loadcell_channel_t channel)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->type != ESP_LOADCELL_ADS1232)
    {
        return ESP_ERR_NOT_SUPPORTED;
    }

    handle->channel = channel;
    int level = (channel == ESP_LOADCELL_CHANNEL_B) ? 1 : 0;
    gpio_set_level(handle->a0_io, level);

    ESP_LOGI(tag, "ADS1232 canal selecionado: %s (A0=%d)", (channel == ESP_LOADCELL_CHANNEL_B) ? "CH B" : "CH A", level);
    return ESP_OK;
}

esp_err_t esp_loadcell_power_down(esp_loadcell_handle_t handle, bool is_enable)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (handle->type != ESP_LOADCELL_ADS1232)
    {
        return ESP_ERR_NOT_SUPPORTED;
    }

    handle->is_power_down = is_enable;
    if (handle->is_power_down)
    {
        // PDWN em nível lógico baixo desliga os circuitos internos do ADS1232
        gpio_set_level(handle->pdwn_io, 0);
        ESP_LOGI(tag, "ADS1232 colocado em modo Power-Down");
    }
    else
    {
        // PDWN em nível lógico alto reativa o chip
        gpio_set_level(handle->pdwn_io, 1);
        ESP_LOGI(tag, "ADS1232 reativado (Power-Up)");
    }

    return ESP_OK;
}

esp_err_t esp_loadcell_init(esp_loadcell_config_t *config, esp_loadcell_handle_t *handle)
{
    esp_err_t ret = ESP_OK;
    esp_loadcell_t *esp_loadcell = NULL;
    ESP_GOTO_ON_FALSE(config && handle, ESP_ERR_INVALID_ARG, err, tag, "Invalid arguments");
    esp_loadcell = calloc(1, sizeof(esp_loadcell_t));

    esp_loadcell->dout = config->dout;
    esp_loadcell->pd_sck = config->pd_sck;

    if (config->type == ESP_LOADCELL_ADS1232)
    {
        esp_loadcell->speed_io = config->speed_io;
        esp_loadcell->gain0_io = config->gain0_io;
        esp_loadcell->gain1_io = config->gain1_io;
        esp_loadcell->pdwn_io = config->pdwn_io;
        esp_loadcell->a0_io = config->a0_io;

        esp_loadcell->type = config->type;
        
        esp_loadcell_set_channel(esp_loadcell, config->channel);

        esp_loadcell_set_speed(esp_loadcell, config->speed);

        esp_loadcell_set_gain(esp_loadcell, config->gain);
        
        esp_loadcell_power_down(esp_loadcell, false);

        gpio_config_t cfg_clk = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = ((1ULL << esp_loadcell->pd_sck) | 
                             (1ULL << esp_loadcell->speed_io) | 
                             (1ULL << esp_loadcell->a0_io) | 
                             (1ULL << esp_loadcell->pdwn_io) | 
                             (1ULL << esp_loadcell->gain1_io) | 
                             (1ULL << esp_loadcell->gain0_io))};
        ESP_ERROR_CHECK(gpio_config(&cfg_clk));

        gpio_set_level(esp_loadcell->speed_io, config->speed);
        gpio_set_level(esp_loadcell->a0_io, esp_loadcell->channel);                // Canal A por padrão (A0 = 0)
        gpio_set_level(esp_loadcell->pdwn_io, 1);              // Ativado por padrão (PDWN = 1)

    /*    int g0_init = 0, g1_init = 0;
        if (config->gain == 2)          { g1_init = 0; g0_init = 1; }
        else if (config->gain == 64)    { g1_init = 1; g0_init = 0; }
        else if (config->gain == 128)   { g1_init = 1; g0_init = 1; }
        else                            { g1_init = 0; g0_init = 0; }

        gpio_set_level(esp_loadcell->gain0_io, g0_init);
        gpio_set_level(esp_loadcell->gain1_io, g1_init);*/
    }
    else if (config->type == ESP_LOADCELL_HX711)
    {
        esp_loadcell->speed_io = GPIO_NUM_NC;
        esp_loadcell->gain0_io = GPIO_NUM_NC;
        esp_loadcell->gain1_io = GPIO_NUM_NC;
        esp_loadcell->pdwn_io = GPIO_NUM_NC;
        esp_loadcell->a0_io = GPIO_NUM_NC;
        esp_loadcell->gain = config->gain;
        esp_loadcell->type = config->type;

        gpio_config_t cfg_clk = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = ((1ULL << esp_loadcell->pd_sck))};
        ESP_ERROR_CHECK(gpio_config(&cfg_clk));
    }

    gpio_config_t cfg_dout = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << esp_loadcell->dout),
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&cfg_dout));

    esp_loadcell->ready = xSemaphoreCreateBinary();
    gpio_install_isr_service(0);
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

static portMUX_TYPE s_loadcell_mux = portMUX_INITIALIZER_UNLOCKED;

esp_err_t esp_loadcell_read(esp_loadcell_handle_t handle, int32_t *data_read)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Impede a leitura se o componente estiver em modo Power-Down
  /*  if (handle->type == ESP_LOADCELL_ADS1232 && handle->is_power_down)
    {
        ESP_LOGW(tag, "Tentativa de leitura com o ADS1232 em Power-Down");
        return ESP_ERR_INVALID_STATE;
    }*/

    gpio_set_intr_type(handle->dout, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(handle->dout);

    if (xSemaphoreTake(handle->ready, 500 / portTICK_PERIOD_MS))
    {
        gpio_intr_disable(handle->dout);
        portENTER_CRITICAL(&s_loadcell_mux);
        uint32_t data = 0;
        for (size_t i = 0; i < 24; i++)
        {
            gpio_set_level(handle->pd_sck, 1);
            esp_rom_delay_us(1);
            data |= gpio_get_level(handle->dout) << (23 - i);
            gpio_set_level(handle->pd_sck, 0);
            esp_rom_delay_us(1);
        }

        if (handle->type == ESP_LOADCELL_HX711)
        {
            for (size_t i = 0; i <= handle->gain; i++)
            {
                gpio_set_level(handle->pd_sck, 1);
                esp_rom_delay_us(1);
                gpio_set_level(handle->pd_sck, 0);
                esp_rom_delay_us(1);
            }
        }
        portEXIT_CRITICAL(&s_loadcell_mux);

        if (data & 0x800000)
        {
            data |= 0xff000000;
        }

        *data_read = data;
        ESP_LOGI(tag, "Read data: %ld", *data_read);
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(tag, "Failed to read sensor");
        return -ESP_FAIL;
    }

    return ESP_FAIL;
}