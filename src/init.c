#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "init.h"

////////////////////////////////////////////////////////////
//                       Chip8 Init                       //
////////////////////////////////////////////////////////////

// initialize a new chip8 given rom file descriptor
Chip8* new_chip8(int rom_fd) {
    Chip8 *emu = (Chip8*)malloc(sizeof(Chip8));

    unsigned _BitInt(8) font[] = {
        0x60, 0xB0, 0xD0, 0x90, 0x60,   // 0
        0x20, 0x60, 0x20, 0x20, 0x70,   // 1
        0x60, 0x90, 0x20, 0x40, 0xF0,   // 2
        0xE0, 0x10, 0x60, 0x10, 0xE0,   // 3
        0x20, 0x60, 0xA0, 0xF0, 0x20,   // 4
        0xF0, 0x80, 0xE0, 0x10, 0xE0,   // 5
        0x60, 0x80, 0xE0, 0x90, 0x60,   // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
        0x60, 0x90, 0x60, 0x90, 0x60,   // 8
        0x60, 0x90, 0x70, 0x10, 0x60,   // 9
        0x60, 0x90, 0xF0, 0x90, 0x90,   // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
        0x70, 0x80, 0x80, 0x80, 0x70,   // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
        0xF0, 0x80, 0xE0, 0x80, 0xF0,   // E
        0xF0, 0x80, 0xE0, 0x80, 0x80    // F
    };
    
    // initialize font in memory
    for (int i=0; i<80; i++) {
        emu->memory[i+0x50] = font[i];
    }

    // initialize program counter to 0x200
    emu->program_counter = 0x200;

    // initialize stack
    emu->stack_top = -1;

    // config defaults
    emu->inst_per_sec = 700;

    // srand
    srand(time(NULL));

    // read & load rom to memory (starting at address 0x200)
    unsigned _BitInt(8) buffer;
    unsigned _BitInt(12) loc = 0x200;
    while (read(rom_fd, &buffer, 1) == 1 && loc <= 0xFFF) {
        emu->memory[loc] = buffer;
        loc++;
    }

    return emu;
}


////////////////////////////////////////////////////////////
//                      Chip8 Config                      //
////////////////////////////////////////////////////////////

// Configure the timing
// - How many instructions per second?
void config_timing(Chip8 *emu, int val) {
    emu->inst_per_sec = val;
}

// Configure shift behavior
// 0. Set VX to value of VY, then shift
// 1. Ignore VY, shift existing VX value
void config_shift(Chip8 *emu, bool val) {
    emu->shift_use_vy = val;
}

// Configure jump with offset behavior
// 0. BNNN - jump to memory[NNN] + V0
// 1. BXNN - jump to memory[XNN] + VX
void config_jump_offset(Chip8 *emu, bool val) {
    emu->jump_offset_vx = val;
}

// Configure index register incrementation behavior
// 0. Don't increment index register during store and load instructions
// 1. Do increment index register during store and load instructions
void config_store_load_inc(Chip8 *emu, bool val) {
    emu->store_load_i_inc = val;
}