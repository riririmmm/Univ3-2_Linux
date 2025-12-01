#define _XOPEN_SOURCE 700
#include "pcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

int	child_cpu_burst;
pid_t	parent_pid;
int	did_io = 0;

void child_handler(int sig) {
    (void)sig;

    if (child_cpu_burst <= 0) return;

    child_cpu_burst--;
    printf("[child %d] CPU tick, burst=%d\n", getpid(), child_cpu_burst);

    // 더 이상 남은 burst가 없을 때
    if (child_cpu_burst <= 0) {
        if (!did_io) {
            int r = rand() % 2;   // 0: 종료, 1: I/O
            if (r == 0) {
                printf("[child %d] didn't I/O and finish\n", getpid());
                _exit(0);
            } else {
                did_io = 1;
                printf("[child %d] request I/O\n", getpid());
                kill(parent_pid, SIGUSR2);
                child_cpu_burst = (rand() % 10) + 1;
            }
        } else {
            printf("[child %d] did I/O and finish\n", getpid());
            exit(0);
		}
	}
}

int main(void) {
    srand((unsigned int)(time(NULL) ^ getpid()));

    parent_pid      = getppid();
    child_cpu_burst = (rand() % 10) + 1;   // 초기 CPU burst (1~10)

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = child_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    printf("[child %d] created, burst=%d\n", getpid(), child_cpu_burst);

    while (1) {
        pause();
    }

    return 0;
}

