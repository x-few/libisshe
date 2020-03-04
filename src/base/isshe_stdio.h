#ifndef _ISSHE_STDIO_H_
#define _ISSHE_STDIO_H_

#include "isshe_common.h"

isshe_char_t *isshe_fgets(isshe_char_t *ptr,
    isshe_int_t n, isshe_fstream_t *stream);

isshe_void_t isshe_fputs(
    const isshe_char_t *ptr, isshe_fstream_t *stream);

#endif