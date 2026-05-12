#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_ITER 1000000

extern double f_1(double x);
extern double f_2(double x);
extern double f_3(double x);

double
root(double (*f)(double),
    double (*g)(double),
    double a,
    double b,
    double eps1) {

    int iter = 0;
    double fa = f(a) - g(a);
    double fb = f(b) - g(b);
    double x, y;

    while (iter < MAX_ITER) {
        x = a - fa * (b - a) / (fb - fa);
        y = f(x) - g(x);

        if (fabs(y) < eps1) return x;

        // checking where the root is
        if (fa * y < 0) {
            b = x;
            fb = y;
        } else {
            a = x;
            fa = y;
        }

        if (fabs(b - a) < eps1) {
            return x;
        }

        ++iter;
    }

    return x;
}

double*
crosses(double a, double b, double eps1) {
    double *xs = calloc(3, sizeof(double));
    xs[0] = root(&f_1, &f_2, a, b, eps1);
    xs[1] = root(&f_2, &f_3, a, b, eps1);
    xs[2] = root(&f_1, &f_3, a, b, eps1);
    return xs;
}
