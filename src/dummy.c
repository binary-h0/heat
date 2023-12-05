#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// #define _GNU_SOURCE

int pid, len, fd[2], ret_signo;
char buf[1024];
sigset_t block_mask, sigset_mask;
FILE *fp;

void handler(int signo, siginfo_t *info, void *context) {
    printf("signo: %d | ", signo);
    printf("sender_pid: %d | ", info->si_pid);
    printf("value: %d | ", info->si_value.sival_int);
    printf("my_pid: %d \n", pid);
    switch (signo) {
        case SIGUSR1:
            printf("SIGUSR1\n");
            break;
        case SIGALRM:
            printf("SIGALRM\n");
            break;
        case SIGINT:
            printf("SIGINT\n");
            exit(1);
            break;
        case SIGCHLD:
            printf("SIGCHLD\n");
            break;
        default:
            break;
    }
}

int main(int argc, char const *argv[]) {
    printf("dummy is running\n");
    printf("\e[1;31mTEST START \e[0m\n");
    printf("\e[s\e[F\e[%dC", 5);
    printf("\e[5@TEST\n\e[u");
    printf("\e[1;32mTEST DONE \e[0m\n");
    // 흰색으로 출력
    printf("\e[1;37mTEST START \e[0m\n");
    system("ps -ef");

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    // sigfillset(&block_mask);
    // sigdelset(&block_mask, SIGINT);
    // if (sigprocmask(SIG_SETMASK, &block_mask, NULL) == -1) {
    //     perror("sigprocmask");
    //     exit(1);
    // }
    int status;
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }
    while (1) {
        printf("TEST\n");
        usleep(1000 * 500);
        pause();
    }
}