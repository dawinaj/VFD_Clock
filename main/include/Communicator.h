#pragma once
#include "COMMON.h"

#include <atomic>
#include <utility>

#include "BitMatrix.h"

namespace Communicator
{
	constexpr const char *const TAG = "Communicator";

	// BitMatrix<16, 16> elements;
	//
	esp_err_t init();
	esp_err_t deinit();

};
