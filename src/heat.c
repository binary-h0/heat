#include "heat.h"

int status, pid, pgid;

void process_info_print() {
    pid = getpid();
    pgid = getpgid(pid);
    printf("[heat] pid: %d, pgid: %d\n", pid, pgid);
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
        perror("\e[1;31m[ERROR] script file not found\e[0m");
        exit(1);
    } else if (access(script, F_OK) == -1) {
        perror("\e[1;31m[ERROR] script file not excutable\e[0m");
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

int main(int argc, char *const argv[]) {
    process_info_print();

    opts *options;
    if ((options = option_process(argc, argv)) == NULL) return 1;

    validation_options(options);

    update_environment(options);

    build_sentinel_process(options);

    while (waitpid(pid, &status, WNOHANG) != pid) {
        printf("\e[1;32mmain is waiting\e[0m\n");
        sleep(10);
    }
    printf("\e[1;32mmain is done\e[0m\n");
    return 0;
}
