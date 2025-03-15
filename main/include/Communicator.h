#pragma once
#include "COMMON.h"

#include <atomic>
#include <utility>

#include "VFD.h"

namespace Communicator
{
	constexpr const char *const TAG = "Communicator";

	VFD &get_vfd();

	void request_exit();
	bool should_exit();
	void confirm_start();
	void confirm_exit();

	esp_err_t init();
	esp_err_t deinit();

};
