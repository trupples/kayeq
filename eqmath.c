#include "eqmath.h"
#include <math.h>
#include <complex.h>

#define PI 3.14159265358979323846

double eqmath_gain_to_db(double gain) {
	return log10(gain) * 20;
}

double eqmath_db_to_gain(double db) {
	return pow(10.0, db / 20.0);
}

void eqmath_one_frequency_response(equalizer *eq, double gain[NFREQ], int cursor) {
	biquad filter = { 0 };
	eqmath_biquad_prepare_peakingeq(&filter, eq->freqs[cursor], eq->gain_db[cursor], eq->q[cursor]);
	for(int i = 0; i < NFREQ; i++) {
		double complex z = cexpf(-2 * I * PI * eq->freqs[i] / SAMPLERATE); // z is actually z^-1 from the formulas
		double complex H = (filter.b0 + filter.b1*z + filter.b2*z*z) / (filter.a0 + filter.a1*z + filter.a2*z*z);
		gain[i] = cabs(H);
	}
}

void eqmath_overall_frequency_response(equalizer *eq, double out[NFREQ]) {
	for(int i = 0; i < NFREQ; i++) out[i] = 1.0;
	for(int i = 0; i < NFREQ; i++) {
		double partial_gain[NFREQ] = { 0 };
		eqmath_one_frequency_response(eq, partial_gain, i);
		for(int j = 0; j < NFREQ; j++) out[j] *= partial_gain[j];
	}
}

// http://shepazu.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
void eqmath_biquad_prepare_peakingeq(biquad *filter, double freq, double gain_db, double Q) {
	double w0 = 2 * PI * freq / SAMPLERATE;
	double c = cos(w0);
	double alpha = sin(w0) / (2 * Q);
	double A = pow(10, gain_db / 40);

	filter->b0 = 1 + alpha * A;
	filter->b1 = -2 * c;
	filter->b2 = 1 - alpha * A;
	filter->a0 = 1 + alpha / A;
	filter->a1 = -2 * c;
	filter->a2 = 1 - alpha / A;
}

void eqmath_biquad_apply(const biquad *filter, sound *in, sound *out) {
	sound_init(out, in->num_samples);

	double *x = in->samples;
	double *y = out->samples;

	y[0] = (filter->b0 * x[0] + filter->b1 * 0    + filter->b2 * 0 - filter->a1 * 0    - filter->a2 * 0) / filter->a0;
	y[1] = (filter->b0 * x[1] + filter->b1 * x[0] + filter->b2 * 0 - filter->a1 * y[0] - filter->a2 * 0) / filter->a0;

	for(int n = 2; n < in->num_samples; n++) {
		y[n] = (filter->b0 * x[n] + filter->b1 * x[n-1] + filter->b2 * x[n-2] - filter->a1 * y[n-1] - filter->a2 * y[n-2]) / filter->a0;
	}
}

void eqmath_process(equalizer *eq, sound *in, sound *out) {
	sound intermediate_in = { 0 }, intermediate_out = { 0 };
	sound_copyinit(&intermediate_in, in);

	// apply each filter "in series"
	for(int i = 0; i < NFREQ; i++) {
		biquad filter;
		eqmath_biquad_prepare_peakingeq(&filter, eq->freqs[i], eq->gain_db[i], eq->q[i]);
		eqmath_biquad_apply(&filter, &intermediate_in, &intermediate_out);
		sound_copyinit(&intermediate_in, &intermediate_out);
	}

	// copy last intermediate result to output
	sound_init(out, intermediate_out.num_samples);
	for(int i = 0; i < intermediate_out.num_samples; i++) {
		out->samples[i] = intermediate_out.samples[i];
	}

	// clean up
	sound_delete(&intermediate_in);
	sound_delete(&intermediate_out);
}
