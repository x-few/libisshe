#include "isshe_common.h"
/*
                     last      end
+-------+-------+------+--------+
| small | large | used | unused |
+-------+-------+------+--------+
    |       |
 [     ] [     ]
    |       |
 [     ] [     ]
     ...
*/

/*
 * 内存池数据结构存放在分配的内存中。
 */
isshe_mempool_t *
isshe_mempool_create(isshe_size_t size, isshe_log_t *log)
{
    isshe_mempool_t         *pool;
    isshe_mempool_data_t    *small;
    isshe_int_t             pagesize;

    pool = (isshe_mempool_t *)isshe_malloc(size);
    if (!pool) {
        return NULL;
    }

    isshe_memzero(pool, size);

    small = (isshe_mempool_data_t *)((isshe_uchar_t *)pool + sizeof(isshe_mempool_t));
    small->last = (isshe_uchar_t *)small + sizeof(isshe_mempool_data_t);
    small->end = (isshe_uchar_t *)pool + size;
    small->next = NULL;
    small->failed = 0;

    pool->small = small;
    pool->current = pool->small;
    pool->last = pool->current;
    pool->large = NULL;
    pool->log = log;
    pool->total_small = size;
    pool->used_small = sizeof(isshe_mempool_t) + sizeof(isshe_mempool_data_t);

    pagesize = getpagesize();
    pool->max = size < pagesize ? size : pagesize;      // 最大一页！

    if (log) {
        isshe_log_debug(log, "create mempool: %p", pool);
    } else {
        isshe_log_stderr(0, "create mempool: %p", pool);
    }

    return pool;
}

isshe_void_t
isshe_mempool_destroy(isshe_mempool_t *pool)
{
    isshe_mempool_data_t *tmp;
    isshe_mempool_data_t *next;
    
    if (!pool) {
        return;
    }

    if (pool->log) {
        isshe_log_debug(pool->log, "destroy mempool: %p", pool);
    } else {
        isshe_log_stderr(0, "destroy mempool: %p", pool);
    }

    // free large
    for (tmp = pool->large; tmp; tmp = tmp->next)
    {
        isshe_free(tmp->last);
    }

    // free small
    for (tmp = pool->small->next; tmp; )
    {
        next = tmp->next;
        if (pool->log) {
            isshe_log_debug(pool->log, "- free small: %p", tmp);
        } else {
            isshe_log_stderr(0, "- free small: %p", tmp);
        }
        isshe_free(tmp);
        tmp = next;
    }

    isshe_free(pool);
}

static isshe_mempool_data_t *
isshe_mpdata_create(isshe_mempool_t *pool, isshe_size_t size)
{
    isshe_size_t            data_block_size;
    isshe_mempool_data_t    *tmp;

    data_block_size = pool->small->end - (isshe_uchar_t *)pool;
    
    size += sizeof(isshe_mempool_data_t);
    size = size > data_block_size ? size : data_block_size;

    tmp = (isshe_mempool_data_t *)isshe_malloc(size);
    if (!tmp) {
        return NULL;
    }

    tmp->last = (isshe_uchar_t *)tmp + sizeof(isshe_mempool_data_t);
    tmp->end = (isshe_uchar_t *)tmp + size;
    tmp->next = NULL;
    tmp->failed = 0;

    pool->last->next = tmp;
    pool->last = tmp;
    pool->total_small += size;
    pool->used_small += sizeof(isshe_mempool_data_t);

        if (pool->log) {
            isshe_log_debug(pool->log, "- alloc small: %p", tmp);
        } else {
            isshe_log_stderr(0, "- alloc small: %p", tmp);
        }

    return tmp;
}

static isshe_void_t
isshe_mempool_current_update(isshe_mempool_t *pool)
{
    isshe_mempool_data_t    *tmp;

    for (tmp = pool->current; tmp; tmp = tmp->next) {
        if (tmp->failed >= ISSHE_MEMPOOL_MAX_FAILED) {
            continue;
        } else {
            pool->current = tmp;
            break;
        }
    }
}

static isshe_void_t *
isshe_mpalloc_small(isshe_mempool_t *pool, isshe_size_t size)
{
    isshe_mempool_data_t    *tmp;
    isshe_uchar_t           *target;

    for (tmp = pool->current; tmp; tmp = tmp->next)
    {
        if (tmp->end - tmp->last >= size) {
            target = tmp->last;
            tmp->last += size;
            pool->used_small += size;
            return target;
        }
        tmp->failed++;
    }

    // new data block
    tmp = isshe_mpdata_create(pool, size);
    if (!tmp) {
        if (pool->log) {
            isshe_log_alert(pool->log, "isshe_mpdata_create failed");
        }
        return NULL;
    }

    target = tmp->last;
    tmp->last += size;
    pool->used_small += size;

    // update current;
    isshe_mempool_current_update(pool);

    return target;
}


static isshe_void_t *
isshe_mpalloc_large(isshe_mempool_t *pool, isshe_size_t size)
{
    isshe_mempool_data_t    *large;
    isshe_void_t                    *data;

    // alloc size from system
    data = isshe_malloc(size);
    if (!data) {
        return NULL;
    }

    // alloc large struct from pool, 放后面是为了错误使用了内存池。
    large = (isshe_mempool_data_t *)isshe_mpalloc_small(pool, sizeof(isshe_mempool_data_t));
    if (!large) {
        isshe_free(data);
        return NULL;
    }

    large->last = (isshe_uchar_t *)data;
    large->end = large->last + size;

    // 尾插
    large->next = pool->large;
    pool->large = large;

    return data;
}

isshe_void_t *
isshe_mpalloc(isshe_mempool_t *pool, isshe_size_t size)
{
    if (!pool) {
        return NULL;
    }

    if (size <= pool->max) {
        return isshe_mpalloc_small(pool, size);
    }

    return isshe_mpalloc_large(pool, size);
}

isshe_void_t
isshe_mpfree(isshe_mempool_t *pool, isshe_void_t *ptr, isshe_size_t hint_size)
{
    isshe_mempool_data_t *tmp;

    if (hint_size > 0 && hint_size < pool->max) {
        return ;
    }

    for (tmp = pool->large; tmp; tmp = tmp->next)
    {
        if (tmp->last == ptr) {
            isshe_free(tmp->last);
            tmp->last = NULL;
            return ;
        }
    }
}

isshe_void_t *
isshe_memdup(const isshe_void_t *src,
    isshe_size_t size, isshe_mempool_t *mempool)
{
    isshe_void_t *dst;

    if (mempool) {
        dst = (isshe_char_t *)isshe_mpalloc(mempool, size);
    } else {
        dst = (isshe_char_t *)isshe_malloc(size);
    }
    if (!dst) {
        return NULL;
    }

    isshe_memcpy(dst, src, size);

    return dst;
}

isshe_int_t
isshe_mempool_log_set(isshe_mempool_t *mempool, isshe_log_t *log)
{
    if (!mempool || !log) {
        return ISSHE_ERROR;
    }

    mempool->log = log;

    return ISSHE_OK;
}

isshe_void_t
isshe_mempool_stat_print(isshe_mempool_t *mempool, isshe_log_t *log)
{
    isshe_char_t *fmt =
        "mempool(%p) stat: \n" \
        "- total small  : %d (bytes)\n" \
        "- used small   : %d (bytes)\n";
    if (log) {
        isshe_log_info(log, fmt, mempool, mempool->total_small, mempool->used_small);
    } else {
        isshe_log_stderr(0, fmt, mempool, mempool->total_small, mempool->used_small);
    }
}


