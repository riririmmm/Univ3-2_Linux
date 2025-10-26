#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define N 4

void sinx_taylor(int n, int terms, double x[], double res[]) {
    for (int i = 0; i < n; i++) {
        double val = 0;
        double t = x[i];
        int sign = 1;

        for (int j = 1; j <= terms; j += 2) {
            val += sign * pow(x[i], j) / tgamma(j + 1);
            sign *= -1;
        }
        res[i] = val;
    }
}


int main() {
	double x[N] = { 0, M_PI/6, M_PI/3., 0.134 };
	double res[N];

	sinx_taylor(N, 3, x, res);
	for (int i = 0; i < N; i++) {
		printf("sin(%.2f) by Taylor series = %f\n", x[i], res[i]);
		printf("sin(%.2f) = %f\n", x[i], sin(x[i]));
	}

	return 0;
}

