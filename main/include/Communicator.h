#pragma once
#include "COMMON.h"

#include <atomic>
#include <utility>


namespace Communicator
{
	constexpr const char *const TAG = "Communicator";

	//
	esp_err_t init();
	esp_err_t deinit();

};
