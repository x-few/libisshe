/*
 * 暂不考虑对齐的问题。
 */

#ifndef _ISSHE_MEMORY_POOL_H_
#define _ISSHE_MEMORY_POOL_H_

#include "isshe_common.h"

#define ISSHE_MEMPOOL_MAX_FAILED            5
#define ISSHE_DEFAULT_MEMPOOL_SIZE          getpagesize()

struct isshe_mempool_data_s
{
    isshe_uchar_t           *last;
    isshe_uchar_t           *end;
    isshe_mempool_data_t    *next;
    isshe_int_t             failed;    // 失败计数
};


struct isshe_mempool_s {
    isshe_size_t                max;
    isshe_mempool_data_t        *small;
    isshe_mempool_data_t        *current;
    isshe_mempool_data_t        *last;          // 尾插
    isshe_mempool_data_t        *large;         // 头插
    isshe_log_t                 *log;
};

isshe_mempool_t *isshe_mempool_create(
    isshe_size_t size, isshe_log_t *log);

isshe_void_t isshe_mempool_destroy(isshe_mempool_t *pool);

isshe_void_t *isshe_mpalloc(
    isshe_mempool_t *pool, isshe_size_t size);

isshe_void_t isshe_mpfree(isshe_mempool_t *pool,
    isshe_void_t *ptr, isshe_size_t hint_size);

isshe_void_t *isshe_memdup(const void *str,
    isshe_size_t size, isshe_mempool_t *mempool);

isshe_int_t
isshe_mempool_log_set(isshe_mempool_t *mempool, isshe_log_t *log);

#endif