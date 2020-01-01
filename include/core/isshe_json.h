/*
 * base on cJSON v1.7.12
 * https://github.com/DaveGamble/cJSON
 */

#ifndef _ISSHE_JSON_H_
#define _ISSHE_JSON_H_

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

/* project version */
#define ISSHE_JSON_VERSION_MAJOR 1
#define ISSHE_JSON_VERSION_MINOR 0
#define ISSHE_JSON_VERSION_PATCH 0

#include "isshe_common.h"

/* isshe_json_t Types: */
#define ISSHE_JSON_INVALID (0)
#define ISSHE_JSON_FALSE  (1 << 0)
#define ISSHE_JSON_TRUE   (1 << 1)
#define ISSHE_JSON_NULL   (1 << 2)
#define ISSHE_JSON_NUMBER (1 << 3)
#define ISSHE_JSON_STRING (1 << 4)
#define ISSHE_JSON_ARRAY  (1 << 5)
#define ISSHE_JSON_OBJECT (1 << 6)
#define ISSHE_JSON_RAW    (1 << 7) /* raw json */

#define ISSHE_JSON_IS_REFERENCE 256
#define ISSHE_JSON_STRING_IS_CONST 512

#define isshe_json_min(a, b) ((a < b) ? a : b)

typedef struct isshe_json_s isshe_json_t;

/* The isshe_json_t structure: */
struct isshe_json_s
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    isshe_json_t *next;
    isshe_json_t *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    isshe_json_t *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==ISSHE_JSON_STRING  and type == ISSHE_JSON_RAW */
    char *vstring;
    /* writing to vint is DEPRECATED, use isshe_json_set_number_value instead */
    int vint;
    /* The item's number, if type==ISSHE_JSON_NUMBER */
    double vdouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
};

typedef struct isshe_json_hooks_s isshe_json_hooks_t;

struct isshe_json_hooks_s
{
      /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
      void *(*malloc_fn)(size_t sz);
      void (*free_fn)(void *ptr);
};

typedef int isshe_json_bool_t;

/* Limits how deeply nested arrays/objects can be before isshe_json_t rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef ISSHE_JSON_NESTING_LIMIT
#define ISSHE_JSON_NESTING_LIMIT 1000
#endif

/* Precision of double variables comparison */
#ifndef ISSHE_JSON_DOUBLE_PRECISION
#define ISSHE_JSON_DOUBLE_PRECISION .0000000000000001
#endif

/* returns the version of isshe_json as a string */
const char* isshe_json_version(void);

/* Supply malloc, realloc and free functions to isshe_json_t */
void isshe_json_init_hooks(isshe_json_hooks_t* hooks);

/* Memory Management: the caller is always responsible to free the results from all variants of isshe_json_parse (with isshe_json_delete) and isshe_json_print (with stdlib free, isshe_json_hooks_t.free_fn, or isshe_json_free as appropriate). The exception is isshe_json_print_pre_allocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a isshe_json_t object you can interrogate. */
isshe_json_t * isshe_json_parse(const char *value);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match isshe_json_get_error_ptr(). */
isshe_json_t * isshe_json_parse_with_opts(const char *value, const char **return_parse_end, isshe_json_bool_t require_null_terminated);

/* Render a isshe_json_t entity to text for transfer/storage. */
char * isshe_json_print(const isshe_json_t *item);
/* Render a isshe_json_t entity to text for transfer/storage without any formatting. */
char * isshe_json_print_unformatted(const isshe_json_t *item);
/* Render a isshe_json_t entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
char * isshe_json_print_buffered(const isshe_json_t *item, int prebuffer, isshe_json_bool_t fmt);
/* Render a isshe_json_t entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: isshe_json_t is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
isshe_json_bool_t isshe_json_print_pre_allocated(isshe_json_t *item, char *buffer, const int length, const isshe_json_bool_t format);
/* Delete a isshe_json_t entity and all subentities. */
void isshe_json_delete(isshe_json_t *item);

/* Returns the number of items in an array (or object). */
int isshe_json_get_array_size(const isshe_json_t *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
isshe_json_t * isshe_json_get_array(const isshe_json_t *array, int index);
/* Get item "string" from object. Case insensitive. */
isshe_json_t * isshe_json_get_object(const isshe_json_t * const object, const char * const string);
isshe_json_t * isshe_json_get_object_case_sensitive(const isshe_json_t * const object, const char * const string);
isshe_json_bool_t isshe_json_has_object(const isshe_json_t *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when isshe_json_parse() returns 0. 0 when isshe_json_parse() succeeds. */
const char * isshe_json_get_error_ptr(void);

/* Check if the item is a string and return its vstring */
char * isshe_json_get_string_value(const isshe_json_t * const item);

/* These functions check the type of an item */
isshe_json_bool_t isshe_json_is_invalid(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_false(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_true(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_bool(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_null(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_number(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_string(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_array(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_object(const isshe_json_t * const item);
isshe_json_bool_t isshe_json_is_raw(const isshe_json_t * const item);

/* These calls create a isshe_json_t item of the appropriate type. */
isshe_json_t * isshe_json_create_null(void);
isshe_json_t * isshe_json_create_true(void);
isshe_json_t * isshe_json_create_false(void);
isshe_json_t * isshe_json_create_bool(isshe_json_bool_t boolean);
isshe_json_t * isshe_json_create_number(double num);
isshe_json_t * isshe_json_create_string(const char *string);
/* raw json */
isshe_json_t * isshe_json_create_raw(const char *raw);
isshe_json_t * isshe_json_create_array(void);
isshe_json_t * isshe_json_create_object(void);

/* Create a string where vstring references a string so
 * it will not be freed by isshe_json_delete */
isshe_json_t * isshe_json_create_string_reference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by isshe_json_delete */
isshe_json_t * isshe_json_create_object_reference(const isshe_json_t *child);
isshe_json_t * isshe_json_create_array_reference(const isshe_json_t *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the number array, otherwise array access will be out of bounds.*/
isshe_json_t * isshe_json_create_int_array(const int *numbers, int count);
isshe_json_t * isshe_json_create_float_array(const float *numbers, int count);
isshe_json_t * isshe_json_create_double_array(const double *numbers, int count);
isshe_json_t * isshe_json_create_string_array(const char *const *strings, int count);

/* Append item to the specified array/object. */
void isshe_json_add_item_to_array(isshe_json_t *array, isshe_json_t *item);
void isshe_json_add_item_to_object(isshe_json_t *object, const char *string, isshe_json_t *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the isshe_json_t object.
 * WARNING: When this function was used, make sure to always check that (item->type & ISSHE_JSON_STRING_IS_CONST) is zero before
 * writing to `item->string` */
void isshe_json_add_to_object_cs(isshe_json_t *object, const char *string, isshe_json_t *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing isshe_json_t to a new isshe_json_t, but don't want to corrupt your existing isshe_json_t. */
void isshe_json_add_reference_to_array(isshe_json_t *array, isshe_json_t *item);
void isshe_json_add_reference_to_object(isshe_json_t *object, const char *string, isshe_json_t *item);

/* Remove/Detach items from Arrays/Objects. */
isshe_json_t * isshe_json_detach_via_pointer(isshe_json_t *parent, isshe_json_t * const item);
isshe_json_t * isshe_json_detach_from_array(isshe_json_t *array, int which);
void isshe_json_delete_from_array(isshe_json_t *array, int which);
isshe_json_t * isshe_json_detach_from_object(isshe_json_t *object, const char *string);
isshe_json_t * isshe_json_detach_from_object_case_sensitive(isshe_json_t *object, const char *string);
void isshe_json_delete_from_object(isshe_json_t *object, const char *string);
void isshe_json_delete_from_object_case_sensitive(isshe_json_t *object, const char *string);

/* Update array items. */
void isshe_json_insert_to_array(isshe_json_t *array, int which, isshe_json_t *newitem); /* Shifts pre-existing items to the right. */
isshe_json_bool_t isshe_json_replace_via_pointer(isshe_json_t * const parent, isshe_json_t * const item, isshe_json_t * replacement);
void isshe_json_replace_item_in_array(isshe_json_t *array, int which, isshe_json_t *newitem);
void isshe_json_replace_item_in_object(isshe_json_t *object,const char *string,isshe_json_t *newitem);
void isshe_json_replace_item_in_object_case_sensitive(isshe_json_t *object,const char *string,isshe_json_t *newitem);

/* Duplicate a isshe_json_t item */
isshe_json_t * isshe_json_duplicate(const isshe_json_t *item, isshe_json_bool_t recurse);
/* Duplicate will create a new, identical isshe_json_t item to the one you pass, in new memory that will
 * need to be released. With recurse!=0, it will duplicate any children connected to the item.
 * The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two isshe_json_t items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
isshe_json_bool_t isshe_json_compare(const isshe_json_t * const a, const isshe_json_t * const b, const isshe_json_bool_t case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from strings.
 * The input pointer json cannot point to a read-only address area, such as a string constant, 
 * but should point to a readable and writable adress area. */
void isshe_json_minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
isshe_json_t * isshe_json_add_null_to_object(isshe_json_t * const object, const char * const name);
isshe_json_t * isshe_json_add_true_to_object(isshe_json_t * const object, const char * const name);
isshe_json_t * isshe_json_add_false_to_object(isshe_json_t * const object, const char * const name);
isshe_json_t * isshe_json_add_bool_to_object(isshe_json_t * const object, const char * const name, const isshe_json_bool_t boolean);
isshe_json_t * isshe_json_add_number_to_object(isshe_json_t * const object, const char * const name, const double number);
isshe_json_t * isshe_json_add_string_to_object(isshe_json_t * const object, const char * const name, const char * const string);
isshe_json_t * isshe_json_add_raw_to_object(isshe_json_t * const object, const char * const name, const char * const raw);
isshe_json_t * isshe_json_add_object_to_object(isshe_json_t * const object, const char * const name);
isshe_json_t * isshe_json_add_array_to_object(isshe_json_t * const object, const char * const name);

/* When assigning an integer value, it needs to be propagated to vdouble too. */
#define isshe_json_set_int_value(object, number) ((object) ? (object)->vint = (object)->vdouble = (number) : (number))
/* helper for the isshe_json_set_number_value macro */
double isshe_json_set_number_helper(isshe_json_t *object, double number);
#define isshe_json_set_number_value(object, number) ((object != NULL) ? isshe_json_set_number_helper(object, (double)number) : (number))

/* Macro for iterating over an array or object */
#define isshe_json_array_for_each(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with isshe_json_init_hooks */
void * isshe_json_malloc(size_t size);
void isshe_json_free(void *object);

isshe_json_t *isshe_read_json(const isshe_char_t *filename);

#ifdef __cplusplus
}
#endif

#endif






