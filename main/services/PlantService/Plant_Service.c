#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "Plant_Service.h"
#include "GPIO_Service.h"


int perform_water_supply(int plantId)
{
    if (plantId == -1) return -1;

    Run_WaterPump();
    vTaskDelay(pdMS_TO_TICKS(10000));
    Stop_WaterPump();

    return 1;
}

void save_executed_water_supply(int plantId)
{

}
