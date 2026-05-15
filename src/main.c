#include <stdio.h>
#include <numeric.h>
#include <math.h>
#include <getopt.h>

#define EPS1 1e-9
#define EPS2 1e-9

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
    double *xs = crosses(a, b, EPS1, print_iters);

    if (print_absc) printf("points: %lf %lf %lf\n", xs[0], xs[1], xs[2]);

    double points[] = {a, xs[0], xs[1], xs[2], b};
    f lowest;
    double area = 0;

    for (int i = 0; i < 4; ++i) {
        lowest = compare((points[i]+points[i+1])/2, &f_1, &f_2, &f_3);
        area += integral(lowest, points[i], points[i+1], EPS2);
    }

    printf("Area: %lf\n", area);
    return 0;
}

