#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define D_EPS 1e-12
#define MAX_ITER 1000000

extern double f_1(double x);
extern double f_2(double x);
extern double f_3(double x);

#if USE_NEWTON
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

    printf("iters: %d\n", iter);
    return x;
}
#else
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

        if (fabs(y) < eps1) {
            printf("iters: %d\n", iter);
            return x;
        }

        // checking where the root is
        if (fa * y < 0) {
            b = x;
            fb = y;
        } else {
            a = x;
            fa = y;
        }

        if (fabs(b - a) < eps1) {
            printf("iters: %d\n", iter);
            return x;
        }

        ++iter;
    }
    printf("iters: %d\n", iter);

    return x;
}
#endif

void
sort(double *xs) {
    double t;
    if (xs[0] > xs[1]) {
        t = xs[0];
        xs[0] = xs[1];
        xs[1] = t;
    }
    // here xs[0] <= xs[1]
    if (xs[1] > xs[2]) {
        t = xs[1];
        xs[1] = xs[2];
        xs[2] = t;
    }
    // here xs[1] <= xs[2]
}

double*
crosses(double a, double b, double eps1) {
    double *xs = calloc(3, sizeof(double));
#if USE_NEWTON
    printf("Newton method is used\n");
    xs[0] = root(&f_1, &f_2, &d_1, &d_2, a, b, eps1);
    xs[1] = root(&f_2, &f_3, &d_2, &d_3, a, b, eps1);
    xs[2] = root(&f_1, &f_3, &d_1, &d_3, a, b, eps1);
#else
    printf("Secant method is used\n");
    xs[0] = root(&f_1, &f_2, a, b, eps1);
    xs[1] = root(&f_2, &f_3, a, b, eps1);
    xs[2] = root(&f_1, &f_3, a, b, eps1);
#endif
    sort(xs);
    return xs;
}
