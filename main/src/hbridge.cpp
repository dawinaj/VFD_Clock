
#include "hbridge.h"

#include "settings.h"

#include <cmath>

static const char *TAG = "HBridge";

HalfBridge::HalfBridge(int mg, gpio_num_t gh, gpio_num_t gl) : MCPWM_GRP(mg), timer(nullptr), oper(nullptr), cmpr(nullptr), gen_hgh(nullptr), gen_low(nullptr)
{
	const mcpwm_timer_config_t timer_cfg = {
		.group_id = MCPWM_GRP,
		.clk_src = MCPWM_TIMER_CLK_SRC_PLL160M,
		.resolution_hz = FullBridge::CLK_FRQ,
		.count_mode = MCPWM_TIMER_COUNT_MODE_UP,
		.period_ticks = FullBridge::TPC,
		.flags = {},
	};

	const mcpwm_operator_config_t oper_cfg = {
		.group_id = MCPWM_GRP,
		.flags = {},
	};

	mcpwm_new_timer(&timer_cfg, &timer);
	mcpwm_new_operator(&oper_cfg, &oper);
	mcpwm_operator_connect_timer(oper, timer);

	//

	const mcpwm_comparator_config_t cmpr_cfg = {
		.flags = {
			.update_cmp_on_tez = true,
			.update_cmp_on_tep = false,
			.update_cmp_on_sync = false,
		},
	};

	const mcpwm_generator_config_t gen_hgh_cfg = {
		.gen_gpio_num = gh,
		.flags = {},
	};
	const mcpwm_generator_config_t gen_low_cfg = {
		.gen_gpio_num = gl,
		.flags = {},
	};

	mcpwm_dead_time_config_t deadtime_hgh_cfg = {
		.posedge_delay_ticks = FullBridge::DTT,
		.negedge_delay_ticks = 0,
		.flags = {.invert_output = false},
	};
	mcpwm_dead_time_config_t deadtime_low_cfg = {
		.posedge_delay_ticks = 0,
		.negedge_delay_ticks = FullBridge::DTT,
		.flags = {.invert_output = true},
	};

	mcpwm_new_comparator(oper, &cmpr_cfg, &cmpr);
	mcpwm_comparator_set_compare_value(cmpr, 0); // off

	mcpwm_new_generator(oper, &gen_hgh_cfg, &gen_hgh);
	mcpwm_new_generator(oper, &gen_low_cfg, &gen_low);

	mcpwm_gen_timer_event_action_t tmr_evt = {
		.direction = MCPWM_TIMER_DIRECTION_UP,
		.event = MCPWM_TIMER_EVENT_EMPTY,
		.action = MCPWM_GEN_ACTION_HIGH,
	};

	mcpwm_gen_compare_event_action_t cmp_evt = {
		.direction = MCPWM_TIMER_DIRECTION_UP,
		.comparator = cmpr,
		.action = MCPWM_GEN_ACTION_LOW,
	};

	mcpwm_gen_timer_event_action_t tmr_end = MCPWM_GEN_TIMER_EVENT_ACTION_END();
	mcpwm_gen_compare_event_action_t cmp_end = MCPWM_GEN_COMPARE_EVENT_ACTION_END();

	mcpwm_generator_set_actions_on_timer_event(gen_hgh, tmr_evt, tmr_end);
	mcpwm_generator_set_actions_on_compare_event(gen_hgh, cmp_evt, cmp_end);

	mcpwm_generator_set_dead_time(gen_hgh, gen_hgh, &deadtime_hgh_cfg);
	mcpwm_generator_set_dead_time(gen_hgh, gen_low, &deadtime_low_cfg);

	ESP_LOGI(TAG, "Enable and start timer");
	mcpwm_timer_enable(timer);
	mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP);

	///

	ESP_ERROR_CHECK(
		mcpwm_generator_set_action_on_timer_event(gena,
												  MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
	ESP_ERROR_CHECK(
		mcpwm_generator_set_action_on_compare_event(gena,
													MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_LOW)));

	mcpwm_dead_time_config_t dead_time_config = {
		.posedge_delay_ticks = 50,
		.negedge_delay_ticks = 0};
	ESP_ERROR_CHECK(mcpwm_generator_set_dead_time(gena, gena, &dead_time_config));
	dead_time_config.posedge_delay_ticks = 0;
	dead_time_config.negedge_delay_ticks = 100;
	dead_time_config.flags.invert_output = true;
	ESP_ERROR_CHECK(mcpwm_generator_set_dead_time(gena, genb, &dead_time_config));
}

HalfBridge::~HalfBridge()
{
	if (gen_hgh)
		mcpwm_del_generator(gen_hgh);
	if (gen_low)
		mcpwm_del_generator(gen_low);
	if (cmpr)
		mcpwm_del_comparator(cmpr);

	if (oper)
		mcpwm_del_operator(oper);
	if (timer)
		mcpwm_del_timer(timer);
}

void HalfBridge::setDuty(float pwm)
{
	pwm = bound(pwm, 0.0f, 1.0f);
	uint32_t cmp_ticks = std::lround(FullBridge::TPC * pwm);
	mcpwm_comparator_set_compare_value(cmpr, cmp_ticks);
}

void HalfBridge::forceOFF()
{
	mcpwm_comparator_set_compare_value(cmpr, 0);
}

void HalfBridge::forceON()
{
	// "The compare value shouldn't exceed timer's count peak, otherwise, the compare event will never get triggered." And will throw an error you stupid bastards...
	mcpwm_comparator_set_compare_value(cmpr, FullBridge::TPC);
}

//

FullBridge::FullBridge(gpio_num_t gih, gpio_num_t gil, gpio_num_t goh, gpio_num_t gol) : inp(0, gih, gil), out(1, goh, gol)
{
}

FullBridge::~FullBridge()
{
}

void FullBridge::setRatio(float ratio)
{
	ratio = bound(ratio, 0.0f, 10.0f);

	if (ratio == 0)
		return forceOFF();

	if (ratio == 1)
		return passthrough();

	if (ratio < 1) // buck, D = Vo/Vi
	{
		inp.setDuty(ratio);
		out.forceON();
	}
	else // boost, D = Vi/Vo
	{
		inp.forceON();
		out.setDuty(1 / ratio);
	}
}

void FullBridge::forceOFF()
{
	inp.forceOFF();
	out.forceOFF();
}

void FullBridge::passthrough()
{
	inp.forceON();
	out.forceON();
}