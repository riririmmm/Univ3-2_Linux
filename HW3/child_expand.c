#define _XOPEN_SOURCE 700
#include "pcb_expand.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

int   child_cpu_burst;
pid_t parent_pid;
int   did_io = 0;

#define MID_IO_PROB 20      // 1/20 ≒ 5%
#define MAX_BURST 100

/*
기본 구현: burst 종료 후 I/O 요청
*/
//void child_handler(int sig) {
//    (void)sig;
//
//    if (child_cpu_burst <= 0)  return;
//
//    child_cpu_burst--;
//    printf("[child %d] CPU tick, burst=%d\n", getpid(), child_cpu_burst);
//
//    // 더 이상 남은 burst가 없을 때
//    if (child_cpu_burst <= 0) {
//        if (!did_io) {
//            int r = rand() % 2;   // 0: 종료, 1: I/O
//            if (r == 0) {
//                printf("[child %d] didn't I/O and finish\n", getpid());
//                _exit(0);
//            } else {
//                did_io = 1;
//                printf("[child %d] request I/O\n", getpid());
//                kill(parent_pid, SIGUSR2);
//                child_cpu_burst = (rand() % 10) + 1;
//            }
//        } else {
//            printf("[child %d] did I/O and finish\n", getpid());
//            _exit(0);
//        }
//    }
//}


/*
확장: 랜덤으로 I/O 발생
*/
void child_handler(int sig) {
    (void)sig;

    if (child_cpu_burst <= 0) return;

    child_cpu_burst--;
    printf("[child %d] CPU tick, burst=%d\n", getpid(), child_cpu_burst);

    // 아직 I/O를 한 번도 안 했고, burst가 남아있는 경우
    if (!did_io && child_cpu_burst > 0) {
        if (rand() % MID_IO_PROB == 0) {
            did_io = 1;
            printf("[child %d] request I/O (mid, rem=%d)\n",
                getpid(), child_cpu_burst);

            kill(parent_pid, SIGUSR2);

            child_cpu_burst = (rand() % MAX_BURST) + 1;
            return;
        }
    }

    // CPU burst를 다 쓴 경우
    if (child_cpu_burst <= 0) {
        if (!did_io) {
            int r = rand() % 2;
            if (r == 0) {
                printf("[child %d] finish without I/O\n", getpid());
                _exit(0);
            }
            else {
                did_io = 1;
                printf("[child %d] request I/O (at end)\n", getpid());
                kill(parent_pid, SIGUSR2);
                child_cpu_burst = (rand() % MAX_BURST) + 1;
            }
        }
        else {
            printf("[child %d] did I/O and finish\n", getpid());
            _exit(0);
        }
    }
}

int main(void) {
    srand((unsigned int)(time(NULL) ^ getpid()));

    parent_pid      = getppid();
    child_cpu_burst = (rand() % MAX_BURST) + 1;   // 초기 CPU burst (1~10)

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
