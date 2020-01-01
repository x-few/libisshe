
#include "isshe_common.h"


isshe_int_t
isshe_string_mirror(
    isshe_char_t **pdst,
    isshe_char_t *src,
    isshe_size_t len)
{
    len += 1;       // for '\0'
    isshe_char_t *dst = (isshe_char_t *)malloc(len);
    if (!dst) {
        return ISSHE_FAILURE;
    }

    isshe_memcpy(dst, src, len);

    *pdst = dst;

    return ISSHE_SUCCESS;
}