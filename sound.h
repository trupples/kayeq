#ifndef INCLUDED_SOUND_H
#define INCLUDED_SOUND_H

#define SAMPLERATE 48000 /**< Sample rate to be used for processing sounds. */

/** \brief Container for a variable length one-channel signal.
 */
typedef struct sound {
    int num_samples;
    double *samples;
} sound;

/** \brief Initialises a silent sound of the given length, deallocating previous data, if any
 *         exists.
 *
 *  \param[out] snd          Pointer to the sound object to initialise
 *  \param[in]  num_samples  Number of samples of silence to generate
 */
void sound_init(sound *snd, int num_samples);

/** \brief Copy-initialises a sound with data from another sound, deallocating previous data, if it
 *         exists.
 *
 *  \param[out] dst  Pointer to sound object to initialise
 *  \param[in]  src  Pointer to sound object to be copied
 */
void sound_copyinit(sound *dst, const sound *src);

/** \brief Deallocates a sound object.
 *
 *  \param[in,out] snd  Pointer to sound object to deallocate.
 */
void sound_delete(sound *snd);

/** \brief Converts a sound with a given sample rate to a sound with the constant SAMPLERATE sample
 *         rate.
 *
 *  \param[out] dst              Pointer to sound object to be initialised with the resampled data
 *  \param[in]  src              Pointer to sound object to be converted
 *  \param[in]  src_sample_rate  Sample rate to convert from
 */
void sound_resample(sound *dst, const sound *src, int src_sample_rate);

/** \brief Initialise sound with data from WAV file.
 *
 *  \param[out] snd       Pointer to sound to be initialised.
 *  \param[in]  filename  Path to WAV file to read.
 */
char *sound_load(sound *snd, const char *filename);

/** \brief Store sound to disk as a 16-bit WAV file.
 *
 *  \param[in] snd       Pointer to sound to store to disk.
 *  \param[in] filename  Path to WAV file to write.
 */
void sound_save(const sound *snd, const char *filename);

/** \brief DEPRECATED: Play a sound to the default audio output device.
 *
 *  \param[in] snd  Pointer to the sound to play.
 */
void sound_play(const sound *snd);

#endif // INCLUDED_SOUND_H
