#ifndef VFD_H
#define VFD_H

#include <array>

#include <esp_log.h>
#include <esp_check.h>

//

class VFD
{
	static constexpr const char *const TAG = "VFD";

public:
	static constexpr size_t anodes = 16;
	static constexpr size_t grids = 16;

	std::array<uint16_t, 16> matrix = {};

public:
	VFD() = default;
	~VFD() = default;

	esp_err_t init()
	{
		return ESP_OK;
	}

	esp_err_t deinit()
	{
		return ESP_OK;
	}

	//

public:
	enum Grids
	{
		SYMBOLS = 0,
		DIGIT1_7,
		DIGIT2_7,
		DIGIT3_9,
		DIGIT4_7_S,
		DIGIT5_14,
		DIGIT6_14,
		DIGIT7_14,
		DIGIT8_14_D,
		DIGIT9_14,
		DIGIT10_14,
		TABLE,
		BAR1_1_5,
		BAR2_6_10,
		BAR3_11_15,
		BAR4_16_20_A,
	};

	void set_grid(Grids g, uint16_t pattern)
	{
		matrix[static_cast<size_t>(g)] = pattern;
	}

private:
};

#endif
