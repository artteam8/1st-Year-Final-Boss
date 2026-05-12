.PHONY: all

CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -O2 -I include $(addprefix -I , $(wildcard include/*))

FUNC_FILE ?= funcs.txt

all: bin/func


CONVERT_SRCS := $(wildcard src/convert/*.c)
CONVERT_OBJS := $(patsubst src/%.c, obj/%.o, $(CONVERT_SRCS))

bin/infix2file: $(CONVERT_OBJS) | bin
	$(CC) $^ -o $@


bin/gen: obj/rpn2asm/rpn2asm.o obj/ast.o | bin
	$(CC) $^ -o $@


funcs.asm: bin/gen $(FUNC_FILE)
	cat $(FUNC_FILE) | ./bin/gen >> funcs.asm

bin/func: func.asm
	nasm -f elf32 func.asm
	$(CC) -m32 -no-pie func.o -o bin/func

func_rpn.txt: func_infix.txt bin/infix2file
	./bin/infix2file


obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

bin:
	mkdir -p bin
