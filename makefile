CC 		:= gcc -D_XOPEN_SOURCE=800
CFLAGS  := -std=c23
RM      := rm -f

TARGET_EXEC := chip8emu
BUILD_DIR := ./build
INC_DIR := ./include
SRC_DIR := ./src

LIBS := -lncurses

# Find all C files we want to compile
SRCS := $(shell find $(SRC_DIR) -name '*.c')

default: all

all:
	$(CC) $(CFLAGS) -I$(INC_DIR) $(SRCS) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LIBS)

clean veryclean:
	$(RM) $(BUILD_DIR)/$(TARGET_EXEC)