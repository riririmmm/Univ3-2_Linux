#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

#define N 4
#define MAXLINE 100

void sinx_taylor(int num_elements, int terms, double* x, double* result) {
	int child_id;
	int fd[2*N], length;
	pid_t pid;
	char message[MAXLINE], line[MAXLINE];
	
	for(int i = 0; i < num_elements; i++) {
		pipe(fd + 2*i);

		child_id = i;
		pid = fork();
		if (pid == 0) {
			break;
		} else {
			close(fd[2*i + 1]);
		}
	}
	
	if(pid == 0){
		int i = child_id;
		close(fd[2*i]);

		double value = x[i];
		double numer = x[i] * x[i] * x[i];
		double denom = 6.;    	
		int sign = -1;

		for (int j = 1; j <= terms; j++){
			value += (double)sign * numer / denom;
			numer *= x[i] * x[i];
			denom *= (2.*(double)j + 2.) * (2.*(double)j + 3.);
			sign *= -1;
		}
		
		result[i] = value;
		sprintf(message, "%lf", result[i]);
		length = strlen(message) + 1;
		write(fd[2*i+1], message, length);

		exit(child_id);
	} else {
		for (int i = 0; i<num_elements; i++) {
			int status;
			wait(&status);
    			int child_id = status >> 8;
			read(fd[2*child_id], line, MAXLINE);
			result[child_id] = atof(line);
		}
	}
}

int main() {
	double x[N] = {0, M_PI / 6., M_PI / 3., 0.134};
	double res[N];
	int fd[N][2];

	sinx_taylor(N, 10, x, res);

	for (int i = 0; i < N; i++) {
		printf("sin(%4.2f) by Taylor series = %.6f\n", x[i], res[i]);
		printf("sin(%4.2f) = %.6f\n", x[i], sin(x[i]));
	}

	return 0;
}

