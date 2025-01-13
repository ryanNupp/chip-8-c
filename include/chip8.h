#pragma once

typedef struct Chip8 {
    // display
    unsigned _BitInt(64) display[32];

    // memory, counter & registers
    unsigned _BitInt(8) memory[4096];
    unsigned _BitInt(12) program_counter;
    unsigned _BitInt(16) index_register;
    unsigned _BitInt(8) var_regs[16];

    // timers
    unsigned _BitInt(8) delay_timer;
    unsigned _BitInt(8) sound_timer;

    // stack
    unsigned _BitInt(16) stack[16];
    int stack_top;

    // configuration
    int inst_per_sec;
    bool shift_use_vy;
    bool jump_offset_vx;
    bool store_load_i_inc;
} Chip8 ;