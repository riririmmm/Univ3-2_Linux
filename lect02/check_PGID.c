#include <unistd.h>
#include <stdio.h>

int main() {
	printf("PID: %d\n", (int)getpid());
	printf("PGRP: %d\n", (int)getpgrp());
	printf("PGID(0): %d\n", (int)getpgid(0));
	printf("PGID(1667): %d\n", (int)getpgid(1667));
}

