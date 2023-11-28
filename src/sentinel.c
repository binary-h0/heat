#include <stdio.h>
#include <string.h>

int interval = 1;
int pid = -1;

int main(int argc, char const *argv[]) {
    pid = atoi(argv[1][1]);
    printf("%d\n", pid);

    return 0;
}
