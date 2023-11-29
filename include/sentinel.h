#ifndef SENTINEL_H
#define SENTINEL_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "circular_linked_list.h"

#define false 0
#define true 1
#define MAX_CHILD_PROCESS 10

enum status { NORMAL, PROCESS, FAULT, RECOVERY };

typedef struct sentinel_env_struct {
    int interval;
    char *script;
    char **script_argv;
    char *name;
    int pid;
    char *signal;
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
    sentinel_env_t env;
    enum status stat;
    cll_t *cp_list;
    int cp_count;
    struct sigaction sa;
    signal_handler_t *signal_handler;
    struct itimerval timer;
    int fault_count;
} sentinel_t;

void sentinel_init(sentinel_t *sentinel);
void sentinel_init_timer(sentinel_t *sentinel);
void sentinel_init_signals(sentinel_t *sentinel);
void sentinel_signal_handler(int signo, siginfo_t *info, void *context);
void sentinel_interval_handler(int signo, siginfo_t *info, void *context);
void sentinel_executor(sentinel_t *sentinel);
int execute_command(const char *command, const char *name, char *const argv[]);
void sentinel_print(sentinel_t *sentinel);

#endif