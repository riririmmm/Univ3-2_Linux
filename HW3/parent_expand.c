#include "pcb_expand.h"
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

typedef enum {
    SCHED_RR = 0,
    SCHED_PRIO
} SchedPolicy;

SchedPolicy sched_policy = SCHED_RR;

int find_index(pid_t pid) {
    for (int i = 0; i < NPROC; i++)
        if (pcb[i].pid == pid)
            return i;
    return -1;
}

void io_handler(int sig, siginfo_t *info, void *ctx) {
    (void)sig;
    (void)ctx;
    int idx = find_index(info->si_pid);
    if (idx >= 0)
        io_req[idx] = 1;
}

void chld_handler(int sig) {
    (void)sig;
    int   status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        int idx = find_index(pid);
        if (idx >= 0)
            term_req[idx] = 1;
    }
}

int all_done(void) {
    for (int i = 0; i < NPROC; i++) {
        if (pcb[i].state != DONE)
            return 0;
    }
    return 1;
}

int all_tq_zero(void) {
    for (int i = 0; i < NPROC; i++) {
        if ((pcb[i].state == READY || pcb[i].state == RUNNING) &&
            pcb[i].tq_rem > 0)
            return 0;
    }
    return 1;
}

int find_next_ready_rr(int cur) {
    for (int step = 1; step <= NPROC; step++) {
        int i = (cur + step) % NPROC;
        if (pcb[i].state == READY)
            return i;
    }
    return -1;
}

int find_next_ready_prio(void) {
    int best = -1;
    int best_pri = -1;
    for (int i = 0; i < NPROC; i++) {
        if (pcb[i].state == READY && pcb[i].priority > best_pri) {
            best_pri = pcb[i].priority;
            best     = i;
        }
    }
    return best;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    // 인자로 "prio" 주면 우선순위 스케줄링
    if (argc >= 2 && strcmp(argv[1], "prio") == 0) {
        sched_policy = SCHED_PRIO;
        printf("Scheduling policy: PRIORITY\n");
    } else {
        sched_policy = SCHED_RR;
        printf("Scheduling policy: ROUND ROBIN\n");
    }

    struct sigaction sa_io;
    memset(&sa_io, 0, sizeof(sa_io));
    sa_io.sa_sigaction = io_handler;
    sa_io.sa_flags     = SA_SIGINFO;
    sigaction(SIGUSR2, &sa_io, NULL);

    // SIGCHLD : 자식 종료
    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));
    sa_chld.sa_handler = chld_handler;
    sigaction(SIGCHLD, &sa_chld, NULL);

    // 자식 생성
    for (int i = 0; i < NPROC; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {
            execl("./child_expand", "./child_expand", NULL);
            perror("execl");
            _exit(1);
        }

        pcb[i].pid          = pid;
        pcb[i].state        = READY;
        pcb[i].tq_rem       = TIME_QUANTUM;
        pcb[i].io_rem       = 0;
        pcb[i].waiting_time = 0;
        // 우선순위: 1~3 범위에서 랜덤
        pcb[i].priority     = (rand() % 3) + 1;

        io_req[i]   = 0;
        term_req[i] = 0;
    }

    int cur  = -1;
    int tick = 0;

    while (!all_done()) {
        tick++;

        for (int i = 0; i < NPROC; i++) {
            if (pcb[i].state == SLEEP && pcb[i].io_rem > 0) {
                pcb[i].io_rem--;
                if (pcb[i].io_rem == 0) {
                    pcb[i].state  = READY;
                    pcb[i].tq_rem = TIME_QUANTUM;
                    printf("[tick %d] PID=%d I/O done → READY\n",
                           tick, pcb[i].pid);
                }
            }
        }

        for (int i = 0; i < NPROC; i++) {
            if (io_req[i]) {
                io_req[i]     = 0;
                pcb[i].state  = SLEEP;
                pcb[i].io_rem = (rand() % (IO_TICKS_MAX - IO_TICKS_MIN + 1))
                              + IO_TICKS_MIN;
                pcb[i].tq_rem = 0;
                if (cur == i)
                    cur = -1;
            }
            if (term_req[i]) {
                term_req[i]     = 0;
                pcb[i].state    = DONE;
                pcb[i].tq_rem   = 0;
                pcb[i].io_rem   = 0;
                if (cur == i)
                    cur = -1;
            }
        }

        int prev = cur;

        if (cur == -1 || pcb[cur].state != RUNNING || pcb[cur].tq_rem <= 0) {
            if (cur != -1 && pcb[cur].state == RUNNING)
                pcb[cur].state = READY;

            if (sched_policy == SCHED_RR) {
                cur = find_next_ready_rr(cur);
            } else {
                cur = find_next_ready_prio();
            }

            if (cur != -1) {
                pcb[cur].state = RUNNING;
                if (pcb[cur].tq_rem == 0)
                    pcb[cur].tq_rem = TIME_QUANTUM;
            }
        }

        // 컨텍스트 스위칭 시 SIGSTOP / SIGCONT 사용
        if (prev != cur) {
            if (prev != -1 && pcb[prev].state != DONE) {
                kill(pcb[prev].pid, SIGSTOP);
            }
            if (cur != -1 && pcb[cur].state == RUNNING) {
                kill(pcb[cur].pid, SIGCONT);
            }
        }

        for (int i = 0; i < NPROC; i++) {
            if (pcb[i].state == READY)
                pcb[i].waiting_time++;
        }

        if (cur != -1 && pcb[cur].state == RUNNING) {
            printf("[tick %d] RUN PID=%d tq=%d prio=%d\n",
                   tick, pcb[cur].pid, pcb[cur].tq_rem, pcb[cur].priority);
            kill(pcb[cur].pid, SIGUSR1);
            pcb[cur].tq_rem--;
        }

        if (all_tq_zero()) {
            for (int i = 0; i < NPROC; i++) {
                if (pcb[i].state == READY || pcb[i].state == RUNNING)
                    pcb[i].tq_rem = TIME_QUANTUM;
            }
            printf("[tick %d] Reset time quantum\n", tick);
        }

        usleep(200 * 1000);
    }

    printf("\n=== 결과 ===\n");
    int total = 0;
    for (int i = 0; i < NPROC; i++) {
        printf("PID=%d prio=%d wait=%d\n",
               pcb[i].pid, pcb[i].priority, pcb[i].waiting_time);
        total += pcb[i].waiting_time;
    }
    printf("평균 대기시간 = %.2f\n", total * 1.0 / NPROC);

    return 0;
}

