#include <stdio.h>
#include <ast.h>

void
copytree(Node *dest, Node *ref) {
    if (!ref) return;
    dest->value = ref->value;
    dest->number = ref->number;

    if (ref->left) {
        dest->left = calloc(1, sizeof(Node));
        copytree(dest->left, ref->left);
    }
    if (ref->right) {
        dest->right = calloc(1, sizeof(Node));
        copytree(dest->right, ref->right);
    }
}

Node*
diff(Node *node) {
    if (node == NULL) return node;
    printf("%c\n", node->value);
    Node *new = calloc(1, sizeof(Node));

    if (node->left != NULL) {
        if (node->right != NULL) {
            ///////////////////////////////////////////////////////
            //                  binary functions                 //
            ///////////////////////////////////////////////////////
            switch(node->value) {
                case '+':
                case '-': {
                    new->left = diff(node->left);
                    new->right = diff(node->right);
                    new->value = node->value;
                    return new;
                }
                case '*': {
                    Node *first = calloc(1, sizeof(Node));
                    Node *second = calloc(1, sizeof(Node));
                    first->value = '*';
                    second->value = '*';
                    first->left = diff(node->left);
                    //first->right = node->right;
                    //second->left = node->left;
                    first->right = calloc(1, sizeof(Node));
                    second->left = calloc(1, sizeof(Node));
                    copytree(first->right, node->right);
                    copytree(second->left, node->left);
                    second->right = diff(node->right);

                    new->value = '+';
                    new->left = first;
                    new->right = second;
                    return new;
                }
                case '/': {
                    Node *first = calloc(1, sizeof(Node));
                    Node *second = calloc(1, sizeof(Node));
                    first->value = '*';
                    second->value = '*';
                    first->left = diff(node->left);
                    //first->right = node->right;
                    //second->left = node->left;
                    first->right = calloc(1, sizeof(Node));
                    second->left = calloc(1, sizeof(Node));
                    copytree(first->right, node->right);
                    copytree(second->left, node->left);
                    second->right = diff(node->right);

                    Node *numenator = calloc(1, sizeof(Node));
                    numenator->value = '-';
                    numenator->left = first;
                    numenator->right = second;

                    Node *denumenator = calloc(1, sizeof(Node));
                    denumenator->left = calloc(1, sizeof(Node));
                    //memcpy(denumenator->left, second, sizeof(Node)); // doesn't copy the subtree, leaves the ptrs
                    copytree(denumenator->left, node->right);
                    denumenator->right = calloc(1, sizeof(Node));
                    denumenator->right->number = 2;
                    denumenator->right->value = -1; // marking as constant
                    denumenator->value = '^';

                    new->left = numenator;
                    new->right = denumenator;
                    new->value = '/';

                    return new;
                }
                case '^': {
                    // f(x) ^ g(x) = f(x) ^ g(x) * (ln(f(x)) * g(x))' = f(x) ^ g(x) * (ln(f(x))*g(x)' + f(x)'/f(x)*g(x))
                    new->value = '*';
                    new->left = calloc(1, sizeof(Node));
                    copytree(new->left, node); // f(x)^g(x)
                    new->right = calloc(1, sizeof(Node));
                    new->right->value = '+';

                    Node *first = calloc(1, sizeof(Node));
                    Node *ln = calloc(1, sizeof(Node));
                    ln->value = pseudohash("ln");
                    //ln->left = node->left; //f(x)
                    ln->left = calloc(1, sizeof(Node));
                    copytree(ln->left, node->left); //f(x)
                    ln->right = NULL;
                    first->value = '*';
                    first->left = ln;
                    first->right = diff(node->right); //g(x)'
                    
                    Node *second = calloc(1, sizeof(Node));
                    Node *f_to_der = calloc(1, sizeof(Node));
                    f_to_der->value = '/';
                    f_to_der->left = diff(node->left);
                    //f_to_der->right = node->left;
                    f_to_der->right = calloc(1, sizeof(Node));
                    copytree(f_to_der->right, node->left);
                    second->value = '*';
                    second->left = f_to_der;
                    //second->right = node->right; //g(x)
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
            if (node->value == pseudohash("sin")) {
                der->value = pseudohash("cos");
                der->left = calloc(1, sizeof(Node));
                copytree(der->left, node->left); //g(x)
            } else if (node->value == pseudohash("cos")) {
                der->value = pseudohash("sin");
                der->left = calloc(1, sizeof(Node));
                copytree(der->left, node->left); //g(x)

                sign = -1;
            } else if (node->value == pseudohash("tan")) {
                der->left = calloc(1, sizeof(Node));
                der->left->value = -1;
                der->left->number = 1;

                der->value = '/';
                der->right = calloc(1, sizeof(Node));
                der->right->value = '^';
                der->right->left = calloc(1, sizeof(Node));
                der->right->right = calloc(1, sizeof(Node));
                der->right->left->value = pseudohash("cos");
                der->right->left->left = calloc(1, sizeof(Node));
                copytree(der->right->left->left, node->left);
                der->right->right->value = -1;
                der->right->right->number = 2;
            } else if (node->value == pseudohash("ctg")) {
                der->left = calloc(1, sizeof(Node));
                der->left->value = -1;
                der->left->number = 1;

                der->value = '/';
                der->right = calloc(1, sizeof(Node));
                der->right->value = '^';
                der->right->left = calloc(1, sizeof(Node));
                der->right->right = calloc(1, sizeof(Node));
                der->right->left->value = pseudohash("sin");
                der->right->left->left = calloc(1, sizeof(Node));
                copytree(der->right->left->left, node->left);
                der->right->right->value = -1;
                der->right->right->number = 2;
                
                sign = -1;
            } else if (node->value == pseudohash("ln")) {
                der->left = calloc(1, sizeof(Node));
                der->left->value = -1;
                der->left->number = 1;
                
                der->right = calloc(1, sizeof(Node));
                copytree(der->right, node->left);
                der->value = '/';
            }

            if (sign == -1) {
                Node *neg = calloc(1, sizeof(Node));
                neg->value = '-';
                neg->left = calloc(1, sizeof(Node));
                neg->left->value = -1;
                neg->right = der;

                der = neg;
            }

            new->value = '*';
            new->left = der;
            new->right = internal;
        }
    } else {
        /////////////////
        //  constants  //
        /////////////////
        new->value = -1;
        if (node->value == 'x') new->number = 1;
        else new->number = 0;
    }

    return new;
}
