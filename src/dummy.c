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
            // ret_signo = sigwaitinfo(&sigset_mask, info);  // info?
            // if (ret_signo == -1) {
            //     perror("sigwait");
            //     exit(1);
            // }
            if (info->si_band & POLL_IN) {
                if ((len = read(fd[0], buf, sizeof(buf))) != 0) {
                    write(STDOUT_FILENO, "parent: ", 8);
                    write(STDOUT_FILENO, buf, len);
                }
            }
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

void enable_rts_event(int fd, int sig) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1) {
        perror("fcntl 1");
        exit(1);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK | O_ASYNC) == -1) {
        perror("fcntl 2");
        exit(1);
    }
    if (fcntl(fd, F_SETSIG, sig) == -1) {
        perror("fcntl 3");
        exit(1);
    }
    if (fcntl(fd, F_SETOWN, getpid()) == -1) {
        perror("fcntl 4");
        exit(1);
    }
}

int main(int argc, char const *argv[]) {
    printf("dummy is running\n");
    printf("\e[1;31mTEST START \e[0m\n");
    printf("\e[s\e[F\e[%dC", 5);
    printf("\e[5@TEST\n\e[u");
    printf("\e[1;32mTEST DONE \e[0m\n");

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
    enable_rts_event(fd[0], SIGUSR1);
    // enable_rts_event(fd[1], SIGUSR1);
    sigemptyset(&sigset_mask);
    sigaddset(&sigset_mask, SIGUSR1);

    // FILE
    if ((fp = fopen("./log/heat.verbose.log", "r+")) == NULL) {
        perror("fopen");
        exit(1);
    }
    int fd_fp = fileno(fp);

    write(fd_fp, "\thello\n", 7);
    write(fd_fp, "hello\n", 6);
    off_t offset = lseek(fd_fp, 0, SEEK_CUR);
    printf("offset: %ld\n", offset);
    offset = lseek(fd_fp, 6, SEEK_SET);
    printf("offset: %ld\n", offset);
    write(fd_fp, " world\n", 7);

    time_t t;

    time(&t);
    sprintf(buf, "%ld : \n", t);
    write(fd_fp, buf, 14);
    printf(buf);
    usleep(1000 * 1000);

    time(&t);
    sprintf(buf, "%ld : \n", t);
    write(fd_fp, buf, 14);
    printf(buf);
    usleep(1000 * 1000);

    time(&t);
    // sprintf(buf, "%ld\r", t);
    switch (pid = fork()) {
        case -1:
            perror("fork");
            exit(1);
            break;
        case 0:
            close(fd[0]);
            write(fd[1], "hello\n", 6);
            if (fd[1] != STDOUT_FILENO) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
            sleep(1);
            execlp("./src/check.sh", "ps", "-ef", NULL);
            close(fd[1]);
            exit(1);
            break;
        default:
            close(fd[1]);

            int ret_signo;
            siginfo_t info;
            while (1) {
                printf("TEST\n");
                usleep(1000 * 500);
                // ret_signo = sigwaitinfo(&sigset_mask, &info);
                // if (ret_signo == -1) {
                //     perror("sigwait");
                //     continue;
                // }
                // if (ret_signo == SIGUSR1) {
                //     printf("sigwait: %d\n", ret_signo);
                //     printf("sigwait: %d\n", info.si_pid);
                //     printf("sigwait: %d\n", info.si_value.sival_int);
                //     if (info.si_band & POLL_IN) {
                //         printf("POLLIN\n");
                //         if ((len = read(fd[0], buf, sizeof(buf))) != 0) {
                //             write(STDOUT_FILENO, "parent: ", 8);
                //             write(STDOUT_FILENO, buf, len);
                //         }
                //     }
                // }
            }

            // while ((len = read(fd[0], buf, sizeof(buf))) != 0) {
            //     write(STDOUT_FILENO, "parent: ", 8);
            //     write(STDOUT_FILENO, buf, len);
            // }
            close(fd[0]);
            waitpid(pid, &status, 0);
            break;
    }

    // while (1) {
    //     usleep(1000 * 4000);
    //     printf("dummy is running\n");
    // }

    return 0;
}
