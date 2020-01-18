

#include "isshe_memory.h"

void *isshe_malloc(isshe_size_t size, isshe_log_t *log)
{
    void    *ptr;

    if ( (ptr = malloc(size)) == NULL) {
        if (log) {
            isshe_log_alert(log, "malloc(%u) failed", size);
        }
        return NULL;
    }

    if (log) {
        isshe_log_debug(log, "malloc(%u): %p", size, ptr);
    }

    return(ptr);
}


void *isshe_calloc(isshe_size_t size, isshe_log_t *log)
{
    void *ptr;

    if ( (ptr = calloc(1, size)) == NULL) {
        if (log) {
            isshe_log_alert(log, "calloc(%u) failed", size);
        }
        return NULL;
    }

    if (log) {
        isshe_log_debug(log, "calloc(%u): %p", size, ptr);
    }

    return(ptr);
}


void isshe_free(void *ptr, isshe_log_t *log)
{
    if (ptr) {
        if (log) {
            isshe_log_debug(log, "free: %p", ptr);
        }
        free(ptr);
    }
}