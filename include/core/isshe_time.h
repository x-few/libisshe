#ifndef _ISSHE_TIME_H_
#define _ISSHE_TIME_H_

#include "isshe_common.h"

int isshe_gettimeofday(struct timeval *tv, struct timezone *tz);

char * isshe_gf_time(void);

#endif