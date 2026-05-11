#include <stdio.h>
#include <stdlib.h>
#include <infix_to_rpn.h>

// i am lazy to write rpn in file myself.
// i will write infix in func_infix.txt.
// and it will write rpn to func_rpn.txt
//
// credits to github.com/stephenwashington/convert for converter.

#define MAX_LEN 1000

int
main(void) {
    FILE *infix_file = fopen("func_infix.txt", "r");
    char *infix = calloc(MAX_LEN+1, sizeof(char)); // +1 for \0
    int pos = 0;

    while (fscanf(infix_file, "%c", infix + pos) == 1) {
        ++pos;
        if (pos >= MAX_LEN) break;
    }
    infix[pos-1] = '\0'; // \n on pos-1

    fclose(infix_file);

    char *rpn = infix_to_rpn(infix);
    FILE *rpn_file = fopen("func_rpn.txt", "w");
    fwrite(rpn, sizeof(char), pos * 2, rpn_file); // *2 for spaces

    free(infix);
    free(rpn);
    fclose(rpn_file);
    return 0;
}

