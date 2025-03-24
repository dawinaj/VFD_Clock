#ifndef SmartTouch_H
#define SmartTouch_H

// #include <cmath>

#include <atomic>
#include <algorithm>
#include <array>
#include <vector>

#include <esp_log.h>
#include <esp_check.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/touch_pad.h>

class SmartTouch
{
	static constexpr const char *const TAG = "SmartTouch";

private:
	static std::atomic<size_t> InstanceCount;

private:
	static constexpr float ALPHA = 0.01f;	   // Smoothing factor
	static constexpr size_t NUM_SAMPLES = 100; // Buffer size for init mean/std calculation

	// TOUCH_PAD_MAX
	std::array<float, TOUCH_PAD_MAX> mean = {0};
	// std::array<float, N> variance = {0};

	const std::vector<touch_pad_t> touch_pins;

	float min_thresh;
	float max_thresh;
	float neg_thresh;

public:
	std::array<float, TOUCH_PAD_MAX> touch_analog;

	uint16_t touch_on = 0;
	uint16_t touch_pe = 0;
	uint16_t touch_ne = 0;

public:
	SmartTouch(const std::vector<touch_pad_t> &pins, float mt = 3, float Mt = 10, float nt = 0.5) : touch_pins(pins), min_thresh(mt / 100), max_thresh(Mt / 100), neg_thresh(nt / 100)
	{
	}

	~SmartTouch()
	{
	}

public:
	esp_err_t init()
	{
		// ESP_RETURN_ON_ERROR(
		// 	Init(),
		// 	TAG, "Failed to Init!");

		for (touch_pad_t pin : touch_pins)
			ESP_RETURN_ON_ERROR(
				touch_pad_config(pin, 0),
				TAG, "Failed to touch_pad_config!");

		// touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
		// touch_pad_filter_start(10);

		vTaskDelay(pdMS_TO_TICKS(500)); // Allow stabilization

		// Initial sampling
		for (touch_pad_t pin : touch_pins)
		{
			for (size_t j = 0; j < NUM_SAMPLES; j++)
			{
				uint16_t sample = 0;
				ESP_RETURN_ON_ERROR(
					touch_pad_read(pin, &sample),
					TAG, "Failed to touch_pad_read!");

				update_stats(pin, sample);
			}
		}

		return ESP_OK;
	}

	esp_err_t deinit()
	{
		// ESP_RETURN_ON_ERROR(
		// 	Deinit(),
		// 	TAG, "Failed to Deinit!");

		return ESP_OK;
	}

	esp_err_t test_pins()
	{
		uint16_t touch_new = 0;

		for (touch_pad_t pin : touch_pins)
		{
			const float mt = mean[pin] * (1 - min_thresh);
			const float Mt = mean[pin] * (0 + max_thresh);
			const float nt = mean[pin] * (1 + neg_thresh);
			const float range = mt - Mt;

			uint16_t sample = 0;

			ESP_RETURN_ON_ERROR(
				touch_pad_read(pin, &sample),
				TAG, "Failed to touch_pad_read!");

			float meas = sample;
			meas = std::clamp(meas, Mt, mt);
			float analog = 1 - (meas - Mt) / range;

			ESP_LOGI(TAG, "Mean: %f, New: %hu, clamp: %f, anal: %f", mean[pin], sample, meas, analog);

			touch_analog[pin] = analog;
			if (analog >= 0.5) // Check if touch detected //
				touch_new |= BIT(pin);

			if (sample > mt && sample < nt) // Check if in permissible range //
				update_stats(pin, sample);
		}

		touch_pe = touch_new & ~touch_on;
		touch_ne = ~touch_new & touch_on;

		touch_on = touch_new;
		return ESP_OK;
	}

private:
	void update_stats(touch_pad_t pin, uint16_t sample)
	{
		if (mean[pin] == 0) [[unlikely]]
			mean[pin] = sample;

		float delta = sample - mean[pin];
		mean[pin] += ALPHA * delta; // Smoothed mean update
	}

	// static
public:
	static esp_err_t Init()
	{
		if (InstanceCount++ == 0)
		{
			ESP_LOGI(TAG, "Initializing touch sensors...");

			ESP_RETURN_ON_ERROR(
				touch_pad_init(),
				TAG, "Failed to touch_pad_init!");

			touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
		}

		return ESP_OK;
	}

	static esp_err_t Deinit()
	{
		--InstanceCount;

		if (InstanceCount == 0)
		{
			ESP_LOGI(TAG, "Deinitializing touch sensors...");

			ESP_RETURN_ON_ERROR(
				touch_pad_deinit(),
				TAG, "Failed to touch_pad_deinit!");
		}

		return ESP_OK;
	}
};

#endif
