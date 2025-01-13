#include <stdlib.h>
#include <string.h>

#include "chip8.h"
#include "instructions.h"

////////////////////////////////////////////////////////////
//                         Display                        //
////////////////////////////////////////////////////////////

// 00E0 : Clear Screen - sets all display bits to 0
void disp_clear(Chip8 *emu) {
    memset(emu->display, 0, 32*sizeof(unsigned _BitInt(64)));
}

// DXYN : Display instruction - draws a sprite to the screen
// x - register number that holds the X coordinate
// y - register number that holds the Y coordinate
// n - number of rows the sprite takes up (1 to 16 rows, represented with 0-15)
void draw(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y, unsigned _BitInt(4) n) {
    size_t x_coord = emu->var_regs[x] & 63;
    size_t y_coord = emu->var_regs[y] & 31;
    unsigned _BitInt(64) sprite_row;
    unsigned _BitInt(64) or;

    // initialize flag reg VF to 0
    emu->var_regs[15] = 0;

    for (size_t i=0; i<n; i++) {
        sprite_row = emu->memory[emu->index_register+i];
        int shift = 56 - x_coord;
        if (shift >= 0) {
            sprite_row <<= shift;
        } else {
            sprite_row >>= shift * -1;
        }
        
        // apply changes, flag VF=1 if collision
        or = emu->display[y_coord+i] | sprite_row;
        emu->display[y_coord+i] ^= sprite_row;
        if (emu->display[y_coord+i] != or) {
            emu->var_regs[15] = 1;
        }
    }
}


////////////////////////////////////////////////////////////
//                          Flow                          //
////////////////////////////////////////////////////////////

// 1NNN : Jump - Set program counter
// n - value to set the program counter to
void jump(Chip8 *emu, unsigned _BitInt(12) n) {
    emu->program_counter = n;
}

// BNNN : Jump with offset - Set program counter (with offset)
void jump_offset(Chip8 *emu, unsigned _BitInt(12) n) {
    int x = 0;
    if (emu->jump_offset_vx) {
        x = (n & 0xF00) >> 8;
    }
    emu->program_counter = n + emu->var_regs[x];
}

// 2NNN : Calls subroutine at NNN
// Push program counter to top of stack, set program counter to n
// return 0 - successful completion
// return 1 - stack overflow
int subroutine_call(Chip8 *emu, unsigned _BitInt(12) n) {
    if (emu->stack_top >= 15) {
        return 1; // stack overflow
    }

    emu->stack[++emu->stack_top] = emu->program_counter;
    emu->program_counter = n;
    return 0;
}

// 00EE : Return from subroutine
// Set program counter to top of stack, pop stack
// return 0 - successful completion
// return 1 - stack underflow
int subroutine_return(Chip8 *emu) {
    if (emu->stack_top < 0) {
        return 1;
    }

    emu->program_counter = emu->stack[emu->stack_top--];
    return 0;
}


////////////////////////////////////////////////////////////
//                          Cond                          //
////////////////////////////////////////////////////////////

// 3XNN : (constant) If equal, skip next instruction
//        Compare register Vx against value n
// x - register number for Vx
// n - value to check against Vx
void skip_equal_const(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(8) n) {
    if (emu->var_regs[x] == n) {
        emu->program_counter += 2;
    }
}

// 4XNN : (constant) If not equal, skip next instruction
//        Compare register Vx against value n
// x - register number for Vx
// n - value to check against Vx
void skip_not_equal_const(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(8) n) {
    if (emu->var_regs[x] != n) {
        emu->program_counter += 2;
    }
}

// 5XY0 : If equal, skip next instruction
//        Compare register Vx against Vy
// x - register number for Vx
// y - register number for Vy
void skip_equal(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    if (emu->var_regs[x] == emu->var_regs[y]) {
        emu-> program_counter += 2;
    }
}

// 9XY0 : If not equal, skip next instruction
//        Compare register Vx against Vy
// x - register number for Vx
// y - register number for Vy
void skip_not_equal(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    if (emu->var_regs[x] != emu->var_regs[y]) {
        emu-> program_counter += 2;
    }
}


////////////////////////////////////////////////////////////
//                          Const                         //
////////////////////////////////////////////////////////////

// 6XNN : Set (constant)
// x - register to set
// n - value to set the register to
void set_const(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(8) n) {
    emu->var_regs[x] = n;
}

// 7XNN : Add (constant)
// x - register to add to
// n - value to add to the register
void add_const(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(8) n) {
    emu->var_regs[x] += n;
}


////////////////////////////////////////////////////////////
//                          Assig                         //
////////////////////////////////////////////////////////////

// 8XY0 : Set value of Vx = Vy
// x - register number for Vx
// y - register number for Vy
void set(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    emu->var_regs[x] = emu->var_regs[y];
}


////////////////////////////////////////////////////////////
//                          BitOp                         //
////////////////////////////////////////////////////////////

// 8XY1
void bitwise_or(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    emu->var_regs[x] |= emu->var_regs[y];
}

// 8XY2
void bitwise_and(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    emu->var_regs[x] &= emu->var_regs[y];
}

// 8XY3
void bitwise_xor(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    emu->var_regs[x] ^= emu->var_regs[y];
}

// 8XY6
void bitwise_shift_right(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    if (emu->shift_use_vy) {
        emu->var_regs[x] = emu->var_regs[y];
    }
    unsigned _BitInt(4) flag = emu->var_regs[x] & 0x01;
    emu->var_regs[x] >>= 1;
    emu->var_regs[0xF] = flag;
}

// 8XYE
void bitwise_shift_left(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    if (emu->shift_use_vy) {
        emu->var_regs[x] = emu->var_regs[y];
    }
    unsigned _BitInt(4) flag = (emu->var_regs[x] & 0x80) >> 7;
    emu->var_regs[x] <<= 1;
    emu->var_regs[0xF] = flag;
}


////////////////////////////////////////////////////////////
//                          Math                          //
////////////////////////////////////////////////////////////

// 8XY4
void add(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    int test1 = emu->var_regs[x];
    int test2 = emu->var_regs[y];

    emu->var_regs[x] += emu->var_regs[y];

    // mark VF=1 if overflow occurred
    int flag = 0;
    if (emu->var_regs[x] != test1 + test2) {
        flag = 1;
    }
    emu->var_regs[0xF] = flag;
}

// 8XY5
void subtract_x_y(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    int test1 = emu->var_regs[x];
    int test2 = emu->var_regs[y];
    
    emu->var_regs[x] -= emu->var_regs[y];

    // mark VF=1 if no underflow occured
    int flag = 0;
    if (emu->var_regs[x] == test1 - test2) {
        flag = 1;
    }
    emu->var_regs[0xF] = flag;
}

// 8XY7
void subtract_y_x(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(4) y) {
    int test1 = emu->var_regs[x];
    int test2 = emu->var_regs[y];
    
    emu->var_regs[x] = emu->var_regs[y] - emu->var_regs[x];

    // mark VF=1 if no underflow occured
    int flag = 0;
    if (emu->var_regs[x] == test2 - test1) {
        flag = 1;
    }
    emu->var_regs[0xF] = flag;
}

////////////////////////////////////////////////////////////
//                           MEM                          //
////////////////////////////////////////////////////////////

// ANNN : Set index register
// n - value index register will be set to
void set_index(Chip8 *emu, unsigned _BitInt(12) n) {
    emu->index_register = n;
}

// FX1E : Add Vx to I - VF not affected
void add_index(Chip8 *emu, unsigned _BitInt(4) x) {
    emu->index_register += emu->var_regs[x];
}

// FX29 : Set index register to memory location for sprite character that represents value in Vx
void sprite_index(Chip8 *emu, unsigned _BitInt(4) x) {
    emu->index_register = 0x050 + (emu->var_regs[x] * 5);
}

// FX55 : register dump V0-Vx into memory, starting at location I
void reg_dump(Chip8 *emu, unsigned _BitInt(4) x) {
    for (int i=0; i<=x; i++) {
        emu->memory[emu->index_register+i] = emu->var_regs[i];
    }
    if (emu->store_load_i_inc) {
        emu->index_register += x + 1;
    }
}

// FX65 : register load V0-Vx from memory, starting at location I
void reg_load(Chip8 *emu, unsigned _BitInt(4) x) {
    for (int i=0; i<=x; i++) {
        emu->var_regs[i] = emu->memory[emu->index_register+i];
    }
    if (emu->store_load_i_inc) {
        emu->index_register += x + 1;
    }
}

////////////////////////////////////////////////////////////
//                          Rand                          //
////////////////////////////////////////////////////////////

// CXNN : Random
void gen_rand(Chip8 *emu, unsigned _BitInt(4) x, unsigned _BitInt(8) n) {
    emu->var_regs[x] = rand() & n;
}


////////////////////////////////////////////////////////////
//                          KeyOp                         //
////////////////////////////////////////////////////////////

// EX9E : Skip if key currently pressed == Vx

// EXA1 : Skip if key currently pressed != Vx

// FX0A : Await input, grab & store key pressed

////////////////////////////////////////////////////////////
//                          Timer                         //
////////////////////////////////////////////////////////////

// FX15 : Set delay timer
void delay_timer(Chip8 *emu, unsigned _BitInt(4) x) {
    emu->delay_timer = emu->var_regs[x];
}

// FX07 : Get delay timer
void get_delay(Chip8 *emu, unsigned _BitInt(4) x) {
    emu->var_regs[x] = emu->delay_timer;
}

////////////////////////////////////////////////////////////
//                          Sound                         //
////////////////////////////////////////////////////////////

// FX18 : Set sound timer
void sound_timer(Chip8 *emu, unsigned _BitInt(4) x) {
    emu->sound_timer = emu->var_regs[x];
}


////////////////////////////////////////////////////////////
//                           BCD                          //
////////////////////////////////////////////////////////////

// FX33 : Binary-coded decimal conversion
void bcd(Chip8 *emu, unsigned _BitInt(4) x) {
    emu->memory[emu->index_register]   = emu->var_regs[x] / 100;
    emu->memory[emu->index_register+1] = (emu->var_regs[x] % 100) / 10;
    emu->memory[emu->index_register+2] = emu->var_regs[x] % 10;
}