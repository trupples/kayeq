#ifndef INCLUDED_EQ_H
#define INCLUDED_EQ_H

#include <stdint.h> // uint8_t

#define NFREQ 75        /**< Number of controllable frequencies/filters. */
#define LOFREQ 20       /**< Lowest controllable frequency. */
#define HIFREQ 20000    /**< Highest controllable frequency. */
#define LOGAIN -20.0    /**< Lowest gain the user can set for a filter, in decibels. */
#define HIGAIN 20.0     /**< Highest gain the user can set for a filter, in decibels. */

/** \brief The numeric values of the 10 options for the Q factor. */
extern const double eq_q_values[10];

/** \brief Equalizer state stores the gain in decibels, Q factor index, and center frequency for
 *         each of the NFREQ filters.
 */
typedef struct equalizer {
    double gain_db[NFREQ];
    uint8_t q_idx[NFREQ];
    double freqs[NFREQ];
} equalizer;

/** \brief Initialise an equalizer state to a default Q = 1.8, gain = 0dB for all frequencies.
 *
 *  \param[in,out] eq  Pointer to the equalizer to initialise.
 */
void eq_init(equalizer *eq);

/** \brief Control the Q factor of a given filter.
 *
 *  \param[in,out] eq        Pointer to the equalizer to change.
 *  \param[in]     freq_idx  Index of selected frequency. [0; NFREQ-1]
 *  \param[in]     q_idx     Index of Q factor value to apply. [0; 9]
 */
void eq_set_q_option(equalizer *eq, int freq_idx, int q_idx);

/** \brief Change the gain of a given filter by a relative amount, clamping to [LOGAIN; HIGAIN].
 *
 *  \param[in,out] eq             Pointer to the equalizer to change.
 *  \param[in]     freq_idx       Index of selected frequency. [0; NFREQ-1]
 *  \param[in]     gain_db_delta  Amount to increase the gain of this frequency, in decibels.
 */
void eq_change_gain(equalizer *eq, int freq_idx, double gain_db_delta);

#endif // INCLUDED_EQ_H
