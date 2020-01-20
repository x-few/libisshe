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

    pool = (isshe_mempool_t *)isshe_malloc(size, log);
    if (!pool) {
        return NULL;
    }
    
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

    pagesize = getpagesize();
    //isshe_log_info(log, "page size: %u", pagesize);
    pool->max = size < pagesize ? size : pagesize;      // 最大一页！

    return pool;
}

void
isshe_mempool_destroy(isshe_mempool_t *pool)
{
    isshe_mempool_data_t *tmp;
    isshe_mempool_data_t *next;
    
    if (!pool) {
        return;
    }

    // free large
    for (tmp = pool->large; tmp; tmp = tmp->next)
    {
        isshe_free(tmp->last, pool->log);
    }

    // free small
    for (tmp = pool->small->next; tmp; )
    {
        next = tmp->next;
        isshe_free(tmp, pool->log);
        tmp = next;
    }

    isshe_free(pool, pool->log);
}

static isshe_mempool_data_t *
isshe_mpdata_create(isshe_mempool_t *pool, isshe_size_t size)
{
    isshe_size_t            data_block_size;
    isshe_mempool_data_t    *tmp;

    data_block_size = pool->small->end - (isshe_uchar_t *)pool;
    
    size += sizeof(isshe_mempool_data_t);
    size = size > data_block_size ? size : data_block_size;

    tmp = (isshe_mempool_data_t *)isshe_malloc(size, pool->log);
    if (!tmp) {
        return NULL;
    }

    tmp->last = (isshe_uchar_t *)tmp + sizeof(isshe_mempool_data_t);
    tmp->end = (isshe_uchar_t *)tmp + size;
    tmp->next = NULL;
    tmp->failed = 0;

    pool->last->next = tmp;
    pool->last = tmp;

    return tmp;
}

static void
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

static void *
isshe_mpalloc_small(isshe_mempool_t *pool, isshe_size_t size)
{
    isshe_mempool_data_t    *tmp;
    isshe_uchar_t           *target;

    for (tmp = pool->current; tmp; tmp = tmp->next)
    {
        if (tmp->end - tmp->last >= size) {
            target = tmp->last;
            tmp->last += size;
            return target;
        }
        tmp->failed++;
    }

    // new data block
    tmp = isshe_mpdata_create(pool, size);
    if (!tmp) {
        isshe_log_alert(pool->log, "isshe_mpdata_create failed");
        return NULL;
    }

    target = tmp->last;
    tmp->last += size;

    // update current;
    isshe_mempool_current_update(pool);

    return target;
}


static void *
isshe_mpalloc_large(isshe_mempool_t *pool, isshe_size_t size)
{
    isshe_mempool_data_t    *large;
    void                    *data;

    // alloc size from system
    data = isshe_malloc(size, pool->log);
    if (!data) {
        return NULL;
    }

    // alloc large struct from pool, 放后面是为了错误使用了内存池。
    large = (isshe_mempool_data_t *)isshe_mpalloc_small(pool, sizeof(isshe_mempool_data_t));
    if (!large) {
        isshe_free(data, pool->log);
        return NULL;
    }

    large->last = (isshe_uchar_t *)data;
    large->end = large->last + size;

    // 尾插
    large->next = pool->large;
    pool->large = large;

    return data;
}

void *
isshe_mpalloc(isshe_mempool_t *pool, isshe_size_t size)
{
    if (size <= pool->max) {
        isshe_log_debug(pool->log, "isshe_mpalloc called isshe_mpalloc_small");
        return isshe_mpalloc_small(pool, size);
    }

    isshe_log_debug(pool->log, "isshe_mpalloc called isshe_mpalloc_large");
    return isshe_mpalloc_large(pool, size);
}

void
isshe_mpfree(isshe_mempool_t *pool, void *ptr, isshe_size_t hint_size)
{
    isshe_mempool_data_t *tmp;

    if (hint_size > 0 && hint_size < pool->max) {
        return ;
    }

    for (tmp = pool->large; tmp; tmp = tmp->next)
    {
        if (tmp->last == ptr) {
            isshe_free(tmp->last, pool->log);
            tmp->last = NULL;
            return ;
        }
    }
}


