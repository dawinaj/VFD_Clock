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
