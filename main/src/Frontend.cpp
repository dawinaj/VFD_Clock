#include "Frontend.h"

// #include <limits>
// #include <cmath>
// #include <atomic>
// #include <mutex>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// #include <driver/gpio.h>

#include "HD44780/HD44780.h"
#include "HD44780/PCF8574.h"

#include "Communicator.h"

namespace Frontend
{
	namespace
	{
		// SPECS

		// HARDWARE

		i2c_master_bus_handle_t i2c_hdl;

		HD44780_PCF8574 lcd_comm(i2c_hdl, 0b111, 400000);
		HD44780 lcd_disp(&lcd_comm, {4, 20}, LCD_2LINE);

	}

	//================================//
	//         IMPLEMENTATION         //
	//================================//

	//----------------//
	//    BACKEND     //
	//----------------//

	static esp_err_t init_i2c()
	{
		i2c_master_bus_config_t i2c_cfg = {
			.i2c_port = -1,
			.sda_io_num = GPIO_NUM_21,
			.scl_io_num = GPIO_NUM_22,
			.clk_source = I2C_CLK_SRC_DEFAULT,
			.glitch_ignore_cnt = 7,
			.intr_priority = 0,
			.trans_queue_depth = 0,
			.flags = {
				.enable_internal_pullup = true,
			},
		};

		ESP_RETURN_ON_ERROR(
			i2c_new_master_bus(&i2c_cfg, &i2c_hdl),
			TAG, "Failed to i2c_new_master_bus!");

		return ESP_OK;
	}
	static esp_err_t deinit_i2c()
	{
		ESP_RETURN_ON_ERROR(
			i2c_del_master_bus(i2c_hdl),
			TAG, "Failed to i2c_del_master_bus!");

		return ESP_OK;
	}

	static esp_err_t init_lcd()
	{
		ESP_RETURN_ON_ERROR(
			lcd_comm.init(),
			TAG, "Failed to lcd_comm.init!");

		ESP_RETURN_ON_ERROR(
			lcd_disp.init(),
			TAG, "Failed to lcd_disp.init!");

		ESP_RETURN_ON_ERROR(
			lcd_disp.backlight(true),
			TAG, "Failed to lcd_disp.backlight!");

		return ESP_OK;
	}
	static esp_err_t deinit_lcd()
	{
		ESP_RETURN_ON_ERROR(
			lcd_disp.deinit(),
			TAG, "Failed to lcd_disp.deinit!");

		ESP_RETURN_ON_ERROR(
			lcd_comm.deinit(),
			TAG, "Failed to lcd_comm.deinit!");

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
			init_i2c(),
			TAG, "Failed to init_i2c!");

		ESP_RETURN_ON_ERROR(
			init_lcd(),
			TAG, "Failed to init_lcd!");

		return ESP_OK;
	}

	esp_err_t deinit()
	{
		ESP_RETURN_ON_ERROR(
			deinit_lcd(),
			TAG, "Failed to deinit_lcd!");

		ESP_RETURN_ON_ERROR(
			deinit_i2c(),
			TAG, "Failed to deinit_i2c!");

		return ESP_OK;
	}

};
