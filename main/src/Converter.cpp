#include "Converter.h"

#include <cmath>
#include <algorithm>

#include <driver/gpio.h>

//
//

McpwmTimer::McpwmTimer(int mg, uint32_t f, uint32_t p) : mcpwm_grp(mg), freq_clk(f), period(p)
{
}

McpwmTimer::~McpwmTimer() = default;

//

esp_err_t McpwmTimer::init()
{
	const mcpwm_timer_config_t timer_cfg = {
		.group_id = mcpwm_grp,
		.clk_src = MCPWM_TIMER_CLK_SRC_PLL160M,
		.resolution_hz = freq_clk,
		.count_mode = MCPWM_TIMER_COUNT_MODE_UP,
		.period_ticks = period,
		.flags = {}, // period is never updated // update_period_on_empty
	};

	ESP_RETURN_ON_ERROR(
		mcpwm_new_timer(&timer_cfg, &timer),
		TAG, "Failed to mcpwm_new_timer!");

	ESP_RETURN_ON_ERROR(
		mcpwm_timer_enable(timer),
		TAG, "Failed to mcpwm_timer_enable!");

	return ESP_OK;
}

esp_err_t McpwmTimer::deinit()
{
	ESP_RETURN_ON_ERROR(
		mcpwm_timer_disable(timer),
		TAG, "Failed to mcpwm_timer_disable!");

	ESP_RETURN_ON_ERROR(
		mcpwm_del_timer(timer),
		TAG, "Failed to mcpwm_del_timer!");

	return ESP_OK;
}

esp_err_t McpwmTimer::start()
{
	ESP_RETURN_ON_ERROR(
		mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP),
		TAG, "Failed to mcpwm_timer_start_stop!");

	return ESP_OK;
}

esp_err_t McpwmTimer::stop()
{
	ESP_RETURN_ON_ERROR(
		mcpwm_timer_start_stop(timer, MCPWM_TIMER_STOP_FULL),
		TAG, "Failed to mcpwm_timer_start_stop!");

	return ESP_OK;
}

//

mcpwm_timer_handle_t McpwmTimer::get_timer()
{
	return timer;
}

int McpwmTimer::get_mcpwm() const
{
	return mcpwm_grp;
}

uint32_t McpwmTimer::ns_to_ticks(uint32_t ns) const
{
	constexpr uint64_t ns2s = 1'000'000'000;

	return static_cast<uint64_t>(ns) * freq_clk / ns2s;
}

uint32_t McpwmTimer::period_ticks() const
{
	return period;
}

//
//

ConverterBase::ConverterBase(bool inv, McpwmTimer &t, gpio_num_t gh, gpio_num_t gl, uint32_t d) : timer(t),
																								  gen_d(!inv ? gen_h : gen_l),
																								  gen_r(!inv ? gen_l : gen_h),
																								  gpio_d(!inv ? gh : gl),
																								  gpio_r(!inv ? gl : gh)
{
	deadtime_ticks = timer.ns_to_ticks(d);

	recessive = GPIO_IS_VALID_OUTPUT_GPIO(gpio_r);

	if (!GPIO_IS_VALID_OUTPUT_GPIO(gpio_d))
		ESP_LOGE(TAG, "Invalid GPIO of dominant switch!");
}

ConverterBase::~ConverterBase() = default;

//

esp_err_t ConverterBase::init()
{
	ESP_RETURN_ON_ERROR(
		init_operator(),
		TAG, "Failed to init_operator!");

	ESP_RETURN_ON_ERROR(
		init_comparators(),
		TAG, "Failed to init_comparators!");

	ESP_RETURN_ON_ERROR(
		init_generators(),
		TAG, "Failed to init_generators!");

	ESP_RETURN_ON_ERROR(
		set_tmr_events(),
		TAG, "Failed to set_tmr_events!");

	ESP_RETURN_ON_ERROR(
		set_cmpr_events(),
		TAG, "Failed to set_cmpr_events!");

	ESP_RETURN_ON_ERROR(
		set_deadtime(),
		TAG, "Failed to set_deadtime!");

	return ESP_OK;
}

esp_err_t ConverterBase::init_operator()
{
	const mcpwm_operator_config_t oper_cfg = {
		.group_id = timer.get_mcpwm(),
		.flags = {},
	};

	ESP_RETURN_ON_ERROR(
		mcpwm_new_operator(&oper_cfg, &oper),
		TAG, "Failed to mcpwm_new_operator!");

	ESP_RETURN_ON_ERROR(
		mcpwm_operator_connect_timer(oper, timer.get_timer()),
		TAG, "Failed to mcpwm_operator_connect_timer!");

	return ESP_OK;
}

esp_err_t ConverterBase::init_comparators()
{
	const mcpwm_comparator_config_t cmpr_cfg = {
		.intr_priority = 0,
		.flags = {
			.update_cmp_on_tez = true,
			.update_cmp_on_tep = false,
			.update_cmp_on_sync = false,
		},
	};

	ESP_RETURN_ON_ERROR(
		mcpwm_new_comparator(oper, &cmpr_cfg, &cmpr_d),
		TAG, "Failed to mcpwm_new_comparator!");

	if (has_recessive())
		ESP_RETURN_ON_ERROR(
			mcpwm_new_comparator(oper, &cmpr_cfg, &cmpr_r),
			TAG, "Failed to mcpwm_new_comparator!");

	return ESP_OK;
}

esp_err_t ConverterBase::init_generators()
{
	const mcpwm_generator_config_t gen_d_cfg = {
		.gen_gpio_num = gpio_d,
		.flags = {}, // nothing
	};
	const mcpwm_generator_config_t gen_r_cfg = {
		.gen_gpio_num = gpio_r,
		.flags = {}, // nothing
	};

	ESP_RETURN_ON_ERROR(
		mcpwm_new_generator(oper, &gen_d_cfg, &gen_d),
		TAG, "Failed to mcpwm_new_generator!");

	if (has_recessive())
		ESP_RETURN_ON_ERROR(
			mcpwm_new_generator(oper, &gen_r_cfg, &gen_r),
			TAG, "Failed to mcpwm_new_generator!");

	return ESP_OK;
}

esp_err_t ConverterBase::set_tmr_events()
{
	const mcpwm_gen_timer_event_action_t tmr_evt_d_cfg = {
		.direction = MCPWM_TIMER_DIRECTION_UP,
		.event = MCPWM_TIMER_EVENT_EMPTY,
		.action = MCPWM_GEN_ACTION_HIGH,
	};

	const mcpwm_gen_timer_event_action_t tmr_evt_r_cfg = {
		.direction = MCPWM_TIMER_DIRECTION_UP,
		.event = MCPWM_TIMER_EVENT_EMPTY,
		.action = MCPWM_GEN_ACTION_LOW,
	};

	ESP_RETURN_ON_ERROR(
		mcpwm_generator_set_action_on_timer_event(gen_d, tmr_evt_d_cfg),
		TAG, "Failed to mcpwm_generator_set_action_on_timer_event!");

	if (has_recessive())
		ESP_RETURN_ON_ERROR(
			mcpwm_generator_set_action_on_timer_event(gen_r, tmr_evt_r_cfg),
			TAG, "Failed to mcpwm_generator_set_action_on_timer_event!");

	return ESP_OK;
}

esp_err_t ConverterBase::set_cmpr_events()
{
	const mcpwm_gen_compare_event_action_t cmpr_d_evt = {
		.direction = MCPWM_TIMER_DIRECTION_UP,
		.comparator = cmpr_d,
		.action = MCPWM_GEN_ACTION_LOW,
	};

	const mcpwm_gen_compare_event_action_t cmpr_r_evt = {
		.direction = MCPWM_TIMER_DIRECTION_UP,
		.comparator = cmpr_r,
		.action = MCPWM_GEN_ACTION_HIGH,
	};

	ESP_RETURN_ON_ERROR(
		mcpwm_generator_set_action_on_compare_event(gen_d, cmpr_d_evt),
		TAG, "Failed to mcpwm_generator_set_action_on_compare_event!");

	if (has_recessive())
		ESP_RETURN_ON_ERROR(
			mcpwm_generator_set_action_on_compare_event(gen_r, cmpr_r_evt),
			TAG, "Failed to mcpwm_generator_set_action_on_compare_event!");

	return ESP_OK;
}

esp_err_t ConverterBase::set_deadtime()
{
	const mcpwm_dead_time_config_t deadtime_d_cfg = {
		.posedge_delay_ticks = deadtime_ticks,
		.negedge_delay_ticks = deadtime_ticks,
		.flags = {
			.invert_output = false,
		},
	};

	ESP_RETURN_ON_ERROR(
		mcpwm_generator_set_dead_time(gen_d, gen_d, &deadtime_d_cfg),
		TAG, "Failed to mcpwm_generator_set_dead_time!");

	return ESP_OK;
}

//

esp_err_t ConverterBase::deinit()
{
	ESP_RETURN_ON_ERROR(
		force_off(),
		TAG, "Failed to force_off!");

	ESP_RETURN_ON_ERROR(
		deinit_generators(),
		TAG, "Failed to deinit_generators!");

	ESP_RETURN_ON_ERROR(
		deinit_comparators(),
		TAG, "Failed to deinit_comparators!");

	ESP_RETURN_ON_ERROR(
		deinit_operator(),
		TAG, "Failed to deinit_operator!");

	return ESP_OK;
}

esp_err_t ConverterBase::deinit_operator()
{
	assert(oper);

	ESP_RETURN_ON_ERROR(
		mcpwm_del_operator(oper),
		TAG, "Failed to mcpwm_del_operator!");

	return ESP_OK;
}

esp_err_t ConverterBase::deinit_comparators()
{
	assert(cmpr_d);

	ESP_RETURN_ON_ERROR(
		mcpwm_del_comparator(cmpr_d),
		TAG, "Failed to mcpwm_del_comparator!");

	cmpr_d = nullptr;

	if (cmpr_r)
		ESP_RETURN_ON_ERROR(
			mcpwm_del_comparator(cmpr_r),
			TAG, "Failed to mcpwm_del_comparator!");

	cmpr_r = nullptr;

	return ESP_OK;
}

esp_err_t ConverterBase::deinit_generators()
{
	assert(gen_d);

	ESP_RETURN_ON_ERROR(
		mcpwm_del_generator(gen_d),
		TAG, "Failed to mcpwm_del_generator!");

	gen_d = nullptr;

	if (gen_r)
		ESP_RETURN_ON_ERROR(
			mcpwm_del_generator(gen_r),
			TAG, "Failed to mcpwm_del_generator!");

	gen_r = nullptr;

	return ESP_OK;
}

//

esp_err_t ConverterBase::set_duty(float pwm)
{
	bool inc = pwm_ticks(pwm);

	if (inc && has_recessive())
		ESP_RETURN_ON_ERROR(
			mcpwm_comparator_set_compare_value(cmpr_r, duty_ticks_r),
			TAG, "Failed to mcpwm_comparator_set_compare_values!");

	ESP_RETURN_ON_ERROR(
		mcpwm_comparator_set_compare_value(cmpr_d, duty_ticks_d),
		TAG, "Failed to mcpwm_comparator_set_compare_values!");

	if (!inc && has_recessive())
		ESP_RETURN_ON_ERROR(
			mcpwm_comparator_set_compare_value(cmpr_r, duty_ticks_r),
			TAG, "Failed to mcpwm_comparator_set_compare_values!");

	if (forced) [[unlikely]]
		ESP_RETURN_ON_ERROR(
			unforce(),
			TAG, "Failed to unforce!");

	return ESP_OK;
}

esp_err_t ConverterBase::force_pass()
{
	if (gen_l)
		ESP_RETURN_ON_ERROR(
			mcpwm_generator_set_force_level(gen_l, 0, true),
			TAG, "Failed to mcpwm_generator_set_force_level!");

	if (gen_h)
		ESP_RETURN_ON_ERROR(
			mcpwm_generator_set_force_level(gen_h, 1, true),
			TAG, "Failed to mcpwm_generator_set_force_level!");

	forced = true;

	return ESP_OK;
}

esp_err_t ConverterBase::force_freewheel()
{
	if (gen_h)
		ESP_RETURN_ON_ERROR(
			mcpwm_generator_set_force_level(gen_h, 0, true),
			TAG, "Failed to mcpwm_generator_set_force_level!");

	if (gen_l)
		ESP_RETURN_ON_ERROR(
			mcpwm_generator_set_force_level(gen_l, 1, true),
			TAG, "Failed to mcpwm_generator_set_force_level!");

	forced = true;

	return ESP_OK;
}

esp_err_t ConverterBase::force_off()
{
	ESP_RETURN_ON_ERROR(
		mcpwm_generator_set_force_level(gen_d, 0, true),
		TAG, "Failed to mcpwm_generator_set_force_level!");

	if (has_recessive())
		ESP_RETURN_ON_ERROR(
			mcpwm_generator_set_force_level(gen_r, 0, true),
			TAG, "Failed to mcpwm_generator_set_force_level!");

	forced = true;

	return ESP_OK;
}

esp_err_t ConverterBase::unforce()
{
	ESP_RETURN_ON_ERROR(
		mcpwm_generator_set_force_level(gen_d, -1, false),
		TAG, "Failed to mcpwm_generator_set_force_level!");

	if (has_recessive())
		ESP_RETURN_ON_ERROR(
			mcpwm_generator_set_force_level(gen_r, -1, false),
			TAG, "Failed to mcpwm_generator_set_force_level!");

	forced = false;

	return ESP_OK;
}

//

bool ConverterBase::pwm_ticks(float pwm)
{
	assert(pwm >= 0.0f && pwm <= 1.0f);

	const uint32_t prd = timer.period_ticks();
	const uint32_t old = duty_ticks_d;

	duty_ticks_d = std::lround(pwm * prd); // std::clamp<uint32_t>(cv, 0, prd);

	duty_ticks_r = duty_ticks_d + 2 * deadtime_ticks;

	if (duty_ticks_r > prd)
		duty_ticks_r = prd;

	return duty_ticks_d > old;
}

bool ConverterBase::has_recessive() const
{
	return recessive;
}

//
//

FullBridge::FullBridge(BuckConverter &i, BoostConverter &o) : bck(i), bst(o)
{
}

FullBridge::~FullBridge()
{
}

esp_err_t FullBridge::set_ratio(float ratio)
{
	ratio = std::clamp(ratio, 0.0f, 10.0f);

	//*/
	if (ratio == 1.0f) [[unlikely]]
	{
		ESP_RETURN_ON_ERROR(
			force_pass(),
			TAG, "Failed to force_pass!");
	}
	else
		//*/
		if (ratio < 1) // buck, D = Vo/Vi
		{
			ESP_RETURN_ON_ERROR(
				bst.force_pass(),
				TAG, "Failed to bst.force_pass!");

			ESP_RETURN_ON_ERROR(
				bck.set_duty(ratio),
				TAG, "Failed to bck.set_duty!");
		}
		else // boost, D = Vi/Vo
		{
			ESP_RETURN_ON_ERROR(
				bck.force_pass(),
				TAG, "Failed to bck.force_pass!");

			ESP_RETURN_ON_ERROR(
				bst.set_duty(1 - 1 / ratio),
				TAG, "Failed to bst.set_duty!");
		}

	return ESP_OK;
}

esp_err_t FullBridge::force_off()
{
	ESP_RETURN_ON_ERROR(
		bck.force_off(),
		TAG, "Failed to bck.force_off!");

	ESP_RETURN_ON_ERROR(
		bst.force_off(),
		TAG, "Failed to bst.force_off!");

	return ESP_OK;
}

esp_err_t FullBridge::force_pass()
{
	ESP_RETURN_ON_ERROR(
		bck.force_pass(),
		TAG, "Failed to bck.force_pass!");

	ESP_RETURN_ON_ERROR(
		bst.force_pass(),
		TAG, "Failed to bst.force_pass!");

	return ESP_OK;
}

esp_err_t FullBridge::force_freewheel()
{
	ESP_RETURN_ON_ERROR(
		bck.force_freewheel(),
		TAG, "Failed to bck.force_freewheel!");

	ESP_RETURN_ON_ERROR(
		bst.force_freewheel(),
		TAG, "Failed to bst.force_freewheel!");

	return ESP_OK;

	return ESP_OK;
}
