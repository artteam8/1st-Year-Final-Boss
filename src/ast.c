#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LEN 1000
#define VALUE_LEN 10
#define CMP_EPS 1e-6

/// @defgroup tree TreeUtils
/// @brief Basic functions to work with the Tree.
/// @{

/// Node.value can store an operator -- either binary or unary -- or an operand -- a constant, or the variable (x)
typedef struct Node {
    char value[VALUE_LEN];
    struct Node *left;
    struct Node *right;
} Node;

/// copies tree (an expression) to a new independent tree. space for dest should be allocated first
void
copytree(Node *dest, Node *ref) {
    if (!ref) return;
    snprintf(dest->value, VALUE_LEN, "%s", ref->value);
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

/// compares two trees to be fully identical
int
cmptree(Node *root, Node *ref) {
    if (root == NULL || ref == NULL) return (root == NULL) && (ref == NULL);
    return (strcmp(root->value, ref->value) == 0) && cmptree(root->left, ref->left) && cmptree(root->right, ref->right);
}

/// compares two trees to be identical with a more precise check for associative operators
int
cmptree_v2(Node *root, Node *ref) {
    if (root == NULL || ref == NULL) return (root == NULL) && (ref == NULL);

    // if operator is associative, operands can be reversed, despite the operation being the same; check if they both are a + or a *
    if ((strlen(root->value) == 1) && (strlen(ref->value) == 1)
        && (((root->value[0] == '+') && (ref->value[0] == '+')) || ((root->value[0] == '*') && (ref->value[0] == '*'))))
        return (cmptree_v2(root->left, ref->left) && cmptree_v2(root->right, ref->right)) || (cmptree_v2(root->left, ref->right) && cmptree_v2(root->right, ref->left));
        
    // else, they should be fully identical
    return (strcmp(root->value, ref->value) == 0) && cmptree_v2(root->left, ref->left) && cmptree_v2(root->right, ref->right);
}

/// frees all trees and the tree itself (avoid Use-After-Free!)
void
freetree(Node *root) {
    if (root == NULL) return;
    freetree(root->left);
    freetree(root->right);
    free(root);
}

/// prints tree in reverse Polish notation
void
print_rpn(Node *node) {
    if (node == NULL) return;

    print_rpn(node->left);
    print_rpn(node->right);

    printf("%s ", node->value);
}
/// @}

/// splits the power into halfs, and builds chains of multiplications, minimazing tree height
Node*
power_tree(Node *ref, int power) {
    Node *node = calloc(1, sizeof(Node));
    if (power == 1) {
        copytree(node, ref);
        return node;
    } else {
        int left_power = power / 2;
        int right_power = power - left_power;
        snprintf(node->value, VALUE_LEN, "%s", "*");
        node->left = power_tree(ref, left_power);
        node->right = power_tree(ref, right_power);
        return node;
    }
}

/// processes each token;
/// if it is a constant/variable, puts in onto stack; 
/// if it is an operator, grabs one/two nodes from stack,
/// makes a new node with the operator, and puts it onto stack
///
/// SAFE_EXP=1 allows only integer powers
Node*
build_ast(char **expr, int len) {
    Node *stack[MAX_LEN];
    int stack_ptr = 0;

    for (int i = 0; i < len; ++i) {
        char *token = expr[i];
        Node *node = calloc(1, sizeof(Node));
        snprintf(node->value, VALUE_LEN, "%s", "0");

        if ((token[0] == 'x') || (token[0] == 'e') || (strcmp(token, "pi") == 0)) {
            snprintf(node->value, VALUE_LEN, "%s", token);
            node->right = NULL;
            node->left = NULL;
        } else if (strlen(token) == 1 && ('*' <= token[0] && token[0] <= '/')) {
            snprintf(node->value, VALUE_LEN, "%s", token);
            node->right = stack[--stack_ptr];
            node->left = stack[--stack_ptr];
#if SAFE_EXP
        } else if (strlen(token) == 1 && (token[0] == '^')) { // only constant integer powers are allowed; treats x^y = x*x*...*x with y amount of multiplications; x^-y = 1 / x^y
            double power = atof(stack[--stack_ptr]->value);
            Node *ref = stack[--stack_ptr];

            double integer_part;
            if (fabs(modf(power, &integer_part)) > CMP_EPS) {
                fprintf(stderr, "\n\nPower is not an integer, exiting...\n\n");
                exit(1);
            }

            if (power > 1) {
                int left_power = power / 2;
                int right_power = power - left_power;
                snprintf(node->value, VALUE_LEN, "%s", "*");
                node->left = power_tree(ref, left_power);
                node->right = power_tree(ref, right_power);
            } else if (power == 1) {
                free(node);
                node = ref;
            } else if (power == 0) {
                snprintf(node->value, VALUE_LEN, "%s", "1");
            } else {
                snprintf(node->value, VALUE_LEN, "%s", "/");
                node->left = calloc(1, sizeof(Node));
                snprintf(node->left->value, VALUE_LEN, "%s", "1");
                node->right = power_tree(ref, -power);
            }
        }
#else
        } else if (strlen(token) == 1 && (token[0] == '^')) {
            snprintf(node->value, VALUE_LEN, "%s", token);
            node->right = stack[--stack_ptr];
            node->left = stack[--stack_ptr];
        }
#endif
        else if ('c' <= token[0] && token[0] <= 't'){ // should be on of sin, cos, tan, ctg, ln
            snprintf(node->value, VALUE_LEN, "%s", token);
            node->right = NULL;
            node->left = stack[--stack_ptr];
        } else { // a constant number
            snprintf(node->value, VALUE_LEN, "%s", token); // as we have value as a string, lets just store it in string. can be used for negative
            node->right = NULL;
            node->left = NULL;
        }

        stack[stack_ptr++] = node;
    }
    return stack[0];
}

/// @defgroup opt TreeOptimization
/// @brief Functions that reduce tree size and precompute if possible.
/// @{



/// @defgroup balance TreeBalance
/// @brief Functions that balance the tree.
/// Balancing is available only if the operators are associative,
/// because only if they are, we can rearrange operands.
/// @{

/// finds a flat of associative operations;
/// stores the nodes in flat and the operators in oper_pool
void
flatten(Node *node, char *oper, Node **flat, int *node_cnt, Node **oper_pool, int *oper_cnt) {
    if (strcmp(node->value, oper) == 0) {
        oper_pool[(*oper_cnt)++] = node;
        flatten(node->left, oper, flat, node_cnt, oper_pool, oper_cnt);
        flatten(node->right, oper, flat, node_cnt, oper_pool, oper_cnt);
    } else {
        flat[(*node_cnt)++] = node;
    }
}

/// recreates the tree with balanced structure;
/// uses the same operators, just rearranges them
/// avoids rearranging the root operator
Node*
balance_tree(Node **flat, int start, int end, char *oper, Node **oper_pool, int *oper_idx, Node *root) {
    if (start == end) return flat[start];
    int mid = (start + end) / 2;
    //Node *cur = calloc(1, sizeof(Node));
    Node *cur = root ? root : oper_pool[(*oper_idx)++];
    //snprintf(cur->value, VALUE_LEN, "%s", oper);
    //is already there
    cur->left = balance_tree(flat, start, mid, oper, oper_pool, oper_idx, NULL);
    cur->right = balance_tree(flat, mid + 1, end, oper, oper_pool, oper_idx, NULL);
    return cur;
}

/// find the flat and balance
Node*
check_tree(Node *root) {
    if (root == NULL) return root;

    if (strlen(root->value) == 1 && (root->value[0] == '+' || root->value[0] == '*')) {
        Node *flat[MAX_LEN];
        Node *oper_pool[MAX_LEN];
        int node_cnt = 0;
        int oper_cnt = 0;
        flatten(root, root->value, flat, &node_cnt, oper_pool, &oper_cnt);
        int oper_idx = 1; // skip the root
        if (node_cnt > 3) return balance_tree(flat, 0, node_cnt - 1, root->value, oper_pool, &oper_idx, root);
    }

    root->left = check_tree(root->left);
    root->right = check_tree(root->right);
    return root;
}

/// @}

/// checks if the string stores a constant number
int
is_num(char *s) {
    if (strcmp(s, "pi") == 0 || strcmp(s, "e") == 0) return 1;
    char *endptr;
    strtod(s, &endptr);
    return *endptr == '\0';
}

Node*
clear_tree(Node *root);

/// set expressions like:
/// 0*f(x)
/// 0/f(x)
/// 0+0
/// 0-0
/// equal to 0,
/// and f(x)^0 equal to 1.
Node*
remove_zero(Node *root) {
    if (strlen(root->value) == 1) {
        if (((root->value[0] == '*') && ((strcmp(root->left->value, "0") == 0) || (strcmp(root->right->value, "0") == 0)))
        || ((root->value[0] == '/') && (strcmp(root->left->value, "0") == 0))
        || ((root->value[0] == '+' || (strcmp(root->value, "-") == 0)) && ((strcmp(root->left->value, "0") == 0)) && (strcmp(root->right->value, "0") == 0))) {
    
            freetree(root->left);
            freetree(root->right);
            root->left = NULL;
            root->right = NULL;
            snprintf(root->value, VALUE_LEN, "%s", "0");
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
        } else if ((strcmp(root->value, "-") == 0) && (strcmp(root->right->value, "0") == 0)){
            Node *temp = root->left;
            free(root->right);
            free(root);
            root = temp;
        } else if ((root->value[0] == '^') && (strcmp(root->right->value, "0") == 0)) {
            freetree(root);
            snprintf(root->value, VALUE_LEN, "%s", "1");
        }
    }
    return root;
}

/// set
/// f(x)*1=f(x)
/// f(x)/1=f(x)
/// f(x)^1=f(x)
Node*
reduce_one(Node *root) {
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
    } else if ((root->value[0] == '^') && strcmp(root->right->value, "1") == 0) {
        Node *expr = root->left;
        free(root->right);
        free(root);
        root = expr;
    }
    return root;
}

/// set:
/// f(x)/f(x) = 1
/// f(x)^a / f(x)^b = f(x)^(a-b)
/// if exp is not specified, exp is 1:
/// f(x)^a / f(x) = f(x)^(a-1)
/// f(x) / f(x)^a = f(x)^(1-a)
/// and
/// f(x)-f(x)=0
void
reduce_ident(Node *root) {
    if (root->value[0] == '/') {
        if (cmptree_v2(root->left, root->right)) {
            snprintf(root->value, VALUE_LEN, "%s", "1");
            freetree(root->left);
            root->left = NULL;
            freetree(root->right);
            root->right = NULL;
        } else if ((root->left->value[0] == '^') && (root->right->value[0] == '^')
            && cmptree_v2(root->left->left, root->right->left)) {
            Node *power_left = root->left->right;

            Node *expr = root->left->left;

            snprintf(root->value, VALUE_LEN, "%s", "^");
            free(root->left);
            root->left = expr;

            snprintf(root->right->value, VALUE_LEN, "%s", "-");
            freetree(root->right->left);
            root->right->left = power_left;

            root->right = clear_tree(root->right); // updated the child, maybe a-b can be reduced
        } else if ((root->left->value[0] == '^')
            && cmptree_v2(root->left->left, root->right)) {
            Node *power_left = root->left->right;

            Node *expr = root->left->left;

            snprintf(root->value, VALUE_LEN, "%s", "^");
            free(root->left);
            root->left = expr;

            freetree(root->right->left);
            freetree(root->right->right);
            snprintf(root->right->value, VALUE_LEN, "%s", "-");
            root->right->left = power_left;
            root->right->right = calloc(1, sizeof(Node));
            snprintf(root->right->right->value, VALUE_LEN, "%s", "1");

            root->right = clear_tree(root->right); // updated the child, maybe a-1 can be reduced
        } else if ((root->right->value[0] == '^')
            && cmptree_v2(root->right->left, root->left)) {
            snprintf(root->value, VALUE_LEN, "%s", "^");
            snprintf(root->right->value, VALUE_LEN, "%s", "-");

            Node *power_right = root->right->right;
            freetree(root->right->left);
            root->right->left = calloc(1, sizeof(Node));
            snprintf(root->right->left->value, VALUE_LEN, "%s", "1");

            root->right->right = power_right;

            root->right = clear_tree(root->right); // updated the child, maybe 1-a can be reduced
        }
    } else if ((strcmp(root->value, "-") == 0) && (cmptree_v2(root->left, root->right))) {
            snprintf(root->value, VALUE_LEN, "%s", "0");
            freetree(root->left);
            root->left = NULL;
            freetree(root->right);
            root->right = NULL;
    }
}

/// if both children of operator are constants,
/// we can precompute them right now
void
precompute(Node *root) {
    if (root->left != NULL && root->right != NULL) {
        if (is_num(root->left->value) && is_num(root->right->value)) {
            double res = 0;
            double left = atof(root->left->value);
            double right = atof(root->right->value);
            if (strcmp(root->left->value, "e") == 0) left = 2.718281828459045;
            if (strcmp(root->right->value, "e") == 0) right = 2.718281828459045;
            if (strcmp(root->left->value, "pi") == 0) left = 3.141592653589793;
            if (strcmp(root->right->value, "pi") == 0) right = 3.141592653589793;

            switch(root->value[0]) {
                case '+':
                    res = left + right;
                    break;
                case '-':
                    if (left < right) return;
                    res = left - right;
                    break;
                case '*':
                    res = left * right;
                    break;
                case '/':
                    res = left / right;
                    break;
                case '^':
                    res = pow(left, right);
            }
            double integer_part;
            if (fabs(modf(res, &integer_part)) <= CMP_EPS) snprintf(root->value, VALUE_LEN, "%.0lf", integer_part);
            else snprintf(root->value, VALUE_LEN, "%.4lf", res);

            free(root->left);
            free(root->right);
            root->left = NULL;
            root->right = NULL;
        }
    }
}

/// applies optimizations in post-order
Node*
clear_tree(Node *root) {
    if (root == NULL) return root;

    root->left = clear_tree(root->left);
    root->right = clear_tree(root->right);
    if (!(('*' <= root->value[0] && root->value[0] <= '/') || root->value[0] == '^')) return root;

    
    root = remove_zero(root);

    root = reduce_one(root);

    // set f(x) * C/g(x) --> C * f(x)/g(x); for reducing identical nodes
    if (root->value[0] == '*') {
        if ((root->left->value[0] == '/')
            && is_num(root->left->left->value)) {
            Node *num = root->left->left;
            root->left->left = root->right;
            root->right = num;

            root->left = clear_tree(root->left);
        } else if ((root->right->value[0] == '/')
            && is_num(root->right->left->value)) {
            Node *num = root->right->left;
            root->right->left = root->left;
            root->left = num;

            root->right = clear_tree(root->right);
        }
    }

    reduce_ident(root);

    precompute(root);
    
    return root;
}

/// @}

