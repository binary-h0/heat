#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void handler(int signo, siginfo_t *info, void *context) {
    printf("check\n");
    printf("signo: %d\n", signo);
    printf("info: %d\n", info->si_pid);

    exit(0);
}

int main(int argc, char const *argv[]) {
    printf("dummy is running\n");
    system("ps -ef");
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGUSR1);

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    while (1) {
        usleep(1000 * 0.1);
    }

    return 0;
}
