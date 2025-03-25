#include "Frontend.h"

// #include <cmath>
#include <ctime>

// #include <limits>
// #include <atomic>
// #include <mutex>
#include <chrono>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>

#include "BME280.h"
#include "SmartTouch.h"
#include "SignalProcessing.h"

#include "Settings.h"
#include "Communicator.h"

namespace Frontend
{
	namespace
	{
		// SPECS

		// HARDWARE
		constexpr spi_host_device_t spi_host = SPI3_HOST;

		BME280_SPI bme280(spi_host, GPIO_NUM_2);

		SmartTouch st({TOUCH_PAD_NUM8});
		Hysteresis hys(0.4, 0.6);
		BoolLowpass bllp(5);

		// DATA STORES
		std::array<char, 6> buf_temperature;
		std::array<char, 6> buf_pressure;
		std::array<char, 6> buf_humidity;

		std::array<char, 4> buf_year;
		std::array<char, 2> buf_month;
		std::array<char, 2> buf_day;
		std::array<char, 2> buf_hour;
		std::array<char, 2> buf_minute;
		std::array<char, 2> buf_second;

	}

	//================================//
	//         IMPLEMENTATION         //
	//================================//

	//----------------//
	//    BACKEND     //
	//----------------//

	static esp_err_t init_spi()
	{
		spi_bus_config_t bus_cfg = {
			.mosi_io_num = GPIO_NUM_23,
			.miso_io_num = GPIO_NUM_19, // GPIO_NUM_19, GPIO_NUM_NC
			.sclk_io_num = GPIO_NUM_18,
			.quadwp_io_num = GPIO_NUM_NC,
			.quadhd_io_num = GPIO_NUM_NC,
			.data4_io_num = GPIO_NUM_NC,
			.data5_io_num = GPIO_NUM_NC,
			.data6_io_num = GPIO_NUM_NC,
			.data7_io_num = GPIO_NUM_NC,
			.max_transfer_sz = 0,
			.flags = SPICOMMON_BUSFLAG_MASTER,
			.isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
			.intr_flags = 0,
		};

		ESP_RETURN_ON_ERROR(
			spi_bus_initialize(spi_host, &bus_cfg, SPI_DMA_DISABLED),
			TAG, "Failed to spi_bus_initialize!");

		return ESP_OK;
	}
	static esp_err_t deinit_spi()
	{
		ESP_RETURN_ON_ERROR(
			spi_bus_free(spi_host),
			TAG, "Failed to spi_bus_free!");

		return ESP_OK;
	}

	static esp_err_t init_bme280()
	{
		ESP_RETURN_ON_ERROR(
			bme280.init(false),
			TAG, "Failed to bme280.init!");

		bme280.settings_filt(BME280_FILTER_4);
		bme280.settings_press(BME280_SAMPLES_4);
		bme280.settings_temp(BME280_SAMPLES_4);
		bme280.settings_hum(BME280_SAMPLES_4);
		bme280.settings_sby(BME280_TSBY_250MS);

		ESP_RETURN_ON_ERROR(
			bme280.apply_settings(),
			TAG, "Failed to bme280.apply_settings!");

		ESP_RETURN_ON_ERROR(
			bme280.continuous_mode(true),
			TAG, "Failed to bme280.continuous_mode!");

		return ESP_OK;
	}
	static esp_err_t deinit_bme280()
	{

		ESP_RETURN_ON_ERROR(
			bme280.deinit(),
			TAG, "Failed to bme280.deinit!");

		return ESP_OK;
	}

	void format_float_for_4_2(float value, char *buffer, size_t buflen)
	{
		// Convert to an integer representation (4 integer + 2 decimal places)
		int scaledValue = std::round(value * 100);

		// Ensure at least one leading zero and a total width of 6 characters
		if (scaledValue < 100)							   // If the number is less than 1
			snprintf(buffer, buflen, "% 6d", scaledValue); // Space padding for alignment
		else
			snprintf(buffer, buflen, "   0%02d", scaledValue); // Zero padding for small numbers
	}

	static void controlloop_task(void *arg)
	{
		__attribute__((unused)) esp_err_t ret; // used in on_false macros

		time_t now;
		struct tm timeinfo;
		char dt_buffer[64];

		ESP_LOGI(TAG, "Starting the Frontend loop...");

		// while (1)
		// {
		// 	time(&now);
		// 	localtime_r(&now, &timeinfo);

		// 	strftime(dt_buffer, sizeof(dt_buffer), "%c", &timeinfo); // Format the time string
		// 	ESP_LOGI(TAG, "Local date and time: %s", dt_buffer);
		// 	vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 second
		// }

		while (1)
		{
			const auto now = std::chrono::system_clock::now();
			std::time_t now_tt = std::chrono::system_clock::to_time_t(now);
			std::tm tm = *std::localtime(&now_tt);

			std::strftime(buf_year.data(), 6, "%Y", &tm);
			std::strftime(buf_month.data(), 6, "%m", &tm);
			std::strftime(buf_day.data(), 6, "%d", &tm);
			std::strftime(buf_hour.data(), 6, "%H", &tm);
			std::strftime(buf_minute.data(), 6, "%M", &tm);
			std::strftime(buf_second.data(), 6, "%S", &tm);

			ESP_LOGI(TAG, "Local date and time: %s", dt_buffer);
			vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 second
		}

		ESP_LOGI(TAG, "Exiting Frontend loop...");

		vTaskDelete(nullptr);
		// *dies*
	}

	//----------------//
	//    FRONTEND    //
	//----------------//

	esp_err_t cleanup()
	{
		return ESP_OK;
	}

	esp_err_t init()
	{
		ESP_RETURN_ON_ERROR(
			init_spi(),
			TAG, "Failed to init_spi!");

		ESP_RETURN_ON_ERROR(
			init_bme280(),
			TAG, "Failed to init_bme280!");

		return ESP_OK;
	}

	esp_err_t deinit()
	{
		ESP_RETURN_ON_ERROR(
			deinit_bme280(),
			TAG, "Failed to deinit_bme280!");

		ESP_RETURN_ON_ERROR(
			deinit_spi(),
			TAG, "Failed to init_spi!");

		return ESP_OK;
	}

};
