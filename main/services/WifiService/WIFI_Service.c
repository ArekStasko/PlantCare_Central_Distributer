#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "NVS_Service.h"
#include "nvs_flash.h"
#include "esp_netif_ip_addr.h"
#include "esp_http_client.h"
#include "esp_sleep.h"
#include "sdkconfig.h"

static bool wifi_started = false;
char *WIFI_LOG_TAG = "Plantcare Central Distributor - wifi service";

void enter_deep_sleep()
{
	esp_wifi_disconnect();
	esp_wifi_stop();
	esp_wifi_deinit();

	esp_sleep_enable_timer_wakeup(3600000000ULL);
	esp_deep_sleep_start();
}

void save_error_code_to_nvs(esp_err_t error_code)
{
  	nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
       ESP_LOGE(WIFI_LOG_TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }

    nvs_set_str(nvs_handle, "error", error_code);
    nvs_close(nvs_handle);
}

void server_call(void)
{

}

void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting WIFI_EVENT_STA_START ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected WIFI_EVENT_STA_CONNECTED ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection WIFI_EVENT_STA_DISCONNECTED ... \n");
        esp_wifi_connect();
        break;
    case IP_EVENT_STA_GOT_IP:
        {
      		vTaskDelay(pdMS_TO_TICKS(500));
			xTaskCreate(server_call, "server_call", 8192, NULL, 5, NULL);
    		break;
    	}
    default:
        break;
    }
}

void connect_to_wifi()
{
    if (wifi_started) return;
    wifi_started = true;

    printf("WiFi connecting to WiFi network ...\n");
  	char* wifiName = getWifiName();
    char* wifiPassword = getWifiPassword();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    wifi_config_t wifi_configuration = {0};
    snprintf((char*)wifi_configuration.sta.ssid, sizeof(wifi_configuration.sta.ssid), "%s", wifiName);
    snprintf((char*)wifi_configuration.sta.password, sizeof(wifi_configuration.sta.password), "%s", wifiPassword);

    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_wifi_connect();
}
