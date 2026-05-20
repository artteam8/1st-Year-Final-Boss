.PHONY: all force

SPEC_FILE = in.txt
SAFE_EXP=1

CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -m32 -lm -O2 -DUSE_NEWTON=$(USE_NEWTON) -DUSE_SIMPSON=$(USE_SIMPSON) -DSAFE_EXP=$(SAFE_EXP) -DSPEC_FILE=\"$(SPEC_FILE)\"  -I include

all: bin/main obj/main.o obj/numeric.o
force: ;

bin/main:  obj/main.o obj/numeric.o obj/funcs.o | bin
	$(CC) -m32 $^ -o $@

obj/funcs.o: funcs.asm
	mkdir -p $(dir $@)
	nasm -f elf32 $< -o $@

obj/numeric.o: src/numeric.c force
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/main.o: src/main.c force
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

funcs.asm: bin/gen $(SPEC_FILE)
	tail -n +2 $(SPEC_FILE) | ./bin/gen

bin/gen: obj/rpn2asm.o obj/ast.o obj/diff_ast.o | bin
	$(CC) -m32 -lm $^ -o $@

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

bin:
	mkdir -p bin
