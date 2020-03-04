#ifndef _ISSHE_TIME_H_
#define _ISSHE_TIME_H_

#include "isshe_common.h"

isshe_int_t isshe_gettimeofday(
    isshe_timeval_t *tv, isshe_timezone_t *tz);

isshe_char_t *isshe_gf_time(isshe_void_t);

#endif