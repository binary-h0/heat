#include "sentinel.h"

sentinel_t sentinel;

void sentinel_init(sentinel_t *sentinel) {
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
    sentinel->env.pid = atoi(getenv("HEAT_PID"));
    sentinel->env.signal = getenv("HEAT_SIGNAL");
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
    signal_handler_t handlers[] = {
        {SIGALRM, sentinel_interval_handler},
        // {SIGQUIT, sentinel_signal_handler},
    };
    sentinel->sa.sa_flags = SA_SIGINFO;
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

void sentinel_signal_handler(int signo, siginfo_t *info, void *context) {
    psignal(signo, "received signal");
    sentinel_executor(&sentinel);
}

void sentinel_print(sentinel_t *sentinel) {
    printf("interval: %d\n", sentinel->env.interval);
    printf("script: %s\n", sentinel->env.script);
    printf("script_argv: %s\n", sentinel->env.script_argv[0]);
    printf("name: %s\n", sentinel->env.name);
    printf("pid: %d\n", sentinel->env.pid);
    printf("signal: %s\n", sentinel->env.signal);
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
    psignal(signo, "interval handler");
    sentinel_executor(&sentinel);
}

int sentinel_check_process(sentinel_t *sentinel, int pid, int *cp_status) {
    if (waitpid(pid, cp_status, WNOHANG) == pid) return 1;
    return 0;
}

process_stat_t sentinel_check_child_process(sentinel_t *sentinel) {
    int cp_status;
    int cp_list_size = sentinel->cp_count;
    process_stat_t ps;
    ps.pid = 0;
    ps.status = 0;
    // printf("cp_size: %d\n", cp_list_size);
    while (cp_list_size--) {
        int pid = cll_get(sentinel->cp_list);
        // printf("pid: %d ", pid);
        if (sentinel_check_process(sentinel, pid, &cp_status)) {
            cll_delete(sentinel->cp_list);
            sentinel->cp_count--;
            ps.pid = pid;
            ps.status = cp_status;
            return ps;
        } else {
            // printf("process is running\n");
            cll_next(sentinel->cp_list);
        }
        usleep(1000 * 100);
    }

    return ps;
}

void sentinel_run(sentinel_t *sentinel) {
    process_stat_t ps;
    while (1) {
        ps = sentinel_check_child_process(sentinel);
        if (ps.pid == 0) continue;

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
