#include "sound.h"

#include <stdlib.h> // calloc, free
#include <string.h> // memcpy
#include <math.h>   // floor, ceil
#include <stdint.h>
#include <stdio.h>

void sound_init(sound *snd, int num_samples) {
    free(snd->samples); // shouldn't be necessary. it prevents double initialisation
    snd->num_samples = num_samples;

    // dynamically allocate a list of zeros of the right length
    snd->samples = calloc(num_samples, sizeof(double));
}

void sound_delete(sound *snd) {
    if(snd == NULL) return;
    snd->num_samples = 0;
    free(snd->samples);
    snd->samples = NULL;
}

void sound_copyinit(sound* dest, const sound *src) {
    // initialise dest with a correct amount of zeros
    sound_init(dest, src->num_samples);

    // copy samples from src->samples to dest->samples, byte by byte
    memcpy(dest->samples, src->samples, sizeof(double) * src->num_samples);
}

void sound_resample(sound *out, const sound *in, int in_sample_rate) {
    sound_init(out, (int) ceil(1.0 * in->num_samples * SAMPLERATE / in_sample_rate));

    for(int i = 0; i < out->num_samples; i++) {
        double in_pos = 1.0 * i * in_sample_rate / SAMPLERATE;
        if(in_pos > in->num_samples) {
            // BAD! Shouldn't happen!
            in_pos = in->num_samples;
        }
        const double lo = in->samples[(int) floor(in_pos)];
        const double hi = in->samples[(int) ceil(in_pos)];
        const double fract = in_pos - floor(in_pos);
        out->samples[i] = lo * (1 - fract) + hi * fract;
    }
}

#if BYTE_ORDER == LITTLE_ENDIAN

struct riff_chunk {
    uint32_t id;            // 0x46464952 'RIFF'
    uint32_t size;
    uint32_t format;        // 0x45564157 'WAVE'
};

struct fmt__chunk {
    uint32_t id;            // 0x20746d66 'fmt '
    uint32_t size;
    uint16_t audio_format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

struct data_header {
    uint32_t id;            // 0x61746164 'data'
    uint32_t size;
};

struct wave_file {
    struct riff_chunk riff;
    struct fmt__chunk fmt_;
    struct data_header data_header;
};

void sound_save(const sound *snd, const char *filename) {
    FILE *f = fopen(filename, "wb");

    const struct wave_file wav_headers = {
        .riff = {
            0x46464952,                 // 'RIFF'
            36 + 2 * snd->num_samples,  // total size
            0x45564157                  // 'WAVE'
        },
        .fmt_ = {
            0x20746d66,                 // 'fmt '
            16,                         // fmt size
            1,                          // audio format = 1 PCM
            1,                          // num channels
            SAMPLERATE,                 // sample rate
            SAMPLERATE * 2,             // byte rate
            2,                          // block align
            16                          // bits per sample
        },
        .data_header = {
            0x61746164,                 // 'data'
            2 * snd->num_samples        // data size
        }
    };

    fwrite(&wav_headers, sizeof(wav_headers), 1, f);

    for(int i = 0; i < snd->num_samples; i++) {
        double x = snd->samples[i] * 32767;
        if(x < -32767) x = -32767;
        if(x > 32767) x = 32767;
        const uint16_t sample_data = (uint16_t) (int16_t) x;
        fwrite(&sample_data, 2, 1, f);
    }

    fclose(f);
}

char *sound_load(sound *snd, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if(f == NULL) return strerror(errno);

#define FAIL(err) { sound_delete(snd); fclose(f); return err; }
#define TRY(stmt) stmt; if(ferror(f)) FAIL(strerror(errno))

    struct riff_chunk riff = { 0 };
    TRY(fread(&riff, sizeof(riff), 1, f));

    if(riff.id != 0x46464952 || riff.format != 0x45564157)
        FAIL("File is not a wav file");

    struct fmt__chunk fmt = { 0 };

    unsigned int remaining_bytes = riff.size - 4;
    while(remaining_bytes > 0) {
        uint32_t next_chunk_id = 0;
        uint32_t next_chunk_size = 0;
        TRY(fread(&next_chunk_id, 4, 1, f));
        TRY(fread(&next_chunk_size, 4, 1, f));
        remaining_bytes -= 8;

        if(next_chunk_id == 0x20746d66) { // 'fmt '
            TRY(fseek(f, -8, SEEK_CUR));
            TRY(fread(&fmt, sizeof(fmt), 1, f));
            TRY(fseek(f, next_chunk_size-sizeof(fmt)+8, SEEK_CUR)); // skip any extra fields

            // check format is something we can deal with
            if(fmt.audio_format != 1 && fmt.audio_format != 3)
                FAIL("KayEQ only supports PCM and float audio formats");

            if(fmt.channels != 1)
                FAIL("KayEQ only supports mono audio");

            if(fmt.block_align != fmt.bits_per_sample * fmt.channels / 8 ||
                    fmt.byte_rate != fmt.sample_rate * fmt.bits_per_sample * fmt.channels / 8)
                FAIL("Format chunk is inconsistent");
        } else if(next_chunk_id == 0x61746164) { // 'data'
            const uint32_t num_samples = next_chunk_size / fmt.block_align;
            sound_init(snd, num_samples);

            if(fmt.audio_format == 1) { // PCM
                if(fmt.bits_per_sample == 8) {
                    for(uint32_t i = 0; i < num_samples; i++) {
                        uint8_t sample_data = 0;
                        fread(&sample_data, 1, 1, f);
                        snd->samples[i] = sample_data / 128.0 - 1.0;
                    }
                } else if(fmt.bits_per_sample == 16) {
                    for(uint32_t i = 0; i < num_samples; i++) {
                        int16_t sample_data = 0;
                        fread(&sample_data, 2, 1, f);
                        snd->samples[i] = sample_data / 32767.0;
                    }
                } else FAIL("KayEQ only supports 8 and 16 bit PCM");
            } else if(fmt.audio_format == 3) { // float
                if(fmt.bits_per_sample == 32) {
                    for(uint32_t i = 0; i < num_samples; i++) {
                        float sample = 0;
                        fread(&sample, 4, 1, f);
                        snd->samples[i] = sample;
                    }
                } else FAIL("KayEQ only supports 32 bit float");
            }
        } else {
            TRY(fseek(f, next_chunk_size, SEEK_CUR)); // skip this chunk
        }
        remaining_bytes -= next_chunk_size;
    }

#undef TRY
#undef FAIL

    fclose(f);

    sound before_resample = { 0 };
    sound_copyinit(&before_resample, snd);
    sound_resample(snd, &before_resample, fmt.sample_rate);

    return "";
}

#else

#error sound_save() and sound_load() are only implemented for little endian machines.

#endif

void sound_play(const sound *snd) {
    return;
}
