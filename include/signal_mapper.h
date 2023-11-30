#ifndef SIGNAL_MAPPER_H
#define SIGNAL_MAPPER_H

#include <signal.h>
#include <string.h>

typedef struct signal_mapper_struct {
    const char *signal_name;
    int signal_id;
} signal_mapper_t;

static const signal_mapper_t signals[] = {
    // 모든 시그널 설정
    {"SIGHUP", SIGHUP},       {"SIGINT", SIGINT},       {"SIGQUIT", SIGQUIT},
    {"SIGILL", SIGILL},       {"SIGTRAP", SIGTRAP},     {"SIGABRT", SIGABRT},
    {"SIGBUS", SIGBUS},       {"SIGFPE", SIGFPE},       {"SIGKILL", SIGKILL},
    {"SIGUSR1", SIGUSR1},     {"SIGSEGV", SIGSEGV},     {"SIGUSR2", SIGUSR2},
    {"SIGPIPE", SIGPIPE},     {"SIGALRM", SIGALRM},     {"SIGTERM", SIGTERM},
    {"SIGSTKFLT", SIGSTKFLT}, {"SIGCHLD", SIGCHLD},     {"SIGCONT", SIGCONT},
    {"SIGSTOP", SIGSTOP},     {"SIGTSTP", SIGTSTP},     {"SIGTTIN", SIGTTIN},
    {"SIGTTOU", SIGTTOU},     {"SIGURG", SIGURG},       {"SIGXCPU", SIGXCPU},
    {"SIGXFSZ", SIGXFSZ},     {"SIGVTALRM", SIGVTALRM}, {"SIGPROF", SIGPROF},
    {"SIGWINCH", SIGWINCH},   {"SIGIO", SIGIO},         {"SIGPWR", SIGPWR},
    {"SIGSYS", SIGSYS},
    // 추후에 리얼타임 시그널 설정
    // {"SIGMIN", 34}
    // {"SIGMIN", }
};

int get_signal_id(const char *signal_name);

#endif