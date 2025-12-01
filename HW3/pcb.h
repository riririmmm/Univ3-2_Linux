#ifndef PCB_H
#define PCB_H

#include <sys/types.h>

#define NPROC        10
#define TIME_QUANTUM 3
#define IO_TICKS_MIN 3
#define IO_TICKS_MAX 7

typedef enum {
    NEW = 0,
    READY,
    RUNNING,
    SLEEP,
    DONE
} state_t;

typedef struct {
    pid_t   pid;
    state_t state;
    int     tq_rem;
    int     io_rem;
    int     waiting_time;
} PCB;

#endif

