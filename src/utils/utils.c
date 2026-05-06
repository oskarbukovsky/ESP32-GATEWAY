#include "utils.h"

#include <stdio.h>
#include <time.h>

void utils_get_time_str(char *buf, size_t buf_len)
{
    time_t now = time(NULL);
    if (now > 1700000000) {
        struct tm tm_now;
        localtime_r(&now, &tm_now);
        strftime(buf, buf_len, "%Y-%m-%d %H:%M:%S", &tm_now);
    } else {
        snprintf(buf, buf_len, "<no-sync>");
    }
}
