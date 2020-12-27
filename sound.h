#ifndef INCLUDED_SOUND_H
#define INCLUDED_SOUND_H

#define SAMPLERATE 48000

/** \brief Container for a variable length mono signal.
 */
typedef struct sound {
	int num_samples;
	double *samples;
} sound;

/** \brief Initialises a silent sound of the given length, deallocating previous data, if it exists.
 *	\param[out] snd			Pointer to the sound object to initialise
 *	\param[in]	num_samples	Number of samples of silence to generate
 */
void sound_init(sound *snd, int num_samples);

/** \brief Copy-initialises a sound with data from another sound, deallocating previous data, if it
 *	       exists.
 *	\param[out] dst	Pointer to sound object to initialise
 *	\param[in]	src	Pointer to sound object to be copied
 */
void sound_copyinit(sound *dst, sound *src);

/** \brief Deallocates a sound object.
 *	\param[in,out]	snd	Pointer to sound object to deallocate.
 */
void sound_delete(sound *snd);

/** \brief Converts a sound with a given sample rate to a sound with the constant SAMPLERATE sample
 *         rate.
 *	\param[out]	dst	Pointer to sound object to be initialised with the resampled data
 *	\param[in]	src	Pointer to sound object to be converted
 *	\param[in]	src_sample_rate	Sample rate to convert from
 */
void sound_resample(sound *dst, sound *src, int src_sample_rate);

char *sound_load(sound *snd, const char *filename);
void sound_save(sound *snd, const char *filename);
void sound_play(sound *snd);

#endif // INCLUDED_SOUND_H
