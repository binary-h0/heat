#ifndef HEAT_H
#define HEAT_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "option_handler.h"
#include "signal_mapper.h"

extern int errno;

void process_info_print();
int verify_options(opts *options);
void verify_command(char *command);
void verify_script(char *script);
void verify_recovery(char *script);
void verify_fail(char *script);
void check_file_permission(char *filename);

void update_environment(opts *options);
int build_sentinel_process(opts *options);

#endif