#ifndef _ISSHE_STRING_H_
#define _ISSHE_STRING_H_

#include "isshe_common.h"

#define LF                  (isshe_uchar_t) '\n'
#define CR                  (isshe_uchar_t) '\r'
#define CRLF                "\r\n"

#define isshe_linefeed(p)   *p++ = LF;
#define ISSHE_LINEFEED_SIZE 1
#define ISSHE_LINEFEED      "\x0a"

#define ISSHE_MAX_INT32_STR_LEN (sizeof("-2147483648") - 1)
#define ISSHE_MAX_INT64_STR_LEN (sizeof("-9223372036854775808") - 1)

#define isshe_sprintf           sprintf
#define isshe_strlen            strlen

struct isshe_string_s{
    isshe_size_t      len;
    isshe_char_t     *data;
};

isshe_char_t *isshe_strdup(isshe_char_t *src, isshe_size_t size);

isshe_char_t *isshe_strdup_mp(isshe_char_t *src,
    isshe_size_t size, isshe_mempool_t *mempool);

isshe_int_t isshe_string_mirror(isshe_char_t **pdst,
    isshe_char_t *src, isshe_size_t len);

isshe_int_t isshe_vsnprintf(
    isshe_char_t *buf, isshe_size_t size,
    const char *fmt, va_list args);

isshe_int_t isshe_snprintf(
    isshe_char_t *buf, isshe_size_t max,
    const isshe_char_t *fmt, ...);

#endif