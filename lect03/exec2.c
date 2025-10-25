#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	printf("Parent process\n");
	if (fork() == 0) {
		printf("Child process\n");
		execl("/bin/echo", "echo", "hello\n", NULL);
		printf("End of child process \n");
	}

	printf("End of parent proces\n");
	return 0;
}

