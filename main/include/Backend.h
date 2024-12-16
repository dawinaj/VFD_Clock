#pragma once
#include "COMMON.h"

// #include <atomic>
// #include <limits>
// #include <cmath>
// #include <functional>

//

#define BACKEND_MEM (8 * 1024)
#define BACKEND_PRT (configMAX_PRIORITIES - 1)

//

namespace Backend
{
	constexpr const char *const TAG = "Backend";

	constexpr float u_ref = 4.096;

	constexpr float ItoU_input = 1000.0f / 40.0f; // 40mV / A => 1000mA / 40mV => 25A/V

	constexpr float volt_div = 10;

	esp_err_t init();
	esp_err_t deinit();

};
