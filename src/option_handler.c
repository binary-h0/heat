#include "heat.h"
#pragma GCC diagnostic ignored "-Wformat-overflow="

struct option optop[] = {
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

int option_preproceessor(int argc, char *const argv[]) {
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

opts *option_process(int argc, char *const argv[]) {  // TODO: 함수명 변경 필요
    extern char *optarg;
    // extern int optind, opterr, optopt;
    extern int errno;
    int opt, parsing_point, option_index = 0;
    // option values
    opts *options = (opts *)malloc(sizeof(opts));

    parsing_point = option_preproceessor(argc, argv);

    while ((opt = getopt_long(parsing_point, argv, "s:i:", optop, NULL)) !=
           -1) {
        switch (opt) {
            case 'i':  // TODO: overflow check 필요
                if (atoi(optarg) > 0)
                    options->interval = optarg;
                else {
                    errno = EINVAL;
                    perror(
                        "\e[1;31m"
                        "[ERROR] -i value must be positive integer"
                        "\e[0m");
                    return NULL;
                }
                break;
            case 's':
                options->script = optarg;
                char *cp_arg =
                    (char *)malloc(strlen(optarg) + 1);  // TODO: free 필요
                strcpy(cp_arg, optarg);
                char *stack[MAX_COMMAND_LENGTH];
                char *tok = strtok(cp_arg, "/");
                int i = 0;
                while (tok != NULL) {
                    stack[i++] = tok;
                    tok = strtok(NULL, "/");
                }
                tok = strtok(stack[i - 1], ".");
                options->name = tok;
                break;
            case 'p':  // TODO: overflow check 필요
                if (atoi(optarg) > 0)
                    options->pid = optarg;
                else {
                    errno = EINVAL;
                    perror(
                        "\e[1;31m"
                        "[ERROR] --pid value must be positive integer"
                        "\e[0m");
                    return NULL;
                }
                break;
            case 'S':
                options->signal = optarg;
                break;
            case 'f':
                options->fail = optarg;
                break;
            case 'r':
                options->recovery = optarg;
                break;
            case 't':  // TODO: overflow check 필요
                if (atoi(optarg) > 0)
                    options->threshold = optarg;
                else {
                    errno = EINVAL;
                    perror(
                        "\e[1;31m"
                        "[ERROR] --threshold value must be positive integer"
                        "\e[0m");
                    return NULL;
                }
                break;
            case 'F':
                options->fault_signal = optarg;
                break;
            case 'g':
                options->success_signal = optarg;
                break;
            case 'R':  // TODO: overflow check 필요
                if (atoi(optarg) > 0)
                    options->recovery_timeout = optarg;
                else {
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
        char s[MAX_COMMAND_LENGTH];
        char *cp_arg =
            (char *)malloc(strlen(argv[parsing_point]) + 1);  // TODO: free 필요
        strcpy(cp_arg, argv[parsing_point]);
        char *stack[MAX_COMMAND_LENGTH];
        char *tok = strtok(cp_arg, "/");
        int i = 0;
        while (tok != NULL) {
            stack[i++] = tok;
            tok = strtok(NULL, "/");
        }
        tok = strtok(stack[i - 1], ".");
        options->name = tok;

        int n = sprintf(s, "%s", argv[parsing_point]);
        if (n < 0) {
            perror("[ERROR] sprintf");
            return NULL;
        }
        for (i = parsing_point + 1; i < argc; i++) {
            if ((n = sprintf(s, "%s %s", s, argv[i])) < 0) {
                perror("[ERROR] sprintf");
                return NULL;
            }
        }
        options->script = (char *)malloc(strlen(s) + 1);  // TODO: free 필요
        strcpy(options->script, s);
    }

    if (options->interval == NULL) options->interval = "1";
    if (options->script == NULL) options->script = "echo hello";
    if (options->name == NULL) options->name = "hello";
    if (options->pid == NULL) {
        options->pid = (char *)malloc(sizeof(char) * 10);
        sprintf(options->pid, "%d", getpid());
    }
    if (options->signal == NULL) options->signal = "";
    if (options->fail == NULL) options->fail = "";
    if (options->recovery == NULL) options->recovery = "";
    if (options->threshold == NULL) options->threshold = "";
    if (options->fault_signal == NULL) options->fault_signal = "";
    if (options->success_signal == NULL) options->success_signal = "";
    if (options->recovery_timeout == NULL) options->recovery_timeout = "";

    return options;
}
