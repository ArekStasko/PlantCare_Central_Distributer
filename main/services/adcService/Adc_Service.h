//
// Created by arekstasko on 4/17/26.
//
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#ifndef ADC_SERVICE_H
#define ADC_SERVICE_H

void ADC_Init(void);
adc_oneshot_unit_handle_t ADC_GetHandle(void);

#endif //ADC_SERVICE_H
