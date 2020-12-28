#ifndef INCLUDED_UI_H
#define INCLUDED_UI_H

#include "eq.h"

#define FDCYAN  "\x1b[38;2;0;170;170m"
#define FCYAN   "\x1b[38;2;85;255;255m"
#define FWHITE  "\x1b[38;2;255;255;255m"
#define FDGREEN "\x1b[38;2;0;170;0m"
#define FGREEN  "\x1b[38;2;85;255;85m"
#define FBROWN  "\x1b[38;2;170;85;0m"
#define FGRAY   "\x1b[38;2;128;128;128m"
#define FDGRAY  "\x1b[38;2;85;85;85m"
#define FRED    "\x1b[38;2;255;85;85m"

#define BBLACK "\x1b[48;2;0;0;0m"
#define BDGRAY "\x1b[48;2;85;85;85m"
#define BWHITE "\x1b[48;2;255;255;255m"

void ui_init();
void ui_reset();
void ui_to_screen();

void ui_clear_curves(const char *bg);
void ui_curve(double curve[NFREQ], const char *color);
void ui_prompt(const char *prompt, const char *error, char *input, int maxsize);
void ui_scale();
void ui_cursor(equalizer *eq, int cursor_pos, double overall_db);
void ui_options();
void ui_status(const char *filename, const char *status);

#endif // INCLUDED_UI_H
