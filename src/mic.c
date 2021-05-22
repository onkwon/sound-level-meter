#include "mic.h"
#include <stdlib.h>
#include <math.h>
#include "abov/hal/adc.h"
#include "abov/hal/gpio.h"

#define MIN(a, b)			((a) > (b)? (b) : (a))
#define MAX(a, b)			((a) < (b)? (b) : (a))

static void mic_gpio_init(void)
{
	gpio_open(PERIPH_GPIOA, 0, &(struct gpio_cfg) {
			.mode = GPIO_MODE_ANALOG,
			.altfunc = true, });
}

void mic_init(void)
{
	mic_gpio_init();

	adc_enable(PERIPH_ADC1);
	adc_set_mode(PERIPH_ADC1, ADC_MODE_CONTINUOUS_CONVERSION);
	adc_set_trigger(PERIPH_ADC1, ADC_TRIGGER_MANUAL);
	adc_select_channel(PERIPH_ADC1, ADC_CHANNEL_0);
	adc_set_sample_time(PERIPH_ADC1, ADC_CHANNEL_0, 240);

	adc_calibrate(PERIPH_ADC1);
}

void mic_activate(void)
{
	adc_start(PERIPH_ADC1);
}

void mic_measure(unsigned int samples, struct mic_raw *raw)
{
	uint32_t avg = 0;
	uint32_t max = 0;
	uint32_t min = 4096/2;

	for (unsigned int i = 0; i < samples; i++) {
		while (!adc_is_completed(PERIPH_ADC1)) { /* waiting */ }
		uint32_t adc = adc_get_measurement(PERIPH_ADC1);
		avg += adc;
		max = MAX(max, adc);
		min = MIN(min, adc);
	}

	avg /= samples;

	raw->avg = avg;
	raw->max = max;
	raw->min = min;
}

void mic_measure_volumn(unsigned int samples, struct mic_vol *vol)
{
	uint32_t avg = 0;
	uint32_t max = 0;
	uint64_t square_sum = 0;
	uint32_t amp_base = 4096/2;

	for (unsigned int i = 0; i < samples; i++) {
		while (!adc_is_completed(PERIPH_ADC1)) { /* waiting */ }
		uint32_t adc = adc_get_measurement(PERIPH_ADC1);
		uint32_t amp = (uint32_t)abs((int)adc - (int)amp_base);
		avg += amp;
		max = MAX(amp, max);
		square_sum += (uint64_t)amp * amp;
	}

	avg /= samples;
	float rms = sqrtf((float)(square_sum / samples));
	float dB = 20.f * log10f(rms / (float)amp_base);

	vol->dB = dB;
	vol->rms = (uint32_t)rms;
	vol->max_pct = (uint8_t)(100 * max / amp_base);
	vol->avg_pct = (uint8_t)(100 * avg / amp_base);
}
