#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <diff_ast.h>

#define MAX_LEN 1000
#define MAX_CONSTS 20
#define MAX_SIZE 100000
#define EPS 1e-6

int
find_in_list(double num, double *arr, int len) {
    for (int i = 0; i < len; ++i) {
        if (-EPS < num-arr[i] && num-arr[i] < EPS) {
            return i;
        }
    }
    return -1;
}

void
find_consts(Node *node, char *secdata_buffer, int *secdata_idx, int *const_index, double *const_list) {
    if (node == NULL || *const_index == MAX_CONSTS) return;

    if (node->left == NULL && node->right == NULL && node->value != 'x' && find_in_list(node->number, const_list, *const_index) == -1) {
        //consts[*index] = node->number;
        int written = snprintf(secdata_buffer + *secdata_idx, MAX_SIZE - *secdata_idx, "    const_%d dq %lf\n", *const_index, node->number);
        if (written < 0 || *secdata_idx + written >= MAX_LEN) {
            fprintf(stderr, "\n\nBUFFER WRITING ERROR!!!!!!!!!!!\n\n");
            exit(1);
        }
        *secdata_idx += written;
        const_list[*const_index] = node->number;
        ++(*const_index);
    } else {
        find_consts(node->left, secdata_buffer, secdata_idx, const_index, const_list);
        find_consts(node->right, secdata_buffer, secdata_idx, const_index, const_list);
    }
}

void
convert_x87(Node *node, char *write_buffer, int *buffer_idx, double *const_list, int const_num) {
    if (node == NULL) return;

    if (node->right == NULL) {
        if (node->left == NULL) { //value
            if (node->value == 'x') *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fld qword[ebp+8]\n");
            else if (node->value == 'e') *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fld qword[const_e]\n");
            else if (node->value == pseudohash("pi")) *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fld qword[const_pi]\n");
            else {
                int const_idx = find_in_list(node->number, const_list, const_num);
                *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fld qword[const_%d]\n", const_idx);
            }
        } else { // unary
            convert_x87(node->left, write_buffer, buffer_idx, const_list, const_num);
            if (node->value == pseudohash("sin")) *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fsin\n");
            else if (node->value == pseudohash("cos")) *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fcos\n");
            else if (node->value == pseudohash("tan")) {
                *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fld st0\n    fsin\n    fxch\n    fcos\n    fdivp st1, st0\n");
            } else if (node->value == pseudohash("ctg")) {
                *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fld st0\n    fcos\n    fxch\n    fsin\n    fdivp st1, st0\n");
            } else if (node->value == pseudohash("ln")) {
                    *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fldln2\n    fxch\n    fyl2x\n");
            }
        }
    } else { //binary
        convert_x87(node->left, write_buffer, buffer_idx, const_list, const_num);
        convert_x87(node->right, write_buffer, buffer_idx, const_list, const_num);
        switch(node->value) {
            case '+': *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    faddp st1, st0\n"); break;
            case '-': *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fsubp st1, st0\n"); break;
            case '*': *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fmulp st1, st0\n"); break;
            case '/': *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    fdivp st1, st0\n"); break;
            case '^': // not fully supported yet due to unique label problem
                *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    ftst\n    fstsw ax\n    fwait\n    sahf\n    jnz .non_zero_exp\n    fstp st0\n    fstp st0\n    fld1\n   jmp .pow_done\n.non_zero_exp:\n    fxch\n    ftst\n    fstsw ax\n    sahf\n    jnz .calc_pow\n    fstp st0\n    fstp st0\n    fldz\n    jmp .pow_done\n.calc_pow:\n    fyl2x\n    fld st0\n    frndint\n    fsub st1, st0\n    fxch\n    f2xm1\n    fld1\n    faddp\n    fscale\n    fstp st1\n.pow_done:\n");
                break;
        }
    }
}

void
gen_x87(Node *root, char *write_buffer, int *buffer_idx, double *const_list, int const_num) {
    *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    push ebp\n    mov ebp, esp\n    push ebx\n    push esi\n    push edi\n");
    convert_x87(root, write_buffer, buffer_idx, const_list, const_num);
    *buffer_idx += snprintf(write_buffer + *buffer_idx, MAX_SIZE - *buffer_idx, "    pop edi\n    pop esi\n    pop ebx\n    pop ebp\n    ret\n");
}

int
main(void) {
    char *secdata_buffer = calloc(MAX_SIZE, sizeof(char));
    int secdata_idx = snprintf(secdata_buffer, MAX_SIZE, "section .data\n    const_pi dq 3.141592653589793\n    const_e dq 2.718281828459045\n");

    char *sectext_buffer = calloc(MAX_SIZE, sizeof(char));
    int sectext_idx = snprintf(sectext_buffer, MAX_SIZE, "section .text\n    global f_1, f_2, f_3, d_1, d_2, d_3\n");

    int const_index = 0;
    double *const_list = calloc(MAX_CONSTS, sizeof(double));

    for (int i = 1; i <= 3; ++i) {
        char **expr = calloc(MAX_LEN, sizeof(char*));
        expr[0] = calloc(10, sizeof(char));
        int idx = 0;
        int pos = 0;
        char c;
    
        while (scanf("%c", &c) == 1 && c != '\n') {
            if (c == ' ') {
                ++idx;
                expr[idx] = calloc(10, sizeof(char));
                pos = 0;
                continue;
            }
            expr[idx][pos] = c;
            ++pos;
        }

        Node *root = build_ast(expr, idx + 1);
        root = check_subtree(root);

        print_rpn(root);
        printf("\n");

        find_consts(root, secdata_buffer, &secdata_idx, &const_index, const_list);
        
        sectext_idx += snprintf(sectext_buffer + sectext_idx, MAX_SIZE - sectext_idx, "f_%d:\n", i);
        gen_x87(root, sectext_buffer, &sectext_idx, const_list, const_index);
    

        Node *diff_root = diff(root);
        diff_root = check_subtree(diff_root);

        print_rpn(diff_root);
        printf("\n");

        find_consts(diff_root, secdata_buffer, &secdata_idx, &const_index, const_list);

        sectext_idx += snprintf(sectext_buffer + sectext_idx, MAX_SIZE - sectext_idx, "d_%d:\n", i);
        gen_x87(diff_root, sectext_buffer, &sectext_idx, const_list, const_index);

        freetree(root);
        freetree(diff_root);
        for (int j = 0; j < idx; ++j) {
            free(expr[j]);
        }
        free(expr);
    }
    
    printf("%d %d\n", secdata_idx, sectext_idx);
    FILE *asm_file = fopen("funcs.asm", "w");
    fwrite(secdata_buffer, 1, secdata_idx, asm_file);
    fwrite(sectext_buffer, 1, sectext_idx, asm_file);
    free(secdata_buffer);
    free(sectext_buffer);
    free(const_list);
    return 0;
}
