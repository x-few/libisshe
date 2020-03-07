
/*
 * base on cJSON v1.7.12
 * https://github.com/DaveGamble/cJSON
 */

#include "isshe_common.h"

#ifdef ENABLE_LOCALES
#include <locale.h>
#endif

// static functions
static isshe_uint_t parse_hex4(const isshe_uchar_t * const input);


// parse 4 digit hexadecimal number
static isshe_uint_t
parse_hex4(const isshe_uchar_t * const input)
{
    isshe_uint_t h = 0;
    isshe_size_t i = 0;

    for (i = 0; i < 4; i++)
    {
        // parse digit
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int) input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int) 10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int) 10 + input[i] - 'a';
        }
        else // invalid
        {
            return 0;
        }

        if (i < 3)
        {
            // shift left to make place for the next nibble
            h = h << 4;
        }
    }

    return h;
}

/* converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX 
 * return: 0 - error
 */
isshe_uchar_t 
isshe_json_utf16_to_utf8(
    const isshe_uchar_t * const input_pointer,
    const isshe_uchar_t * const input_end,
    isshe_uchar_t **output_pointer)
{
    isshe_uint_t        codepoint = 0;
    isshe_uint_t        first_code = 0;
    const isshe_uchar_t *first_sequence = input_pointer;
    isshe_uchar_t       utf8_length = 0;
    isshe_uchar_t       utf8_position = 0;
    isshe_uchar_t       sequence_length = 0;
    isshe_uchar_t       first_byte_mark = 0;

    if ((input_end - first_sequence) < 6)
    {
        // input ends unexpectedly
        return 0;
    }

    // get the first utf16 sequence
    first_code = parse_hex4(first_sequence + 2);

    // check that the code is valid
    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
    {
        return 0;
    }

    // UTF16 surrogate pair
    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const isshe_uchar_t *second_sequence = first_sequence + 6;
        isshe_uint_t second_code = 0;
        sequence_length = 12; // \uXXXX\uXXXX

        if ((input_end - second_sequence) < 6)
        {
            // input ends unexpectedly
            return 0;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            // missing second half of the surrogate pair
            return 0;
        }

        // get the second utf16 sequence
        second_code = parse_hex4(second_sequence + 2);
        // check that the code is valid
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            // invalid second half of the surrogate pair
            return 0;
        }


        // calculate the unicode codepoint from the surrogate pair
        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6; // \uXXXX
        codepoint = first_code;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     */
    if (codepoint < 0x80)
    {
        // normal ascii, encoding 0xxxxxxx
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        // two bytes, encoding 110xxxxx 10xxxxxx
        utf8_length = 2;
        first_byte_mark = 0xC0; // 11000000
    }
    else if (codepoint < 0x10000)
    {
        // three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx
        utf8_length = 3;
        first_byte_mark = 0xE0; // 11100000
    }
    else if (codepoint <= 0x10FFFF)
    {
        // four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx
        utf8_length = 4;
        first_byte_mark = 0xF0; // 11110000
    }
    else
    {
        // invalid unicode codepoint
        return 0;
    }

    // encode as utf8
    for (utf8_position = (isshe_uchar_t)(utf8_length - 1); utf8_position > 0; utf8_position--)
    {
        // 10xxxxxx
        (*output_pointer)[utf8_position] = (isshe_uchar_t)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    // encode first byte
    if (utf8_length > 1)
    {
        (*output_pointer)[0] = (isshe_uchar_t)((codepoint | first_byte_mark) & 0xFF);
    }
    else
    {
        (*output_pointer)[0] = (isshe_uchar_t)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    return sequence_length;
}

isshe_json_t *
isshe_json_new_item(isshe_mempool_t *mempool)
{
    isshe_json_t* node;
    
    node = (isshe_json_t*)isshe_mpalloc(mempool, sizeof(isshe_json_t));
    if (node)
    {
        isshe_memzero(node, sizeof(isshe_json_t));
    }

    return node;
}


static isshe_json_t *
isshe_json_object_item_get(const isshe_json_t * const object,
    const isshe_char_t * const name,
    const isshe_bool_t case_sensitive)
{
    isshe_json_t *current = NULL;

    if ((object == NULL) || (name == NULL))
    {
        return NULL;
    }

    current = object->child;
    if (case_sensitive)
    {
        while ((current != NULL)
        && (current->kstring != NULL)
        && (isshe_strcmp(name, current->kstring) != 0))
        {
            current = current->next;
        }
    }
    else
    {
        while ((current != NULL)
        && (isshe_strcmp_case_insensitive(name, current->kstring) != 0))
        {
            current = current->next;
        }
    }

    if ((current == NULL)
    || (current->kstring == NULL)) {
        return NULL;
    }

    return current;
}

// get the decimal point character of the current locale
isshe_uchar_t
isshe_json_decimal_point_get(isshe_void_t)
{
#ifdef ENABLE_LOCALES
    struct lconv *lconv = localeconv();
    return (isshe_uchar_t) lconv->decimal_point[0];
#else
    return '.';
#endif
}

isshe_bool_t
isshe_json_is_equal_double(isshe_double_t a, isshe_double_t b)
{
    return (fabs(a - b) <= ISSHE_JSON_DOUBLE_PRECISION);
}


isshe_bool_t isshe_json_is_invalid(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xFF) == ISSHE_JSON_INVALID;
}

isshe_bool_t isshe_json_is_false(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xFF) == ISSHE_JSON_FALSE;
}

isshe_bool_t isshe_json_is_true(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xff) == ISSHE_JSON_TRUE;
}


isshe_bool_t isshe_json_is_bool(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & (ISSHE_JSON_TRUE | ISSHE_JSON_FALSE)) != 0;
}
isshe_bool_t isshe_json_is_null(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xFF) == ISSHE_JSON_NULL;
}

isshe_bool_t isshe_json_is_number(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xFF) == ISSHE_JSON_NUMBER;
}

isshe_bool_t isshe_json_is_string(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xFF) == ISSHE_JSON_STRING;
}

isshe_bool_t isshe_json_is_array(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xFF) == ISSHE_JSON_ARRAY;
}

isshe_bool_t isshe_json_is_object(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xFF) == ISSHE_JSON_OBJECT;
}

isshe_bool_t isshe_json_is_raw(const isshe_json_t * const item)
{
    if (item == NULL)
    {
        return ISSHE_FALSE;
    }

    return (item->type & 0xFF) == ISSHE_JSON_RAW;
}


isshe_char_t *
isshe_json_get_string_value(const isshe_json_t * const item)
{
    if (!isshe_json_is_string(item)) {
        return NULL;
    }

    return item->vstring;
}


// Delete a isshe_json_t structure.
isshe_void_t
isshe_json_delete(isshe_json_t *item, isshe_mempool_t *mempool)
{
    isshe_json_t *next = NULL;
    while (item != NULL)
    {
        next = item->next;
        if (item->child != NULL)
        {
            isshe_json_delete(item->child, mempool);
        }
        if (item->vstring != NULL)
        {
            isshe_mpfree(mempool, item->vstring,
                isshe_strlen(item->vstring) + sizeof(""));
        }
        if (item->kstring != NULL)
        {
            isshe_mpfree(mempool, item->kstring,
                isshe_strlen(item->kstring) + sizeof(""));
        }

        isshe_mpfree(mempool, item, sizeof(isshe_json_t));

        item = next;
    }
}

isshe_json_t *
isshe_json_file_parse(const isshe_char_t *filename, isshe_mempool_t *mempool)
{
    isshe_fd_t          fd;
    ssize_t             len;
    char                *buf;

    // 打开文件
    fd = isshe_open(filename, ISSHE_FILE_RDONLY);

    // 读取文件
    buf = isshe_read_all(fd, &len);
    isshe_close(fd);
    if (!buf) {
        printf("read json file failed\n");
        return NULL;
    }

    // 解析json
    isshe_json_t* json = isshe_json_parse(buf, mempool);
    isshe_free(buf);
    if (!json) {
        printf("json parse failed\n");
        return NULL;
    }

    return json;
}

isshe_json_t *
isshe_json_object_get(
    const isshe_json_t * const object,
    const isshe_char_t * const string)
{
    return isshe_json_object_item_get(object, string, ISSHE_TRUE);
}

isshe_bool_t
isshe_json_has_object(const isshe_json_t *object, const isshe_char_t *string)
{
    return isshe_json_object_get(object, string) ? ISSHE_TRUE : ISSHE_FALSE;
}

isshe_json_t *
isshe_json_object_get_case_insensitive(
    const isshe_json_t * const object, 
    const isshe_char_t * const string)
{
    return isshe_json_object_item_get(object, string, ISSHE_FALSE);
}

// Get Array size/item / object item.
isshe_size_t
isshe_json_array_size(const isshe_json_t *array)
{
    isshe_json_t *child = NULL;
    isshe_size_t size = 0;

    if (array == NULL)
    {
        return 0;
    }

    child = array->child;

    while(child != NULL)
    {
        size++;
        child = child->next;
    }

    return size;
}

isshe_json_t *
isshe_json_array_item_get(const isshe_json_t *array, isshe_size_t index)
{
    isshe_json_t *current_child = NULL;

    if (array == NULL)
    {
        return NULL;
    }

    current_child = array->child;
    while ((current_child != NULL) && (index > 0))
    {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}


