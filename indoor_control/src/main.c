#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "display_dogs164.h"

#define LED_PIN 18

// comandos display
static const char *TAG = "I2C";

uint8_t led_level = 0;

static esp_err_t init_led_pin(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

static esp_err_t blink_led(void)
{
    led_level = !led_level;
    gpio_set_level(LED_PIN, led_level);
    return ESP_OK;
}

void app_main()
{
    int a = 0;
    init_led_pin();
    ESP_LOGI(TAG, "Inicializando I2C");
    ESP_ERROR_CHECK(set_i2c());
    ESP_LOGI(TAG, "Inicializando DISPLAY");
    display_init();
    ESP_LOGI(TAG, "Termina inicializacion del DISPLAY");

    while (true)
    {
        ESP_LOGI("wait", "...");
        if (a == 0)
        {
            set_cursor(1, 5);
            display_send_data(0xE0);
            a = 1;
        }
        else
        {
            set_cursor(1, 5);
            display_send_data(0xDE);
            a = 0;
        }

        blink_led();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}