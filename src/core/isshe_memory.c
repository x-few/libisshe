

#include "isshe_memory.h"

isshe_void_t *isshe_malloc(isshe_size_t size, isshe_log_t *log)
{
    isshe_void_t    *ptr;

    if ( (ptr = malloc(size)) == NULL) {
        if (log) {
            isshe_log_alert(log, "malloc(%ud) failed", size);
        }
        return NULL;
    }

    if (log) {
        isshe_log_debug(log, "malloc(%ud): %p", size, ptr);
    }

    return(ptr);
}


isshe_void_t *isshe_calloc(isshe_size_t size, isshe_log_t *log)
{
    isshe_void_t *ptr;

    if ( (ptr = calloc(1, size)) == NULL) {
        if (log) {
            isshe_log_alert(log, "calloc(%ud) failed", size);
        }
        return NULL;
    }

    if (log) {
        isshe_log_debug(log, "calloc(%ud): %p", size, ptr);
    }

    return(ptr);
}


isshe_void_t isshe_free(isshe_void_t *ptr, isshe_log_t *log)
{
    if (ptr) {
        if (log) {
            isshe_log_debug(log, "free: %p", ptr);
        }
        free(ptr);
    }
}