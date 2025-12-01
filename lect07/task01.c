#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

pid_t child_pid;
volatile sig_atomic_t done_dan;

void parent_handler(int signo) {
    if (signo == SIGINT) {
        kill(child_pid, SIGINT);
    }
}

void child_handler(int signo) {
    if (signo == SIGINT) {
        if (done_dan < 9) {
            printf("구구단 실행 중이니 종료 불가\n");
        } else {
            printf("9단 이상 출력 완료, 종료합니다.\n");
            kill(getppid(), SIGKILL); 
            kill(getpid(), SIGKILL);
        }
    }
}

int main(void)
{
    child_pid = fork();

    if (child_pid < 0) {
        perror("fork");
        exit(1);
    }

    if (child_pid == 0) {
        signal(SIGINT, child_handler);  
        done_dan = 1;

        for (int dan = 2; dan <= 20; dan++) {
            printf("=== %d단 ===\n", dan);
            for (int i = 1; i <= 9; i++) {
                printf("%d x %d = %d\n", dan, i, dan * i);
                sleep(1);
            }
            done_dan = dan; 
            sleep(1);
        }

        return 0;
    }

    else {
        signal(SIGINT, parent_handler); 

        while (1) {
            sleep(1);
        }
    }

    return 0;
}

