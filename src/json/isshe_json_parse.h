#ifndef _ISSHE_JSON_PARSE_H_
#define _ISSHE_JSON_PARSE_H_

#include "isshe_json.h"

#define ISSHE_JSON_NUMBER_STRING_MAX    64

struct isshe_json_error_s {
    const isshe_uchar_t *json;
    isshe_size_t position;
};

struct isshe_json_parse_buffer_s {
    isshe_uchar_t *content;
    isshe_size_t length;
    isshe_size_t offset;
    isshe_size_t depth;
    isshe_mempool_t *mempool;
    isshe_log_t     *log;
};

// isshe_json_parse_buffer_t:
// check if the given size is left to read in a given parse buffer (starting with 1)
#define parse_buffer_can_read(buffer, size) \
    ((buffer != NULL) \
    && (((buffer)->offset + size) <= (buffer)->length))

// check if the buffer can be accessed at the given index (starting with 0)
#define parse_buffer_can_access(buffer, index) \
    ((buffer != NULL) \
    && (((buffer)->offset + index) < (buffer)->length))

#define parse_buffer_cannot_access(buffer, index) (!parse_buffer_can_access(buffer, index))

// get a pointer to the buffer at the position
#define parse_buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)


/**
 * Memory Management: the caller is always responsible to
 * free the results from all variants of isshe_json_parse (with isshe_json_delete) 
 * and isshe_json_print (with stdlib free, isshe_json_hooks_t.free_fn, 
 * or isshe_json_free as appropriate). 
 * The exception is isshe_json_print_pre_allocated, 
 * where the caller has full responsibility of the buffer.
 * Supply a block of JSON, and this returns a isshe_json_t object you can interrogate.
 */
isshe_json_t * isshe_json_parse(
    const isshe_char_t *value,
    isshe_mempool_t *mempool);

isshe_json_t *isshe_json_file_parse(
    const isshe_char_t *filename,
    isshe_mempool_t *mempool);

const isshe_char_t *isshe_json_parse_error_get(isshe_void_t);

#endif