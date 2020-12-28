#include "eq.h"
#include "eqmath.h"
#include <math.h>

#include "sound.h" // for SAMPLERATE

const double eq_q_values[10] = {0.5, 0.7, 1.0, 1.3, 1.8, 2.5, 3.4, 4.7, 6.5, 9.0};

void eq_init(equalizer *eq) {
    for(int i = 0; i < NFREQ; i++) {
        eq->gain_db[i] = 0.0;
        eq->q_idx[i] = 4;
        eq->freqs[i] = LOFREQ * pow(1.0 * HIFREQ / LOFREQ, 1.0 * i / (NFREQ - 1));
    }
}

void eq_set_q_option(equalizer *eq, int cursor_pos, int q_opt) {
    eq->q_idx[cursor_pos] = q_opt;
}

void eq_change_gain(equalizer *eq, int cursor_pos, double delta) {
    eq->gain_db[cursor_pos] += delta;
    if(eq->gain_db[cursor_pos] > HIGAIN) eq->gain_db[cursor_pos] = HIGAIN;
    if(eq->gain_db[cursor_pos] < LOGAIN) eq->gain_db[cursor_pos] = LOGAIN;
}
