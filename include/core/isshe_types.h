
#ifndef _ISSHE_STDINT_H_
#define _ISSHE_STDINT_H_

#include "isshe_common.h"

#define ISSHE_TRUE      1
#define ISSHE_FALSE     0

typedef int             isshe_int_t;
typedef unsigned int    isshe_uint_t;
typedef uint8_t         isshe_uint8_t;
typedef uint16_t        isshe_uint16_t;
typedef uint32_t        isshe_uint32_t;
typedef uint64_t        isshe_uint64_t;


typedef char            isshe_char_t;
typedef unsigned char   isshe_uchar_t;
typedef size_t          isshe_size_t;

typedef int             isshe_errno_t;

typedef time_t          isshe_time_t;
typedef struct tm       isshe_tm_t;


typedef pid_t           isshe_pid_t;

typedef int             isshe_bool_t;

#endif