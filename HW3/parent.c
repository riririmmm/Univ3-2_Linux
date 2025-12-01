#include "pcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

PCB pcb[NPROC];
volatile sig_atomic_t io_req[NPROC];
volatile sig_atomic_t term_req[NPROC];

int find_index(pid_t pid) {
    for (int i = 0; i < NPROC; i++) if (pcb[i].pid == pid) return i;
    return -1;
}

void io_handler(int sig, siginfo_t *info, void *ctx) {
    int idx = find_index(info->si_pid);
    if (idx >= 0) io_req[idx] = 1;
}

void chld_handler(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        int idx = find_index(pid);
        if (idx >= 0) term_req[idx] = 1;
    }
}

int all_done() {
    for (int i = 0; i < NPROC; i++)
        if (pcb[i].state != DONE) return 0;
    return 1;
}

int all_tq_zero() {
    for (int i = 0; i < NPROC; i++) {
        if ((pcb[i].state == READY || pcb[i].state == RUNNING) &&
            pcb[i].tq_rem > 0)
            return 0;
    }
    return 1;
}

int find_next_ready(int cur) {
    for (int step = 1; step <= NPROC; step++) {
        int i = (cur + step) % NPROC;
        if (pcb[i].state == READY) return i;
    }
    return -1;
}

int main() {
    srand(time(NULL));

    struct sigaction sa_io;
    memset(&sa_io, 0, sizeof(sa_io));
    sa_io.sa_sigaction = io_handler;
    sa_io.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &sa_io, NULL);

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = chld_handler;
    sigaction(SIGCHLD, &sa_chld, NULL);

    // create children
    for (int i = 0; i < NPROC; i++) {
        pid_t pid = fork();
        if (pid == 0) execl("./child", "./child", NULL);

        pcb[i] = (PCB){ pid, READY, TIME_QUANTUM, 0, 0 };
        io_req[i] = term_req[i] = 0;
    }

    int cur = -1, tick = 0;

    while (!all_done()) {
        tick++;

        // I/O 처리
        for (int i = 0; i < NPROC; i++) {
            if (pcb[i].state == SLEEP && pcb[i].io_rem > 0) {
                pcb[i].io_rem--;
                if (pcb[i].io_rem == 0) {
                    pcb[i].state = READY;
                    pcb[i].tq_rem = TIME_QUANTUM;
                    printf("[tick %d] PID=%d I/O done → READY\n", tick, pcb[i].pid);
                }
            }
        }

        // I/O 요청 반영
        for (int i = 0; i < NPROC; i++) {
            if (io_req[i]) {
                io_req[i] = 0;
                pcb[i].state = SLEEP;
                pcb[i].io_rem = (rand() % (IO_TICKS_MAX - IO_TICKS_MIN + 1)) + IO_TICKS_MIN;
                pcb[i].tq_rem = 0;
                if (cur == i) cur = -1;
            }
            if (term_req[i]) {
                term_req[i] = 0;
                pcb[i].state = DONE;
                pcb[i].tq_rem = 0;
                pcb[i].io_rem = 0;
                if (cur == i) cur = -1;
            }
        }

        // 스케줄러 선택
        if (cur == -1 || pcb[cur].state != RUNNING || pcb[cur].tq_rem <= 0) {
            if (cur != -1 && pcb[cur].state == RUNNING)
                pcb[cur].state = READY;

            cur = find_next_ready(cur);
            if (cur != -1) {
                pcb[cur].state = RUNNING;
                if (pcb[cur].tq_rem == 0)
                    pcb[cur].tq_rem = TIME_QUANTUM;
            }
        }

        // READY 대기시간 증가
        for (int i = 0; i < NPROC; i++)
            if (pcb[i].state == READY) pcb[i].waiting_time++;

        // 실행
        if (cur != -1 && pcb[cur].state == RUNNING) {
            printf("[tick %d] RUN PID=%d tq=%d\n", tick, pcb[cur].pid, pcb[cur].tq_rem);
            kill(pcb[cur].pid, SIGUSR1);
            pcb[cur].tq_rem--;
        }

        if (all_tq_zero()) {
            for (int i = 0; i < NPROC; i++)
                if (pcb[i].state == READY || pcb[i].state == RUNNING)
                    pcb[i].tq_rem = TIME_QUANTUM;

            printf("[tick %d] Reset time quantum\n", tick);
        }

        usleep(200000);
    }

    // 결과 출력
    printf("\n=== 결과 ===\n");
    int total = 0;
    for (int i = 0; i < NPROC; i++) {
        printf("PID=%d wait=%d\n", pcb[i].pid, pcb[i].waiting_time);
        total += pcb[i].waiting_time;
    }
    printf("평균 대기시간 = %.2f\n", total * 1.0 / NPROC);

    return 0;
}

