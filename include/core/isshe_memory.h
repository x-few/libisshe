#ifndef _ISSHE_MEMORY_H_
#define _ISSHE_MEMORY_H_

#include "isshe_common.h"

#define ISSHE_DEFAULT_BUFFER_LEN    1024

#define isshe_memzero(buf, n)       (void) memset(buf, 0, n)
#define isshe_memset(buf, c, n)     (void) memset(buf, c, n)

#define isshe_memcpy(dst, src, n)   (void) memcpy(dst, src, n)
#define isshe_cpymem(dst, src, n)   (((isshe_uchar_t *) memcpy(dst, src, n)) + (n))

#define isshe_memcmp(dst, src, n)   memcmp(dst, src, n)

void *isshe_malloc(isshe_size_t size, isshe_log_t *log);
void *isshe_calloc(isshe_size_t size, isshe_log_t *log);
void isshe_free(void *ptr, isshe_log_t *log);

#endif