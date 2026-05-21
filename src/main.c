#include <stdio.h>
#include <numeric.h>
#include <math.h>
#include <getopt.h>
#include <stdlib.h>

#define EPS1 1e-4
#define EPS2 1e-4

extern double f_1(double x);
extern double f_2(double x);
extern double f_3(double x);
extern double d_1(double x);
extern double d_2(double x);
extern double d_3(double x);

typedef double (*f)(double);

/// sorts array of three functions in points x
void
sort_func(double x, f fs[3]) {
    if (fs[0](x) > fs[1](x)) {
        f temp = fs[1];
        fs[1] = fs[0];
        fs[0] = temp;
    }
    if (fs[1](x) > fs[2](x)) {
        f temp = fs[2];
        fs[2] = fs[1];
        fs[1] = temp;
        if (fs[0](x) > fs[1](x)) {
            temp = fs[1];
            fs[1] = fs[0];
            fs[0] = temp;
        }
    }
}

/// I/O and general logic
int
main(int argc, char *argv[]) {
    int print_absc = 0;
    int print_iters = 0;
    struct option cli_keys[] = {{"help", no_argument, 0,  'h'}, {0,0,0,0}};


    
    int opt;
    while ((opt = getopt_long(argc, argv, "ai", cli_keys, NULL)) != -1) {
        switch(opt) {
            case 'a':
                print_absc = 1;
                break;
            case 'i':
                print_iters = 1;
                break;
            case 'h':
                printf("--help       show this message and quit\n -a          show intersection points\n -i          show number of iterations\n");
                return 0;
        }
    }


#if USE_NEWTON
    printf("Newton method is used\n");
#else
    printf("Secant method is used\n");
#endif

#if USE_SIMPSON
    printf("Simpson rule is used\n");
#else
    printf("Trapezium rule is used\n");
#endif


    FILE *spec_file = fopen(SPEC_FILE, "r");
    double a, b;
    fscanf(spec_file, "%lf%lf", &a, &b);
    //printf("a,b: %lf %lf\n", a, b);
#if USE_NEWTON
    double *xs = crosses(f_1, f_2, f_3, d_1, d_2, d_3, a, b, EPS1, print_iters);
#else
    double *xs = crosses(f_1, f_2, f_3, a, b, EPS1, print_iters);
#endif

    if (print_absc) printf("points: %lf %lf %lf\n", xs[0], xs[1], xs[2]);


    f fs[] = {f_1, f_2, f_3};
    double area = 0;
    int sign = 1;

    // determine if the longer curve is lower than the point of intersection of two short ones
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (i != j && fabs(fs[i](xs[1]) - fs[j](xs[1])) < EPS1) {
                int k = 0+1+2 - i - j;
                if (fs[k](xs[1]) < fs[i](xs[1])) {
                    sign = -1;
                }
                break;
            }
        }
    }

    for (int i = 0; i < 2; ++i) {
        sort_func((xs[i]+xs[i+1])/2, fs);
        area += sign * (integral(fs[1 + sign], xs[i], xs[i+1], EPS2) - integral(fs[1], xs[i], xs[i+1], EPS2));
    }

    printf("Area: %.4lf\n", area);
    free(xs);
    return 0;
}

