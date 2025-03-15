#include "Communicator.h"

namespace Communicator
{
	namespace
	{
		std::atomic_bool please_exit;
		std::atomic_bool producer_running;

		VFD vfd;
	}

	//================================//
	//         IMPLEMENTATION         //
	//================================//

	//----------------//
	//    FRONTEND    //
	//----------------//

	VFD &get_vfd()
	{
		return vfd;
	}

	void request_exit()
	{
		please_exit = true;
	}

	bool should_exit()
	{
		return please_exit;
	}

	void confirm_start()
	{
		producer_running = true;
	}
	void confirm_exit()
	{
		producer_running = false;
		please_exit = false;
	}

	//

	esp_err_t cleanup()
	{
		return ESP_OK;
	}

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

	//

};
