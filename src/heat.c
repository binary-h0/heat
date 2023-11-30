#include "heat.h"

int status, pid, pgid;

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
            while (waitpid(pid, &status, WNOHANG) != pid) {
                printf("\e[1;32mmain is waiting\e[0m\n");
                sleep(10);
            }
            printf("\e[1;32mmain is done\e[0m\n");
            break;
    }
    return 0;
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
    if ((sig_id = get_signal_id(options->signal)) == -1) {
        errno = EINVAL;
        perror("\e[1;31m\n[ERROR] --signal value\e[0m");
        exit(1);
    }
    sprintf(sig, "%d", sig_id);
    ret += setenv("HEAT_SIGNAL", sig, 1);
    ret += setenv("HEAT_FAIL", options->fail, 1);
    ret += setenv("HEAT_RECOVERY", options->recovery, 1);
    ret += setenv("HEAT_THRESHOLD", options->threshold, 1);
    ret += setenv("HEAT_FAULT_SIGNAL", options->fault_signal, 1);
    ret += setenv("HEAT_SUCCESS_SIGNAL", options->success_signal, 1);
    ret += setenv("HEAT_RECOVERY_TIMEOUT", options->recovery_timeout, 1);
    if (ret != 0) {
        perror("\e[1;31m\n[ERROR] setenv\e[0m");
        exit(1);
    }
    printf("\e[1;32mupdate environment \e[0m");
}

void process_info_print() {
    pid = getpid();
    pgid = getpgid(pid);
    printf("[heat] pid: %d, pgid: %d\n", pid, pgid);
}

void check_script(char *script) {
    if (script == NULL) {
        perror("\e[1;31m[ERROR] script file not found\e[0m");
        exit(1);
    }
    if (access(script, F_OK) == -1) {
        perror("\e[1;31m[ERROR] script file not excutable\e[0m");
        exit(1);
    }
}

void check_command(char *command) {
    if (command == NULL) {
        perror("\e[1;31m[ERROR] command not found\e[0m");
        exit(1);
    }
}

int main(int argc, char *const argv[]) {
    process_info_print();
    opts *options;
    if ((options = option_process(argc, argv)) == NULL) return 1;
    if (atoi(options->is_command))
        check_command(options->script);
    else
        check_script(options->script);
    update_environment(options);
    build_sentinel_process(options);

    return 0;
}
