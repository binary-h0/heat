#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void handler(int signo, siginfo_t *info, void *context) {
    printf("check ");
    printf("signo: %d ", signo);
    printf("info: %d ", info->si_pid);
    printf("value: %d ", info->si_value.sival_int);
    if (signo == SIGALRM) {
        printf("timer\n");
    } else if (signo == SIGUSR1) {
        printf("usr1\n");
    } else if (signo == SIGINT) {
        exit(0);
    }
}

timer_t timer1, timer2;

void make_sev() {
    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_int = 1;
    if (timer_create(CLOCK_REALTIME, &sev, &timer1) == -1) {
        perror("timer_create");
        exit(1);
    }
    sev.sigev_value.sival_int = 0;
    timer_create(CLOCK_REALTIME, &sev, &timer2);
}

int main(int argc, char const *argv[]) {
    printf("dummy is running\n");
    system("ps -ef");
    // sigemptyset(&sa.sa_mask);
    // sigaddset(&sa.sa_mask, SIGUSR1);

    make_sev();
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    struct itimerspec it;
    it.it_value.tv_sec = 1;
    it.it_value.tv_nsec = 0;
    it.it_interval.tv_sec = 1;
    it.it_interval.tv_nsec = 0;

    timer_settime(timer1, 0, &it, NULL);
    it.it_interval.tv_sec = 2;
    timer_settime(timer2, 0, &it, NULL);
    while (1) {
        usleep(1000 * 0.1);
    }

    return 0;
}
