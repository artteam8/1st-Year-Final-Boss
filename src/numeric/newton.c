#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEPS 1e-12
#define MAX_ITER 1000000

extern double f_1(double x);
extern double f_2(double x);
extern double f_3(double x);
extern double d_1(double x);
extern double d_2(double x);
extern double d_3(double x);

double
root(double (*f)(double),
    double (*g)(double),
    double (*f_der)(double),
    double (*g_der)(double),
    double a,
    double b,
    double eps1) {

    int iter = 0;
    double y, dy;
    double x = (a + b) / 2;

    while (iter < MAX_ITER) {
        y = f(x) - g(x);
        if (fabs(y) < eps1) break;

        dy = f_der(x) - g_der(x);
        if(fabs(dy) < D_EPS) break;

        x = x - y/dy;
        ++iter;
    }

    return x;
}

double*
crosses(double a, double b, double eps1) {
    double *xs = calloc(3, sizeof(double));
    xs[0] = root(&f_1, &f_2, &d_1, &d_2, a, b, eps1);
    xs[1] = root(&f_2, &f_3, &d_2, &d_3, a, b, eps1);
    xs[2] = root(&f_1, &f_3, &d_1, &d_3, a, b, eps1);
    return xs;
}
