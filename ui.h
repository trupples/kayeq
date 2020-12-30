/** \file ui.h
 * tata
 */

#ifndef INCLUDED_UI_H
#define INCLUDED_UI_H

#include "eq.h" // equalizer

/** \defgroup Color virtual terminal escape sequences
 * \{
 */
#define FDCYAN  "\x1b[38;2;0;170;170m"    /**< Foreground dark cyan */
#define FCYAN   "\x1b[38;2;85;255;255m"   /**< Foreground cyan */
#define FWHITE  "\x1b[38;2;255;255;255m"  /**< Foreground white */
#define FDGREEN "\x1b[38;2;0;170;0m"      /**< Foreground dark green */
#define FGREEN  "\x1b[38;2;85;255;85m"    /**< Foreground green */
#define FBROWN  "\x1b[38;2;170;85;0m"     /**< Foreground brown */
#define FGRAY   "\x1b[38;2;128;128;128m"  /**< Foreground middle gray */
#define FDGRAY  "\x1b[38;2;85;85;85m"     /**< Foreground dark gray */
#define FRED    "\x1b[38;2;255;85;85m"    /**< Foreground red */

#define BBLACK "\x1b[48;2;0;0;0m"         /**< Background black */
#define BDGRAY "\x1b[48;2;85;85;85m"      /**< Background dark gray */
#define BWHITE "\x1b[48;2;255;255;255m"   /**< Background white */
/** \}
 */

/** \brief Initialise console UI module.
 *
 *  Sets I/O charset, buffering rules, switches to alternate buffer, sets title.
 */
void ui_init();

/** \brief Reset terminal back to a sane normal state. */
void ui_reset();

/** \brief Flushes everything that was drawn using ui_ functions to the screen.
 *
 *  Until this function is called, the displayed graphics may not change. This is used instead of no
 *  buffering or line buffering to prevent flicker.
 */
void ui_to_screen();

/** \brief Clears the screen, moves cursor to top left. */
void ui_clean();

/** \defgroup Graphical widget implementations.
 * \{
 */

/** \brief Clears the area of the screen that contains the frequency response curves. */
void ui_clear_curves();

/** \brief Draws a curve.
 *
 *  \param[in] curve  Array of doubles in [LOGAIN; HIGAIN] to be drawn.
 *  \param[in] color  String of some terminal escape sequences to be run before drawing the curve.
 */
void ui_curve(const double curve[NFREQ], const char *color);

/** \brief Draws the curve scale, with values in [LOGAIN; HIGAIN] */
void ui_scale();

/** \brief Draws cursor on the currently selected frequency, as well as the cursor tooltip below.
 *
 *  \param[in] eq
 *  \param[in] cursor_pos
 *  \param[in] overall_db
 */
void ui_cursor(const equalizer *eq, int cursor_pos, double overall_db);

/** \brief Draws the list of keyboard shortcuts. */
void ui_options();

/** \brief Draws the two-line status bar in the bottom right.
 *
 *  \param[in] filename  First line of the status bar. Should be limited to 35 printable chars.
 *  \param[in] status    Second line of the status bar. Should be limited to 35 printable chars.
 */
void ui_status(const char *filename, const char *status);
/** \}
 */

/** \defgroup User input functions.
 * \{
 */

/** \brief Displays KayEQ logo and an input box with a programmable prompt and error message.
 *
 *  \param[in]  prompt   Prompt string to display.
 *  \param[in]  error    Error string to display.
 *  \param[out] input    Pointer to buffer to store truncated user input to.
 *  \param[in]  maxsize  Size of input buffer, including null terminator.
 */
void ui_prompt(const char *prompt, const char *error, char *input, int maxsize);

/** \brief Reads a character from stdin, or immediately return 0 if there are none queued up.
 *
 *  \return Read byte value, or 0 if no data was ready to be read.
 */
char ui_getchar_nonblocking();

/** \}
 */

#endif // INCLUDED_UI_H
