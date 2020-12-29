#ifndef INCLUDED_EQMATH_H
#define INCLUDED_EQMATH_H

#include "eq.h"    // equalizer
#include "sound.h" // sound

/** \brief Biquadratic filter represented by its direct form 1 coefficients. */
typedef struct biquad {
    double a0, a1, a2, b0, b1, b2;
} biquad;

/** \brief Precompute expensive values needed for computing frequency responses each frame.
 *
 *  \param[out] eq  Pointer to initialised equalizer to get a frequency list from.
 */
void eqmath_init(equalizer *eq);

/** \brief Convert a linear amplitude gain to decibels.
 *
 *  \param[in] gain
 */
double eqmath_gain_to_db(double gain);

/** \brief Convert a decibel value to a linear amplitude gain.
 *
 *  \param[in] db
 */
double eqmath_db_to_gain(double db);

/** \brief Compute the frequency response of a given filter.
 *
 *  \param[in]  eq        Pointer to equalizer object to get a frequency list and filter parameters
 *                        from.
 *  \param[out] gain      Array of doubles to receive the filter's frequency response as linear
 *                        gains.
 *  \param[in]  freq_idx  Index of the filter to analyse.
 */
void eqmath_one_frequency_response(const equalizer *eq, double gain[NFREQ], int cursor);

/** \brief Compute the frequency response of all filters applied in series.
 *
 *  \param[in]  eq    Pointer to equalizer object to get a frequency list and filter parameters
 *                    from.
 *  \param[out] gain  Array of doubles to receive the equalizer's frequency response as linear
 *                    gains.
 */
void eqmath_overall_frequency_response(const equalizer *eq, double gain[NFREQ]);

/** \brief Initialise a biquad filter in a Peaking-EQ configuration.
 *
 *  \param[out] filter    Pointer to biquad struct to initialise.
 *  \param[in]  eq        Equalizer to get filter parameters from.
 *  \param[in]  freq_idx  Index of selected filter.
 */
void eqmath_biquad_prepare_peakingeq(biquad *filter, const equalizer *eq, int freq_idx);

/** \brief Apply a biquad filter to an input signal.
 *
 *  \param[in]  filter  Pointer to biquad filter to apply.
 *  \param[in]  in      Pointer to input signal.
 *  \param[out] out     Pointer to a sound to be initialised with the output of the filter.
 */
void eqmath_biquad_apply(const biquad *filter, const sound *in, sound *out);

/** \brief Apply all filters of an equalizer in series to an input signal. Since this is relatively
 *         slow, a progress callback function is specified, which can be used to notify the user of
 *         the processing progress.
 *
 *  \param[in]  eq                 Pointer to equalizer to use for processing the signal.
 *  \param[in]  in                 Pointer to input signal.
 *  \param[out] out                Pointer to a sound to be initialised with the resulting signal.
 *  \param[in]  progress_callback  double->void function which is called after each intermediate
 *                                 step with a value in [0.0; 1.0] representing current progress.
 */
void eqmath_process(const equalizer *eq, const sound *in, sound *out, void (*progress_callback)(double));

#endif // INCLUDED_EQMATH_H
