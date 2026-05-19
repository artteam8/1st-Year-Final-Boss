#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 1000

/// Node value stores an operator. if value stores "none", Node stores a number
typedef struct Node {
    char *value;
    //double number;
    struct Node *left;
    struct Node *right;
} Node;

/// copies tree (statement) to a new independent tree. space for dest should be allocated first
void
copytree(Node *dest, Node *ref) {
    if (!ref) return;
    dest->value = ref->value;
    //dest->number = ref->number;

    if (ref->left) {
        dest->left = calloc(1, sizeof(Node));
        copytree(dest->left, ref->left);
    }
    if (ref->right) {
        dest->right = calloc(1, sizeof(Node));
        copytree(dest->right, ref->right);
    }
}

/// prints tree in reverse Polish notation
void print_rpn(Node *node) {
    if (node == NULL) return;

    print_rpn(node->left);
    print_rpn(node->right);

    //if (strcmp(node->value, "none") != 0) {
    printf("%s ", node->value);
    //} else {
    //    printf("%lf ", node->number); 
    //}
}

/// splits the power into halfs, and builds chains of multiplications minimazing tree height
Node*
power_tree(Node *ref, int power) {
    Node *node = calloc(1, sizeof(Node));
    if (power == 1) {
        copytree(node, ref);
        return node;
    } else {
        int left_power = power / 2;
        int right_power = power - left_power;
        node->value = "*";
        node->left = power_tree(ref, left_power);
        node->right = power_tree(ref, right_power);
        return node;
    }
}

/// TODO document
Node*
build_ast(char **expr, int len) {
    Node *stack[MAX_LEN];
    int stack_ptr = 0;

    for (int i = 0; i < len; ++i) {
        char *token = expr[i];
        //printf("t: %s\n", token);
        Node *node = calloc(1, sizeof(Node));
        //node->number = 0;
        node->value = "none";

        if ((token[0] == 'x') || (token[0] == 'e') || (strcmp(token, "pi") == 0)) {
            node->value = token;
            node->right = NULL;
            node->left = NULL;
        } else if (strlen(token) == 1 && '*' <= token[0] && token[0] <= '/') {
            node->value = token;
            node->right = stack[--stack_ptr];
            node->left = stack[--stack_ptr];
        } else if (token[0] == '^') { // only constant integer powers are allowed; treats x^y = x*x*...*x with y amount of multiplications; x^-y = 1 / x^y
            //Node *power_node = stack[--stack_ptr];
            //double power = power_node->number;
            //if (strcmp(power_node->value, "none") != 0) power = atof(power_node->value);
            double power = atof(stack[--stack_ptr]->value);
            Node *ref = stack[--stack_ptr];

            if (power > 1) {
                //Node *ref = stack[--stack_ptr];
                int left_power = power / 2;
                int right_power = power - left_power;
                node->value = "*";
                node->left = power_tree(ref, left_power);
                node->right = power_tree(ref, right_power);
            } else if (power == 1) {
                free(node);
                //node = stack[--stack_ptr];
                node = ref;
            } else if (power == 0) {
                //node->value = "none";
                //node->number = 1;
                node->value = "1";
            } else {
                //Node *ref = stack[--stack_ptr];
                node->value = "/";
                node->left = calloc(1, sizeof(Node));
                //node->left->value = "none";
                //node->left->number = 1;
                node->left->value = "1";
                node->right = power_tree(ref, -power);
            }
        } else if ('c' <= token[0] && token[0] <= 't'){ // should be on of sin, cos, tan, ctg, ln
            node->value = token;
            node->right = NULL;
            node->left = stack[--stack_ptr];
        } else { // a constant number
            //node->number = atof(token);
            node->value = token; // as we have value as a string, lets just store it in string. usable for negative
            node->right = NULL;
            node->left = NULL;
        }

        stack[stack_ptr++] = node;
    }
    return stack[0];
}

/// finds and flattens a flat of associative operations
void
flatten(Node *node, char *oper, Node **flat, int *cnt) { // finds a sequence of identical associative operands
    if (strcmp(node->value, oper) == 0) {
        flatten(node->left, oper, flat, cnt);
        flatten(node->right, oper, flat, cnt);
    } else {
        flat[(*cnt)++] = node;
    }
}

/// TODO document
Node*
balance_subtree(Node **flat, int start, int end, char *oper) {
    if (start == end) return flat[start];
    int mid = (start + end) / 2;
    Node *new = calloc(1, sizeof(Node));
    new->value = oper;
    new->left = balance_subtree(flat, start, mid, oper);
    new->right = balance_subtree(flat, mid + 1, end, oper);
    return new;
}

/// TODO document
Node*
check_subtree(Node *root) {
    if (!('*' <= root->value[0] && root->value[0] <= '/')) return root;

    if (root->value[0] == '+' || root->value[0] == '*') {
        Node *flat[MAX_LEN];
        int cnt = 0;
        flatten(root, root->value, flat, &cnt);
        if (cnt > 2) return balance_subtree(flat, 0, cnt - 1, root->value);
    }

    root->left = check_subtree(root->left);
    root->right = check_subtree(root->right);
    return root;
}

Node*
clear_tree(Node *root) {
    if (!('*' <= root->value[0] && root->value[0] <= '/')) return root;

    root->left = clear_tree(root->left);
    root->right = clear_tree(root->right);
    
    // remove zero nodes
    if ((strlen(root->value) == 1) && (((root->value[0] == '*') && ((strcmp(root->left->value, "0") == 0) || (strcmp(root->right->value, "0") == 0)))
    || ((root->value[0] == '/') && (strcmp(root->left->value, "0") == 0))
    || ((root->value[0] == '+' || root->value[0] == '-') && ((strcmp(root->left->value, "0") == 0)) && (strcmp(root->right->value, "0") == 0)))) {
        root->left = NULL;
        root->right = NULL;
        root->value = "0";
    } else if (root->value[0] == '+') {
        if (strcmp(root->left->value, "0") == 0) {
            Node *temp = root->right;
            free(root->left);
            free(root);
            root = temp;
        } else if (strcmp(root->right->value, "0") == 0) {
            Node *temp = root->left;
            free(root->right);
            free(root);
            root = temp;
        }
    } else if ((root->value[0] == '-') && (strcmp(root->right->value, "0") == 0)){
        Node *temp = root->left;
        free(root->right);
        free(root);
        root = temp;
    }

    // set node*1=node and node/1=node
    if (root->value[0] == '*') {
        if (strcmp(root->left->value, "1") == 0) {
            Node *temp = root->right;
            free(root->left);
            free(root);
            root = temp;
        } else if (strcmp(root->right->value, "1") == 0) {
            Node *temp = root->left;
            free(root->right);
            free(root);
            root = temp;
        }
    } else if ((root->value[0] == '/') && (strcmp(root->right->value, "1") == 0)) {
        Node *temp = root->left;
        free(root->right);
        free(root);
        root = temp;
    }

    return root;
}

/// frees subtrees and the tree
void
freetree(Node *root) {
    if (root == NULL) return;
    freetree(root->left);
    freetree(root->right);
    free(root);
}

