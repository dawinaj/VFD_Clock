#pragma once

// #include <stdlib.h>
// #include <string.h>
// #include <sys/cdefs.h>
#include "esp_log.h"
#include "esp_check.h"
#include "hal/gpio_types.h"
#include "driver/mcpwm_prelude.h"

class BridgeTimer
{
	const int MCPWM_GRP;
	mcpwm_timer_handle_t timer;

public:
	BridgeTimer(int, gpio_num_t, gpio_num_t);
	~BridgeTimer();
};

class HalfBridge
{
	const int MCPWM_GRP;
	mcpwm_oper_handle_t oper;
	mcpwm_cmpr_handle_t cmpr;
	mcpwm_gen_handle_t gen_hgh;
	mcpwm_gen_handle_t gen_low;

public:
	HalfBridge(int, gpio_num_t, gpio_num_t);
	~HalfBridge();

	void setDuty(float);
	void forceOFF();
	void forceON();
};

class FullBridge
{
	friend class HalfBridge;

	static constexpr uint32_t CLK_FRQ = 160'000'000;			// 80MHz max
	static constexpr uint32_t PWM_FRQ = 100'000;				// somewhere around 30k-300k, affects duty precision
	static constexpr uint32_t TPC = CLK_FRQ / PWM_FRQ;			// ticks per cycle
	static constexpr uint32_t DTT = CLK_FRQ / 1000000 * 1 / 10; // delay in us to ticks | MHz*us = 1 | 80MHz*100ns = 80MHz*1/10us = 8

	HalfBridge inp;
	HalfBridge out;

public:
	FullBridge(gpio_num_t, gpio_num_t, gpio_num_t, gpio_num_t);
	~FullBridge();

	void setRatio(float);
	void forceOFF();
	void passthrough();
};
