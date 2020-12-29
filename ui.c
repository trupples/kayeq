#include "ui.h"
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200

static DWORD stdout_mode;
static HANDLE stdout_console;
static DWORD stdin_mode, normal_input_mode;
static HANDLE stdin_console;

static char banner_and_inputbox[] =
          "\n"
          "\n"
FDCYAN "                   ██╗  ██╗ █████╗ ██╗   ██╗" FCYAN "███████╗ ██████╗ \n"
FDCYAN "                   ██║ ██╔╝██╔══██╗╚██╗ ██╔╝" FCYAN "██╔════╝██╔═══██╗\n"
FDCYAN "                   █████╔╝ ███████║ ╚████╔╝ " FCYAN "█████╗  ██║   ██║\n"
FDCYAN "                   ██╔═██╗ ██╔══██║  ╚██╔╝  " FCYAN "██╔══╝  ██║▄▄ ██║\n"
FDCYAN "                   ██║  ██╗██║  ██║   ██║   " FCYAN "███████╗╚██████╔╝\n"
FDCYAN "                   ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   " FCYAN "╚══════╝ ╚══▀▀═╝ \n"
       "\n"
FWHITE "                                         ~ by trupples and Slice ~\n"
       "\n"
       "\n"
       "\n"
       "                                                  (\\\n"
       "                                                    \\" FDGREEN "_O\n"
FBROWN "                                                _____" FWHITE "\\" FDGREEN "/)" FBROWN "_____\n"
FCYAN  "     ╭~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" FBROWN "`----" FWHITE "\\" FBROWN "----'" FCYAN "~~~~~~~~~~~~~~╮\n"
       "     │ ?                                    ~~~~ ~~~ ,," FWHITE "\\" FCYAN "   ~~~~ ~~~  ~~~~ │ \n"
       "     │                                                                    │ \n"
       "     │                                                                    │ \n"
       "     │ ˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙˙ │ \n"
       "     ╰────────────────────────────────────────────────────────────────────╯ \n"
FRED   "       !                                                                    \n"
FRED   "                                                                            ";

static char *prompt_ptr = NULL;
static char *error_ptr = NULL;

static void _ui_write(const char *s) { fputs(s, stdout); }
static void _ui_gotoxy(unsigned int x, unsigned int y) { printf("\x1b[%u;%uH", y, x); }

void ui_init() {
    // Input and output UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Enable output buffering (prevents flicker); enable output sequences
    setvbuf(stdout, NULL, _IOFBF, 8192);
    stdout_console = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(stdout_console, &stdout_mode);
    stdout_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // ANSI sequence output
    SetConsoleMode(stdout_console, stdout_mode);

    // Disable input buffering; enable input sequences
    setvbuf(stdin, NULL, _IONBF, 0);
    stdin_console = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(stdin_console, &stdin_mode);
    normal_input_mode = stdin_mode;
    stdin_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT; // arrow key input as escape sequences
    stdin_mode &= ~ENABLE_LINE_INPUT; // "raw mode"
    SetConsoleMode(stdin_console, stdin_mode);

    // Remeber where the prompt and error start in banner_and_inputbox
    prompt_ptr = strchr(banner_and_inputbox, '?');
    error_ptr = strchr(banner_and_inputbox, '!');

    // Switch to alternate buffer
    _ui_write("\x1b[?1049h");

    // Set window title
    _ui_write("\x1b]2;KayEQ\x07");

    // Set Number of Columns to 80; idk just making sure...
    _ui_write("\x1b[?3l");
}

static const int GRAPH_HEIGHT = 23;

void ui_clear_curves() {
    _ui_write(BBLACK FDGRAY);
    for(int y = 1; y <= GRAPH_HEIGHT; y++) {
        _ui_gotoxy(1, y);
        _ui_write("                                                                            ");
    }
}

void ui_curve(double curve[NFREQ], const char *color) {
    _ui_write(BBLACK); _ui_write(color);
    for(int i = 0; i < NFREQ; i++) {
        int level = (int)floor((curve[i] - LOGAIN) * (GRAPH_HEIGHT*3-1) / (HIGAIN - LOGAIN));
        if(level < 0 || level > GRAPH_HEIGHT*3-1) continue;

        int x = i + 2,
            y = GRAPH_HEIGHT - level / 3,
            suby = (level + 300) % 3;

        _ui_gotoxy(x, y);
        if(suby == 2) _ui_write("˙");
        if(suby == 1) _ui_write("·");
        if(suby == 0) _ui_write(".");
    }
}

void ui_prompt(const char *prompt, const char *error, char *input, int maxsize) {
    ui_clean();
    strncpy(prompt_ptr, prompt, 36);
    for(int i = strlen(prompt_ptr); i < 36; i++) prompt_ptr[i] = ' ';
    strncpy(error_ptr, error, 66);
    for(int i = strlen(error_ptr); i < 66; i++) error_ptr[i] = ' ';

    _ui_write(banner_and_inputbox);
    _ui_gotoxy(8,20);
    _ui_write("\033[?25h\033[?12h"); // show cursor while user is typing
    _ui_write(BBLACK FWHITE);
    ui_to_screen();

    SetConsoleMode(stdin_console, normal_input_mode);
    fgets(input, maxsize-1, stdin);
    input[maxsize-1] = '\0';
    *strchr(input, '\n') = '\0';
    SetConsoleMode(stdin_console, stdin_mode);

    _ui_write("\033[?25l\033[?12l"); // hide cursor again
}

void ui_scale() {
    _ui_write(BBLACK FGRAY);
    for(int y = 0; y < GRAPH_HEIGHT; y++) {
        double db = y * (HIGAIN - LOGAIN) / (GRAPH_HEIGHT-1) + LOGAIN;
        _ui_gotoxy(77, GRAPH_HEIGHT - y);
        printf("%4d", (int)floor(db+0.5));
    }
}

void ui_cursor(equalizer *eq, int cursor_pos, double overall_db) {
    // draw vertical cursor axis
    _ui_write(BBLACK FGRAY);
    for(int y = 1; y <= GRAPH_HEIGHT; y++) {
        _ui_gotoxy(cursor_pos + 2, y);
        _ui_write("│");
    }

    // draw cursor info like [ 20000Hz +20dB Q1.8 (+20dB) ]
    char info[30] = { 0 };
    snprintf(info, sizeof(info), "[ %dHz %+ddB Q%.1f (%+ddB) ]",
            (int) round(eq->freqs[cursor_pos]),
            (int) round(eq->gain_db[cursor_pos]),
            eq_q_values[eq->q_idx[cursor_pos]],
            (int) round(overall_db));
    int startx = cursor_pos + 2 - strlen(info) / 2;
    if(startx < 1) startx = 1;
    if(startx + strlen(info) > 80) startx = 81 - strlen(info);
    _ui_gotoxy(startx, 23);
    _ui_write(FWHITE);
    _ui_write(info);
}

void ui_clean() {
    _ui_write(BBLACK FWHITE "\033[2J\033[?25h");
    _ui_gotoxy(1, 1);
}

void ui_reset() {
    _ui_write("\x1b[?1049l"); // Revert to main buffer
}

void ui_options() {
    _ui_write(BWHITE FDGRAY);
    _ui_gotoxy(1, 24); _ui_write("[O] Open       [S] Save/Play  [Q] Quit       ");
    _ui_gotoxy(1, 25); _ui_write("[↔] Frequency  [↕] Gain       [0-9] Q factor ");
}

void ui_status(const char *line1, const char *line2) {
    _ui_write(BDGRAY FCYAN);
    _ui_gotoxy(46, 24); printf("%35s", "");
    _ui_gotoxy(46, 25); printf("%35s", "");
    _ui_gotoxy(46, 24); printf("%s", line1);
    _ui_gotoxy(46, 25); printf("%s", line2);
}

void ui_to_screen() {
    fflush(stdout);
}

static bool is_keydown_event(INPUT_RECORD *inp) {
    return inp->EventType == KEY_EVENT && inp->Event.KeyEvent.bKeyDown == true;
}

char ui_getchar_nonblocking() {
    INPUT_RECORD input = { 0 };
    DWORD numEvents = 0;
    while(true) {
        PeekConsoleInput(stdin_console, &input, 1, &numEvents);
        if(numEvents == 0) return 0;
        if(!is_keydown_event(&input)) { // consume a non-keydown event
            ReadConsoleInput(stdin_console, &input, 1, &numEvents);
        } else {
            return getchar();
        }
    }
}
