/*
 * base on cJSON v1.7.12
 * https://github.com/DaveGamble/cJSON
 */

#ifndef _ISSHE_JSON_H_
#define _ISSHE_JSON_H_

#include "isshe_common.h"
#include "isshe_json_print.h"

// isshe_json_t Types:
#define ISSHE_JSON_INVALID          (0)
#define ISSHE_JSON_FALSE            (1 << 0)
#define ISSHE_JSON_TRUE             (1 << 1)
#define ISSHE_JSON_NULL             (1 << 2)
#define ISSHE_JSON_NUMBER           (1 << 3)
#define ISSHE_JSON_STRING           (1 << 4)
#define ISSHE_JSON_ARRAY            (1 << 5)
#define ISSHE_JSON_OBJECT           (1 << 6)
#define ISSHE_JSON_RAW              (1 << 7)

#define ISSHE_JSON_IS_REFERENCE     256
#define ISSHE_JSON_STRING_IS_CONST  512
#define ISSHE_JSON_NUMBER_LEN_MAX   26

// The isshe_json_t structure:
/**
 * struct isshe_json_s:
 * next/prev: next/prev allow you to walk array/object chains.(GetArraySize...)
 * child: An array or object item will have a child pointer.
 * type: The type of the item, as above.
 * vstring: The item's string, if type==ISSHE_JSON_STRING  and type == ISSHE_JSON_RAW.
 * vint: writing to vint is DEPRECATED, use isshe_json_set_number_value instead
 * vnumber: The item's number, if type==ISSHE_JSON_NUMBER.
 * string: The item's name string
 * 
 */
struct isshe_json_s
{
    isshe_json_t *next;
    isshe_json_t *prev;
    isshe_json_t *child;
    isshe_int_t type;
    isshe_char_t *vstring;
    //isshe_int64_t vint;     // DEPRECATED
    isshe_double_t vnumber;
    isshe_char_t *kstring;  // key string, name
    //isshe_double_t  vnumber;
};

struct isshe_json_error_s {
    const isshe_uchar_t *json;
    isshe_size_t position;
};

struct isshe_json_parse_buffer_s {
    const isshe_uchar_t *content;
    isshe_size_t length;
    isshe_size_t offset;
    isshe_size_t depth;
    isshe_mempool_t *mempool;
};

/* Limits how deeply nested arrays/objects can be before isshe_json_t rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef ISSHE_JSON_NESTING_LIMIT
#define ISSHE_JSON_NESTING_LIMIT 1000
#endif

/* Precision of double variables comparison */
#ifndef ISSHE_JSON_DOUBLE_PRECISION
#define ISSHE_JSON_DOUBLE_PRECISION .0000000000000001
#endif

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

isshe_uchar_t isshe_json_decimal_point_get(isshe_void_t);
isshe_bool_t isshe_json_is_equal_double(isshe_double_t a, isshe_double_t b);

// These functions check the type of an item
isshe_bool_t isshe_json_is_invalid(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_false(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_true(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_bool(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_null(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_number(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_string(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_array(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_object(const isshe_json_t * const item);
isshe_bool_t isshe_json_is_raw(const isshe_json_t * const item);

const isshe_char_t *isshe_json_get_error_ptr(isshe_void_t);

isshe_char_t *isshe_json_get_string_value(const isshe_json_t * const item);
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

isshe_void_t isshe_json_delete(
    isshe_json_t *item,
    isshe_mempool_t *mempool);

isshe_json_t *isshe_json_file_parse(
    const isshe_char_t *filename,
    isshe_mempool_t *mempool);

isshe_bool_t isshe_json_has_object(
    const isshe_json_t *object, const char *string);

isshe_json_t *isshe_json_object_get(
    const isshe_json_t * const object,
    const isshe_char_t * const string);

isshe_json_t *isshe_json_object_get_case_insensitive(
    const isshe_json_t * const object, 
    const isshe_char_t * const string);

isshe_size_t isshe_json_array_size(const isshe_json_t *array);

isshe_json_t *isshe_json_array_item_get(
    const isshe_json_t *array, isshe_size_t index);


// create about



#endif






