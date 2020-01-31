#ifndef _ISSHE_STRING_H_
#define _ISSHE_STRING_H_

#include "isshe_common.h"

#define LF                  (isshe_uchar_t) '\n'
#define CR                  (isshe_uchar_t) '\r'
#define CRLF                "\r\n"

#define isshe_linefeed(p)   *p++ = LF;
#define ISSHE_LINEFEED_SIZE 1
#define ISSHE_LINEFEED      "\x0a"

#define isshe_vsnprintf     vsnprintf
#define isshe_snprintf      snprintf
#define isshe_sprintf       sprintf

typedef struct isshe_str_s          isshe_str_t;

struct isshe_str_s{
    isshe_size_t      len;
    isshe_char_t     *data;
};

isshe_char_t *isshe_strdup(isshe_char_t *src, isshe_size_t size);

isshe_char_t *isshe_strdup_mp(isshe_char_t *src,
    isshe_size_t size, isshe_mempool_t *mempool);

isshe_int_t isshe_string_mirror(isshe_char_t **pdst,
    isshe_char_t *src, isshe_size_t len);

#endif