#include <locale.h>
#include <ncurses.h>

#include "term_disp.h"
#include "chip8.h"

// initialize ncurses for display
void term_disp_init() {
    setlocale(LC_ALL, "en_US.UTF-8");
    initscr();
    curs_set(0);
    cbreak();
    noecho();
    nodelay(stdscr, true);
    scrollok(stdscr, true);
}

// end ncurses display
void term_disp_end() {
    endwin();
}

// print display
void term_disp_print(Chip8 *emu, int *prev_y, int *prev_x) {
    int curr_y = getmaxy(stdscr);
    int curr_x = getmaxx(stdscr);

    // window has been resized since last frame, clear the display first
    if (*prev_y != curr_y || *prev_x != curr_x) {
        clear();
        *prev_y = curr_y;
        *prev_x = curr_x;
    }
    
    move(0, 0);
    if (curr_y > 32 && curr_x > 128) {
        print_display_full(emu);
    }
    else if (curr_y > 16 && curr_x > 64) {
        print_display_half(emu);
    }
    else {
        clear();
        printw("Terminal window size is too small.");
        refresh();
        return;
    }
    printw("Beep: %ls        ", emu->sound_timer > 0 ? L"\u2588\u2588\u2588\u2588" : L"----");
    printw("Sound Timer: %d        ", emu->sound_timer);
    printw("Delay Timer: %d", emu->delay_timer);
    //printw("y: %d - x: %d", curr_y, curr_x);
    refresh();
}

// print display (2 char-width per pixel - two full blocks)
void print_display_full(Chip8 *emu) {
    //setlocale(LC_ALL, "en_US.UTF-8");

    unsigned _BitInt(64) mask;
    size_t column;

    for (int i=0; i<32; i++) {
        mask = 0x8000000000000000;
        column = 0;
        while (mask > 0) {
            printw("%ls", (emu->display[i] & mask) ? L"\u2588\u2588" : L"  ");
            mask/=2;
            column+=2;
        }
        printw("\n");
    }
}

// print display (two pixels per char - top & bottom with half blocks)
void print_display_half(Chip8 *emu) {
    unsigned _BitInt(64) mask;
    size_t column;

    for (int i=0; i<32; i+=2) {
        mask = 0x8000000000000000;
        column = 0;
        while (mask > 0x0) {
            bool top_pixel = emu->display[i] & mask;
            bool bottom_pixel = emu->display[i+1] & mask;

            wchar_t printchar = L' ';
            if (top_pixel || bottom_pixel) {
                printchar = L'\u2588';
                if (!top_pixel)
                    printchar -= 4;
                else if (!bottom_pixel)
                    printchar -= 8;
            }

            printw("%lc", printchar);

            mask/=2;
            column++;
        }
        printw("\n");
    }
    refresh();
}