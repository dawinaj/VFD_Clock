#include "Settings.h"

#include <unordered_map>
#include <mutex>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_log.h>
#include <esp_check.h>
#include <esp_event.h>

#include <nvs_flash.h>

#include <esp_netif.h> // REQUIRES esp_netif

#include <esp_wifi.h>
#include <esp_mac.h>

#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_softap.h>

namespace Settings
{
	namespace
	{
		EventGroupHandle_t ip_event_group;
		static constexpr EventBits_t has_ip_from_sta = 0b00000001;
		static constexpr EventBits_t has_ip_from_eth = 0b00000010;
	}

	//================================//
	//         IMPLEMENTATION         //
	//================================//

	static void event_handler_wifi(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
	{
		assert(event_base == WIFI_EVENT);

		switch (event_id)
		{
		case WIFI_EVENT_STA_START:
			ESP_LOGI(TAG, "STA start. Connecting...");
			esp_wifi_connect();
			break;
		case WIFI_EVENT_STA_DISCONNECTED:
			ESP_LOGI(TAG, "STA disconnected. Reconnecting...");
			esp_wifi_connect();
			break;
		case WIFI_EVENT_AP_STACONNECTED:
		{
			wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
			ESP_LOGI(TAG, "Station connected: " MACSTR ", AID=%d", MAC2STR(event->mac), event->aid);
			break;
		}
		case WIFI_EVENT_AP_STADISCONNECTED:
		{
			wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
			ESP_LOGI(TAG, "Station disconnected: " MACSTR ", AID=%d", MAC2STR(event->mac), event->aid);
			break;
		}
		default:
			break;
		}
	}

	static void event_handler_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
	{
		assert(event_base == IP_EVENT);

		switch (event_id)
		{
		case IP_EVENT_STA_GOT_IP:
		{
			ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
			ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
			xEventGroupSetBits(ip_event_group, has_ip_from_sta);
			break;
		}
		case IP_EVENT_STA_LOST_IP:
		{
			ESP_LOGI(TAG, "Lost IP");
			xEventGroupClearBits(ip_event_group, has_ip_from_sta);
			break;
		}
		case IP_EVENT_ETH_GOT_IP:
		{
			ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
			ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
			xEventGroupSetBits(ip_event_group, has_ip_from_eth);
			break;
		}
		case IP_EVENT_ETH_LOST_IP:
		{
			ESP_LOGI(TAG, "Lost IP");
			xEventGroupClearBits(ip_event_group, has_ip_from_eth);
			break;
		}
		default:
			break;
		}
	}

	static void event_handler_provisioning(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
	{
		assert(event_base == WIFI_PROV_EVENT);

		static int retries = 0;

		switch (event_id)
		{
		case WIFI_PROV_START:
			ESP_LOGI(TAG, "Provisioning started");
			break;
		case WIFI_PROV_CRED_RECV:
		{
			wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
			ESP_LOGI(TAG, "Received Wi-Fi credentials"
						  "\n\tSSID     : %s\n\tPassword : %s",
					 (const char *)wifi_sta_cfg->ssid,
					 (const char *)wifi_sta_cfg->password);
			break;
		}
		case WIFI_PROV_CRED_FAIL:
		{
			wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
			ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
						  "\n\tPlease reset to factory and retry provisioning",
					 (*reason == WIFI_PROV_STA_AUTH_ERROR) ? "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");

			++retries;
			if (retries >= 10)
			{
				ESP_LOGI(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
				wifi_prov_mgr_reset_sm_state_on_failure();
				retries = 0;
			}

			break;
		}
		case WIFI_PROV_CRED_SUCCESS:
			ESP_LOGI(TAG, "Provisioning successful");
			retries = 0;
			break;
		case WIFI_PROV_END:
			ESP_LOGI(TAG, "Provisioning ended");
			break;
		default:
			break;
		}
	}

	static void event_handler_protocomm(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
	{
		assert(event_base == PROTOCOMM_SECURITY_SESSION_EVENT);

		switch (event_id)
		{
		case PROTOCOMM_SECURITY_SESSION_SETUP_OK:
			ESP_LOGI(TAG, "Secured session established!");
			break;
		case PROTOCOMM_SECURITY_SESSION_INVALID_SECURITY_PARAMS:
			ESP_LOGE(TAG, "Received invalid security parameters for establishing secure session!");
			break;
		case PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH:
			ESP_LOGE(TAG, "Received incorrect username and/or PoP for establishing secure session!");
			break;
		default:
			break;
		}
	}

	//----------------//
	//    HELPERS     //
	//----------------//

	namespace nvs
	{
		esp_err_t init()
		{
			esp_err_t ret = nvs_flash_init();

			if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
			{
				ESP_RETURN_ON_ERROR(
					nvs_flash_erase(),
					TAG, "Failed to nvs_flash_erase!");

				ESP_RETURN_ON_ERROR(
					nvs_flash_init(),
					TAG, "Failed to nvs_flash_init!");
			}
			else if (ret != ESP_OK)
			{
				ESP_LOGE(TAG, "Failed to nvs_flash_init: %s!", esp_err_to_name(ret));
				return ret;
			}

			return ESP_OK;
		}
	}

	namespace netif
	{
		esp_err_t init()
		{
			ESP_RETURN_ON_ERROR(
				esp_netif_init(),
				TAG, "Failed to esp_netif_init!");

			return ESP_OK;
		}
	}

	namespace event
	{
		esp_err_t init()
		{
			ESP_RETURN_ON_ERROR(
				esp_event_loop_create_default(),
				TAG, "Failed to esp_event_loop_create_default!");

			// ESP_ERROR_CHECK();
			ip_event_group = xEventGroupCreate();

			ESP_RETURN_ON_ERROR(
				esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler_wifi, NULL),
				TAG, "Failed to esp_event_handler_register!");

			ESP_RETURN_ON_ERROR(
				esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler_ip, NULL),
				TAG, "Failed to esp_event_handler_register!");

			ESP_RETURN_ON_ERROR(
				esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler_provisioning, NULL),
				TAG, "Failed to esp_event_handler_register!");

			ESP_RETURN_ON_ERROR(
				esp_event_handler_register(PROTOCOMM_SECURITY_SESSION_EVENT, ESP_EVENT_ANY_ID, &event_handler_protocomm, NULL),
				TAG, "Failed to esp_event_handler_register!");

			return ESP_OK;
		}

		esp_err_t deinit()
		{
			return ESP_OK;
		}

	}

	namespace wifi
	{
		esp_err_t init()
		{
			esp_netif_create_default_wifi_sta();
			esp_netif_create_default_wifi_ap();

			wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

			ESP_RETURN_ON_ERROR(
				esp_wifi_init(&cfg),
				TAG, "Failed to esp_wifi_init!");

			return ESP_OK;
		}

		esp_err_t init_sta()
		{
			ESP_RETURN_ON_ERROR(
				esp_wifi_set_mode(WIFI_MODE_STA),
				TAG, "Failed to esp_wifi_set_mode!");

			ESP_RETURN_ON_ERROR(
				esp_wifi_start(),
				TAG, "Failed to esp_wifi_start!");

			return ESP_OK;
		}
	}

	//----------------//
	//    FRONTEND    //
	//----------------//

	esp_err_t init()
	{
		ESP_RETURN_ON_ERROR(
			nvs::init(),
			TAG, "Failed to nvs::init!");

		ESP_RETURN_ON_ERROR(
			netif::init(),
			TAG, "Failed to netif::init!");

		ESP_RETURN_ON_ERROR(
			event::init(),
			TAG, "Failed to event::init!");

		ESP_RETURN_ON_ERROR(
			wifi::init(),
			TAG, "Failed to wifi::init!");

		return ESP_OK;
	}

	esp_err_t deinit()
	{
		return ESP_OK;
	}

	//

	bool check_has_ip()
	{
		EventBits_t mask = has_ip_from_sta | has_ip_from_eth;
		return xEventGroupWaitBits(ip_event_group, mask, pdFALSE, pdFALSE, 0);
	}

	void wait_has_ip()
	{
		EventBits_t mask = has_ip_from_sta | has_ip_from_eth;
		EventBits_t ret = 0;
		do
		{
			ret = xEventGroupWaitBits(ip_event_group, mask, pdFALSE, pdFALSE, portMAX_DELAY);
		} //
		while (!ret);
	}

};
