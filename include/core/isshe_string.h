#ifndef _ISSHE_STRING_H_
#define _ISSHE_STRING_H_

#include <stdio.h>
#include <stdint.h>

typedef struct isshe_str_s isshe_str_t;

struct isshe_str_s{
    size_t      len;
    uint8_t     *data;
};

#endif