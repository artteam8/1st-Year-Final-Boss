#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_ITER 1000000

typedef double (*f)(double);

FILE *outp = NULL;

#if USE_NEWTON
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
        if(fabs(dy) < eps1) break;

        x = x - y/dy;
        ++iter;
    }

    fprintf(outp, "iters: %d, x=%lf\n", iter, x);
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
            fprintf(outp, "iters: %d\n", iter);
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
            fprintf(outp, "iters: %d\n", iter);
            return x;
        }

        ++iter;
    }
    fprintf(outp, "iters: %d\n", iter);

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
        // here xs[1] <= xs[2]
        if (xs[0] > xs[1]) {
            t = xs[0];
            xs[0] = xs[1];
            xs[1] = t;
            // here xs[0] <= xs[1] <= xs[2]
        }
    }
}

#if USE_NEWTON
double*
crosses(f f1, f f2, f f3, f d1, f d2, f d3, double a, double b, double eps1, int print_iters) {
    if (print_iters) outp = stdout;
    else outp = fopen("/dev/null", "w");

    double *xs = calloc(3, sizeof(double));
    xs[0] = root(f1, f2, d1, d2, a, b, eps1);
    xs[1] = root(f2, f3, d2, d3, a, b, eps1);
    xs[2] = root(f1, f3, d1, d3, a, b, eps1);
    sort(xs);
    return xs;
}
#else
double*
crosses(f f1, f f2, f f3, double a, double b, double eps1, int print_iters) {
    if (print_iters) outp = stdout;
    else outp = fopen("/dev/null", "w");

    double *xs = calloc(3, sizeof(double));
    xs[0] = root(f1, f2, a, b, eps1);
    xs[1] = root(f2, f3, a, b, eps1);
    xs[2] = root(f1, f3, a, b, eps1);
    sort(xs);
    return xs;
}
#endif










#if USE_SIMPSON
double
simpson(double (*func)(double), double a, double b, int n_splits) {
    double h = (b - a) / n_splits;
    double sum = func(a) + func(b);

    for (int i = 1; i < n_splits; ++i) {
        double x = a + i * h;
        if (i % 2 == 1) {
            sum += 4 * func(x); // odd with coef=4
        } else {
            sum += 2 * func(x); // even with coef=2
        }
    }

    return sum * h / 3;
}
#else
double
trapezium(double (*func)(double), double a, double b, int n_splits) {
    double h = (b - a) / n_splits;
    double sum = (func(a) + func(b)) / 2.0;

    for (int i = 1; i < n_splits; ++i) {
        double x = a + i * h;
        sum += func(x);
    }

    return sum * h;
}
#endif

double
integral(double (*func)(double), double a, double b, double eps2) {
    int n_splits = 10000;
    double I_old, I_new;
#if USE_SIMPSON
    I_new = simpson(func, a, b, n_splits);

    do {
        I_old = I_new;
        n_splits *= 2;
        I_new = simpson(func, a, b, n_splits);
    } while (fabs(I_new - I_old) / 15 > eps2);
#else
    I_new = trapezium(func, a, b, n_splits);

    do {
        I_old = I_new;
        n_splits *= 2;
        I_new = trapezium(func, a, b, n_splits);
    } while (fabs(I_new - I_old) / 3.0 > eps2);
#endif

    return I_new;
}
