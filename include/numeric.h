#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEPS 1e-12
#define MAX_ITER 1000000

typedef double (*f)(double);

#if USE_NEWTON
double*
crosses(f f1, f f2, f f3, f d1, f d2, f d3, double a, double b, double eps1, int print_iters);
#else
double*
crosses(f f1, f f2, f f3, double a, double b, double eps1, int print_iters);
#endif










double
simpson(double (*func)(double), double a, double b, int n_splits);

double
trapezium(double (*func)(double), double a, double b, int n_splits);

double
integral(double (*func)(double), double a, double b, double eps2);
