#ifndef NVS_SERVICE_H
#define NVS_SERVICE_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
bool checkIfWiFiDataExists(void);
char* getServerAddress(void);
char* getWifiName(void);
char* getWifiPassword(void);
char* getModuleId(void);
char* getErrorCode(void);
void resetWifiData(void);

#endif //NVS_SERVICE_H
