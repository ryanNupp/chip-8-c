#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#include <ncurses.h>
#include <unistd.h>

#include "chip8.h"
#include "init.h"
#include "instructions.h"
#include "term_disp.h"

// instruction decode macros
#define OP(ins) ((ins & 0xF000) >> 12)
#define X(ins) ((ins & 0x0F00) >> 8)
#define Y(ins) ((ins & 0x00F0) >> 4)
#define N(ins) (ins & 0x000F)
#define NN(ins) (ins & 0x00FF)
#define NNN(ins) (ins & 0x0FFF)

typedef struct timespec timespec;

int fetch_decode_execute(Chip8*);
void timespec_sum(timespec*, timespec*, timespec*);
bool timespec_less(timespec*, timespec*);

int main(int argc, char ** argv) {
    // check if file arg is present
    if (argc != 2) {
        printf("Usage: chip8emu /path/to/rom\n");
        return EXIT_FAILURE;
    }

    // open rom file
    int rom = open(argv[1], O_RDWR);
    if (rom == -1) {
        printf("ERROR: Incorrect file path\n");
        return EXIT_FAILURE;    
    }
    
    // initialize chip 8 emulator
    Chip8 *emu = new_chip8(rom);
    
    // initialize terminal display
    term_disp_init();
    
    // main fetch / decode / execute loop
    int rtn = fetch_decode_execute(emu);

    // stop terminal display, free memory allocated for emulator
    term_disp_end();
    free(emu);

    // check if error occured during fetch / decode / execute
    if (rtn > 1) {
        printf("ERROR - %d", rtn);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// fetch, decode & execute loop
// return codes:
// 0 - successful completion
// 1 - stack overflow
// 2 - stack underflow
int fetch_decode_execute(Chip8 *emu) {
    timespec inst_cycle_time = { 0, 1000000000 / emu->inst_per_sec };
    timespec cycle_60hz_time = {0, 1000000000 / 60};

    // initiate next cycle times
    timespec inst_cycle_next, cycle_60hz_next, curr_time;
    clock_gettime(CLOCK_MONOTONIC, &curr_time); // curr time
    timespec_sum(&curr_time, &inst_cycle_time, &inst_cycle_next); // cycle time
    timespec_sum(&curr_time, &cycle_60hz_time, &cycle_60hz_next); //    + curr time

    // display width / height
    int disp_x, disp_y;

    while (emu->program_counter < 0xFFF) {
        // FETCH
        unsigned _BitInt(16) curr_ins
            = emu->memory[emu->program_counter] * 0x100
            + emu->memory[emu->program_counter + 1];
        emu->program_counter += 2;
        
        // DECODE & EXECUTE
        switch (OP(curr_ins)) {
        case 0x0:
            switch (NNN(curr_ins)) {
            case 0x0E0: // 00E0
                disp_clear(emu);
                //print_display_full(emu);
                break;

            case 0x0EE: // 00EE
                subroutine_return(emu);
                break;
            }
            break;

        case 0x1: // 1NNN
            jump(emu, NNN(curr_ins));
            break;

        case 0x2: // 2NNN
            subroutine_call(emu, NNN(curr_ins));
            break;

        case 0x3: // 3XNN
            skip_equal_const(emu, X(curr_ins), NN(curr_ins));
            break;

        case 0x4: // 4XNN
            skip_not_equal_const(emu, X(curr_ins), NN(curr_ins));
            break;

        case 0x5: // 5XY0
            skip_equal(emu, X(curr_ins), Y(curr_ins));
            break;

        case 0x6: // 6XNN
            set_const(emu, X(curr_ins), NN(curr_ins));
            break;

        case 0x7: // 7XNN
            add_const(emu, X(curr_ins), NN(curr_ins));
            break;

        case 0x8:
            switch (N(curr_ins)) {
            case 0x0: // 8XY0
                set(emu, X(curr_ins), Y(curr_ins));
                break;
            case 0x1: // 8XY1
                bitwise_or(emu, X(curr_ins), Y(curr_ins));
                break;
            case 0x2: // 8XY2
                bitwise_and(emu, X(curr_ins), Y(curr_ins));
                break;
            case 0x3: // 8XY3
                bitwise_xor(emu, X(curr_ins), Y(curr_ins));
                break;
            case 0x4: // 8XY4
                add(emu, X(curr_ins), Y(curr_ins));
                break;
            case 0x5: // 8XY5
                subtract_x_y(emu, X(curr_ins), Y(curr_ins));
                break;
            case 0x6: // 8XY6
                bitwise_shift_right(emu, X(curr_ins), Y(curr_ins));
                break;
            case 0x7: // 8XY7
                subtract_y_x(emu, X(curr_ins), Y(curr_ins));
                break;
            case 0xE: // 8XYE
                bitwise_shift_left(emu, X(curr_ins), Y(curr_ins));
                break;
            }
            break;

        case 0x9: // 9XY0
            skip_not_equal(emu, X(curr_ins), Y(curr_ins));
            break;

        case 0xA: // ANNN
            set_index(emu, NNN(curr_ins));
            break;

        case 0xB: // BNNN
            jump_offset(emu, NNN(curr_ins));
            break;

        case 0xC: // CXNN
            gen_rand(emu, X(curr_ins), NN(curr_ins));
            break;

        case 0xD: // DXYN
            draw(emu, X(curr_ins), Y(curr_ins), N(curr_ins));
            break;

        case 0xE:
            switch (NN(curr_ins)) {
            case 0x9E: // EX9E
                // skip if key pressed == Vx (lowest four bits)
                break;
            case 0xA1: // EXA1
                // skip if key pressed != Vx (lowest four bits)
                break;
            }
            break;

        case 0xF:
            switch (NN(curr_ins)) {
            case 0x07: // FX07
                get_delay(emu, X(curr_ins));
                break;
            case 0x0A: // FX0A
                 // key op get key
                break;
            case 0x15: // FX15
                delay_timer(emu, X(curr_ins));
                break;
            case 0x18: // FX18
                sound_timer(emu, X(curr_ins));
                break;
            case 0x1E: // FX1E
                add_index(emu, X(curr_ins));
                break;
            case 0x29: // FX29
                sprite_index(emu, X(curr_ins));
                break;
            case 0x33: // FX33
                bcd(emu, X(curr_ins));
                break;
            case 0x55: // FX55
                reg_dump(emu, X(curr_ins));
                break;
            case 0x65: // FX65
                reg_load(emu, X(curr_ins));
                break;
            }
            break;
        }

        // busy loop awaiting instruction cycle to end
        // take advantage of busy loop time to check on timers
        // with longer cycles...
        for (;;) {
            // get current time
            clock_gettime(CLOCK_MONOTONIC, &curr_time);

            // move onto next cycle once inst_cycle_next time is hit
            if (!timespec_less(&curr_time, &inst_cycle_next)) {
                timespec_sum(&inst_cycle_next, &inst_cycle_time, &inst_cycle_next);
                break;
            }

            // process end of 60hz cycle
            if (!timespec_less(&curr_time, &cycle_60hz_next)) {
                // display
                term_disp_print(emu, &disp_y, &disp_x);

                // decrement sound timer if above 0
                if (emu->sound_timer > 0) {
                    emu->sound_timer--;
                }

                // decrement delay timer if above 0
                if (emu->delay_timer > 0) {
                    emu->delay_timer--;
                }

                // set 60hz cycle timers for the next 60hz cycle
                timespec_sum(&cycle_60hz_next, &cycle_60hz_time, &cycle_60hz_next);
            }
        }
    }

    return 0;
}

// Take sum of time1 and time2 and place result into sum
void timespec_sum(timespec *time1, timespec *time2, timespec *sum) {
    // add sec and nsec from both times
    time_t sec = time1->tv_sec + time2->tv_sec;
	time_t nsec = time1->tv_nsec + time2->tv_nsec;

    // move nsec exceeding 999999999 over to sec
    sec += nsec / 1000000000;
	nsec %= 1000000000;

    // place result in sum
    sum->tv_sec  = sec;
    sum->tv_nsec = nsec;
}

// return true if time1 < time2
bool timespec_less(timespec *time1, timespec *time2) {
    return  (time1->tv_sec < time2->tv_sec) || 
			(time1->tv_nsec < time2->tv_nsec && time1->tv_sec == time2->tv_sec);
}