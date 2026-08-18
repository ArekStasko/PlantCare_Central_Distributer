#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "NVS_Service.h"
#include "Bluetooth_Service.h"
#include "WIFI_Service.h"
#include "esp_log.h"
#include "esp_event.h"
#include "Adc_Service.h"

void app_main(void)
{
    nvs_flash_init();
    ADC_Init();

    bool isWifiDataExist = checkIfWiFiDataExists();
    if (!isWifiDataExist)
    {
        enable_bt();
    }
    if(isWifiDataExist)
    {
      connect_to_wifi();

    }
}