#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ast.h>

#define MAX_LEN 1000

void
convert_x87(Node *node) {

    if (node == NULL) return;

    if (node->right == NULL) {
        if (node->left == NULL) { //value
            if (node->value == 'x') printf("    fld [ebp+8]\n");
            else if (node->value == 'e') printf("    fld dword[e]\n");
            else if (node->value == pseudohash("pi")) printf("    fld dword[pi]\n");
            else printf("    fld [const_%d]\n", node->number);
        } else { // unary
            convert_x87(node->left);
            if (node->value == pseudohash("sin")) printf("    fsin\n");
            else if (node->value == pseudohash("cos")) printf("    fcos\n");
            else if (node->value == pseudohash("tan")) {
                printf("    fld st0\n");
                printf("    fsin\n");
                printf("    fxch\n");
                printf("    fcos\n");
                printf("    fdivp st1, st0\n");
            } else if (node->value == pseudohash("ctg")) {
                printf("    fld st0\n");
                printf("    fcos\n");
                printf("    fxch\n");
                printf("    fsin\n");
                printf("    fdivp st1, st0\n");
            } else if (node->value == pseudohash("ln")) {
                    printf("    fldln2\n");
                    printf("    fxch\n");
                    printf("    fyl2x\n");
            }
        }
    } else { //binary
        convert_x87(node->left);
        convert_x87(node->right);
        switch(node->value) {
            case '+': printf("    faddp st1, st0\n"); break;
            case '-': printf("    fsubp st1, st0\n"); break;
            case '*': printf("    fmulp st1, st0\n"); break;
            case '/': printf("    fdivp st1, st0\n"); break;
            case '^':
                printf("    fxch\n");
                printf("    fyl2x\n");
                printf("    fld st0\n");
                printf("    frndint\n");
                printf("    fsub st1, st0\n");
                printf("    fxch\n");
                printf("    f2xm1\n");
                printf("    fld1\n");
                printf("    faddp\n");
                printf("    fscale\n");
                printf("    fstp st1\n");
                break;
        }
    }
}

void
gen_x87(Node *root) {
    printf("f:\n    push ebp\n    mov ebp, esp\n");
    convert_x87(root);
    printf("    pop ebp\n    ret\n");
}

int
main(void) {
    char **expr = calloc(MAX_LEN, sizeof(char*));
    expr[0] = calloc(10, sizeof(char));
    int idx = 0;

    while (scanf("%s", expr[idx]) == 1) {
        ++idx;
        expr[idx] = calloc(10, sizeof(char));
    }
    
    Node *root = build_ast(expr, idx);
    root = check_subtree(root);
    //print_rpn(root);
    //printf("\n");
    
    gen_x87(root);
    return 0;
}
