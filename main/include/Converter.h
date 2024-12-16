#pragma once

#include "CTOR.h"

#include <utility>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/cdefs.h>
#include <esp_log.h>
#include <esp_check.h>
#include <soc/gpio_num.h>
#include <driver/mcpwm_prelude.h>

class McpwmTimer
{
	static constexpr const char *const TAG = "McpwmTimer";

	const int mcpwm_grp;
	const uint32_t freq_clk;
	const uint32_t period;

	mcpwm_timer_handle_t timer;

public:
	McpwmTimer(int, uint32_t, uint32_t);
	~McpwmTimer();

	esp_err_t init();
	esp_err_t deinit();

	esp_err_t start();
	esp_err_t stop();

	mcpwm_timer_handle_t get_timer();
	int get_mcpwm() const;

	uint32_t ns_to_ticks(uint32_t) const;
	uint32_t period_ticks() const;
};

//

class ConverterBase
{
protected:
	static constexpr const char *const TAG = "Converter";

private:
	enum class OutputMode : uint8_t
	{
		Undefined = 0,
		Normal,
		ForcePass,
		ForceFreewheel,
		ForceOFF,
	};

private:
	McpwmTimer &timer;
	const gpio_num_t gpio_d; // mosfet / master / Dominant
	const gpio_num_t gpio_r; // diode  / slave  / Recessive

	bool hasrecessive = false;
	OutputMode outputMode = OutputMode::Undefined;

	uint32_t deadtime_ticks = 0;
	uint32_t duty_ticks_d = 0;
	uint32_t duty_ticks_r = 0;

	mcpwm_oper_handle_t oper = nullptr;
	mcpwm_gen_handle_t gen_d = nullptr;
	mcpwm_gen_handle_t gen_r = nullptr;
	mcpwm_cmpr_handle_t cmpr_d = nullptr;
	mcpwm_cmpr_handle_t cmpr_r = nullptr;

	mcpwm_gen_handle_t &gen_h;
	mcpwm_gen_handle_t &gen_l;

public:
	ConverterBase(bool, McpwmTimer &, gpio_num_t, gpio_num_t, uint32_t = 0);
	~ConverterBase();

	esp_err_t init();
	esp_err_t deinit();

	esp_err_t set_duty(float);

	esp_err_t force_pass();
	esp_err_t force_freewheel();
	esp_err_t force_off();
	esp_err_t unforce();

private:
	esp_err_t init_operator();
	esp_err_t init_generators();
	esp_err_t init_comparators();

	esp_err_t set_tmr_events();
	esp_err_t set_cmpr_events();
	esp_err_t set_deadtime();

	esp_err_t deinit_operator();
	esp_err_t deinit_generators();
	esp_err_t deinit_comparators();

	bool pwm_ticks(float);
	bool has_recessive() const;
};

//

class BuckConverter : public ConverterBase
{
public:
	template <typename... Ts>
	BuckConverter(Ts &&...ts) : ConverterBase(false, std::forward<Ts>(ts)...)
	{
	}
};

class BoostConverter : public ConverterBase
{
public:
	template <typename... Ts>
	BoostConverter(Ts &&...ts) : ConverterBase(true, std::forward<Ts>(ts)...)
	{
	}
};

//

class BuckBoostConverter
{
	static constexpr const char *const TAG = "BuckBoostConverter";

	BuckConverter &bck;
	BoostConverter &bst;

public:
	BuckBoostConverter(BuckConverter &, BoostConverter &);
	~BuckBoostConverter();

	esp_err_t init();
	esp_err_t deinit();

	esp_err_t set_ratio(float);
	esp_err_t force_off();
	esp_err_t force_pass();
	esp_err_t force_freewheel();
};
