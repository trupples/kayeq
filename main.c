/** \file main.c
 *  \brief The main file implements the application logic of KayEQ, passing data between the other
 *         modules.
 *
 *  \author Dragomir Ioan (trupples)
 *  \author Dan Cristian
 */

#include <stdbool.h>
#include <stdio.h>
#include <time.h>    // clock, clock_t
#include <string.h>
#include <windows.h>

#include "sound.h"
#include "eq.h"
#include "eqmath.h"
#include "ui.h"

/** \brief Animates an input string to scroll over time within another fixed size string.
 *
 *  If the source string is shorter than the destination, no animation is done. Else, the source
 *  string padded with 4 spaces is scrolled with wrap-around based on clock().
 *
 * \param[in] src       Source string
 * \param[out] dst      Destination string
 * \param[in] dst_size  Usable length of dst
 */
void marquee(const char *src, char *dst, const int dst_size) {
    const int src_len = strlen(src);
    if(src_len < dst_size) {
        strcpy(dst, src);
        return;
    }

    const int marquee_len = src_len + 4;
    const double MARQUEE_SPEED = 2.0;
    const int startj = (int) (clock() * MARQUEE_SPEED / CLOCKS_PER_SEC) % marquee_len;

    for(int i = 0; i < dst_size; i++) {
        const int j = (startj + i) % marquee_len;
        if(j < src_len)
            dst[i] = src[j];
        else
            dst[i] = ' ';
    }
}

/** \brief Sleeps until clock() ticks a given amount of time.
 *
 *  \param[in] delay  time to sleep * CLOCKS_PER_SEC
 */
void sleep(clock_t delay) {
   const clock_t when = clock() + delay;
   while(clock() < when) Sleep(0);
}

char scrolling_filename[36] = { 0 };    /**< \brief Will receive the scrolling input_filename. */

/** \brief Used during audio processing, which is slow, to draw a progress bar in the bottom right.
 *
 *  \param[in] progress  A double from 0.0 to 1.0 representing the current progress.
 */
void progress_callback(double progress) {
    const int total_halfbars = 70;
    const int halfbars = progress * total_halfbars;
    char loading_bar[35 * 4] = { '\0' };
    int i = 0;

    while(i < halfbars / 2) {
        strcat(loading_bar, "█");
        i++;
    }

    if(halfbars % 2 == 1) {
        strcat(loading_bar, "▌");
        i++;
    }

    while(i < total_halfbars / 2) {
        strcat(loading_bar, " ");
        i++;
    }

    ui_status(scrolling_filename, loading_bar);
    ui_to_screen();
}

int main() {
    bool running = true;                /**< \brief Set true until the user chooses to exit the
                                                    program. */

    char input_filename[67] = { 0 };    /**< \brief Empty string means no file is loaded. */
    sound input_sound = { 0 };          /**< \brief Sound loaded from file given by input_filename. */
    char *input_error = "";             /**< \brief Describes error of last sound_load() call. */

    char output_filename[67] = { 0 };   /**< \brief Filename to output to. */
    sound output_sound = { 0 };         /**< \brief Filtered sound. */

    int cursor_pos = 0;                 /**< \brief 0..(NFREQ-1); Selected frequency index. */

    equalizer eq;                       /**< \brief Container for the equalizer state. */

    eq_init(&eq);
    eqmath_init(&eq);
    ui_init();

    while(running) {
        // If no file is loaded, display the prompt.
        if(input_filename[0] == '\0') {
            ui_prompt("Input wav file", input_error, input_filename, sizeof(input_filename));
            input_error = sound_load(&input_sound, input_filename);
            if(input_error[0] != '\0') {
                input_filename[0] = '\0';
            }
            continue;
        }

        marquee(input_filename, scrolling_filename, 35);

        // Draw UI elements
        ui_options();
        ui_scale();
        ui_status(scrolling_filename, "");

        // Calculate curves to be drawn & convert gain to dB
        double selected_curve[NFREQ] = { 0 };
        eqmath_one_frequency_response(&eq, selected_curve, cursor_pos);
        for(int i = 0; i < NFREQ; i++)
            selected_curve[i] = eqmath_gain_to_db(selected_curve[i]);

        double overall_curve[NFREQ] = { 0 };
        eqmath_overall_frequency_response(&eq, overall_curve);
        for(int i = 0; i < NFREQ; i++)
            overall_curve[i] = eqmath_gain_to_db(overall_curve[i]);

        // Draw frequency response curves and cursor
        ui_clear_curves();
        ui_cursor(&eq, cursor_pos, overall_curve[cursor_pos]);
        ui_curve(selected_curve, FGRAY);
        ui_curve(overall_curve, FWHITE);

        // Flush all UI to screen
        ui_to_screen();

        // Process user input
        char command = ui_getchar_nonblocking();
        if(command >= 'a' && command <= 'z') command -= 32;
        switch(command) {
        case 'O': { // [O] Open
            input_filename[0] = '\0';   // Reset filename so we reload next iteration
            break;
        }
        case 'S': { // [S] Save
            ui_status(scrolling_filename, "Processing...");
            ui_clear_curves();
            ui_cursor(&eq, cursor_pos, overall_curve[cursor_pos]);
            ui_curve(selected_curve, FGRAY);
            ui_curve(overall_curve, FWHITE);
            ui_to_screen();

            eqmath_process(&eq, &input_sound, &output_sound, progress_callback);

            ui_prompt("Output wav file (empty for playback)", "", output_filename,
                      sizeof(output_filename));

            if(output_filename[0] == '\0') {
                sound_play(&output_sound);
            } else {
                sound_save(&output_sound, output_filename);
            }

            break;
        }
        case '0':   // [0-9] Q factor
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            eq_set_q_option(&eq, cursor_pos, command - '0');
            break;
        }
        case '\x1b': {  // [↔] Frequency   [↕] Gain
            // escape sequence "\x1b[A/B/C/D" for up/down/right/left arrow keys
            const char bracket = getchar();
            const char arrow = getchar();
            if(bracket != '[') break;

            if(arrow == 'A') eq_change_gain(&eq, cursor_pos, +1);
            if(arrow == 'B') eq_change_gain(&eq, cursor_pos, -1);
            if(arrow == 'C') if(cursor_pos < 74) cursor_pos++;
            if(arrow == 'D') if(cursor_pos > 0) cursor_pos--;
            break;
        }
        case 'Q': { // [Q] Quit
            running = false;
            break;
        }
        default:
            break;
        }

        sleep(0.01 * CLOCKS_PER_SEC);
    }

    sound_delete(&input_sound);
    sound_delete(&output_sound);

    ui_reset();

    return 0;
}
