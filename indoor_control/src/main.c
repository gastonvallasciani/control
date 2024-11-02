#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

#include "driver/i2c.h"
#include "esp_log.h"

#include "display_manager.h"

#define LED_PIN 18

#define BUTTON 25

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

void setup_gpio_input(gpio_num_t pin)
{
    // Configuración del GPIO
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;        // Sin interrupciones
    io_conf.mode = GPIO_MODE_INPUT;               // Configuración como entrada
    io_conf.pin_bit_mask = (1ULL << pin);         // Pin específico
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // Sin pull-down
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;      // Activar pull-up (si necesario)

    // Aplicar la configuración
    gpio_config(&io_conf);
}

void app_main()
{
    // char v = 'V';
    // uint8_t i = 1;
    init_led_pin();
    setup_gpio_input(BUTTON);
    int pin_value;
    // ESP_LOGI(TAG, "Inicializando I2C");
    // ESP_ERROR_CHECK(set_i2c()); // inicio el i2c
    // ESP_LOGI(TAG, "Inicializando DISPLAY");
    display_manager_init();
    display_manager_start(75, 'V');
    // display_init();
    // vTaskDelay(2000 / portTICK_PERIOD_MS);
    // ESP_LOGI(TAG, "Termina inicializacion del DISPLAY");
    // display_set_screen_two();
    // vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (true)
    {
        // ESP_LOGI("wait", "...");
        blink_led();
        vTaskDelay(250 / portTICK_PERIOD_MS);
        pin_value = gpio_get_level(BUTTON);
        if (pin_value == 0)
        {
            display_manager_change_screen(75, 'V');
        }

        // blink_line(2);
        // display_set_screen_two();
        /*if (i == 1)
        {
            display_set_screen_one(99, v, true, true, 13, 55);
            i = 2;
        }
        else if (i == 2)
        {
            display_set_screen_two();
            i = 3;
        }
        else
        {
            display_set_screen_three();
            i = 1;
        }*/
    }
}