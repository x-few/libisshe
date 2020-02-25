#ifndef _ISSHE_PROCESS_TITLE_H_
#define _ISSHE_PROCESS_TITLE_H_

#include "isshe_common.h"

#if (defined ISSHE_BSD)
#define isshe_process_title_init(argc, argv)
#define isshe_process_title_set         setproctitle

#elif (defined ISSHE_LINUX || defined ISSHE_APPLE)

#define ISSHE_PROCESS_TITLE_PAD         '\0'
//#define ISSHE_PROCESS_TITLE_MAX         255

struct isshe_process_title_s{
    isshe_int_t             argc;
    isshe_char_t            **argv;
    isshe_char_t            *argv_last;
};

isshe_int_t isshe_process_title_init(isshe_int_t argc, isshe_char_t *argv[]);
isshe_void_t isshe_process_title_set(const isshe_char_t *fmt, ...);
#else

#define isshe_process_title_init(argc, argv)
#define isshe_process_title_set(fmt, ...)

#endif

#endif