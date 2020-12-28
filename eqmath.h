#ifndef INCLUDED_EQMATH_H
#define INCLUDED_EQMATH_H

#include "eq.h"
#include "sound.h"

typedef struct biquad {
    double a0, a1, a2, b0, b1, b2;
} biquad;

void eqmath_init(equalizer *eq);
double eqmath_gain_to_db(double gain);
double eqmath_db_to_gain(double db);
void eqmath_one_frequency_response(equalizer *eq, double gain[NFREQ], int cursor);
void eqmath_overall_frequency_response(equalizer *eq, double gain[NFREQ]);
void eqmath_biquad_prepare_peakingeq(biquad *filter, equalizer *eq, int i);
void eqmath_biquad_apply(const biquad *filter, sound *in, sound *out);
void eqmath_process(equalizer *eq, sound *in, sound *out);

#endif // INCLUDED_EQMATH_H
