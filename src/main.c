#include "console.h"
#include "mic.h"
#include "tick.h"

#define SPECTRUM
//#define RAW_VALUE

#if defined(SPECTRUM)
#include "arm_math.h"
#include "arm_const_structs.h"
#define SAMPLE_CYCLE			71
#define FFT_SIZE			256
#define assert(expr)			((void)(expr))
static uint16_t adc_sample[FFT_SIZE];
static q15_t fft_result[FFT_SIZE*2];
static void do_fft(void)
{
	mic_read_samples(adc_sample, FFT_SIZE);

	arm_rfft_instance_q15 fft_ctx;

	arm_status rc = arm_rfft_init_q15(&fft_ctx, FFT_SIZE, 0, 1);
	assert(rc == 0);
	arm_rfft_q15(&fft_ctx, (q15_t *)adc_sample, fft_result);
	arm_abs_q15(fft_result, fft_result, FFT_SIZE);

	for (int i = 0; i < FFT_SIZE/2; i++) {
		printf("%d, %d\r\n", i+1, fft_result[i]);
	}
}
#elif defined(RAW_VALUE)
#define SAMPLE_CYCLE			240
#define SAMPLES				2048
static void do_raw_value(void)
{
	uint32_t t0 = tick_get_ms();
	struct mic_raw raw;
	mic_measure(SAMPLES, &raw);
	uint32_t elapsed = tick_get_ms() - t0;

	printf("[%u] min: %u max: %u avg: %u span %u %u %u\r\n",
			elapsed,
			raw.min, raw.max, raw.avg,
			raw.max - raw.min,
			raw.max - raw.avg,
			raw.avg - raw.min);
}
#else
#define SAMPLE_CYCLE			240
#define SAMPLES				2048
static void do_measure(void)
{
	uint32_t t0 = tick_get_ms();
	struct mic_vol vol;
	mic_measure_volumn(SAMPLES, &vol);
	uint32_t elapsed = tick_get_ms() - t0;

	printf("[%u] dB: %.3f rms: %u max: %u%% avg: %u%%\r\n",
			elapsed,
			(double)vol.dB,
			vol.rms,
			vol.max_pct,
			vol.avg_pct);
}
#endif

int main(void)
{
	console_init();
	mic_init(SAMPLE_CYCLE);
	tick_init();

	mic_activate();

	while (1) {
#if defined(SPECTRUM)
		do_fft();
#elif defined(RAW_VALUE)
		do_raw_value();
#else
		do_measure();
#endif
	}

	return 0;
}
