#ifndef OPTION_HANDLER_H
#define OPTION_HANDLER_H

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#define false 0
#define true 1
#define MAX_COMMAND_LENGTH 100

typedef int bool;
typedef struct opts_struct {
    char *interval;
    char *script;
    char *name;
    char *pid;
    char *signal;
    char *fail;
    char *recovery;
    char *threshold;
    char *fault_signal;
    char *success_signal;
    char *recovery_timeout;
} opts;

int option_preproceessor(int argc, char *const argv[]);
opts *option_process(int argc, char *const argv[]);

#endif