#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEPS 1e-12
#define MAX_ITER 1000000

double
f_1(double x);
double
f_2(double x);
double
f_3(double x);

double
d_1(double x);
double
d_2(double x);
double
d_3(double x);

double*
crosses(double a, double b, double eps1, int print_iters);










double
simpson(double (*func)(double), double a, double b, int n_splits);

double
trapezium(double (*func)(double), double a, double b, int n_splits);

double
integral(double (*func)(double), double a, double b, double eps2);
