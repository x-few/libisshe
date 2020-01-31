#ifndef _ISSHE_PROCESS_TITLE_H_
#define _ISSHE_PROCESS_TITLE_H_

#include "isshe_common.h"

#if (defined ISSHE_BSD)
#define isshe_process_title_init(log, argc, argv)
#define isshe_process_title_set                     setproctitle

#elif (defined ISSHE_LINUX || defined ISSHE_APPLE)

#define ISSHE_PROCESS_TITLE_PAD         '\0'
//#define ISSHE_PROCESS_TITLE_MAX         255

struct isshe_process_title_s{
    int             argc;
    char            **argv;
    char            *argv_last;
};

isshe_int_t isshe_process_title_init(int argc, char *argv[]);
void isshe_process_title_set(const char *fmt, ...);
#else

#define isshe_process_title_init(log, argc, argv)
#define isshe_process_title_set(fmt, ...)

#endif

#endif