

#include "isshe_memory.h"

isshe_void_t *isshe_malloc(isshe_size_t size)
{
    isshe_void_t    *ptr;

    if ( (ptr = malloc(size)) == NULL) {
        isshe_log_stderr(0, "malloc(%ud) failed", size);
        return NULL;
    }
#ifdef ISSHE_MEMORY_DEBUG
    isshe_log_stderr(0, "malloc(%ud): %p", size, ptr);
#endif

    return(ptr);
}


isshe_void_t *isshe_calloc(isshe_size_t size)
{
    isshe_void_t *ptr;

    if ( (ptr = calloc(1, size)) == NULL) {
        isshe_log_stderr(0, "calloc(%ud) failed", size);
        return NULL;
    }
#ifdef ISSHE_MEMORY_DEBUG
    isshe_log_stderr(0, "calloc(%ud): %p", size, ptr);
#endif

    return(ptr);
}


isshe_void_t isshe_free(isshe_void_t *ptr)
{
    if (ptr) {
#ifdef ISSHE_MEMORY_DEBUG
        isshe_log_stderr(0, "free: %p", ptr);
#endif
        free(ptr);
    }
}
