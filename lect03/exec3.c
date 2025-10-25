#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* 자식 프로세스를 생성하여 echo 명령어 실행 */
int main() {
	if (fork() == 0) {
		char *argv[3];
		argv[0] = "echo"; argv[1] = "hello\n"; argv[2] = NULL;
		execv("/bin/echo", argv);
	}

	printf("End of parent process\n");
	return 0;
}

