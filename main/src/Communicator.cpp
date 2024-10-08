#include "Communicator.h"

#include "HD44780/HD44780.h"
#include "HD44780/PCF8574.h"

namespace Communicator
{
	namespace
	{
		std::atomic_bool please_exit;
		std::atomic_bool producer_running;

		// SETTINGS
		size_t time_bytes = 0;
		size_t res_wrt_4b = 0;
	}

	//================================//
	//         IMPLEMENTATION         //
	//================================//

	//----------------//
	//    FRONTEND    //
	//----------------//

	esp_err_t init()
	{
		ESP_RETURN_ON_ERROR(
			cleanup(),
			TAG, "Failed to cleanup!");

		return ESP_OK;
	}

	esp_err_t deinit()
	{
		ESP_RETURN_ON_ERROR(
			cleanup(),
			TAG, "Failed to cleanup!");

		return ESP_OK;
	}

	esp_err_t time_settings(size_t b)
	{
		time_bytes = std::min<size_t>(b, 8);
		res_wrt_4b = time_bytes + 4;
		return ESP_OK;
	}

	//

	bool is_running()
	{
		return producer_running.load(std::memory_order::relaxed);
	}

	void start_running()
	{
		producer_running.store(true, std::memory_order::relaxed);
		producer_running.notify_one();
	}

	// void ask_to_exit()
	// {
	// 	please_exit.store(true, std::memory_order::relaxed);
	// 	Board::give_sem_emergency();
	// }

	bool should_exit()
	{
		return please_exit.load(std::memory_order::relaxed);
	}

	void confirm_exit()
	{
		producer_running.store(false, std::memory_order_relaxed);
		producer_running.notify_one();
	}
};
