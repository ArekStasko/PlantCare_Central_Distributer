#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static adc_oneshot_unit_handle_t adc_handle = NULL;

void ADC_Init(void)
{
    if (adc_handle != NULL) return;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    esp_err_t ret = adc_oneshot_new_unit(&init_config, &adc_handle);

    printf("ADC init ret: %s, handle: %p\n", esp_err_to_name(ret), adc_handle);
}

adc_oneshot_unit_handle_t ADC_GetHandle(void)
{
    return adc_handle;
}