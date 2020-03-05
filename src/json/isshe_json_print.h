#ifndef _ISSHE_JSON_PRINT_H_
#define _ISSHE_JSON_PRINT_H_

#include "isshe_json.h"

//typedef struct isshe_json_print_buffer_s    isshe_json_print_buffer_t;

struct isshe_json_print_buffer_s
{
    isshe_uchar_t *buffer;
    isshe_size_t length;
    isshe_size_t offset;
    isshe_size_t depth;           // current nesting depth (for formatted printing)
    //isshe_bool_t noalloc;
    isshe_bool_t format;    // is this print a formatted print
    isshe_mempool_t *mempool;
    isshe_log_t     *log;
};

// print about
isshe_size_t isshe_json_print_length(
    const isshe_json_t * const item);

isshe_size_t isshe_json_print_format_length(
    const isshe_json_t * const item);

isshe_int_t isshe_json_print(
    const isshe_json_t * const item, isshe_log_t *log);

isshe_int_t isshe_json_print_format(
    const isshe_json_t * const item, isshe_log_t *log);

isshe_int_t isshe_json_print_buffer(
    const isshe_json_t * const item, isshe_uchar_t *buffer,
    isshe_size_t buflen, isshe_log_t *log);

isshe_int_t isshe_json_print_format_buffer(
    const isshe_json_t * const item, isshe_uchar_t *buffer,
    isshe_size_t buflen, isshe_log_t *log);

#endif