#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 1000

typedef struct Node {
    char value[10];
    //double number;
    struct Node *left;
    struct Node *right;
} Node;

void
copytree(Node *dest, Node *ref);

Node*
build_ast(char **expr, int len);

Node*
check_tree(Node *root);

Node*
clear_tree(Node *root);

void
freetree(Node *root);

void print_rpn(Node *node);
