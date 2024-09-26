#pragma once
#include "COMMON.h"

#include <atomic>
#include <utility>

namespace Communicator
{
	constexpr const char *const TAG = "Communicator";

	//
	esp_err_t cleanup();
	esp_err_t init();
	esp_err_t deinit();

	bool is_running();
	bool has_data();

	void start_running();
	void ask_to_exit();

	bool should_exit();
	void confirm_exit();

};
