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

#define BUTTON_DOWN 25

#define BUTTON_VF 14

#define BUTTON_AUX 35

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

uint8_t read_button(uint8_t button)
{
    static int last_state = 1; // Estado anterior del botón (1 = no presionado)
    int current_state = gpio_get_level(button);

    if (current_state != last_state)
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);    // Esperar el tiempo de antirrebote
        current_state = gpio_get_level(button); // Leer el estado nuevamente
    }
    /*if (current_state == last_state)
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }*/

    last_state = current_state;
    return current_state;
}

void app_main()
{
    // char v = 'V';
    uint8_t i = 1;
    init_led_pin();
    setup_gpio_input(BUTTON_DOWN);
    setup_gpio_input(BUTTON_VF);
    setup_gpio_input(BUTTON_AUX);
    uint8_t button1;
    uint8_t button2;
    uint8_t button3;
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
        button1 = read_button(BUTTON_DOWN);
        button2 = read_button(BUTTON_VF);
        button3 = read_button(BUTTON_AUX);
        if (button3 == 0)
        {
            ESP_LOGI("Button", "Apreto boton 3");

            display_manager_aux();
        }
        if (button2 == 0)
        {
            ESP_LOGI("Button", "Apreto boton 2");
            display_manager_auxt();
        }
        if (button1 == 0)
        {
            ESP_LOGI("Button", "Apreto boton 1");
            display_manager_vf();
        }

        vTaskDelay(150 / portTICK_PERIOD_MS);

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