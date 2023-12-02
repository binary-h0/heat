#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

int pid;

void handler(int signo, siginfo_t *info, void *context) {
    printf("check %d ", pid);
    printf("signo: %d ", signo);
    printf("info: %d ", info->si_pid);
    printf("value: %d\n", info->si_value.sival_int);
}

int main(int argc, char const *argv[]) {
    printf("dummy is running\n");
    system("ps -ef");
    // sigemptyset(&sa.sa_mask);
    // sigaddset(&sa.sa_mask, SIGUSR1);

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);

    switch (pid = fork()) {
        case -1:
            perror("fork");
            exit(1);
            break;
        case 0:
            printf("child is running\n");
            while (1) {
                usleep(1000 * 2000);
                printf("child is running\n");
            }
            break;
        default:
            break;
    }

    while (1) {
        usleep(1000 * 4000);
        printf("dummy is running\n");
    }

    return 0;
}
