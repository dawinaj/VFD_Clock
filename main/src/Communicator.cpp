#include "Communicator.h"

namespace Communicator
{
	namespace
	{
		std::atomic_bool please_exit;
		std::atomic_bool producer_running;

		// SETTINGS
		Electricity limits;
		Electricity operating;
	}

	//================================//
	//         IMPLEMENTATION         //
	//================================//

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

	esp_err_t set_limits(Electricity &e)
	{
		limits = e;
		return ESP_OK;
	}

	esp_err_t set_operating(Electricity &e)
	{
		operating = e;
		return ESP_OK;
	}

	//

};
