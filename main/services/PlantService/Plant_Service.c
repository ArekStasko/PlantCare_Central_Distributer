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

void process_executed_water_supply(int plantId, int statusCode)
{
    if(statusCode == 200)
    {
        savePlantId("-1");
        return;
    }

    savePlantId((char)(plantId));
}

bool verify_awaiting_water_supply(int plantId)
{
  char saved_plant_id = getPlantId();
  if(saved_plant_id == -1) return;

  return plantId == saved_plant_id;
}
