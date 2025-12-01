#ifndef PCB_H
#define PCB_H

#include <sys/types.h>

#define NPROC        20
#define TIME_QUANTUM 10
#define IO_TICKS_MIN 3
#define IO_TICKS_MAX 7

typedef enum {
    NEW = 0,
    READY,
    RUNNING,
    SLEEP,
    DONE
} State;

typedef struct {
    pid_t pid;
    State state; 
    int   tq_rem;
    int   io_rem;
    int   waiting_time;
    int   priority;             // 우선순위 (숫자 클수록 우선순위 높다고 가정)
} PCB;

#endif

