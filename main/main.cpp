#include "COMMON.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Settings.h"
#include "Communicator.h"
#include "Frontend.h"
#include "Backend.h"

// #include <driver/gpio.h>
// #include <driver/spi_master.h>
// #include <driver/i2c_master.h>

#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_softap.h>

#include <esp_sntp.h>
#include <esp_netif_sntp.h>

#include <cstdlib>
#include <cstring>
#include <ctime>

static const char *TAG = "[" __TIME__ "]VFD_Clock";

//

#define PROV_QR_VERSION "v1"
#define PROV_TRANSPORT_SOFTAP "softap"
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html?data="

static void wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport)
{
	if (!name || !transport)
	{
		ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
		return;
	}
	char payload[150] = {0};
	if (pop)
	{
		snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\",\"pop\":\"%s\",\"transport\":\"%s\"}",
				 PROV_QR_VERSION, name, pop, transport);
	}
	else
	{
		snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\",\"transport\":\"%s\"}",
				 PROV_QR_VERSION, name, transport);
	}

	ESP_LOGI(TAG, "Goto: \n%s%s", QRCODE_BASE_URL, payload);
}

//

extern "C" void app_main(void)
{
	ESP_LOGI(TAG, "H E N L O B E N C, Wait");
	esp_log_level_set("*", ESP_LOG_VERBOSE);

	vTaskDelay(pdMS_TO_TICKS(1000));

	Settings::init();

	Communicator::init();
	Frontend::init();
	Backend::init();

	//*/
	{
		wifi_prov_mgr_config_t config = {
			.scheme = wifi_prov_scheme_softap,
			.scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE,
		};
		ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

		bool provisioned = false;
		ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

		if (!provisioned)
		{
			ESP_LOGI(TAG, "Starting provisioning");

			const char *service_name = "VFD_Clock_Prov";
			const char *pop = "VFDClock8867"; // abcd1234

			wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
			wifi_prov_security1_params_t *sec_params = pop;

			const char *username = NULL;
			const char *service_key = NULL;

#ifdef CONFIG_EXAMPLE_REPROVISIONING
			wifi_prov_mgr_disable_auto_stop(1000);
#endif

			ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *)sec_params, service_name, service_key));

			wifi_prov_mgr_wait();
			wifi_prov_mgr_deinit();

			wifi_prov_print_qr(service_name, username, pop, PROV_TRANSPORT_SOFTAP);
		}
		else
		{
			ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi STA");

			wifi_prov_mgr_deinit();

			Settings::wifi::init_sta();
		}
	}
	//*/

	/*/
	{
		Settings::wait_has_ip();

		sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);

		esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
		esp_netif_sntp_init(&config);

		if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK)
		{
			printf("Failed to update system time within 10s timeout");
		}
	}
	//*/

	setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
	tzset();

	// Print the local time
	time_t now;
	struct tm timeinfo;
	while (1)
	{
		time(&now);
		localtime_r(&now, &timeinfo);
		char buffer[64];
		strftime(buffer, sizeof(buffer), "%c", &timeinfo); // Format the time string
		ESP_LOGI(TAG, "Local date and time: %s", buffer);
		vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 second
	}

	// what the heck you stupid formatter stop stealing my newline faggot
}
