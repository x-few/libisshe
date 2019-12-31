

#include "isshe_memory.h"

void *isshe_malloc(isshe_size_t size)
{
    void    *ptr;

    if ( (ptr = malloc(size)) == NULL) {
        return NULL;
    }

    return(ptr);
}


void *isshe_calloc(isshe_size_t size)
{
    void *ptr;

    if ( (ptr = calloc(1, size)) == NULL) {
        return NULL;
    }

    return(ptr);
}


void isshe_free(void *ptr)
{
    if (ptr) {
        free(ptr);
    }
}