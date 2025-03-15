#include "Frontend.h"

// #include <limits>
// #include <cmath>
// #include <atomic>
// #include <mutex>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>

#include "BME280.h"

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

		// DATA STORES
		std::array<char, 6> temperature;
		std::array<char, 6> pressure;
		std::array<char, 6> humidity;
		std::array<char, 6> hms;
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
