#include "freertos/FreeRTOS.h"

#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "../include/version.h"
#include "../include/jumpers_manager.h"
#include "../include/nv_flash_manager.h"
#include "../include/global_manager.h"
#include "../include/s_run_manager.h"
#include "esp_log.h"
#define PHASE_NUMBER 3

void app_main()
{
    char version[10];
    uint8_t version_len;

    nv_flash_manager_init();

    global_manager_init();

    s_run_manager_init();

    printf("Project: Indoor control \n");
    get_version(version, &version_len);
    printf("Phase: %d, Version number: %s \n", PHASE_NUMBER, version);
    while (true)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}