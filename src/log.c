#include "log.h"

void _ssl_debug(void *ctx, int level, const char *file, int line, const char *str)
{
    ((void) level);

    LOGD("%s:%04d: %s", FILENAME(file), line, str);
}