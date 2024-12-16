#include "COMMON.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <driver/i2c_master.h>

#include "Communicator.h"
#include "Frontend.h"
#include "Backend.h"

static const char *TAG = "[" __TIME__ "]🅱 rzedwornidza";

extern "C" void app_main(void)
{
	ESP_LOGI(TAG, "H E N L O B E N C, Wait");

	esp_log_level_set("*", ESP_LOG_VERBOSE);

	vTaskDelay(pdMS_TO_TICKS(1000));

	Communicator::init();
	Frontend::init();
	Backend::init();

	// setup_lcd();

	// lcd_write_str(&lcd_handle, "Start... ");

	// for turning PSU ON
	// gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
	// gpio_set_pull_mode(GPIO_NUM_32, GPIO_PULLUP_ONLY);
	// gpio_set_level(GPIO_NUM_32, 1);
	// for reading if PSU OK
	// gpio_set_direction(GPIO_NUM_35, GPIO_MODE_INPUT);

	// lcd_write_str(&lcd_handle, "ustawiony... ");

	vTaskDelay(pdMS_TO_TICKS(10000));

	// gpio_set_level(GPIO_NUM_32, 0);

	// lcd_write_str(&lcd_handle, "odpalony...");

	// while (!gpio_get_level(GPIO_NUM_35))
	// vTaskDelay(pdMS_TO_TICKS(10));

	// lcd_write_str(&lcd_handle, "gotowy.");

	// driver.set_ratio(1);

	while (true)
		vTaskDelay(pdMS_TO_TICKS(1000));

	// what the heck you stupid formatter stop stealing my newline faggot
}
