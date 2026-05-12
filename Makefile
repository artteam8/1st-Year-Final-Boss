.PHONY: all force_numeric

SPEC_FILE = in.txt

CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -m32 -O2 -DSPEC_FILE=\"$(SPEC_FILE)\"  -I include $(addprefix -I , $(wildcard include/*))

all: bin/main

bin/main:  obj/main.o obj/numeric.o obj/funcs.o | bin
	$(CC) -m32 $^ -o $@

obj/funcs.o: funcs.asm
	mkdir -p $(dir $@)
	nasm -f elf32 $< -o $@

obj/numeric.o: src/numeric.c force_numeric
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -DUSE_NEWTON=$(USE_NEWTON) -c $< -o $@

funcs.asm: bin/gen $(SPEC_FILE)
	tail -n +2 $(SPEC_FILE) | ./bin/gen

bin/gen: obj/rpn2asm/rpn2asm.o obj/ast.o obj/diff_ast.o | bin
	$(CC) -m32 $^ -o $@

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

bin:
	mkdir -p bin



CONVERT_SRCS := $(wildcard src/convert/*.c)
CONVERT_OBJS := $(patsubst src/%.c, obj/%.o, $(CONVERT_SRCS))

func_rpn.txt: func_infix.txt bin/infix2file
	./bin/infix2file

bin/infix2file: $(CONVERT_OBJS) | bin
	$(CC) -m32 $^ -o $@

