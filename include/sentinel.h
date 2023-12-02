#ifndef SENTINEL_H
#define SENTINEL_H

#include <errno.h>
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

extern int errno;

enum sentinel_status {
    NORMAL,
    CHECK_RUNNING,
    FAULT_RUNNING,
    RECOVERY_RUNNING,
    VALIDATE
};
enum return_code { SUCCESS, FAIL };
enum child_type { FAIL_PROCESS, RECOVERY_PROCESS, CHECK_PROCESS };
enum timer_type { CHECK_TIMER, RECOVERY_TIMER };

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
    enum sentinel_status stat;
    int continuous_fault_count;
    int total_fault_count;
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
    // timer
    struct itimerspec normal_it;
    struct itimerspec recovery_it;
    struct itimerspec stop_it;
    timer_t normal_timer;
    timer_t recovery_timer;

} sentinel_t;

void sentinel_init(sentinel_t *sentinel);
void sentinel_init_timer(sentinel_t *sentinel);
void sentinel_init_signals(sentinel_t *sentinel);

void sentinel_sigalrm_handler(int signo, siginfo_t *info, void *context);
void sentinel_sigchld_handler(int signo, siginfo_t *info, void *context);
void sentinel_fault_handler(sentinel_t *sentinel);
void sentinel_success_handler(sentinel_t *sentine);

void fail_process_handler(siginfo_t *info, sentinel_t *sentinel);
void check_process_handler(siginfo_t *info, sentinel_t *sentinel);
void recovery_process_handler(siginfo_t *info, sentinel_t *sentinel);

void execute_recovery_script(sentinel_t *sentinel);
int is_recovery_check_count_exceed_threshold(sentinel_t *sentinel);

void sentinel_signal_to_target_pid(sentinel_t *sentinel);
void sentinel_executor(sentinel_t *sentinel);
int execute_command(const char *command, const char *name, char *const argv[]);
void sentinel_print(sentinel_t *sentinel);

#endif