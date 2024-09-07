#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "../include/version.h"
#include "../include/jumpers_manager.h"
#include "../include/nv_flash_manager.h"
#include "../include/global_manager.h"

void app_main() 
{
    char version[10];
     uint8_t version_len;

    nv_flash_manager_init();
    global_manager_init();

    get_version(version, &version_len);
    printf("Version number %s \n", version);
    
    while(true)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}