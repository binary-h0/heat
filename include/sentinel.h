#ifndef SENTINEL_H
#define SENTINEL_H

#include <errno.h>
#include <fcntl.h>
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
#define MAX_CHILD_PROCESS 100
#define MAX_BUF_SIZE 1024
#define STDIN_LOG_FORMAT "%ld : INFO : %s : \n"
#define STDERR_LOG_FORMAT "%ld : FAIL : %s : \n"
#define FAULT_LOG_FORMAT "%ld : ERROR : sentinel : \n%s"

extern int errno;

enum sentinel_status {
    NORMAL,
    VALIDATE,
    TIMEOUT,
    CHECK_RUNNING,
    FAIL_RUNNING,
    RECOVERY_RUNNING,
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
    int fault_signal;
    int success_signal;
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
    enum sentinel_status is_timeout;
    int continuous_fault_count;
    int total_fault_count;
    time_t current_time;
    int is_set_recovery_signal;
    // manage process info
    int child_pid_lis[MAX_CHILD_PROCESS];
    int cp_count;
    int check_pid;
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
    // logger
    char buf[MAX_BUF_SIZE];
    off_t log_offset;
    // file descriptor
    int log_fd;
    int check_stdin_fifo_fd, check_stderr_fifo_fd;
    int fail_stdin_fifo_fd, fail_stderr_fifo_fd;
    int recovery_stdin_fifo_fd, recovery_stderr_fifo_fd;

} sentinel_t;

void sentinel_init(sentinel_t *sentinel);
void sentinel_init_timer(sentinel_t *sentinel);
void sentinel_init_signals(sentinel_t *sentinel);
void sentinel_init_logger(sentinel_t *sentinel);

void sentinel_sigalrm_handler(int signo, siginfo_t *info, void *context);
void sentinel_sigchld_handler(int signo, siginfo_t *info, void *context);
void sentinel_fault_handler(sentinel_t *sentinel);
void sentinel_success_handler(sentinel_t *sentine);

void fail_process_handler(siginfo_t *info, sentinel_t *sentinel);
void check_process_handler(siginfo_t *info, sentinel_t *sentinel);
void recovery_process_handler(siginfo_t *info, sentinel_t *sentinel);

int is_recovery_check_count_exceed_threshold(sentinel_t *sentinel);

void sentinel_signal_to_target_pid(sentinel_t *sentinel);
void sentinel_executor(sentinel_t *sentinel);
int execute_command(const char *script, const char *name, char *const argv[],
                    enum child_type type);
int execute_fault_script(sentinel_t *sentinel);
int execute_check_script(sentinel_t *sentinel);
int execute_recovery_script(sentinel_t *sentinel);
void sentinel_print(sentinel_t *sentinel);
void enable_rts_event(int fd, int sig);

void sentinel_logger(sentinel_t *sentinel, int fd, char *prefix, char *script);

#endif

// 1231231234 : CHECK    : RUNNING :
// 1231231234 : CHECK    : OK      :
// 1231231234 : CHECK    : IGNORE      :
// 1231231234 : CHECK    : FAIL    : Exit Code %d
// 1231231234 : RECOVERY : RUNNING :
// 1231231234 : FAIL     : FAULT   :