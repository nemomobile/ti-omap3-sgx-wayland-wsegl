#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static int logLevel = -1;

static void
log_init()
{
    if (logLevel == -1) {
        const char *dbg = getenv("WSEGL_DEBUG");
        if (dbg)
            logLevel = atoi(dbg);
        else
            logLevel = 0;
    }
}

void
wsegl_info(const char *fmt, ...)
{
    log_init();

    if (logLevel == 0)
        return;

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}

void
wsegl_debug(const char *fmt, ...)
{
    log_init();

    if (logLevel < 2)
        return;

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");
}

