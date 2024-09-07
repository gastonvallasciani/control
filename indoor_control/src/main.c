#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "../include/version.h"
#include "../include/jumpers_manager.h"
#include "../include/nv_flash_manager.h"
#include "../include/global_manager.h"

void app_main() 
{
    nv_flash_manager_init();
    global_manager_init();
    
    while(true)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}