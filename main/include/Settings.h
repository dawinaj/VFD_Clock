#pragma once
#include "COMMON.h"

#include <esp_event.h>

// #include <esp_netif_types.h>

namespace Settings
{
	constexpr const char *const TAG = "Settings";

	esp_err_t init();
	esp_err_t deinit();

	esp_err_t wait_for_event(esp_event_base_t event_base, int32_t event_id, TickType_t delay = portMAX_DELAY);

	namespace wifi
	{
		esp_err_t init_sta();
	}

	bool check_has_ip();
	void wait_has_ip();

};
