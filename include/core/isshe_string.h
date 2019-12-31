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



typedef struct isshe_str_s  isshe_str_t;

struct isshe_str_s{
    size_t      len;
    uint8_t     *data;
};

#endif