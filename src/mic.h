#ifndef MIC_H
#define MIC_H

#include <stdint.h>

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

void mic_init(void);
void mic_activate(void);
void mic_measure(unsigned int samples, struct mic_raw *raw);
void mic_measure_volumn(unsigned int samples, struct mic_vol *vol);

#endif /* MIC_H */
