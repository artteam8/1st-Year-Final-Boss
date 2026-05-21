#include <stdio.h>
#include <ast.h>

#define VALUE_LEN 10

/// a single function for differentiating an AST.
/// rules used:
/// (f(x)+g(x))' = f'(x) + g'(x)
///
/// (f(x)-g(x))' = f'(x) - g'(x)
///
/// (f(x)*g(x))' = f'(x)*g(x) + f(x)*g'(x)
///
/// (f(x)/g(x))' = (f'(x)*g(x) - f(x)*g'(x)) / (g(x)^2)
///
/// (f(x)^g(x))' = (e^(ln(f(x)) * g(x)))' = f(x)^g(x) * (g(x) * f'(x)/f(x) + ln(f(x)) * g'(x))
Node*
diff(Node *node) {
    if (node == NULL) return node;
    Node *new = calloc(1, sizeof(Node));

    if (node->left != NULL) {
        if (node->right != NULL) {
            ///////////////////////////////////////////////////////
            //                  binary functions                 //
            ///////////////////////////////////////////////////////
            switch(node->value[0]) {
                case '+':
                case '-': {
                    new->left = diff(node->left);
                    new->right = diff(node->right);
                    snprintf(new->value, VALUE_LEN, "%s", node->value);
                    return new;
                }
                case '*': {
                    Node *first = calloc(1, sizeof(Node));
                    Node *second = calloc(1, sizeof(Node));
                    snprintf(first->value, VALUE_LEN, "%s", "*");
                    snprintf(second->value, VALUE_LEN, "%s", "*");
                    first->left = diff(node->left);
                    first->right = calloc(1, sizeof(Node));
                    second->left = calloc(1, sizeof(Node));
                    copytree(first->right, node->right);
                    copytree(second->left, node->left);
                    second->right = diff(node->right);

                    snprintf(new->value, VALUE_LEN, "%s", "+");
                    new->left = first;
                    new->right = second;
                    return new;
                }
                case '/': {
                    Node *first = calloc(1, sizeof(Node));
                    Node *second = calloc(1, sizeof(Node));
                    snprintf(first->value, VALUE_LEN, "%s", "*");
                    snprintf(second->value, VALUE_LEN, "%s", "*");
                    first->left = diff(node->left);
                    first->right = calloc(1, sizeof(Node));
                    second->left = calloc(1, sizeof(Node));
                    copytree(first->right, node->right);
                    copytree(second->left, node->left);
                    second->right = diff(node->right);

                    Node *numenator = calloc(1, sizeof(Node));
                    snprintf(numenator->value, VALUE_LEN, "%s", "-");
                    numenator->left = first;
                    numenator->right = second;

                    Node *denumenator = calloc(1, sizeof(Node));
                    denumenator->left = calloc(1, sizeof(Node));

                    copytree(denumenator->left, node->right);
                    denumenator->right = calloc(1, sizeof(Node));

                    copytree(denumenator->right, node->right);
                    snprintf(denumenator->value, VALUE_LEN, "%s", "*");

                    new->left = numenator;
                    new->right = denumenator;
                    snprintf(new->value, VALUE_LEN, "%s", "/");

                    return new;
                }
                case '^': {
                    snprintf(new->value, VALUE_LEN, "%s", "*");
                    new->left = calloc(1, sizeof(Node));
                    copytree(new->left, node); // f(x)^g(x)
                    new->right = calloc(1, sizeof(Node));
                    snprintf(new->right->value, VALUE_LEN, "%s", "+");

                    Node *first = calloc(1, sizeof(Node));
                    Node *ln = calloc(1, sizeof(Node));
                    snprintf(ln->value, VALUE_LEN, "%s", "ln");
                    ln->left = calloc(1, sizeof(Node));
                    copytree(ln->left, node->left); //f(x)
                    ln->right = NULL;
                    snprintf(first->value, VALUE_LEN, "%s", "*");
                    first->left = ln;
                    first->right = diff(node->right); //g'(x)
                    
                    Node *second = calloc(1, sizeof(Node));
                    Node *f_to_der = calloc(1, sizeof(Node));
                    snprintf(f_to_der->value, VALUE_LEN, "%s", "/");
                    f_to_der->left = diff(node->left);

                    f_to_der->right = calloc(1, sizeof(Node));
                    copytree(f_to_der->right, node->left);
                    snprintf(second->value, VALUE_LEN, "%s", "*");
                    second->left = f_to_der;

                    second->right = calloc(1, sizeof(Node));
                    copytree(second->right, node->right); //g(x)

                    new->right->left = first;
                    new->right->right = second;
                    return new;
                }
            }
        } else {
            ///////////////////////////////////////////////////////
            //           unary functions: sin, cos, ...          //
            //           (f(g(x)))' = f'(g(x)) * g'(x)           //
            ///////////////////////////////////////////////////////
            Node *internal = diff(node->left); // g'(x)
            Node *der = calloc(1, sizeof(Node));
            int sign = 1;
            if (strcmp(node->value, "sin") == 0) {
                snprintf(der->value, VALUE_LEN, "%s", "cos");
                der->left = calloc(1, sizeof(Node));
                copytree(der->left, node->left); //g(x)
            } else if (strcmp(node->value, "cos") == 0) {
                snprintf(der->value, VALUE_LEN, "%s", "sin");
                der->left = calloc(1, sizeof(Node));
                copytree(der->left, node->left); //g(x)

                sign = -1;
            } else if (strcmp(node->value, "tan") == 0) {
                der->left = calloc(1, sizeof(Node));
                snprintf(der->left->value, VALUE_LEN, "%s", "1");

                snprintf(der->value, VALUE_LEN, "%s", "/");
                der->right = calloc(1, sizeof(Node));
                snprintf(der->right->value, VALUE_LEN, "%s", "*");

                der->right->left = calloc(1, sizeof(Node));
                der->right->right = calloc(1, sizeof(Node));
                snprintf(der->right->left->value, VALUE_LEN, "%s", "cos");
                der->right->left->left = calloc(1, sizeof(Node));
                copytree(der->right->left->left, node->left);

                snprintf(der->right->right->value, VALUE_LEN, "%s", "cos");
                der->right->right->left = calloc(1, sizeof(Node));
                copytree(der->right->right->left, node->left);
            } else if (strcmp(node->value, "ctg")) {
                der->left = calloc(1, sizeof(Node));

                snprintf(der->left->value, VALUE_LEN, "%s", "1");

                snprintf(der->value, VALUE_LEN, "%s", "/");
                der->right = calloc(1, sizeof(Node));

                snprintf(der->right->value, VALUE_LEN, "%s", "*");
                der->right->left = calloc(1, sizeof(Node));
                der->right->right = calloc(1, sizeof(Node));

                snprintf(der->right->left->value, VALUE_LEN, "%s", "sin");
                der->right->left->left = calloc(1, sizeof(Node));
                copytree(der->right->left->left, node->left);

                snprintf(der->right->right->value, VALUE_LEN, "%s", "sin");
                der->right->right->left = calloc(1, sizeof(Node));
                copytree(der->right->right->left, node->left);
                
                sign = -1;
            } else if (strcmp(node->value, "ln") == 0) {
                der->left = calloc(1, sizeof(Node));

                snprintf(der->left->value, VALUE_LEN, "%s", "1");
                
                der->right = calloc(1, sizeof(Node));
                copytree(der->right, node->left);
                snprintf(der->value, VALUE_LEN, "%s", "/");
            }

            if (sign == -1) {
                Node *neg = calloc(1, sizeof(Node));
                snprintf(neg->value, VALUE_LEN, "%s", "-");
                neg->left = calloc(1, sizeof(Node));
                snprintf(neg->left->value, VALUE_LEN, "%s", "0");
                neg->right = der;

                der = neg;
            }

            snprintf(new->value, VALUE_LEN, "%s", "*");
            new->left = der;
            new->right = internal;
        }
    } else {
        /////////////////
        //  constants  //
        /////////////////
        if (node->value[0] == 'x') snprintf(new->value, VALUE_LEN, "%s", "1");
        else snprintf(new->value, VALUE_LEN, "%s", "0");
    }

    return new;
}
