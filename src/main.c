#include "console.h"
#include "mic.h"
#include "tick.h"

#define SAMPLES				2048

int main(void)
{
	console_init();
	mic_init();
	tick_init();

	mic_activate();

	while (1) {
		uint32_t t0 = tick_get_ms();
#if defined(RAW_VALUE)
		struct mic_raw raw;
		mic_measure(SAMPLES, &raw);
		uint32_t elapsed = tick_get_ms() - t0;

		printf("[%u] min: %u max: %u avg: %u span %u %u %u\r\n",
				elapsed,
				raw.min, raw.max, raw.avg,
				raw.max - raw.min,
				raw.max - raw.avg,
				raw.avg - raw.min);
#else
		struct mic_vol vol;
		mic_measure_volumn(SAMPLES, &vol);
		uint32_t elapsed = tick_get_ms() - t0;

		printf("[%u] dB: %.3f rms: %u max: %u%% avg: %u%%\r\n",
				elapsed,
				(double)vol.dB,
				vol.rms,
				vol.max_pct,
				vol.avg_pct);
#endif
	}

	return 0;
}
