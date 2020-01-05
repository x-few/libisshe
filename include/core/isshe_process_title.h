#ifndef _ISSHE_PROCESS_TITLE_H_
#define _ISSHE_PROCESS_TITLE_H_

#include "isshe_common.h"

#if (defined __linux__ || defined __APPLE__)
#define ISSHE_SETPROCTITLE_PAD       '\0'

typedef struct isshe_process_title_s isshe_process_title_t;

struct isshe_process_title_s{
    isshe_char_t *arg0;
    isshe_char_t *base;
    isshe_char_t *end;
};

isshe_int_t isshe_process_title_init();
void isshe_process_title_set(const char *title);
#else

#define isshe_process_title_init(log)
#define isshe_process_title_set(title)

#endif

#endif