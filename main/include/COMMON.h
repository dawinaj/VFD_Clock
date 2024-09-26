#pragma once

// SETTINGS

// CONSTANTS

#define CPU0 (0)
#define CPU1 (1)

// PERMANENT INCLUDES

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>

// TYPEDEFS
// #include <esp_system.h>
// #define LOG_MEMORY_MAX ESP_LOGD(TAG, "Max memory: %i", uxTaskGetStackHighWaterMark(NULL))

template <typename T>
T bound(const T &val, const T &mn, const T &mx)
{
	if (val < mn)
		return mn;
	if (val > mx)
		return mx;
	return val;
}