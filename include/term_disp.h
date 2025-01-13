#pragma once

#include "chip8.h"

void term_disp_init();
void term_disp_end();
void term_disp_print(Chip8*, int*, int*);
void print_display_full(Chip8*);
void print_display_half(Chip8*);