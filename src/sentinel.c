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
    sentinel->fault_count = 0;
    sentinel_init_timer(sentinel);
    sentinel_init_signals(sentinel);
}

void sentinel_init_timer(sentinel_t *sentinel) {
    // 이걸로 바꿔야 할 수도 있음
    sentinel->timer.it_value.tv_sec = sentinel->env.interval;
    sentinel->timer.it_value.tv_usec = 0;
    sentinel->timer.it_interval.tv_sec = sentinel->env.interval;
    sentinel->timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &(sentinel->timer), NULL);
    sentinel_executor(sentinel);
}

void sentinel_init_signals(sentinel_t *sentinel) {
    sentinel->sa.sa_flags = SA_SIGINFO;
    signal_handler_t handlers[] = {
        {SIGALRM, sentinel_interval_handler},
        {SIGCHLD, sentinel_check_child_handler},
    };
    for (int i = 0; i < sizeof(handlers) / sizeof(signal_handler_t); i++) {
        sentinel->sa.sa_sigaction = handlers[i].handler;
        sigaction(handlers[i].signo, &(sentinel->sa), NULL);
    }
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

void sentinel_executor(sentinel_t *sentinel) {
    int pid = execute_command(sentinel->env.script, sentinel->env.name,
                              sentinel->env.script_argv);
    cll_append(sentinel->cp_list, pid);
    sentinel->cp_count++;
    if (sentinel->cp_count == MAX_CHILD_PROCESS) {
        // TODO: 한계 도달 시 처리 필요
    }
}

int execute_command(const char *script, const char *name, char *const argv[]) {
    int pid;
    switch (pid = vfork()) {
        case -1:  // fork failed
            perror("fork failed");
            exit(1);
            break;
        case 0:  // child process
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

void sentinel_interval_handler(int signo, siginfo_t *info, void *context) {
    if (sentinel.stat == NORMAL)
        sentinel_executor(&sentinel);
    else if (sentinel.stat == FAULT) {
        // timer 초기화
    }
}

int check_status(sentinel_t *sentinel, siginfo_t *info) {
    if (info->si_status == 0) {
        return 0;
    } else {
        sentinel->fault_count++;
        return 1;
    }
}

enum child_type check_child_type(sentinel_t *sentinel, siginfo_t *info) {
    if (info->si_pid == sentinel->fail_pid) return FAIL_PROCESS;
    if (info->si_pid == sentinel->recovery_pid) return RECOVERY_PROCESS;
    return NORMAL_PROCESS;
}

void set_fail_env(sentinel_t *sentinel, siginfo_t *info) {
    setenv("HEAT_FAIL_CODE", info->si_status, 1);
    char str[20] = "";
    sprintf(str, "%d", sentinel->last_check_time);
    setenv("HEAT_FAIL_TIME", str, 1);
    setenv("HEAT_FAIL_INTERVAL", getenv("HEAT_INTERVAL"), 1);
    setenv("HEAT_FAIL_PID", info->si_pid, 1);
}

void execute_fault_script(sentinel_t *sentinel) {
    if (sentinel->stat == FAULT) return;
    sentinel->fail_pid =
        execute_command(sentinel->env.fail, sentinel->env.fail, (char **)"");
    sentinel->stat = FAULT;
}

void execute_recovery_script(sentinel_t *sentinel) {
    if (sentinel->stat == RECOVERY) return;
    sentinel->recovery_pid = execute_command(
        sentinel->env.recovery, sentinel->env.recovery, (char **)"");
    sentinel->stat = RECOVERY;
}

int is_exceed_threshold(sentinel_t *sentinel) {
    if (sentinel->fault_count >= sentinel->env.threshold) return 1;
    return 0;
}

void check_normal_process(siginfo_t *info, sentinel_t *sentinel) {
    switch (info->si_code) {
        case CLD_EXITED:
            if (check_status(sentinel, info)) {  // FAIL
                sentinel->fault_count++;
                printf("fault_count: %d\n", sentinel->fault_count);
                signal_to_target_pid(sentinel);
                if (is_exceed_threshold(sentinel)) {
                    execute_recovery_script(sentinel);
                } else {
                    set_fail_env(sentinel, info);
                    execute_fault_script(sentinel);
                }

            } else {  // SUCCESS
                sentinel_success_handler(sentinel);
            }
            break;
        case CLD_KILLED:
            printf("Normal process killed\n");
            break;
        case CLD_STOPPED:
            printf("Normal process stopped\n");
            break;
        case CLD_CONTINUED:
            printf("Normal process continued\n");
            break;
        default:  // UNKOWN
            printf("Normal process unknown\n");
            break;
    }
}

void check_fail_process(siginfo_t *info, sentinel_t *sentinel) {
    switch (info->si_code) {
        case CLD_EXITED:
            printf("Fail process exited\n");
            if (check_status(sentinel, info)) {
                // perrror("Fail process exited with error\n");  // 여기 처리
                // 무조건 필요
                exit(1);
            } else {
                sentinel->stat = NORMAL;
                // 바로 인터벌 실행
            }
            break;
        case CLD_KILLED:
            printf("Fail process killed\n");
            break;
        case CLD_STOPPED:
            printf("Fail process stopped\n");
            break;
        case CLD_CONTINUED:
            printf("Fail process continued\n");
            break;
        default:  // UNKOWN
            printf("Fail process unknown\n");
            break;
    }
}

void check_recovery_process(siginfo_t *info, sentinel_t *sentinel) {
    switch (info->si_code) {
        case CLD_EXITED:
            printf("Recovery process exited\n");
            if (check_status(sentinel, info)) {
                // perrror("Recovery process exited with error\n");  // 여기
                // 처리 무조건 필요
                exit(1);
            } else {
                sentinel->stat = NORMAL;
                // 바로 인터벌 실행
            }
            break;
        case CLD_KILLED:
            printf("Recovery process killed\n");
            break;
        case CLD_STOPPED:
            printf("Recovery process stopped\n");
            break;
        case CLD_CONTINUED:
            printf("Recovery process continued\n");
            break;
        default:  // UNKOWN
            printf("Recovery process unknown\n");
            break;
    }
}

void sentinel_check_child_handler(int signo, siginfo_t *info, void *context) {
    int options = WNOHANG | WEXITED | WSTOPPED | WCONTINUED;
    while (1) {
        if (waitid(P_ALL, 0, info, options) == 0 && info->si_pid != 0) {
            time(sentinel.last_check_time);  // 이거 위치 설정 고려 필요
            switch (check_child_type(&sentinel, info)) {
                case FAIL_PROCESS:
                    check_fail_process(info, &sentinel);
                    break;
                case RECOVERY_PROCESS:
                    check_recovery_process(info, &sentinel);
                    break;
                case NORMAL_PROCESS:
                    check_normal_process(info, &sentinel);
                    break;
            }
        } else {
            break;
        }
    }
}

void signal_to_target_pid(sentinel_t *sentinel) {
    // kill(sentinel->ppid, 10);  // 실패 시 부모에게 시그널 전달 ->
    // 해결해야함
    if (sentinel->env.target_pid != sentinel->ppid)
        kill(sentinel->env.target_pid, sentinel->env.signal);
}

int sentinel_check_process(sentinel_t *sentinel, int pid, int *cp_status) {
    if (waitpid(pid, cp_status, WNOHANG) == pid) return 1;
    return 0;
}

int check_fault(sentinel_t *sentinel, process_stat_t ps) {
    // 0 : no fault, 0 < : fault
    if (ps.pid == -1) return 0;
    if (WIFEXITED(ps.status)) {
        if (WEXITSTATUS(ps.status) == 0) {
            return 0;
        } else {
            return WEXITSTATUS(ps.status);
        }
    }
    // else if (WIFSIGNALED(ps.status)) {
    //     printf("Child terminated by signal %d\n", WTERMSIG(ps.status));
    // } else if (WIFSTOPPED(ps.status)) {
    //     printf("Child stopped by signal %d\n", WSTOPSIG(ps.status));
    // } else if (WIFCONTINUED(ps.status)) {
    //     printf("Child continued\n");
    // }
    return 0;
}

void sentinel_success_handler(sentinel_t *sentine) {}

// void sentinel_check_child_process_is_done(sentinel_t *sentinel,
//                                           process_stat_t *ps) {
//     int cp_status;
//     int cp_list_size = sentinel->cp_count;
//     // printf("cp_size: %d\n", cp_list_size);
//     ps->pid = -1;
//     ps->status = -1;
//     while (cp_list_size--) {
//         int pid = cll_get(sentinel->cp_list);
//         // printf("pid: %d ", pid);
//         if (sentinel_check_process(sentinel, pid, &cp_status)) {
//             cll_delete(sentinel->cp_list);
//             sentinel->cp_count--;
//             ps->pid = pid;
//             ps->status = cp_status;
//             return;
//         } else {
//             // printf("process is running\n");
//             cll_next(sentinel->cp_list);
//         }
//         usleep(1000 * 100);
//     }
//     return;
// }

void sentinel_run(sentinel_t *sentinel) {
    process_stat_t ps;
    while (1) {
        // sentinel_check_child_process_is_done(sentinel, &ps);
        // check_fault(sentinel, ps) ? sentinel_fault_handler(sentinel)
        //                           : sentinel_success_handler(sentinel);

        usleep(1000 * 100);
    }
}

void process_info_print() {
    int pid, pgid;
    pid = getpid();
    pgid = getpgid(pid);
    printf("[sentinel] pid: %d, pgid: %d\n", pid, pgid);
}

int main(int argc, char *argv[]) {
    process_info_print();  // TODO: remove this line
    sentinel_init(&sentinel);
    sentinel_print(&sentinel);  // TODO: remove this line

    printf("sentinel is running\n");
    sentinel_run(&sentinel);

    // pause();

    return 0;
}
