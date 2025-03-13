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

#include "Communicator.h"

namespace Frontend
{
	namespace
	{
		// SPECS

		// HARDWARE
		constexpr spi_host_device_t spi_host = SPI3_HOST;

		BME280_SPI bme(spi_host, GPIO_NUM_2);

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

	//----------------//
	//    FRONTEND    //
	//----------------//

	esp_err_t cleanup()
	{
		return ESP_OK;
	}

	esp_err_t init()
	{
		return ESP_OK;
	}

	esp_err_t deinit()
	{
		return ESP_OK;
	}

};
