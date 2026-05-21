#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_ITER 10000

typedef double (*f)(double);

/// where to output iters
FILE *outp;

/// @defgroup root_methods Find Root Methods
/// @brief Numeric methods to find root of an equation
/// (more specifically, a point of intersection of two curves)
/// {@
#if USE_NEWTON
/// Newton method.
/// tangent equation in point x=x_n:
/// y = (f'(x) - g'(x)) * (x - x_n) + (f(x_n) - g(x_n))
/// next step is where tangent crosses X axis, so, y=0
/// x = x_n - (f(x) - g(x)) / (f'(x_n) - g'(x_n))
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
    double x = (a + b) / 2; // lets use the middle

    while (iter < MAX_ITER) {
        y = f(x) - g(x);
        if (fabs(y) < eps1) break; // root is found

        dy = f_der(x) - g_der(x);
        
        x = x - y/dy;
        if (x > b) x = b; // if x got too big, get it back
        if (x < a) x = a; //  if x got too small, get it back

        ++iter;
    }

    fprintf(outp, "iters: %d, x=%lf\n", iter, x);
    return x;
}
#else
/// Secant method.
/// secant equation through points a and b:
/// y = (f(a) - g(a)) + ((f(b) - g(b)) - (f(a) - g(a)) * (x - a) / (b - a)
/// let y=0
/// -(f(a) - g(a)) = ((f(b) - g(b)) - (f(a) - g(a)) * (x - a) / (b - a)
/// x - a = -(f(a) - g(a)) * (b - a) / ((f(b) - g(b)) - (f(a) - g(a))
/// x = a - (f(a) - g(a)) * (b - a) / ((f(b) - g(b)) - (f(a) - g(a))
///
/// we have segments (a; x) and (x; b)
/// determine which segment to take, by checking which one has the sign different on its start and end
double
root(double (*f)(double),
    double (*g)(double),
    double a,
    double b,
    double eps1) {

    int iter = 0;
    double ya = f(a) - g(a);
    double yb = f(b) - g(b);
    double x, y;

    while (iter < MAX_ITER) {
        x = a - ya * (b - a) / (yb - ya);
        y = f(x) - g(x);

        if (fabs(y) < eps1) {
            fprintf(outp, "iters: %d\n", iter);
            return x;
        }

        // false position method
        if (ya * y < 0) {
            b = x;
            yb = y;
        } else {
            a = x;
            ya = y;
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

/// sorts array of three elements
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

/// a wrapper, finds intersections of three given curves
#if USE_NEWTON
double*
crosses(f f1, f f2, f f3, f d1, f d2, f d3, double a, double b, double eps1, int print_iters) {
    // if no -i flag, output it straight to /dev/null blackhole
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
    // if no -i flag, output it straight to /dev/null blackhole
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

/// @}







/// @defgroup int_methods Integration Methods
/// @brief Numeric methods to find a definite integral of a function
/// {@

/// Simpson rule.
/// for one split:
/// let h = length of one segment
/// ∫ = h/3 * (f(a) + 4f(x1) + f(b))
/// for more splits, it is h/3 * (f(a) + 4f(x1) + 2f(x2) + ... + f(b))
/// check paper for more details
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
/// Trapezium rule.
/// for one split
/// let h = length of segment
/// ∫ = (f(xi) + f(x{i+1}))/2 * h = f(xi)/2 * h + f(x{i+1})/2 * h
/// for more splits, each point except a and b appears twice
/// ∫ = (f(a)/2 + f(x1)/2 + f(x1)/2 + f(x2)/2 + ... + f(b)/2) * h = 
/// = (f(a)/2 + f(x1) + ... + f(b)/2) * h
double
trapezium(double (*func)(double), double a, double b, int n_splits) {
    double h = (b - a) / n_splits;
    double sum = (func(a) + func(b)) / 2.0;

    for (int i = 1; i < n_splits; ++i) {
        double x = a + i * h; // xi
        sum += func(x);
    }

    return sum * h;
}
#endif

/// repeats integration with increasing number of splits until eps2 is reached.
/// Runge rule is used, check paper for more details
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

/// @}
