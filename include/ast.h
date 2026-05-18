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
pseudohash(char *func);

int
unpseudohash(char hash);

void
copytree(Node *dest, Node *ref);

void
copytree(Node *dest, Node *ref);


Node*
build_ast(char **expr, int len);

void
flatten(Node *node, char oper, Node **flat, int *cnt);

Node*
balance_subtree(Node **flat, int start, int end, char oper);

Node*
check_subtree(Node *root);

void print_rpn(Node *node);
