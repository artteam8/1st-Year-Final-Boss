.PHONY: all force clean

SPEC_FILE?=in.txt
SAFE_EXP?=1
USE_NEWTON?=0
USE_SIMPSON?=1

CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -m32 -O2 -DUSE_NEWTON=$(USE_NEWTON) -DUSE_SIMPSON=$(USE_SIMPSON) -DSAFE_EXP=$(SAFE_EXP) -DSPEC_FILE=\"$(SPEC_FILE)\" -DMADE_WITH_MAKE=1 -I include

all: report/report.pdf bin/main
force: ;
clean:
	rm -rf obj
	rm bin/gen
	rm src/funcs.asm

bin/main:  obj/main.o obj/numeric.o obj/funcs.o | bin
	$(CC) -m32 -lm $^ -o $@
	@if command -v figlet >/dev/null 2>&1; then figlet -f slant "main built!"; else echo "main built\n\n\n"; fi

obj/funcs.o: src/funcs.asm
	@mkdir -p $(dir $@)
	nasm -f elf32 $< -o $@

obj/numeric.o: src/numeric.c force
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

obj/main.o: src/main.c force
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

src/funcs.asm: bin/gen $(SPEC_FILE)
	tail -n +2 $(SPEC_FILE) | ./bin/gen

bin/gen: obj/rpn2asm.o obj/ast.o obj/diff_ast.o | bin
	$(CC) -m32 -lm $^ -o $@

obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

report/report.pdf: report/report.tex report/graph.png report/make_graph.png
	pdflatex -shell-escape -output-directory=report report/report.tex
	@rm report/report.aux
	@rm report/report.log
	@rm report/report.toc
	
	@if command -v figlet >/dev/null 2>&1; then figlet -f slant "report built!"; else echo "report built\n\n\n"; fi
bin:
	@mkdir -p bin
