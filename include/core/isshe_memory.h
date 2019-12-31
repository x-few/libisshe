#ifndef _ISSHE_MEMORY_H_
#define _ISSHE_MEMORY_H_

#include "isshe_common.h"

#define isshe_memzero(buf, n)       (void) memset(buf, 0, n)
#define isshe_memset(buf, c, n)     (void) memset(buf, c, n)

#define isshe_memcpy(dst, src, n)   (void) memcpy(dst, src, n)
#define isshe_cpymem(dst, src, n)   (((isshe_uchar_t *) memcpy(dst, src, n)) + (n))

void *isshe_malloc(isshe_size_t size);
void *isshe_calloc(isshe_size_t size);
void isshe_free(void *ptr);

#endif