#ifndef INCLUDED_EQ_H
#define INCLUDED_EQ_H

#include <stdint.h>

#define NFREQ 75
#define LOFREQ 20
#define HIFREQ 20000
#define LOGAIN -20.0
#define HIGAIN 20.0

extern const double eq_q_values[10];

typedef struct equalizer {
    double gain_db[NFREQ];
    uint8_t q_idx[NFREQ];
    double freqs[NFREQ];
} equalizer;

void eq_init(equalizer *eq);
void eq_set_q_option(equalizer *eq, int cursor_pos, int q_opt);
void eq_change_gain(equalizer *eq, int cursor_pos, double delta);

#endif // INCLUDED_EQ_H
