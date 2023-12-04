#include "sentinel.h"

sentinel_t sentinel;

void sentinel_init(sentinel_t *sentinel) {
    sentinel->ppid = getppid();
    char *argv;
    char *tmp = getenv("HEAT_SCRIPT_ARGV");
    char **argvec;
    if (tmp[0] == '\0') {
        argv = (char *)malloc(sizeof(char) * 1);
        argvec = (char **)malloc(sizeof(char *) * 1);
        argvec[0] = (char *)malloc(sizeof(char) * 1);
    } else {
        argv = (char *)malloc(sizeof(char) * strlen(tmp));
        strcpy(argv, tmp);
        char *tok = strtok(argv, " ");
        int argc = 1;
        while (strtok(NULL, " ") != NULL) argc++;
        argvec = (char **)malloc(sizeof(char *) * (argc + 1));
        strcpy(argv, getenv("HEAT_SCRIPT_ARGV"));
        tok = strtok(argv, " ");
        for (int i = 1; i <= argc; i++) {
            argvec[i] = (char *)malloc(sizeof(char) * strlen(tok));
            strcpy(argvec[i], tok);
            tok = strtok(NULL, " ");
        }
    }
    sentinel->env.script_argv = argvec;
    sentinel->env.script = getenv("HEAT_SCRIPT");
    sentinel->env.interval = atoi(getenv("HEAT_INTERVAL"));
    sentinel->env.name = getenv("HEAT_NAME");
    sentinel->env.script_argv[0] = sentinel->env.name;
    sentinel->env.target_pid = atoi(getenv("HEAT_PID"));
    sentinel->env.signal = atoi(getenv("HEAT_SIGNAL"));
    sentinel->env.fail = getenv("HEAT_FAIL");
    sentinel->env.recovery = getenv("HEAT_RECOVERY");
    sentinel->env.threshold = atoi(getenv("HEAT_THRESHOLD"));
    sentinel->env.fault_signal = getenv("HEAT_FAULT_SIGNAL");
    sentinel->env.success_signal = getenv("HEAT_SUCCESS_SIGNAL");
    sentinel->env.recovery_timeout = atoi(getenv("HEAT_RECOVERY_TIMEOUT"));
    sentinel->stat = NORMAL;
    sentinel->cp_list = cll_init();
    sentinel->cp_count = 0;
    sentinel->continuous_fault_count = 0;
    sentinel->total_fault_count = 0;
    sentinel_init_signals(sentinel);
    sentinel_init_logger(sentinel);
    sentinel_init_timer(sentinel);
}

void sentinel_fault_logger(sentinel_t *sentinel, char *error) {
    time(&(sentinel->current_time));
    memset(sentinel->buf, 0, sizeof(sentinel->buf));
    sprintf(sentinel->buf, FAULT_LOG_FORMAT, sentinel->current_time, error);
    perror(sentinel->buf);
}

void sentinel_init_logger(sentinel_t *sentinel) {
    FILE *fp;
    if ((fp = fopen("./log/heat.verbose.log", "a")) == NULL) {
        perror("fopen");
        exit(1);
    };
    sentinel->log_fd = fileno(fp);
    dup2(sentinel->log_fd, STDERR_FILENO);

    sentinel->check_stdin_fifo_fd = open("./check_stdin_fifo", O_RDWR);
    if (sentinel->check_stdin_fifo_fd == -1) {
        perror("open fifo error");
        exit(1);
    }
    sentinel->check_stderr_fifo_fd = open("./check_stderr_fifo", O_RDWR);
    if (sentinel->check_stderr_fifo_fd == -1) {
        perror("open fifo error");
        exit(1);
    }
    sentinel->fail_stdin_fifo_fd = open("./fail_stdin_fifo", O_RDWR);
    if (sentinel->fail_stdin_fifo_fd == -1) {
        perror("open");
        exit(1);
    }
    sentinel->fail_stderr_fifo_fd = open("./fail_stderr_fifo", O_RDWR);
    if (sentinel->fail_stderr_fifo_fd == -1) {
        perror("open");
        exit(1);
    }
    sentinel->recovery_stdin_fifo_fd = open("./recovery_stdin_fifo", O_RDWR);
    if (sentinel->recovery_stdin_fifo_fd == -1) {
        perror("open");
        exit(1);
    }
    sentinel->recovery_stderr_fifo_fd = open("./recovery_stderr_fifo", O_RDWR);
    if (sentinel->recovery_stderr_fifo_fd == -1) {
        perror("open");
        exit(1);
    }
    enable_rts_event(sentinel->check_stdin_fifo_fd, SIGUSR1);
    enable_rts_event(sentinel->check_stderr_fifo_fd, SIGUSR1);
    enable_rts_event(sentinel->fail_stdin_fifo_fd, SIGUSR1);
    enable_rts_event(sentinel->fail_stderr_fifo_fd, SIGUSR1);
    enable_rts_event(sentinel->recovery_stdin_fifo_fd, SIGUSR1);
    enable_rts_event(sentinel->recovery_stderr_fifo_fd, SIGUSR1);
}

void sentinel_init_timer(sentinel_t *sentinel) {
    struct sigevent sev;
    sev = (struct sigevent){
        .sigev_notify = SIGEV_SIGNAL,
        .sigev_signo = SIGALRM,
        .sigev_value = {.sival_int = CHECK_TIMER},
    };
    timer_create(CLOCK_REALTIME, &sev, &(sentinel->normal_timer));
    sev.sigev_value.sival_int = RECOVERY_TIMER;
    timer_create(CLOCK_REALTIME, &sev, &(sentinel->recovery_timer));
    // init itimerspec
    sentinel->normal_it = (struct itimerspec){
        .it_value = {.tv_sec = sentinel->env.interval, .tv_nsec = 0},
        .it_interval = {.tv_sec = sentinel->env.interval, .tv_nsec = 0},
    };
    sentinel->recovery_it = (struct itimerspec){
        .it_value = {.tv_sec = sentinel->env.recovery_timeout, .tv_nsec = 0},
        .it_interval = {.tv_sec = 0, .tv_nsec = 0},
    };
    sentinel->stop_it = (struct itimerspec){
        .it_value = {.tv_sec = 0, .tv_nsec = 0},
        .it_interval = {.tv_sec = 0, .tv_nsec = 0},
    };
    // set timer // 여기 좀 더 깔끔하게 분리 가능할듯
    timer_settime(sentinel->normal_timer, 0, &(sentinel->normal_it), NULL);
    execute_check_script(sentinel);
}

void sentinel_sigusr1_handler(int signo, siginfo_t *info, void *context) {
    // 좀 더 깔끔하게 가능할듯 ARRAY 이용
    if (info->si_band & POLL_IN) {
        if (info->si_fd == sentinel.check_stdin_fifo_fd) {
            sentinel_logger(&sentinel, sentinel.check_stdin_fifo_fd,
                            STDIN_LOG_FORMAT, sentinel.env.script);
        } else if (info->si_fd == sentinel.check_stderr_fifo_fd) {
            sentinel_logger(&sentinel, sentinel.check_stderr_fifo_fd,
                            STDERR_LOG_FORMAT, sentinel.env.script);
        } else if (info->si_fd == sentinel.fail_stdin_fifo_fd) {
            sentinel_logger(&sentinel, sentinel.fail_stdin_fifo_fd,
                            STDIN_LOG_FORMAT, sentinel.env.fail);
        } else if (info->si_fd == sentinel.fail_stderr_fifo_fd) {
            sentinel_logger(&sentinel, sentinel.fail_stderr_fifo_fd,
                            STDERR_LOG_FORMAT, sentinel.env.fail);
        } else if (info->si_fd == sentinel.recovery_stdin_fifo_fd) {
            sentinel_logger(&sentinel, sentinel.recovery_stdin_fifo_fd,
                            STDIN_LOG_FORMAT, sentinel.env.recovery);
        } else if (info->si_fd == sentinel.recovery_stderr_fifo_fd) {
            sentinel_logger(&sentinel, sentinel.recovery_stderr_fifo_fd,
                            STDERR_LOG_FORMAT, sentinel.env.recovery);
        }
        return;
    }
}

void sentinel_logger_prefix(sentinel_t *sentinel, char *prefix, char *script) {
    time(&(sentinel->current_time));
    memset(sentinel->buf, 0, sizeof(sentinel->buf));
    sprintf(sentinel->buf, prefix, sentinel->current_time, script);
    write(sentinel->log_fd, sentinel->buf, strlen(sentinel->buf));
}

void sentinel_logger(sentinel_t *sentinel, int fd, char *prefix, char *script) {
    int len;
    sentinel_logger_prefix(sentinel, prefix, script);
    while ((len = read(fd, sentinel->buf, MAX_BUF_SIZE)) != -1) {
        write(sentinel->log_fd, sentinel->buf, len);
    }
}

void sentinel_init_signals(sentinel_t *sentinel) {
    sentinel->sa.sa_flags = SA_SIGINFO;
    // TODO: 하나의 시그널 핸들러에서 스위치로 분기 시킬 수 있음
    signal_handler_t handlers[] = {
        {SIGALRM, sentinel_sigalrm_handler},
        {SIGCHLD, sentinel_sigchld_handler},
        {SIGUSR1, sentinel_sigusr1_handler},
    };
    for (int i = 0; i < sizeof(handlers) / sizeof(signal_handler_t); i++) {
        sentinel->sa.sa_sigaction = handlers[i].handler;
        sigaction(handlers[i].signo, &(sentinel->sa), NULL);
    }
    // TODO
    // 여기 의미상 처리 필요
    sigemptyset(&sentinel->sa.sa_mask);
    sigaddset(&sentinel->sa.sa_mask, SIGALRM);
    sigaddset(&sentinel->sa.sa_mask, SIGQUIT);
    // 여기까지
    // sigaddset(&sentinel->sa.sa_mask, SIGUSR1);
}

void sentinel_print(sentinel_t *sentinel) {
    printf("interval: %d\n", sentinel->env.interval);
    printf("script: %s\n", sentinel->env.script);
    printf("script_argv: %s\n", sentinel->env.script_argv[0]);
    printf("name: %s\n", sentinel->env.name);
    printf("pid: %d\n", sentinel->env.target_pid);
    printf("signal: %d\n", sentinel->env.signal);
    printf("fail: %s\n", sentinel->env.fail);
    printf("recovery: %s\n", sentinel->env.recovery);
    printf("threshold: %d\n", sentinel->env.threshold);
    printf("fault_signal: %s\n", sentinel->env.fault_signal);
    printf("success_signal: %s\n", sentinel->env.success_signal);
    printf("recovery_timeout: %d\n", sentinel->env.recovery_timeout);
}

int execute_check_script(sentinel_t *sentinel) {
    // if (sentinel->stat == CHECK_RUNNING) return 1; // 이거 한줄로 스로틀링
    // 가능
    sentinel->stat = CHECK_RUNNING;
    sentinel->check_pid =
        execute_command(sentinel->env.script, sentinel->env.name,
                        sentinel->env.script_argv, CHECK_PROCESS);
    return 0;
}

int execute_command(const char *script, const char *name, char *const argv[],
                    enum child_type type) {
    int pid, fd1, fd2;
    switch (pid = fork()) {
        case -1:  // fork failed
            perror("fork failed");
            exit(1);
            break;
        case 0:  // child process
            switch (type) {
                case CHECK_PROCESS:
                    if ((fd1 = open("./check_stdin_fifo", O_WRONLY)) == -1) {
                        perror("open");
                        exit(1);
                    }
                    if ((fd2 = open("./check_stderr_fifo", O_WRONLY)) == -1) {
                        perror("open");
                        exit(1);
                    }
                    break;
                case FAIL_PROCESS:
                    if ((fd1 = open("./fail_stdin_fifo", O_WRONLY)) == -1) {
                        perror("open");
                        exit(1);
                    }
                    if ((fd2 = open("./fail_stderr_fifo", O_WRONLY)) == -1) {
                        perror("open");
                        exit(1);
                    }
                    break;
                case RECOVERY_PROCESS:
                    if ((fd1 = open("./recovery_stdin_fifo", O_WRONLY)) == -1) {
                        perror("open");
                        exit(1);
                    }
                    if ((fd2 = open("./recovery_stderr_fifo", O_WRONLY)) ==
                        -1) {
                        perror("open");
                        exit(1);
                    }
                    break;
            }
            dup2(fd1, STDOUT_FILENO);
            dup2(fd2, STDERR_FILENO);
            if (execvp(script, argv) == -1) {
                perror("execlp failed");
                exit(1);
            }
            break;
        default:  // parent process
            return pid;
            break;
    }
    return -1;
}

void check_timer_handler(sentinel_t *sentinel) {
    switch (sentinel->stat) {
        case NORMAL:
            execute_check_script(sentinel);
            break;
        case CHECK_RUNNING:
            // timer_settime(sentinel->normal_timer, 0, &(sentinel->stop_it),
            //               NULL);
            // sentinel->stat = TIMEOUT;
            execute_check_script(sentinel);
            break;
        case FAIL_RUNNING:
            timer_settime(sentinel->normal_timer, 0, &(sentinel->stop_it),
                          NULL);
            break;
        case RECOVERY_RUNNING:
            timer_settime(sentinel->normal_timer, 0, &(sentinel->stop_it),
                          NULL);
            break;
        case VALIDATE:
            printf("%ld: ", sentinel->current_time);
            execute_check_script(sentinel);
            break;
    }
}

void recovery_timer_handler(sentinel_t *sentinel) {
    switch (sentinel->stat) {
        case NORMAL:
            printf("SUCCESS RECOVERY\n");
            break;
        case CHECK_RUNNING:
            break;
        case FAIL_RUNNING:
            printf("FAIL RECOVERY\n");
            break;
        case RECOVERY_RUNNING:
            printf("RECOVERY PROCESSING NOT DONE, BUT TIMEOUT.\n");
            printf("RECOVERY PROCESSING AGAIN\n");
            execute_recovery_script(sentinel);
            break;
        case VALIDATE:
            sentinel->total_fault_count += sentinel->continuous_fault_count;
            sentinel->continuous_fault_count = 0;
            execute_recovery_script(sentinel);
            break;
    }
}

void sentinel_sigalrm_handler(int signo, siginfo_t *info, void *context) {
    // printf("STATE: %d | ", sentinel.stat);
    // printf("val: %d | ", info->si_value.sival_int);
    // printf("CONT_FL_CNT: %d | ", sentinel.continuous_fault_count);
    // printf("TOTAL_FL_CNT: %d | \n", sentinel.total_fault_count);
    switch (info->si_value.sival_int) {
        case CHECK_TIMER:
            check_timer_handler(&sentinel);
            break;
        case RECOVERY_TIMER:
            recovery_timer_handler(&sentinel);
            break;
    }
}

enum return_code check_status(sentinel_t *sentinel, siginfo_t *info) {
    if (info->si_status == 0) {
        return SUCCESS;
    } else {
        return FAIL;
    }
}

enum child_type check_child_type(sentinel_t *sentinel, siginfo_t *info) {
    if (info->si_pid == sentinel->fail_pid) return FAIL_PROCESS;
    if (info->si_pid == sentinel->recovery_pid) return RECOVERY_PROCESS;
    return CHECK_PROCESS;
}

void set_fail_env(sentinel_t *sentinel, siginfo_t *info) {
    char str_buf[20];
    memset(str_buf, 0, sizeof(str_buf));
    sprintf(str_buf, "%d", info->si_status);
    setenv("HEAT_FAIL_CODE", str_buf, 1);
    memset(str_buf, 0, sizeof(str_buf));
    sprintf(str_buf, "%ld", sentinel->fail_time);
    setenv("HEAT_FAIL_TIME", str_buf, 1);
    memset(str_buf, 0, sizeof(str_buf));
    sprintf(str_buf, "%ld", sentinel->fail_time_last);
    setenv("HEAT_FAIL_TIME_LAST", str_buf, 1);
    setenv("HEAT_FAIL_INTERVAL", getenv("HEAT_INTERVAL"), 1);
    memset(str_buf, 0, sizeof(str_buf));
    sprintf(str_buf, "%d", info->si_pid);
    setenv("HEAT_FAIL_PID", str_buf, 1);
    memset(str_buf, 0, sizeof(str_buf));
    sprintf(str_buf, "%d", sentinel->total_fault_count);
    setenv("HEAT_FAIL_CNT", str_buf, 1);
}

int execute_fault_script(sentinel_t *sentinel) {
    if (sentinel->stat == FAIL_RUNNING) return 1;
    if (sentinel->env.fail[0] == '\0') return 1;
    sentinel->stat = FAIL_RUNNING;
    sentinel->fail_pid = execute_command(sentinel->env.fail, sentinel->env.fail,
                                         (char **)"", FAIL_PROCESS);
    return 0;
}

int execute_recovery_script(sentinel_t *sentinel) {
    if (sentinel->stat == RECOVERY_RUNNING) return 1;
    if (sentinel->env.recovery[0] == '\0')
        return execute_fault_script(sentinel);
    sentinel->stat = RECOVERY_RUNNING;
    sentinel->continuous_fault_count = 0;

    sentinel->recovery_pid =
        execute_command(sentinel->env.recovery, sentinel->env.recovery,
                        (char **)"", RECOVERY_PROCESS);
    return 0;
}

int is_continuous_fault_count_exceed_threshold(sentinel_t *sentinel) {
    if (sentinel->continuous_fault_count >= sentinel->env.threshold) return 1;
    return 0;
}

void sentinel_continuous_fault_counter(sentinel_t *sentinel) {
    if ((sentinel->stat == NORMAL) | (sentinel->stat == TIMEOUT) |
        (sentinel->stat == CHECK_RUNNING)) {
        (sentinel->total_fault_count == 0) ? time(&(sentinel->fail_time))
                                           : time(&(sentinel->fail_time_last));
        sentinel->continuous_fault_count++;
        sentinel->total_fault_count++;
    } else if (sentinel->stat == VALIDATE) {
        sentinel->continuous_fault_count++;
        sentinel->total_fault_count++;
    }
}

void check_process_fail_handler(sentinel_t *sentinel) {
    if (is_continuous_fault_count_exceed_threshold(sentinel)) {
        execute_recovery_script(sentinel);
    } else {
        if (sentinel->stat == RECOVERY_RUNNING |
            sentinel->stat ==
                FAIL_RUNNING)  // | sentinel->stat == CHECK_RUNNING)
            return;
        execute_fault_script(sentinel);
    }
}

void check_process_handler(siginfo_t *info, sentinel_t *sentinel) {
    switch (info->si_code) {
        case CLD_EXITED:
            switch (check_status(sentinel, info)) {
                case SUCCESS:
                    // TODO: HEAT 한테 시그널 보내기? 바로 인터벌 실행
                    printf("OK\n");
                    sentinel->stat = NORMAL;
                    sentinel->continuous_fault_count = 0;
                    sentinel->total_fault_count = 0;
                    break;
                case FAIL:
                    printf(
                        "Failed: Exit Code %d, details in "
                        "./log/heat.verbose.log\n",
                        info->si_status);
                    sentinel_continuous_fault_counter(sentinel);
                    set_fail_env(sentinel, info);
                    sentinel_signal_to_target_pid(sentinel);
                    check_process_fail_handler(sentinel);
                    break;
            }
            if (sentinel->stat == TIMEOUT) {
                timer_settime(sentinel->normal_timer, 0, &(sentinel->normal_it),
                              NULL);
                execute_check_script(sentinel);
            }
            break;
        case CLD_KILLED:
            perror("Normal process killed");
            exit(1);
            break;
        case CLD_STOPPED:
            perror("Normal process stopped");
            exit(1);
            break;
        case CLD_CONTINUED:
            printf("Normal process continued\n");
            break;
        default:  // UNKOWN
            perror("Normal process unknown");
            exit(1);
            break;
    }
}

void fail_process_handler(siginfo_t *info, sentinel_t *sentinel) {
    switch (info->si_code) {
        case CLD_EXITED:
            printf("Fail process exited\n");
            switch (check_status(sentinel, info)) {
                case SUCCESS:
                    if (sentinel->stat != VALIDATE) sentinel->stat = NORMAL;
                    timer_settime(sentinel->normal_timer, 0,
                                  &(sentinel->normal_it), NULL);
                    execute_check_script(sentinel);
                    break;
                case FAIL:
                    errno = EINVAL;
                    perror("Fail script process exited with error");
                    exit(1);
            }
            break;
        case CLD_KILLED:
            perror("Fail process killed");
            exit(1);
            break;
        case CLD_STOPPED:
            perror("Fail process stopped");
            exit(1);
            break;
        case CLD_CONTINUED:
            printf("Fail process continued\n");
            break;
        default:  // UNKOWN
            perror("Fail process unknown");
            exit(1);
            break;
    }
}

void recovery_process_handler(siginfo_t *info, sentinel_t *sentinel) {
    switch (info->si_code) {
        case CLD_EXITED:
            switch (check_status(sentinel, info)) {
                case SUCCESS:
                    sentinel->continuous_fault_count = 0;
                    timer_settime(sentinel->recovery_timer, 0,
                                  &(sentinel->recovery_it), NULL);
                    sentinel->stat = VALIDATE;
                    timer_settime(sentinel->normal_timer, 0,
                                  &(sentinel->normal_it), NULL);
                    execute_check_script(sentinel);
                    // 바로 인터벌 실행
                    break;
                case FAIL:
                    // 복구 스크립트자체의 오류 프로그램 바로 종료
                    // perror("Recovery process exited with error\n");
                    // 여기 처리 무조건 필요
                    exit(1);
                    break;
            }
            break;
        case CLD_KILLED:
            perror("Recovery process killed");
            exit(1);
            break;
        case CLD_STOPPED:
            perror("Recovery process stopped");
            exit(1);
            break;
        case CLD_CONTINUED:
            printf("Recovery process continued\n");
            break;
        default:  // UNKOWN
            perror("Recovery process unknown");
            exit(1);
            break;
    }
}

void sentinel_sigchld_handler(int signo, siginfo_t *info, void *context) {
    int options = WNOHANG | WEXITED | WSTOPPED | WCONTINUED;
    while (1) {  // TODO: 여기 무한루프 처리 필요 없애도 될까?
        if (waitid(P_ALL, 0, info, options) == 0 && info->si_pid != 0) {
            // time(&(sentinel.current_time));  // 이거 위치 설정 고려 필요
            switch (check_child_type(&sentinel, info)) {
                case FAIL_PROCESS:
                    fail_process_handler(info, &sentinel);
                    break;
                case RECOVERY_PROCESS:
                    recovery_process_handler(info, &sentinel);
                    break;
                case CHECK_PROCESS:
                    check_process_handler(info, &sentinel);
                    break;
            }
        } else {
            break;
        }
    }
}

void sentinel_signal_to_target_pid(sentinel_t *sentinel) {
    // TODO
    // kill(sentinel->ppid, 10);  // 실패 시 부모에게 시그널 전달 ->
    // 해결해야함
    if (sentinel->env.target_pid != sentinel->ppid)
        kill(sentinel->env.target_pid, sentinel->env.signal);
}

void sentinel_run(sentinel_t *sentinel) {
    while (1) {
        usleep(1000 * 100);
        pause();
    }
}

void process_info_print() {
    int pid, pgid;
    pid = getpid();
    pgid = getpgid(pid);
    printf("[sentinel] pid: %d, pgid: %d\n", pid, pgid);
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

int main(int argc, char *argv[]) {
    printf("sentinel is running\n");
    // process_info_print();  // TODO: remove this line
    sentinel_init(&sentinel);
    // sentinel_print(&sentinel);  // TODO: remove this line
    sentinel_run(&sentinel);

    // pause();

    return 0;
}
