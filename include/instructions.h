#pragma once

#include "chip8.h"

// display
void disp_clear(Chip8*);
void draw(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4), unsigned _BitInt(4));

// flow
void jump(Chip8*, unsigned _BitInt(12));
void jump_offset(Chip8*, unsigned _BitInt(12));
int  subroutine_call(Chip8*, unsigned _BitInt(12));
int  subroutine_return(Chip8*);

// cond
void skip_equal_const(Chip8*, unsigned _BitInt(4), unsigned _BitInt(8));
void skip_not_equal_const(Chip8*, unsigned _BitInt(4), unsigned _BitInt(8));
void skip_equal(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));
void skip_not_equal(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));

// const
void set_const(Chip8*, unsigned _BitInt(4), unsigned _BitInt(8));
void add_const(Chip8*, unsigned _BitInt(4), unsigned _BitInt(8));

// assig
void set(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));

// bitop
void bitwise_or(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));
void bitwise_and(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));
void bitwise_xor(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));
void bitwise_shift_right(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));
void bitwise_shift_left(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));

// math
void add(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));
void subtract_x_y(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));
void subtract_y_x(Chip8*, unsigned _BitInt(4), unsigned _BitInt(4));

// mem
void set_index(Chip8*, unsigned _BitInt(12));
void add_index(Chip8*, unsigned _BitInt(4));
void sprite_index(Chip8*, unsigned _BitInt(4));
void reg_dump(Chip8*, unsigned _BitInt(4));
void reg_load(Chip8*, unsigned _BitInt(4));

// rand
void gen_rand(Chip8*, unsigned _BitInt(4), unsigned _BitInt(8));

// keyop
// -
// -
// -

// timer
void delay_timer(Chip8*, unsigned _BitInt(4));
void get_delay(Chip8*, unsigned _BitInt(4));

// sound
void sound_timer(Chip8*, unsigned _BitInt(4));

// bcd
void bcd(Chip8*, unsigned _BitInt(4));
