#include "tick.h"
#include "abov/asm/arm/systick.h"
#include "abov/irq.h"

static uint32_t monotonic_counter;

void tick_init(void)
{
	systick_set_frequency(1000);
	systick_clear();
	systick_start();
}

uint32_t tick_get_ms(void)
{
	return monotonic_counter;
}

void ISR_systick(void)
{
	monotonic_counter++;
}
