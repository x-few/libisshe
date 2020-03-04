
#ifndef _ISSHE_RAND_H_
#define _ISSHE_RAND_H_

#include "isshe_common.h"

//#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#define ISSHE_DEV_URANDOM "/dev/urandom"
//#endif

int isshe_rand_bytes(isshe_char_t *buf, isshe_int_t len);

#endif