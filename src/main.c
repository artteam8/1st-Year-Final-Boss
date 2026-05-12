#include <stdio.h>
#include <numeric.h>

#define EPS 1e-3
#define N_SPLITS 1000

extern double f_1(double x);
extern double f_2(double x);
extern double f_3(double x);

typedef double (*f)(double);

f
compare(double x, f f_1, f f_2, f f_3) {
    if (f_1(x) < f_2(x)) {
        if (f_3(x) < f_1(x)) return f_3;
        else return f_1;
    } else {
        if (f_2(x) < f_3(x)) return f_2;
        else return f_3;
    }
}


double
simpson(f func, double left, double right) {
    double h = (right - left) / N_SPLITS;
    double sum = func(left) + func(right);
    
    for (int i = 1; i < N_SPLITS; ++i) {
        double x = left + i * h;
        if (i % 2 == 1) {
            sum += 4 * func(x); // odd with coef=4
        } else {
            sum += 2 * func(x); // even with coef=2
        }
    }
    
    return sum * h / 3;
}

int
main(void) {
    FILE *spec_file = fopen(SPEC_FILE, "r");
    double a, b;
    fscanf(spec_file, "%lf%lf", &a, &b);
    double *xs = crosses(a, b, EPS);
    
    double points[] = {a, xs[0], xs[1], xs[2], b};
    f lowest;
    double area = 0;

    for (int i = 0; i < 4; ++i) {
        lowest = compare((points[i]+points[i+1])/2, &f_1, &f_2, &f_3);
        printf("%lf %lf\n", points[i], points[i+1]);
        area += simpson(lowest, points[i], points[i+1]);
    }

    printf("%lf\n", area);
    return 0;
}

