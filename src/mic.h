#ifndef MIC_H
#define MIC_H

#include <stdint.h>
#include <stddef.h>

struct mic_raw {
	uint32_t max;
	uint32_t min;
	uint32_t avg;
};

struct mic_vol {
	float dB;
	uint32_t rms;
	uint8_t avg_pct;
	uint8_t max_pct;
};

void mic_init(uint32_t sample_cycle);
void mic_activate(void);
void mic_read_samples(uint16_t *samples, size_t n);
void mic_measure(unsigned int samples, struct mic_raw *raw);
void mic_measure_volumn(unsigned int samples, struct mic_vol *vol);

#endif /* MIC_H */
