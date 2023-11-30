#ifndef SENTINEL_H
#define SENTINEL_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "circular_linked_list.h"

#define false 0
#define true 1
#define MAX_CHILD_PROCESS 10

enum status { NORMAL, FAULT, RECOVERY };
enum child_type { FAIL_PROCESS, RECOVERY_PROCESS, NORMAL_PROCESS };

typedef struct sentinel_env_struct {
    int interval;
    char *script;
    char **script_argv;
    char *name;
    int target_pid;
    int signal;
    char *fail;
    char *recovery;
    int threshold;
    char *fault_signal;
    char *success_signal;
    int recovery_timeout;
} sentinel_env_t;

typedef struct sigaction_handler_struct {
    int signo;
    void (*handler)(int, siginfo_t *, void *);
} signal_handler_t;

typedef struct process_stat_struct {
    int pid;
    int status;
} process_stat_t;

typedef struct sentinel_struct {
    // environment
    int ppid;
    sentinel_env_t env;
    enum status stat;
    int fault_count;
    time_t current_time;
    // manage process info
    cll_t *cp_list;
    int cp_count;
    int fail_pid;
    int recovery_pid;
    time_t fail_time;
    time_t fail_time_last;
    // signal
    struct sigaction sa;
    signal_handler_t *signal_handler;
    // time
    struct itimerval timer;

} sentinel_t;

void sentinel_init(sentinel_t *sentinel);
void sentinel_init_timer(sentinel_t *sentinel);
void sentinel_init_signals(sentinel_t *sentinel);

void sentinel_interval_handler(int signo, siginfo_t *info, void *context);
void sentinel_check_child_handler(int signo, siginfo_t *info, void *context);
void sentinel_fault_handler(sentinel_t *sentinel);
void sentinel_success_handler(sentinel_t *sentine);

void check_fail_process(siginfo_t *info, sentinel_t *sentinel);
void check_normal_process(siginfo_t *info, sentinel_t *sentinel);
void check_recovery_process(siginfo_t *info, sentinel_t *sentinel);

void signal_to_target_pid(sentinel_t *sentinel);
void sentinel_executor(sentinel_t *sentinel);
int execute_command(const char *command, const char *name, char *const argv[]);
void sentinel_print(sentinel_t *sentinel);

#endif