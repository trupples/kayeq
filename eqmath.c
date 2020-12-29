#include "eqmath.h"
#include <math.h>    // cos, sin, log10, pow
#include <complex.h> // complex, cexpf, cabs
#include <assert.h>

#define PI 3.14159265358979323846

static double memo_cos[NFREQ] = { 0.0 };
static double memo_alpha[10][NFREQ] = {{ 0.0 }};

void eqmath_init(equalizer *eq) {
    for(int i = 0; i < NFREQ; i++) {
        const double w0 = 2 * PI * eq->freqs[i] / SAMPLERATE;
        memo_cos[i] = cos(w0);
        for(int j = 0; j < 10; j++) {
            memo_alpha[j][i] = sin(w0) / (2 * eq_q_values[j]);
        }
    }
}

double eqmath_gain_to_db(double gain) {
    return log10(gain) * 20;
}

double eqmath_db_to_gain(double db) {
    return pow(10.0, db / 20.0);
}

void eqmath_one_frequency_response(const equalizer *eq, double gain[NFREQ], int cursor) {
    biquad filter = { 0 };
    eqmath_biquad_prepare_peakingeq(&filter, eq, cursor);
    for(int i = 0; i < NFREQ; i++) {
        // z is actually z^-1 from the formulas
        const double complex z = cexpf(-2 * I * PI * eq->freqs[i] / SAMPLERATE);
        const double complex H = (filter.b0 + filter.b1 * z + filter.b2 * z * z) /
                                 (filter.a0 + filter.a1 * z + filter.a2 * z * z);
        gain[i] = cabs(H);
    }
}

void eqmath_overall_frequency_response(const equalizer *eq, double out[NFREQ]) {
    for(int i = 0; i < NFREQ; i++) out[i] = 1.0;
    for(int i = 0; i < NFREQ; i++) {
        double partial_gain[NFREQ] = { 0 };
        eqmath_one_frequency_response(eq, partial_gain, i);
        for(int j = 0; j < NFREQ; j++) out[j] *= partial_gain[j];
    }
}

// http://shepazu.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
void eqmath_biquad_prepare_peakingeq(biquad *filter, const equalizer *eq, int i) {
    const double alpha = memo_alpha[eq->q_idx[i]][i];
    const double c = memo_cos[i];
    const double A = pow(10, eq->gain_db[i] / 40);

    filter->b0 = 1 + alpha * A;
    filter->b1 = -2 * c;
    filter->b2 = 1 - alpha * A;
    filter->a0 = 1 + alpha / A;
    filter->a1 = -2 * c;
    filter->a2 = 1 - alpha / A;
}

void eqmath_biquad_apply(const biquad *filter, const sound *in, sound *out) {
    assert(in->num_samples == out->num_samples);

    double *x = in->samples;
    double *y = out->samples;

    y[0] = (filter->b0 * x[0] + filter->b1 * 0    + filter->b2 * 0 - filter->a1 * 0    - filter->a2 * 0) / filter->a0;
    y[1] = (filter->b0 * x[1] + filter->b1 * x[0] + filter->b2 * 0 - filter->a1 * y[0] - filter->a2 * 0) / filter->a0;

    for(int n = 2; n < in->num_samples; n++) {
        y[n] = (filter->b0 * x[n] + filter->b1 * x[n-1] + filter->b2 * x[n-2] - filter->a1 * y[n-1] - filter->a2 * y[n-2]) / filter->a0;
    }
}

void eqmath_process(const equalizer *eq, const sound *in, sound *out, void (*progress_callback)(double)) {
    sound intermediate1 = { 0 }, intermediate2 = { 0 };
    sound *intermediate_in = &intermediate1, *intermediate_out = &intermediate2;
    sound_copyinit(intermediate_in, in);
    sound_init(intermediate_out, in->num_samples);

    progress_callback(0.0);

    // apply each filter "in series"
    for(int i = 0; i < NFREQ; i++) {
        biquad filter;
        eqmath_biquad_prepare_peakingeq(&filter, eq, i);
        eqmath_biquad_apply(&filter, intermediate_in, intermediate_out);

        sound *tmp = intermediate_in;
        intermediate_in = intermediate_out;
        intermediate_out = tmp;

        progress_callback(i * 1.0 / (NFREQ-1));
    }

    // copy last intermediate result to output
    sound_copyinit(out, intermediate_in);

    // clean up
    sound_delete(intermediate_in);
    sound_delete(intermediate_out);
}
