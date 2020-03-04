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


#define isshe_tostring(str)     { sizeof(str) - 1, (ishse_char_t *) str }
#define isshe_null_string       { 0, NULL }

#define isshe_sprintf           sprintf
#define isshe_strlen            strlen
#define isshe_strsize(s)        (sizeof(s) - sizeof(""))
#define isshe_strcmp            strcmp

struct isshe_string_s{
    isshe_size_t      len;
    isshe_char_t     *data;
};

isshe_char_t *isshe_strdup(isshe_char_t *src, isshe_mempool_t *mempool);

isshe_int_t isshe_strcmp_case_insensitive(
    const isshe_char_t *str1,
    const isshe_char_t *str2);

isshe_int_t isshe_vsnprintf(
    isshe_char_t *buf, isshe_size_t size,
    const char *fmt, va_list args);

isshe_int_t isshe_snprintf(
    isshe_char_t *buf, isshe_size_t max,
    const isshe_char_t *fmt, ...);

isshe_string_t *isshe_string_create(const isshe_char_t *str,
    isshe_size_t size, isshe_mempool_t *mempool);

isshe_void_t isshe_string_destroy(
    isshe_string_t *string, isshe_mempool_t *mempool);

#endif