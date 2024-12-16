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
#include <driver/gpio.h>

#include "Communicator.h"

#include "Converter.h"

#include "MCP3x0x.h"

namespace Backend
{
	namespace
	{
		constexpr uint32_t slowdown = 1000;
		constexpr uint32_t dtsd = 10;
		// SPECSs
		constexpr int MCPWM_GRP = 0;
		constexpr uint32_t CLK_FRQ = 160'000'000 / slowdown; // 80MHz max?
		constexpr uint32_t PWM_FRQ = 100'000 / slowdown;	 // somewhere around 30k-300k, affects duty precision, response, losses, etc
		constexpr uint32_t DTNS = 100 * slowdown * dtsd;	 // dead time in ns

		constexpr uint32_t CTRL_US = 100;
		constexpr float SAMPLING_S = CTRL_US / 1e6; // us to s

		// HARDWARE
		constexpr spi_host_device_t spi_host = SPI3_HOST;

		McpwmTimer mcpwm_timer(MCPWM_GRP, CLK_FRQ, CLK_FRQ / PWM_FRQ);
		BuckConverter buck_conv(mcpwm_timer, GPIO_NUM_25, GPIO_NUM_26, DTNS);
		BoostConverter boost_conv(mcpwm_timer, GPIO_NUM_32, GPIO_NUM_33, DTNS);
		BuckBoostConverter conv(buck_conv, boost_conv);

		MCP3204 adc(spi_host, GPIO_NUM_5, 2'000'000);

		//================================//
		//            HELPERS             //
		//================================//

		// SOFTWARE SETUP
		TaskHandle_t ctrlloop_task = nullptr;

		gptimer_handle_t sync_timer = nullptr;

		// ADC/DAC TRANSACTIONS
		// std::array<spi_transaction_t, 4> trx_adc;

		// CONSTANTS

		// I/O conversion
		constexpr int32_t halfrangein = adc.ref / 2;
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

	static inline constexpr int32_t adc_offset(MCP3204::out_t val)
	{
		return static_cast<int32_t>(val) - halfrangein;
	}

	// template <typename num_t, int32_t mul = 1>
	// static num_t bin_to_phy(int32_t sum, int32_t cnt)
	// {
	// 	constexpr num_t ratio = u_ref * mul / halfrangein;
	//
	// 	size_t inidx = static_cast<size_t>(in) - 1;
	// 	size_t rngidx = static_cast<size_t>(an_in_range[inidx]);
	// 	switch (in)
	// 	{
	// 	case Input::In1:
	// 	case Input::In2:
	// 	case Input::In3:
	// 		return sum * ratio * volt_divs[rngidx] / cnt;
	// 	case Input::In4:
	// 		return sum * ratio / curr_gains[rngidx] / cnt * ItoU_input;
	// 	default:
	// 		return 0;
	// 	}
	// }

	//----------------//
	//    BACKEND     //
	//----------------//

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

		int d = 0;

		ESP_LOGI(TAG, "Starting the Control loop...");

		mcpwm_timer.start();

		ESP_GOTO_ON_ERROR(
			gptimer_start(sync_timer),
			label_fail, TAG, "Failed to gptimer_start!");

		vTaskDelay(pdMS_TO_TICKS(500));

		while (d <= 100)
		{
			uint32_t cycles = ulTaskNotifyTake(pdTRUE, 0);

			while (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 0)
				;

			++cycles;

			float DT = cycles * SAMPLING_S;

			conv.set_ratio(d++ * 0.01f);
			// boost_conv.set_duty(0.4);

			// ESP_LOGI(TAG, "Cycles: %lu, Time: %f", cycles, DT);

			vTaskDelay(pdMS_TO_TICKS(100));
		}

	label_fail:

		mcpwm_timer.stop();

		conv.force_freewheel();

		gptimer_stop(sync_timer);

		ESP_LOGI(TAG, "Exiting...");
		// Communicator::confirm_exit();

		ctrlloop_task = nullptr;
		vTaskDelete(ctrlloop_task);
	}

	//----------------//
	//    HELPERS     //
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

	static esp_err_t init_adc()
	{
		// ADC/DAC
		// for (size_t i = 0; i < an_in_num; ++i)
		// 	trx_adc[i] = MCP3204::make_trx(i);

		ESP_RETURN_ON_ERROR(
			adc.init(),
			TAG, "Failed to adc.init!");

		ESP_RETURN_ON_ERROR(
			adc.acquire_spi(),
			TAG, "Failed to adc.acquire_spi!");

		return ESP_OK;
	}
	static esp_err_t deinit_adc()
	{
		ESP_RETURN_ON_ERROR(
			adc.release_spi(),
			TAG, "Failed to adc.release_spi!");

		ESP_RETURN_ON_ERROR(
			adc.deinit(),
			TAG, "Failed to adc.deinit!");

		return ESP_OK;
	}

	static esp_err_t init_gptimer()
	{
		const gptimer_config_t timer_cfg = {
			.clk_src = GPTIMER_CLK_SRC_DEFAULT,
			.direction = GPTIMER_COUNT_UP,
			.resolution_hz = 1'000'000, // 1MHz, 1 tick = 1us
			.intr_priority = 0,			// auto default
			.flags = {
				.intr_shared = false,
			},
		};

		const gptimer_alarm_config_t alarm_cfg = {
			.alarm_count = CTRL_US,
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

	static esp_err_t init_bridge()
	{
		ESP_RETURN_ON_ERROR(
			mcpwm_timer.init(),
			TAG, "Failed to mcpwm_timer.init!");

		ESP_RETURN_ON_ERROR(
			buck_conv.init(),
			TAG, "Failed to buck_conv.init!");

		ESP_RETURN_ON_ERROR(
			boost_conv.init(),
			TAG, "Failed to boost_conv.init!");

		ESP_RETURN_ON_ERROR(
			conv.init(),
			TAG, "Failed to conv.init!");

		return ESP_OK;
	}
	static esp_err_t deinit_bridge()
	{
		ESP_RETURN_ON_ERROR(
			conv.deinit(),
			TAG, "Failed to conv.deinit!");

		ESP_RETURN_ON_ERROR(
			boost_conv.deinit(),
			TAG, "Failed to boost_conv.deinit!");

		ESP_RETURN_ON_ERROR(
			buck_conv.deinit(),
			TAG, "Failed to buck_conv.deinit!");

		ESP_RETURN_ON_ERROR(
			mcpwm_timer.deinit(),
			TAG, "Failed to mcpwm_timer.deinit!");

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

	//----------------//
	//    FRONTEND    //
	//----------------//

	esp_err_t init()
	{
		ESP_LOGI(TAG, "Initing Board...");

		// // GPIO
		// for (size_t i = 0; i < dg_in_num; ++i)
		// {
		// 	gpio_set_direction(dig_in[i], GPIO_MODE_INPUT);
		// 	gpio_pulldown_en(dig_in[i]);
		// }

		// for (size_t i = 0; i < dg_out_num; ++i)
		// 	gpio_set_direction(dig_out[i], GPIO_MODE_OUTPUT);

		// INIT ADC
		init_spi();
		init_adc();

		// SPAWN GPTIMER
		init_gptimer();

		// INIT BRIDGE
		init_bridge();

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

		// DEINIT BRIDGE
		deinit_bridge();

		// KILL GPTIMER
		deinit_gptimer();

		// DEINIT ADC
		deinit_adc();
		deinit_spi();

		ESP_LOGI(TAG, "Done!");
		return ESP_OK;
	}

	// INTERFACE
}
