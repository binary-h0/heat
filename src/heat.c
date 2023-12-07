#include "heat.h"

int status, pid, pgid;

void process_info_print() {
    pid = getpid();
    pgid = getpgid(pid);
    // printf("[heat] pid: %d, pgid: %d\n", pid, pgid);
}

void validation_options(opts *options) {
    if (atoi(options->is_command))
        verify_command(options->script);
    else
        verify_script(options->script);
    verify_recovery(options->recovery);
    verify_fail(options->fail);
}

void verify_command(char *command) {
    int len = strlen(command);
    if (command[len - 3] == '.') {
        verify_script(command);
    } else if (command == "") {
        errno = EINVAL;
        perror("\e[1;31m[ERROR] command not found\e[0m");
        exit(1);
    }
}

void verify_script(char *script) {
    if (script == "") {
        errno = EINVAL;
        perror("[ERROR] script file not found");
        exit(1);
    } else if (access(script, F_OK) == -1) {
        perror("[ERROR] script file not excutable");
        exit(1);
    }
    check_file_permission(script);
}

void verify_recovery(char *script) {
    if (strlen(script) == 0) {
        return;
    }
    verify_script(script);
    check_file_permission(script);
}

void verify_fail(char *script) {
    if (strlen(script) == 0) {
        return;
    }
    verify_script(script);
    check_file_permission(script);
}

void check_file_permission(char *filename) {
    char buf[100];
    sprintf(buf, "ls %s", filename);
    if (system(buf)) {
        perror("\e[1;31m[ERROR] file not found\e[0m");
        exit(1);
    }
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        perror("\e[1;31m[ERROR] file not found\e[0m");
        exit(1);
    }
    // 실행 권한 확인 근데 소유자만?
    if (!(file_stat.st_mode & S_IXUSR)) errno = EACCES;

    if (errno != 0) {
        perror("\e[1;31m[ERROR] file permission\e[0m");
        exit(1);
    }
    errno = 0;
}

void update_environment(opts *options) {
    int ret = 0;
    ret += setenv("HEAT_INTERVAL", options->interval, 1);
    ret += setenv("HEAT_SCRIPT", options->script, 1);
    ret += setenv("HEAT_SCRIPT_ARGV", options->script_argv, 1);
    ret += setenv("HEAT_NAME", options->name, 1);
    if (kill(atoi(options->pid), 0) == -1) {
        errno = EINVAL;
        perror("\e[1;31m\n[ERROR] --pid value\e[0m");
        exit(1);
    }
    ret += setenv("HEAT_PID", options->pid, 1);
    char sig[2];
    int sig_id;
    if (strlen(options->signal) == 0) {
        sprintf(sig, "%d", 0);
    } else {
        if ((sig_id = get_signal_id(options->signal)) == -1) {
            errno = EINVAL;
            perror("\e[1;31m\n[ERROR] --signal value\e[0m");
            exit(1);
        }
        sprintf(sig, "%d", sig_id);
    }
    ret += setenv("HEAT_SIGNAL", sig, 1);
    if (strlen(options->fault_signal) == 0) {
        sprintf(sig, "%d", 0);
    } else {
        if ((sig_id = get_signal_id(options->fault_signal)) == -1) {
            errno = EINVAL;
            perror("\e[1;31m\n[ERROR] --fault-signal value\e[0m");
            exit(1);
        }
        sprintf(sig, "%d", sig_id);
    }
    ret += setenv("HEAT_FAULT_SIGNAL", sig, 1);
    if (strlen(options->success_signal) == 0) {
        sprintf(sig, "%d", 0);
    } else {
        if ((sig_id = get_signal_id(options->success_signal)) == -1) {
            errno = EINVAL;
            perror("\e[1;31m\n[ERROR] --success-signal value\e[0m");
            exit(1);
        }
        sprintf(sig, "%d", sig_id);
    }
    ret += setenv("HEAT_SUCCESS_SIGNAL", sig, 1);

    ret += setenv("HEAT_FAIL", options->fail, 1);
    ret += setenv("HEAT_RECOVERY", options->recovery, 1);
    ret += setenv("HEAT_THRESHOLD", options->threshold, 1);
    ret += setenv("HEAT_RECOVERY_TIMEOUT", options->recovery_timeout, 1);
    if (ret != 0) {
        perror("\e[1;31m\n[ERROR] setenv\e[0m");
        exit(1);
    }
}

int build_sentinel_process(opts *options) {
    int pid = -1;
    switch (pid = vfork()) {
        case -1:  // fork failed
            perror("fork failed");
            return 1;
            break;
        case 0:  // child process
            if (execlp("bin/sentinel", "sentinel", NULL) == -1) {
                perror("execlp failed");
                return 1;
            }
            break;
        default:
            break;
    }
    return 0;
}

void heat_init_fifo() {
    // mkfifo
    if (mkfifo("check_stdin_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
    if (mkfifo("check_stderr_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
    if (mkfifo("fail_stdin_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
    if (mkfifo("fail_stderr_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
    if (mkfifo("recovery_stdin_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
    if (mkfifo("recovery_stderr_fifo", 0666) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            exit(1);
        }
    }
}

void handler(int signo, siginfo_t *info, void *context) {
    switch (signo) {
        case SIGCHLD:
            printf("\nHEAT is exited [log file: ./log/heat.verbose.log]\n");
            exit(1);
            break;
        case SIGINT:
            printf("\nHEAT is interupted [log file: ./log/heat.verbose.log]\n");
            exit(0);
            break;
        default:
            break;
    }
}

void sig_init() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigfillset(&sa.sa_mask);
    sigdelset(&sa.sa_mask, SIGINT);
    sigdelset(&sa.sa_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
}

int main(int argc, char *const argv[]) {
    process_info_print();  // pid, pgid 의존성 해결해야함
    opts *options;
    if ((options = option_process(argc, argv)) == NULL) return 1;

    validation_options(options);

    update_environment(options);

    heat_init_fifo();
    fflush(stdout);

    build_sentinel_process(options);
    sig_init();

    while (waitpid(pid, &status, WNOHANG) != pid) {
        sleep(10);
    }
    return 0;
}
