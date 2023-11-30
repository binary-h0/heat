#include "signal_mapper.h"

int get_signal_id(const char *signal_name) {
    for (int i = 0; i < sizeof(signals) / sizeof(signals[0]); i++)
        if (strcmp(signals[i].signal_name, signal_name) == 0)
            return signals[i].signal_id;
    return -1;
}