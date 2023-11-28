#ifndef OPTION_HANDLER_H
#define OPTION_HANDLER_H

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#define false 0
#define true 1

typedef int bool;
typedef struct opts_struct {
    int interval;
    char *script;
    int pid;
    char *signal;
    char *fail;
    char *recovery;
    int threshold;
    char *fault_signal;
    char *success_signal;
    int recovery_timeout;
} opts;

int option_preproceessor(int argc, char *argv[]);
opts *option_process(int argc, char *argv[]);

#endif