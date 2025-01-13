#pragma once

#include "chip8.h"

// chip 8 initialization
struct Chip8* new_chip8(int);

// chip 8 configuration
void config_timing(Chip8 *emu, int val);
void config_shift(struct Chip8 *emu, bool val);
void config_jump_offset(struct Chip8 *emu, bool val);
void config_store_load_inc(struct Chip8 *emu, bool val);