#include "Backend.h"

#include <array>
#include <limits>
#include <cmath>
#include <atomic>
#include <mutex>

// #include <esp_timer.h>
// #include <soc/gpio_reg.h>
#include <driver/gptimer.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/spi_master.h>
#include <driver/i2c_master.h>
#include <driver/gpio.h>

#include "Communicator.h"

#include "MCP230XX.h"

namespace Backend
{
	namespace
	{
		constexpr uint32_t TIMER_HZ = 1'000'000;
		constexpr uint32_t CTRL_LOOP_TICKS = 1'000;
		constexpr float SAMPLING_S = float(CTRL_LOOP_TICKS) / TIMER_HZ; // us to s

		constexpr uint32_t i2c_chz = 800'000;

		// HARDWARE
		constexpr spi_host_device_t spi_host = SPI3_HOST;
		i2c_master_bus_handle_t i2c_hdl;

		MCP23017 expander_grids(i2c_hdl, 0b000, i2c_chz);
		MCP23017 expander_anodes(i2c_hdl, 0b100, i2c_chz);

		constexpr gpio_num_t filament_pin = GPIO_NUM_17;

		//================================//
		//            HELPERS             //
		//================================//

		// SOFTWARE SETUP
		TaskHandle_t ctrlloop_task = nullptr;

		gptimer_handle_t sync_timer = nullptr;

		// ADC/DAC TRANSACTIONS
		// std::array<spi_transaction_t, 4> trx_adc;

		// CONSTANTS

	}

	//================================//
	//          DECLARATIONS          //
	//================================//

	//================================//
	//         IMPLEMENTATION         //
	//================================//

	//----------------//
	//    HELPERS     //
	//----------------//

	static esp_err_t vfd_for_for()
	{
		static size_t grid = 0;
		static size_t anode = 0;

		ESP_RETURN_ON_ERROR(
			expander_grids.set_pins(BIT(grid)),
			TAG, "Failed to expander_grids.set_pins!");

		ESP_RETURN_ON_ERROR(
			expander_anodes.set_pins(BIT(anode)),
			TAG, "Failed to expander_anodes.set_pins!");

		if (++anode == 16)
		{
			anode = 0;
			if (++grid == 16)
				grid = 0;
		}

		return ESP_OK;
	}

	static esp_err_t vfd_for_all()
	{
		static size_t grid = 0;

		ESP_RETURN_ON_ERROR(
			expander_grids.set_pins(0),
			TAG, "Failed to expander_grids.set_pins!");

		ESP_RETURN_ON_ERROR(
			expander_anodes.set_pins(-1),
			TAG, "Failed to expander_anodes.set_pins!");

		ESP_RETURN_ON_ERROR(
			expander_grids.set_pins(BIT(grid)),
			TAG, "Failed to expander_grids.set_pins!");

		++grid;
		grid &= 0b1111;

		return ESP_OK;
	}

	static esp_err_t vfd_all_all()
	{
		ESP_RETURN_ON_ERROR(
			expander_grids.set_pins(-1),
			TAG, "Failed to expander_grids.set_pins!");

		ESP_RETURN_ON_ERROR(
			expander_anodes.set_pins(-1),
			TAG, "Failed to expander_anodes.set_pins!");

		return ESP_OK;
	}

	static esp_err_t vfd_none()
	{
		ESP_RETURN_ON_ERROR(
			expander_grids.set_pins(0),
			TAG, "Failed to expander_grids.set_pins!");

		ESP_RETURN_ON_ERROR(
			expander_anodes.set_pins(0),
			TAG, "Failed to expander_anodes.set_pins!");

		return ESP_OK;
	}

	//----------------//
	//    BACKEND     //
	//----------------//

	static esp_err_t init_gptimer();

	// INTERRUPT

	static IRAM_ATTR bool sync_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
	{
		BaseType_t high_task_awoken = pdFALSE;

		xTaskNotifyFromISR(ctrlloop_task, 1, eIncrement, &high_task_awoken);

		return high_task_awoken == pdTRUE;
	}

	// EXECUTABLE

	static void controlloop_task(void *arg)
	{
		__attribute__((unused)) esp_err_t ret; // used in on_false macros
		uint32_t cycles = 0;

		ESP_LOGI(TAG, "Starting the Control loop...");

		ESP_GOTO_ON_ERROR(
			init_gptimer(),
			label_fail, TAG, "Failed to init_gptimer!");

		ESP_GOTO_ON_ERROR(
			gptimer_start(sync_timer),
			label_fail, TAG, "Failed to gptimer_start!");

		// vTaskDelay(pdMS_TO_TICKS(500));

		filament_state(true);

		while (1)
		{
			cycles = ulTaskNotifyTake(pdTRUE, 0);
			while (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 0)
				;
			++cycles;
			// float DT = cycles * SAMPLING_S;

			// if (DT < 1.0f)
			vfd_for_all();
			// else
			// vfd_none();

			// if (DT > 2.0f)
			// cycles = 0;
		}

	label_fail:

		gptimer_stop(sync_timer);

		ESP_LOGI(TAG, "Exiting...");
		// Communicator::confirm_exit();

		ctrlloop_task = nullptr;
		vTaskDelete(ctrlloop_task);
		// *dies*
	}

	//----------------//
	//    HELPERS     //
	//----------------//

	static esp_err_t init_gpio()
	{
		ESP_RETURN_ON_ERROR(
			gpio_set_direction(filament_pin, GPIO_MODE_OUTPUT),
			TAG, "Failed to gpio_set_direction!");

		ESP_RETURN_ON_ERROR(
			gpio_set_level(filament_pin, 1),
			TAG, "Failed to gpio_set_level!");

		return ESP_OK;
	}
	static esp_err_t deinit_gpio()
	{
		ESP_RETURN_ON_ERROR(
			gpio_reset_pin(filament_pin),
			TAG, "Failed to gpio_reset_pin!");

		return ESP_OK;
	}

	static esp_err_t init_i2c()
	{
		i2c_master_bus_config_t bus_cfg = {
			.i2c_port = -1,
			.sda_io_num = GPIO_NUM_21,
			.scl_io_num = GPIO_NUM_22,
			.clk_source = I2C_CLK_SRC_DEFAULT,
			.glitch_ignore_cnt = 7,
			.intr_priority = 0,
			.trans_queue_depth = 0,
			.flags = {
				.enable_internal_pullup = false,
			},
		};

		ESP_RETURN_ON_ERROR(
			i2c_new_master_bus(&bus_cfg, &i2c_hdl),
			TAG, "Failed to i2c_new_master_bus!");

		return ESP_OK;
	}
	static esp_err_t deinit_i2c()
	{
		ESP_RETURN_ON_ERROR(
			i2c_del_master_bus(i2c_hdl),
			TAG, "Failed to i2c_del_master_bus!");

		i2c_hdl = nullptr;

		return ESP_OK;
	}

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

	static esp_err_t init_gptimer()
	{
		const gptimer_config_t timer_cfg = {
			.clk_src = GPTIMER_CLK_SRC_DEFAULT,
			.direction = GPTIMER_COUNT_UP,
			.resolution_hz = TIMER_HZ, // 1MHz, 1 tick = 1us
			.intr_priority = 3,		   // 0 auto default
			.flags = {
				.intr_shared = false,
			},
		};

		const gptimer_alarm_config_t alarm_cfg = {
			.alarm_count = CTRL_LOOP_TICKS,
			.reload_count = 0,
			.flags = {
				.auto_reload_on_alarm = true,
			},
		};

		const gptimer_event_callbacks_t evt_cb_cfg = {
			.on_alarm = sync_callback,
		};

		ESP_RETURN_ON_ERROR(
			gptimer_new_timer(&timer_cfg, &sync_timer),
			TAG, "Failed to gptimer_new_timer!");

		ESP_RETURN_ON_ERROR(
			gptimer_set_alarm_action(sync_timer, &alarm_cfg),
			TAG, "Failed to gptimer_set_alarm_action!");

		ESP_RETURN_ON_ERROR(
			gptimer_register_event_callbacks(sync_timer, &evt_cb_cfg, nullptr),
			TAG, "Failed to gptimer_register_event_callbacks!");

		ESP_RETURN_ON_ERROR(
			gptimer_enable(sync_timer),
			TAG, "Failed to gptimer_enable!");

		return ESP_OK;
	}
	static esp_err_t deinit_gptimer()
	{
		ESP_RETURN_ON_ERROR(
			gptimer_disable(sync_timer),
			TAG, "Failed to gptimer_disable!");

		ESP_RETURN_ON_ERROR(
			gptimer_del_timer(sync_timer),
			TAG, "Failed to gptimer_del_timer!");

		sync_timer = nullptr;

		return ESP_OK;
	}

	static esp_err_t init_task()
	{
		ESP_RETURN_ON_FALSE(
			xTaskCreatePinnedToCore(controlloop_task, "ControlLoop", BACKEND_MEM, nullptr, BACKEND_PRT, &ctrlloop_task, CPU1),
			ESP_ERR_NO_MEM, TAG, "Failed to xTaskCreatePinnedToCore!");

		return ESP_OK;
	}
	static esp_err_t deinit_task()
	{
		vTaskDelete(ctrlloop_task); // void
		ctrlloop_task = nullptr;

		return ESP_OK;
	}

	static esp_err_t init_expanders()
	{
		ESP_RETURN_ON_ERROR(
			expander_grids.init(),
			TAG, "Failed to expander_grids.init!");

		ESP_RETURN_ON_ERROR(
			expander_anodes.init(),
			TAG, "Failed to expander_anodes.init!");

		ESP_RETURN_ON_ERROR(
			expander_grids.set_direction(0x0000),
			TAG, "Failed to expander_grids.set_direction!");

		ESP_RETURN_ON_ERROR(
			expander_anodes.set_direction(0x0000),
			TAG, "Failed to expander_anodes.set_direction!");

		return ESP_OK;
	}
	static esp_err_t deinit_expanders()
	{
		ESP_RETURN_ON_ERROR(
			expander_grids.deinit(),
			TAG, "Failed to expander_grids.deinit!");

		ESP_RETURN_ON_ERROR(
			expander_anodes.deinit(),
			TAG, "Failed to expander_anodes.deinit!");

		return ESP_OK;

		return ESP_OK;
	}

	//----------------//
	//    FRONTEND    //
	//----------------//

	esp_err_t filament_state(bool on)
	{
		ESP_RETURN_ON_ERROR(
			gpio_set_level(filament_pin, on),
			TAG, "Failed to gpio_set_level!");

		return ESP_OK;
	}

	esp_err_t init()
	{
		ESP_LOGI(TAG, "Initing Board...");

		init_gpio();

		init_i2c();
		init_spi();

		init_expanders();

		// SPAWN GPTIMER
		// init_gptimer();

		vTaskDelay(pdMS_TO_TICKS(500));

		// SPAWN EXE TASK
		init_task();

		ESP_LOGI(TAG, "Done!");
		return ESP_OK;
	}

	esp_err_t deinit()
	{
		ESP_LOGI(TAG, "Deiniting Board...");

		// KILL EXE TASK
		deinit_task();

		// KILL GPTIMER
		// deinit_gptimer();

		deinit_expanders();

		deinit_spi();
		deinit_i2c();

		deinit_gpio();

		ESP_LOGI(TAG, "Done!");
		return ESP_OK;
	}

	// INTERFACE
}
