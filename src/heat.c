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
    printf("\e[1;32mupdate environment \e[0m");
    int ret = 0;
    ret += setenv("HEAT_INTERVAL", options->interval, 1);
    ret += setenv("HEAT_SCRIPT", options->script, 1);
    ret += setenv("HEAT_SCRIPT_ARGV", options->script_argv, 1);
    ret += setenv("HEAT_NAME", options->name, 1);
    ret += setenv("HEAT_PID", options->pid, 1);
    ret += setenv("HEAT_SIGNAL", options->signal, 1);
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
    printf("\e[1;32mdone\e[0m\n");
}

void process_info_print() {
    pid = getpid();
    pgid = getpgid(pid);
    printf("[heat] pid: %d, pgid: %d\n", pid, pgid);
}

int main(int argc, char *const argv[]) {
    process_info_print();
    opts *options;
    if ((options = option_process(argc, argv)) == NULL) return 1;
    update_environment(options);
    build_sentinel_process(options);

    return 0;
}
