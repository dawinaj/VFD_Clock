#pragma once
#include "COMMON.h"

#include <atomic>
#include <utility>

#include "VFD.h"

namespace Communicator
{
	constexpr const char *const TAG = "Communicator";

	VFD &get_vfd();

	esp_err_t init();
	esp_err_t deinit();

};
