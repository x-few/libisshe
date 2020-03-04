
/*
 * base on cJSON v1.7.12
 * https://github.com/DaveGamble/cJSON
 */

#include "isshe_common.h"

#ifdef ENABLE_LOCALES
#include <locale.h>
#endif

static isshe_json_error_t global_error = { NULL, 0 };

// static functions
static isshe_json_t *isshe_json_new_item(isshe_mempool_t *mempool);
static isshe_json_parse_buffer_t *skip_utf8_bom(
    isshe_json_parse_buffer_t * const buffer);
static isshe_json_parse_buffer_t *buffer_skip_whitespace(
    isshe_json_parse_buffer_t * const buffer);
static isshe_uint_t parse_hex4(const isshe_uchar_t * const input);
static isshe_uchar_t utf16_literal_to_utf8(
    const isshe_uchar_t * const input_pointer,
    const isshe_uchar_t * const input_end,
    isshe_uchar_t **output_pointer);
static isshe_bool_t isshe_json_parse_string(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);
static isshe_bool_t isshe_json_parse_number(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);
static isshe_bool_t isshe_json_parse_array(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);
static isshe_bool_t isshe_json_parse_object(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);
static isshe_bool_t isshe_json_parse_value(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);

static isshe_json_t *
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


// skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer
static isshe_json_parse_buffer_t *
skip_utf8_bom(isshe_json_parse_buffer_t * const buffer)
{
    if ((buffer == NULL)
    || (buffer->content == NULL)
    || (buffer->offset != 0)){
        return NULL;
    }

    if (parse_buffer_can_access(buffer, 4)
    && (strncmp((const isshe_char_t *)parse_buffer_at_offset(buffer),
    "\xEF\xBB\xBF", 3) == 0)) {
        buffer->offset += 3;
    }

    return buffer;
}

// Utility to jump whitespace and cr/lf
static isshe_json_parse_buffer_t *
buffer_skip_whitespace(isshe_json_parse_buffer_t * const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
    {
        return NULL;
    }

    while (parse_buffer_can_access(buffer, 0)
    && (parse_buffer_at_offset(buffer)[0] <= 32))
    {
        buffer->offset++;
    }

    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    return buffer;
}

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
static isshe_uchar_t 
utf16_literal_to_utf8(
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

// Parse the input text into an unescaped cinput, and populate item.
static isshe_bool_t
isshe_json_parse_string(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer)
{
    const isshe_uchar_t *input_pointer = parse_buffer_at_offset(input_buffer) + 1;
    const isshe_uchar_t *input_end = parse_buffer_at_offset(input_buffer) + 1;
    isshe_uchar_t       *output_pointer = NULL;
    isshe_uchar_t       *output = NULL;
    isshe_size_t        alloc_len = 0;
    isshe_size_t        skipped_bytes = 0;
    isshe_uchar_t       sequence_length = 0;

    // not a string
    if (parse_buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto parse_string_fail;
    }

    // calculate approximate size of the output (overestimate)
    while (((isshe_size_t)(input_end - input_buffer->content) < input_buffer->length) 
    && (*input_end != '\"'))
    {
        // is escape sequence
        if (input_end[0] == '\\')
        {
            if ((isshe_size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
            {
                // prevent buffer overflow when last input character is a backslash
                goto parse_string_fail;
            }
            skipped_bytes++;
            input_end++;
        }
        input_end++;
    }

    if (((size_t)(input_end - input_buffer->content) >= input_buffer->length)
    || (*input_end != '\"'))
    {
        goto parse_string_fail; // string ended unexpectedly
    }

    // This is at most how much we need for the output
    alloc_len = (size_t)(input_end - parse_buffer_at_offset(input_buffer))
        - skipped_bytes  + sizeof("");
    output = (isshe_uchar_t*)isshe_mpalloc(
        input_buffer->mempool, alloc_len);
    if (output == NULL)
    {
        goto parse_string_fail; // allocation failure
    }

    output_pointer = output;
    // loop through the string literal
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        // escape sequence
        else
        {
            sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto parse_string_fail;
            }

            switch (input_pointer[1])
            {
                case 'b':
                    *output_pointer++ = '\b';
                    break;
                case 'f':
                    *output_pointer++ = '\f';
                    break;
                case 'n':
                    *output_pointer++ = '\n';
                    break;
                case 'r':
                    *output_pointer++ = '\r';
                    break;
                case 't':
                    *output_pointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *output_pointer++ = input_pointer[1];
                    break;

                // UTF-16 literal
                case 'u':
                    sequence_length = utf16_literal_to_utf8(
                        input_pointer, input_end, &output_pointer);
                    if (sequence_length == 0)
                    {
                        // failed to convert UTF16-literal to UTF-8
                        goto parse_string_fail;
                    }
                    break;

                default:
                    goto parse_string_fail;
            }
            input_pointer += sequence_length;
        }
    }

    // zero terminate the output
    *output_pointer = '\0';

    item->type = ISSHE_JSON_STRING;
    item->vstring = (char*)output;

    input_buffer->offset = (size_t) (input_end - input_buffer->content);
    input_buffer->offset++;

    return ISSHE_TRUE;

parse_string_fail:
    if (output != NULL)
    {
        isshe_mpfree(input_buffer->mempool, output, alloc_len);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return ISSHE_FALSE;
}


// Parse the input text to generate a number, and populate the result into item.
static isshe_bool_t
isshe_json_parse_number(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer)
{
    isshe_double_t number = 0;
    isshe_uchar_t *after_end = NULL;
    isshe_uchar_t number_c_string[64];
    isshe_uchar_t decimal_point = isshe_json_decimal_point_get();
    isshe_size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return ISSHE_FALSE;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input
     */
    for (i = 0; (i < (sizeof(number_c_string) - 1))
    && parse_buffer_can_access(input_buffer, i); i++)
    {
        switch (parse_buffer_at_offset(input_buffer)[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
                number_c_string[i] = parse_buffer_at_offset(input_buffer)[i];
                break;

            case '.':
                number_c_string[i] = decimal_point;
                break;

            default:
                goto parse_number_loop_end;
        }
    }
parse_number_loop_end:
    number_c_string[i] = '\0';

    number = strtod((const char*)number_c_string, (char**)&after_end);
    if (number_c_string == after_end)
    {
        return ISSHE_FALSE; // parse_error
    }

    item->vdouble = number;

    /*
    // use saturation in case of overflow
    if (number >= ISSHE_LLONG_MAX)
    {
        item->vint = ISSHE_LLONG_MAX;
    }
    else if (number <= (isshe_double_t)ISSHE_LLONG_MIN)
    {
        item->vint = ISSHE_LLONG_MIN;
    }
    else
    {
        item->vint = (isshe_int64_t)number;
    }
    */

    item->type = ISSHE_JSON_NUMBER;

    input_buffer->offset += (size_t)(after_end - number_c_string);
    return ISSHE_TRUE;
}

// Build an array from input text.
static isshe_bool_t
isshe_json_parse_array(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer)
{
    isshe_json_t *head = NULL; // head of the linked list
    isshe_json_t *current = NULL;

    if (input_buffer->depth >= ISSHE_JSON_NESTING_LIMIT)
    {
        return ISSHE_FALSE; // to deeply nested
    }
    input_buffer->depth++;

    if (parse_buffer_at_offset(input_buffer)[0] != '[')
    {
        // not an array
        goto parse_array_fail;
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (parse_buffer_can_access(input_buffer, 0)
    && (parse_buffer_at_offset(input_buffer)[0] == ']'))
    {
        // empty array
        goto parse_array_success;
    }

    // check if we skipped to the end of the buffer
    if (parse_buffer_cannot_access(input_buffer, 0))
    {
        input_buffer->offset--;
        goto parse_array_fail;
    }

    // step back to character in front of the first element
    input_buffer->offset--;
    // loop through the comma separated array elements
    do
    {
        // allocate next item
        isshe_json_t *new_item = isshe_json_new_item(input_buffer->mempool);
        if (new_item == NULL)
        {
            goto parse_array_fail; // allocation failure
        }

        // attach next item to list
        if (head == NULL)
        {
            // start the linked list
            current = head = new_item;
        }
        else
        {
            // add to the end and advance
            current->next = new_item;
            new_item->prev = current;
            current = new_item;
        }

        // parse next value
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!isshe_json_parse_value(current, input_buffer))
        {
            goto parse_array_fail; // failed to parse value
        }
        buffer_skip_whitespace(input_buffer);
    } while (parse_buffer_can_access(input_buffer, 0)
            && (parse_buffer_at_offset(input_buffer)[0] == ','));

    if (parse_buffer_cannot_access(input_buffer, 0)
    || parse_buffer_at_offset(input_buffer)[0] != ']')
    {
        goto parse_array_fail; // expected end of array
    }

parse_array_success:
    input_buffer->depth--;

    item->type = ISSHE_JSON_ARRAY;
    item->child = head;

    input_buffer->offset++;

    return ISSHE_TRUE;

parse_array_fail:
    if (head != NULL)
    {
        isshe_json_delete(head, input_buffer->mempool);
    }

    return ISSHE_FALSE;
}

// Build an object from the text.
static isshe_bool_t
isshe_json_parse_object(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer)
{
    isshe_json_t *head = NULL; // linked list head
    isshe_json_t *current = NULL;

    if (input_buffer->depth >= ISSHE_JSON_NESTING_LIMIT)
    {
        return ISSHE_FALSE; // to deeply nested
    }
    input_buffer->depth++;

    if (parse_buffer_cannot_access(input_buffer, 0)
    || (parse_buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto parse_object_fail; // not an object
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (parse_buffer_can_access(input_buffer, 0)
    && (parse_buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto parse_object_success; // empty object
    }

    // check if we skipped to the end of the buffer
    if (parse_buffer_cannot_access(input_buffer, 0))
    {
        input_buffer->offset--;
        goto parse_object_fail;
    }

    // step back to character in front of the first element
    input_buffer->offset--;
    // loop through the comma separated array elements
    do {
        // allocate next item
        isshe_json_t *new_item = isshe_json_new_item(input_buffer->mempool);
        if (new_item == NULL)
        {
            goto parse_object_fail; // allocation failure
        }

        // attach next item to list
        if (head == NULL)
        {
            // start the linked list
            current = head = new_item;
        }
        else
        {
            // add to the end and advance
            current->next = new_item;
            new_item->prev = current;
            current = new_item;
        }

        // parse the name of the child
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!isshe_json_parse_string(current, input_buffer))
        {
            goto parse_object_fail; // failed to parse name
        }
        buffer_skip_whitespace(input_buffer);

        // swap vstring and string, because we parsed the name
        current->kstring = current->vstring;
        current->vstring = NULL;

        if (parse_buffer_cannot_access(input_buffer, 0)
        || (parse_buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto parse_object_fail; // invalid object
        }

        // parse the value
        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!isshe_json_parse_value(current, input_buffer))
        {
            goto parse_object_fail; // failed to parse value
        }
        buffer_skip_whitespace(input_buffer);
    } while (parse_buffer_can_access(input_buffer, 0)
            && (parse_buffer_at_offset(input_buffer)[0] == ','));

    if (parse_buffer_cannot_access(input_buffer, 0)
    || (parse_buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto parse_object_fail; // expected end of object
    }

parse_object_success:
    input_buffer->depth--;

    item->type = ISSHE_JSON_OBJECT;
    item->child = head;

    input_buffer->offset++;
    return ISSHE_TRUE;

parse_object_fail:
    if (head != NULL)
    {
        isshe_json_delete(head, input_buffer->mempool);
    }

    return ISSHE_FALSE;
}


// Parser core - when encountering text, process appropriately.
static isshe_bool_t
isshe_json_parse_value(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return ISSHE_FALSE;
    }

    // parse the different types of values
    // null
    if (parse_buffer_can_read(input_buffer, 4)
    && (strncmp((const char*)parse_buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = ISSHE_JSON_NULL;
        input_buffer->offset += 4;
        return ISSHE_TRUE;
    }
    // false
    if (parse_buffer_can_read(input_buffer, 5)
    && (strncmp((const char*)parse_buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = ISSHE_JSON_FALSE;
        input_buffer->offset += 5;
        return ISSHE_TRUE;
    }
    // true
    if (parse_buffer_can_read(input_buffer, 4)
    && (strncmp((const char*)parse_buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = ISSHE_JSON_TRUE;
        //item->vint = 1;
        input_buffer->offset += 4;
        return ISSHE_TRUE;
    }
    // string
    if (parse_buffer_can_access(input_buffer, 0)
    && (parse_buffer_at_offset(input_buffer)[0] == '\"'))
    {
        return isshe_json_parse_string(item, input_buffer);
    }
    // number
    if (parse_buffer_can_access(input_buffer, 0)
    && ((parse_buffer_at_offset(input_buffer)[0] == '-')
        || ((parse_buffer_at_offset(input_buffer)[0] >= '0')
        && (parse_buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return isshe_json_parse_number(item, input_buffer);
    }
    // array
    if (parse_buffer_can_access(input_buffer, 0)
    && (parse_buffer_at_offset(input_buffer)[0] == '['))
    {
        return isshe_json_parse_array(item, input_buffer);
    }
    // object
    if (parse_buffer_can_access(input_buffer, 0)
    && (parse_buffer_at_offset(input_buffer)[0] == '{'))
    {
        return isshe_json_parse_object(item, input_buffer);
    }

    return ISSHE_FALSE;
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

// Parse an object - create a new root, and populate.
isshe_json_t *
isshe_json_parse_with_opts(
    const isshe_char_t *value, const isshe_char_t **return_parse_end,
    isshe_bool_t require_null_terminated, isshe_mempool_t *mempool)
{
    isshe_json_parse_buffer_t   buffer;
    isshe_json_t                *item = NULL;
    isshe_json_error_t          local_error;

    if (value == NULL)
    {
        goto json_parse_fail;
    }

    // reset error position
    global_error.json = NULL;
    global_error.position = 0;

    isshe_memzero(&buffer, sizeof(isshe_json_parse_buffer_t));
    buffer.content = (const isshe_uchar_t*)value;
    buffer.length = isshe_strlen((const char*)value) + sizeof("");
    buffer.offset = 0;
    buffer.mempool = mempool;

    item = isshe_json_new_item(mempool);
    if (item == NULL)
    {
        goto json_parse_fail;
    }

    if (!isshe_json_parse_value(item,
    buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        // parse failure. ep is set.
        goto json_parse_fail;
    }

    // if we require null-terminated JSON without appended garbage, skip and then check for a null terminator
    if (require_null_terminated)
    {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length)
        || parse_buffer_at_offset(&buffer)[0] != '\0')
        {
            goto json_parse_fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char*)parse_buffer_at_offset(&buffer);
    }

    return item;

json_parse_fail:
    //printf("failed!!!\n");
    if (item != NULL)
    {
        isshe_json_delete(item, mempool);
    }

    if (value != NULL)
    {
        local_error.json = (const isshe_uchar_t*)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char*)local_error.json + local_error.position;
        }

        global_error = local_error;
    }

    return NULL;
}


const isshe_char_t *
isshe_json_get_error_ptr(isshe_void_t)
{
    return (const isshe_char_t *) (global_error.json + global_error.position);
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


// Default options for isshe_json_parse
isshe_json_t *
isshe_json_parse(const isshe_char_t *value, isshe_mempool_t *mempool)
{
    return isshe_json_parse_with_opts(value, 0, 0, mempool);
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


