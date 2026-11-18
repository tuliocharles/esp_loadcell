#ifndef _esp_loadcell_H_
#define _esp_loadcell_H_

#include <stdio.h>
#include "esp_err.h"
#include "esp_check.h"
#include "driver/gpio.h"

typedef struct esp_loadcell_t *esp_loadcell_handle_t;

typedef enum 
{
    ESP_LOADCELL_ADS1232 = 0,
    ESP_LOADCELL_HX711
} esp_loadcell_adc_type_t;

typedef enum 
{
    ESP_LOADCELL_GAIN_1,
    ESP_LOADCELL_GAIN_2,
    ESP_LOADCELL_GAIN_64,
    ESP_LOADCELL_GAIN_128
} esp_loadcell_gain_t;

typedef enum 
{
    ESP_LOADCELL_CHANNEL_A = 0, 
    ESP_LOADCELL_CHANNEL_B = 1
} esp_loadcell_channel_t;

typedef enum 
{
    ESP_LOADCELL_SPEED_10HZ = 0,
    ESP_LOADCELL_SPEED_80HZ = 1
} esp_loadcell_speed_t;

typedef struct {
    gpio_num_t dout;
    gpio_num_t pd_sck;
    gpio_num_t speed_io;
    gpio_num_t gain0_io;
    gpio_num_t gain1_io;
    gpio_num_t pdwn_io;
    gpio_num_t a0_io;
    esp_loadcell_adc_type_t type;
    esp_loadcell_channel_t channel;  // 0 --> Channel A, 1 --> Channel B
    esp_loadcell_speed_t speed;       // 1 --> 80Hz, 0 --> 10Hz
    esp_loadcell_gain_t gain;
} esp_loadcell_config_t;

/**
 * @brief Initializes the load cell ADC driver (ADS1232 or HX711).
 * 
 * Configures the required GPIOs, allocates internal driver resources, 
 * creates the data-ready binary semaphore, and attaches the DOUT falling-edge ISR.
 * 
 * @param[in]  config Pointer to the configuration structure containing pin mappings and ADC parameters.
 * @param[out] handle Pointer to store the initialized load cell driver handle.
 * 
 * @return
 *      - ESP_OK: Driver initialized successfully.
 *      - ESP_ERR_INVALID_ARG: If config or handle pointer is NULL.
 */
esp_err_t esp_loadcell_init(esp_loadcell_config_t *config, esp_loadcell_handle_t *handle);

/**
 * @brief Reads a 24-bit signed raw conversion value from the load cell ADC.
 * 
 * Waits up to 500 ms for the DOUT pin falling edge before shifting out 24 bits 
 * of data (plus gain/channel clock pulses for HX711) and sign-extending to 32 bits.
 * 
 * @param[in]  handle    Load cell driver handle.
 * @param[out] data_read Pointer to store the sign-extended 32-bit raw ADC reading.
 * 
 * @return
 *      - ESP_OK: Data read successfully.
 *      - ESP_ERR_INVALID_ARG: If handle is NULL.
 *      - ESP_ERR_INVALID_STATE: If the ADS1232 is currently in power-down mode.
 *      - -ESP_FAIL: Timeout waiting for ADC data ready (DOUT falling edge).
 */
esp_err_t esp_loadcell_read(esp_loadcell_handle_t handle, int32_t *data_read);

/**
 * @brief Sets the sampling rate (output data rate) of the ADC.
 * 
 * Updates the internal speed state and drives the SPEED pin if the device is an ADS1232
 * with a valid speed GPIO configured.
 * 
 * @param[in] handle Load cell driver handle.
 * @param[in] speed  Sampling rate selector (1 for 80 Hz, 0 for 10 Hz).
 * 
 * @return
 *      - ESP_OK: Speed updated successfully.
 *      - ESP_ERR_INVALID_ARG: If handle is NULL.
 */
esp_err_t esp_loadcell_set_speed(esp_loadcell_handle_t handle,  esp_loadcell_speed_t speed);

/**
 * @brief Dynamically configures the programmable gain amplifier (PGA) of the ADS1232.
 * 
 * Drives the GAIN0 and GAIN1 GPIO pins according to the selected gain level.
 * 
 * @param[in] handle Load cell driver handle.
 * @param[in] gain   Gain selection enum (ADS1232_GAIN_1, ADS1232_GAIN_2, ADS1232_GAIN_64, or ADS1232_GAIN_128).
 * 
 * @return
 *      - ESP_OK: Gain configured successfully.
 *      - ESP_ERR_INVALID_ARG: If handle is NULL or gain value is invalid.
 *      - ESP_ERR_NOT_SUPPORTED: If the initialized ADC type is not ADS1232.
 */
esp_err_t esp_loadcell_set_gain(esp_loadcell_handle_t handle, esp_loadcell_gain_t gain);

/**
 * @brief Selects the active differential input channel on the ADS1232.
 * 
 * Drives the A0 GPIO pin (0 for Channel A, 1 for Channel B).
 * 
 * @param[in] handle  Load cell driver handle.
 * @param[in] channel Input channel selection (ESP_LOADCELL_CHANNEL_A or ESP_LOADCELL_CHANNEL_B).
 * 
 * @return
 *      - ESP_OK: Input channel selected successfully.
 *      - ESP_ERR_INVALID_ARG: If handle is NULL.
 *      - ESP_ERR_NOT_SUPPORTED: If the initialized ADC type is not ESP_LOADCELL_ADS1232.
 */
esp_err_t esp_loadcell_set_channel(esp_loadcell_handle_t handle, esp_loadcell_channel_t channel);

/**
 * @brief Controls the Power-Down / Reset state of the ADS1232.
 * 
 * Drives the PDWN pin low to enter standby/power-down mode or high to wake up and reset the ADC.
 * 
 * @param[in] handle Load cell driver handle.
 * @param[in] enable Set to true to enter power-down mode (PDWN = LOW), or false to wake up (PDWN = HIGH).
 * 
 * @return
 *      - ESP_OK: Power state updated successfully.
 *      - ESP_ERR_INVALID_ARG: If handle is NULL.
 *      - ESP_ERR_NOT_SUPPORTED: If the initialized ADC type is not ADS1232.
 */
esp_err_t esp_loadcell_power_down(esp_loadcell_handle_t handle, bool enable);

#endif