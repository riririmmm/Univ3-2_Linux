#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int num1 = atoi(argv[1]);
	char op = argv[2][0];
	int num2 = atoi(argv[3]);
	int result;

	switch(op) {
		case '+':
			result = num1 + num2;
			break;
		case '-':
			result = num1 - num2;
			break;
		case '*':
			result = num1 * num2;
			break;
		case '/':
			result = num1 / num2;
			break;
	}

	printf("%d\n", result);
	return 0;
}

