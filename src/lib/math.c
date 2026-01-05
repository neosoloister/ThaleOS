#include "math.h"

double absl(double x) {
    if (x < 0) return -x;
    return x;
}

double power(double base, int expo) {
    if (expo == 0) return 1;
    if (expo > 0) return base * power(base, expo - 1);
    if (expo < 0) {
        expo *= -1;
        return 1 / power(base, expo);
    }
    return 0;
}

double sqrt(double S) {
    double x = S / 2;
    while (1) {
        double xn = 0.5 * (x + S / x);
        if (absl(S - power(xn, 2)) <= 0.0000001) return xn;
        x = xn;
    }
}