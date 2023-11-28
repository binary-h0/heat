#include "option_handler.h"

struct option options[] = {
    {"interval", required_argument, 0, 'i'},
    {"script", required_argument, 0, 's'},
    {"pid", required_argument, 0, 'p'},
    {"signal", required_argument, 0, 'S'},
    {"fail", required_argument, 0, 'f'},
    {"recovery", required_argument, 0, 'r'},
    {"threshold", required_argument, 0, 't'},
    {"fault-signal", required_argument, 0, 'F'},
    {"success-signal", required_argument, 0, 'g'},
    {"recovery-timeout", required_argument, 0, 'R'},
};

int option_preproceessor(int argc, char *argv[]) {
    int parsing_point = 1;
    bool is_opt = false;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (is_opt) {
                return parsing_point;
            }
            is_opt = true;
        } else {
            if (!is_opt) {
                return parsing_point;
            }
            is_opt = false;
        }
        parsing_point++;
    }

    return parsing_point;
}

opts *option_process(int argc, char *argv[]) {  // TODO: 함수명 변경 필요
    extern char *optarg;
    // extern int optind, opterr, optopt;
    extern int errno;
    int opt, parsing_point, option_index = 0;
    // option values
    int interval = 1;
    char *script = NULL;
    int pid = -1;
    char *signal = NULL;
    char *fail = NULL;
    char *recovery = NULL;
    int threshold = 1;
    char *fault_signal = NULL;
    char *success_signal = NULL;
    int recovery_timeout = -1;

    parsing_point = option_preproceessor(argc, argv);

    while ((opt = getopt_long(parsing_point, argv, "s:i:", options, NULL)) !=
           -1) {
        switch (opt) {
            case 'i':  // TODO: overflow check 필요
                if ((interval = atoi(optarg)) > 0) {
                    break;
                } else {
                    errno = EINVAL;
                    perror(
                        "\e[1;31m"
                        "[ERROR] -i value must be positive integer"
                        "\e[0m");
                    return NULL;
                }
            case 's':
                script = optarg;
                break;
            case 'p':  // TODO: overflow check 필요
                if ((pid = atoi(optarg)) > 0) {
                    break;
                } else {
                    errno = EINVAL;
                    perror(
                        "\e[1;31m"
                        "[ERROR] --pid value must be positive integer"
                        "\e[0m");
                    return NULL;
                }
                break;
            case 'S':
                signal = optarg;
                break;
            case 'f':
                fail = optarg;
                break;
            case 'r':
                recovery = optarg;
                break;
            case 't':  // TODO: overflow check 필요
                if ((threshold = atoi(optarg)) > 0) {
                    break;
                } else {
                    errno = EINVAL;
                    perror(
                        "\e[1;31m"
                        "[ERROR] --threshold value must be positive integer"
                        "\e[0m");
                    return NULL;
                }
                break;
            case 'F':
                fault_signal = optarg;
                break;
            case 'g':
                success_signal = optarg;
                break;
            case 'R':  // TODO: overflow check 필요
                if ((recovery_timeout = atoi(optarg)) > 0) {
                    break;
                } else {
                    errno = EINVAL;
                    perror(
                        "\e[1;31m"
                        "[ERROR] --recovery-timeout value must be positive "
                        "integer"
                        "\e[0m");
                    return NULL;
                }
                break;
        }
    }

    if (argc != parsing_point) {
        char s[100];
        int n = sprintf(s, "%s", argv[parsing_point]);
        if (n < 0) {
            perror("[ERROR] sprintf");
            return NULL;
        }
        for (int i = parsing_point + 1; i < argc; i++) {
            if ((n = sprintf(s, "%s %s", s, argv[i])) < 0) {
                perror("[ERROR] sprintf");
                return NULL;
            }
        }
        script = s;
    }

    opts *options = (opts *)malloc(sizeof(opts));
    options->interval = interval;
    options->script = script;
    options->pid = pid;
    options->signal = signal;
    options->fail = fail;
    options->recovery = recovery;
    options->threshold = threshold;
    options->fault_signal = fault_signal;
    options->success_signal = success_signal;
    options->recovery_timeout = recovery_timeout;

    return options;
}
