#include "Plant_Service.h"
#include "GPIO_Service.h"
#include "freertos/timers.h"


int perform_water_supply(int plantId)
{
    if (plantId == -1) return -1;

    Run_WaterPump();
    vTaskDelay(pdMS_TO_TICKS(10000));
    Stop_WaterPump();

    return 1;
}
