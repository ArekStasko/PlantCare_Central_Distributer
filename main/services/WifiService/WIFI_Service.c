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
#include "Plant_Service.h"

static bool wifi_started = false;
char *WIFI_LOG_TAG = "Plantcare Central Distributor - wifi service";

static int water_supply_result = -1;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
        case HTTP_EVENT_ON_DATA:
        {
            if (evt->data_len > 0)
            {
                char buffer[32];

                int len = evt->data_len;

                if (len >= sizeof(buffer))
                {
                    len = sizeof(buffer) - 1;
                }

                memcpy(buffer, evt->data, len);
                buffer[len] = '\0';

                water_supply_result = atoi(buffer);
            }

            break;
        }

        default:
            break;
    }

    return ESP_OK;
}

void enter_deep_sleep()
{
	esp_wifi_disconnect();
	esp_wifi_stop();
	esp_wifi_deinit();

	esp_sleep_enable_timer_wakeup(600000000ULL);
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

int remove_water_supply(char* moduleId, int plantId)
{
    char *serverAddress = getServerAddress();

    if (!moduleId || !serverAddress) return -1;

    char full_url[128];
    const int serverPort = 8080;
    snprintf(full_url, sizeof(full_url), "http://%s:%d/api/distributor/%s/%d/water-supply", serverAddress, serverPort, moduleId, plantId);

    esp_http_client_config_t config = {
        .url = full_url,
        .method = HTTP_METHOD_DELETE,
        .timeout_ms = 5000,
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");

    const char *auth_token = "deb1197807e28b36bc6a7e5b9d6ad13c9fdc92e407364a5615d31518705057a5";

    char auth_header[128];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", auth_token);
    esp_http_client_set_header(client, "Authorization", auth_header);

    water_supply_result = -1;

    esp_err_t err = esp_http_client_perform(client);

    if (err != ESP_OK)
    {
        save_error_code_to_nvs(err);
        esp_http_client_cleanup(client);
        return -1;
    }

    int status_code = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

	return status_code;
}

int get_water_supply_status(char* moduleId)
{
    char *serverAddress = getServerAddress();

    if (!moduleId || !serverAddress) return -1;

    char full_url[128];
    const int serverPort = 8080;
    snprintf(full_url, sizeof(full_url), "http://%s:%d/api/distributor/%s/water-supply/status", serverAddress, serverPort, moduleId);

    esp_http_client_config_t config = {
        .url = full_url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 5000,
        .event_handler = http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");

    const char *auth_token = "deb1197807e28b36bc6a7e5b9d6ad13c9fdc92e407364a5615d31518705057a5";

    char auth_header[128];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", auth_token);
    esp_http_client_set_header(client, "Authorization", auth_header);

    water_supply_result = -1;

    esp_err_t err = esp_http_client_perform(client);

    if (err != ESP_OK)
    {
        save_error_code_to_nvs(err);
        esp_http_client_cleanup(client);
        return -1;
    }

    int status_code = esp_http_client_get_status_code(client);
    if(status_code != 200) enter_deep_sleep();

    esp_http_client_cleanup(client);

	return water_supply_result;
}

void run_remove_water_supply(char* moduleId, int plantId)
{
    int removal_result = 0;
    if(water_supply_result == 1)
    {
        removal_result = remove_water_supply(moduleId, plantId);
        process_executed_water_supply(plantId, removal_result);
    }
}

void run_get_water_supply_status(void)
{
  	char *moduleId = getModuleId();

    // TODO: CHANGE PLANT ID TO STATUS ID
    int plantId = get_water_supply_status(moduleId);
    bool awaiting_water_result = verify_awaiting_water_supply(plantId);

    if(awaiting_water_result)
    {
        run_remove_water_supply(moduleId, plantId);
        return;
    }

    int processing_result = perform_water_supply(plantId);
    if(processing_result == -1) enter_deep_sleep();
    run_remove_water_supply(moduleId, plantId);
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
			xTaskCreate(run_get_water_supply_status, "run_get_water_supply_status", 8192, NULL, 5, NULL);
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
