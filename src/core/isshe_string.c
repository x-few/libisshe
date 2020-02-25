
#include "isshe_common.h"

isshe_char_t *
isshe_strdup(isshe_char_t *src, isshe_size_t size)
{
    isshe_char_t *dst;

    dst = (isshe_char_t *)isshe_malloc(size, NULL);
    if (!dst) {
        return NULL;
    }

    isshe_memcpy(dst, src, size);

    return dst;
}

isshe_char_t *
isshe_strdup_mp(isshe_char_t *src,
    isshe_size_t size, isshe_mempool_t *mempool)
{
    isshe_char_t *dst;

    dst = (isshe_char_t *)isshe_mpalloc(mempool, size);
    if (!dst) {
        return NULL;
    }

    isshe_memcpy(dst, src, size);

    return dst;
}

isshe_int_t
isshe_string_mirror(
    isshe_char_t **pdst,
    isshe_char_t *src,
    isshe_size_t len)
{
    len += 1;       // for '\0'
    *pdst = isshe_strdup(src, len);
    if (!*pdst) {
        return ISSHE_ERROR;
    }

    return ISSHE_OK;
}