#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 1000

typedef struct Node {
    char value;
    double number;
    struct Node *left;
    struct Node *right;
} Node;

char
pseudohash(char *func) {
    char s = 0;
    while (*func != '\0') {
        s += *func - 'a';
        ++func;
    }
    return s;
}

int
unpseudohash(char hash) {
    char *values[] = {"sin", "cos", "tan", "ctg", "ln", "pi"};
    int len = 6;
    for (int i = 0; i < len; ++i) {
        //printf("trying %s...\n", values[i]);
        if (pseudohash(values[i]) == hash) {
            printf("%s ", values[i]);
            return 1;
        }
    }
    //printf("nah ):\n");
    return 0;
}

Node*
build_ast(char **expr, int len) {
    Node *stack[MAX_LEN];
    int stack_ptr = 0;

    for (int i = 0; i < len; ++i) {
        char *token = expr[i];
        Node *node = calloc(1, sizeof(Node));
        node->number = 0;
        node->value = 0;

        if (strcmp(token, "x") == 0 || strcmp(token, "e") == 0) {
            node->value = token[0];
            node->right = NULL;
            node->left = NULL;
        } else if (('*' <= token[0] && token[0] <= '/') || token[0] == '^') {
            node->value = token[0];
            node->right = stack[--stack_ptr];
            node->left = stack[--stack_ptr];
        }
        else if (strcmp(token, "pi") == 0) {
            node->value = pseudohash(token);
            node->right = NULL;
            node->left = NULL;
        } else if ('c' <= token[0] && token[0] <= 't'){ // should be on of sin, cos, tan, ctg, ln
            node->value = pseudohash(token);
            node->right = NULL;
            node->left = stack[--stack_ptr];
        } else { // a constant number
            node->number = atof(token);
            node->right = NULL;
            node->left = NULL;
        }

        stack[stack_ptr++] = node;
    }

    return stack[0];
}

void
flatten(Node *node, char oper, Node **flat, int *cnt) { // finds a sequence of identical associative operands
    if (node->value == oper) {
        flatten(node->left, oper, flat, cnt);
        flatten(node->right, oper, flat, cnt);
    } else {
        flat[(*cnt)++] = node;
    }
}

Node*
balance_subtree(Node **flat, int start, int end, char oper) {
    if (start == end) return flat[start];
    int mid = (start + end) / 2;
    Node *new = calloc(1, sizeof(Node));
    new->value = oper;
    new->left = balance_subtree(flat, start, mid, oper);
    new->right = balance_subtree(flat, mid + 1, end, oper);
    return new;
}

Node*
check_subtree(Node *root) {
    if (!('*' <= root->value && root->value <= '/')) return root;

    if (root->value == '+' || root->value == '*') {
        Node *flat[MAX_LEN];
        int cnt = 0;
        flatten(root, root->value, flat, &cnt);
        if (cnt > 2) return balance_subtree(flat, 0, cnt - 1, root->value);
    }

    root->left = check_subtree(root->left);
    root->right = check_subtree(root->right);
    
    return root;
}

///*
void print_rpn(Node *node) {
    if (node == NULL) return;

    print_rpn(node->left);
    print_rpn(node->right);

    if (node->value > 0) {
        if (!unpseudohash(node->value)) printf("%c ", node->value); 
    } else {
        printf("%lf ", node->number); 
    }
}
//*/

