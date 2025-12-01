#include <stdio.h>
#include <unistd.h>

int main() {
	alarm(5);
	printf("loop... \n");

	while(1) {
		sleep(1);
		printf("1 sec...\n");
	}

	printf("End of main\n");
}

