#ifndef _ISSHE_PROCESS_TITLE_H_
#define _ISSHE_PROCESS_TITLE_H_

#include "isshe_common.h"

#if (defined __NetBSD__ || defined __FreeBSD__ || defined __OpenBSD__ || defined __DragonFly__)
#define isshe_process_title_init(log, argc, argv)
#define isshe_process_title_set                     setproctitle

#elif (defined __linux__ || defined __APPLE__)

#define ISSHE_PROCESS_TITLE_PAD         '\0'
//#define ISSHE_PROCESS_TITLE_MAX         255

typedef struct isshe_process_title_s isshe_process_title_t;

struct isshe_process_title_s{
    ilog_t  *log;
    int     argc;
    char    **argv;
    char    *argv_last;
};

isshe_int_t isshe_process_title_init(ilog_t *log, int argc, char *argv[]);
void isshe_process_title_set(const char *fmt, ...);
#else

#define isshe_process_title_init(log, argc, argv)
#define isshe_process_title_set(fmt, ...)

#endif

#endif