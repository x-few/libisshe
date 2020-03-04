
#include "isshe_common.h"


isshe_char_t *
isshe_fgets(isshe_char_t *ptr,
    isshe_int_t n, isshe_fstream_t *stream)
{
    isshe_char_t    *rptr;

    if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
        isshe_sys_error_exit("fgets error");

    return (rptr);
}

isshe_void_t
isshe_fputs(const isshe_char_t *ptr,
    isshe_fstream_t *stream)
{
    if (fputs(ptr, stream) == EOF) {
        isshe_sys_error_exit("fputs error");
    }
}