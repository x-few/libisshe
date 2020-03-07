#include "isshe_json.h"

// 静态变量
static isshe_json_error_t global_error = { NULL, 0 };


static isshe_json_parse_buffer_t *skip_utf8_bom(
    isshe_json_parse_buffer_t * const buffer);
static isshe_json_parse_buffer_t *buffer_skip_whitespace(
    isshe_json_parse_buffer_t * const buffer);
static isshe_bool_t number_parse(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);
static isshe_bool_t string_parse(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);
static isshe_bool_t array_parse(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);
static isshe_bool_t object_parse(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);
static isshe_bool_t value_parse(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const input_buffer);

const isshe_char_t *
isshe_json_parse_error_get(isshe_void_t)
{
    return (const isshe_char_t *)
        (global_error.json + global_error.position);
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


static isshe_bool_t
number_parse(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const pbuf)
{
    isshe_double_t number = 0.0;
    isshe_uchar_t *endptr;
    isshe_size_t i = 0;
    isshe_size_t pi = 0;
    isshe_bool_t loop_end;
    isshe_uchar_t number_c_string[ISSHE_JSON_NUMBER_STRING_MAX];
    isshe_uchar_t *p = pbuf->content + pbuf->offset;
    isshe_uchar_t *last = pbuf->content + pbuf->length;
    isshe_uchar_t decimal_point = isshe_json_decimal_point_get();

    if (!item || !pbuf || !pbuf->content) {
        isshe_log_error(pbuf->log, "parse number error: invalid parameters");
        return ISSHE_FALSE;
    }

    loop_end = ISSHE_FALSE;
    for (i = 0; i < ISSHE_JSON_NUMBER_STRING_MAX - 1; i++, p++) {
        if (p > last) {
            isshe_log_error(pbuf->log, "parse number error: number to large");
            return ISSHE_FALSE;
        }

        switch (*p)
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
                number_c_string[i] = *p;
                break;
            case '.':
                number_c_string[i] = decimal_point;
                break;
            default:
                loop_end = ISSHE_TRUE;
                break;
        }

        if (loop_end == ISSHE_TRUE) {
            break;
        }
    }

    number_c_string[i] = '\0';

    number = strtod((const char*)number_c_string, (char **)&endptr);
    if (number_c_string == endptr)
    {
        isshe_log_error(pbuf->log, "parse number error: to number failed");
        return ISSHE_FALSE; // parse_error
    }

    item->vnumber = number;
    item->type = ISSHE_JSON_NUMBER;
    pbuf->offset += (isshe_size_t)(endptr - number_c_string);

    return ISSHE_TRUE;
}

static isshe_size_t
string_length_get(isshe_json_parse_buffer_t * const pbuf,
    isshe_size_t *skipped_bytes)
{
    isshe_uchar_t       *p = pbuf->content + pbuf->offset;
    isshe_uchar_t       *last = pbuf->content + pbuf->length;
    isshe_size_t        skb = 0;

    if (*p != '\"') {
        isshe_log_error(pbuf->log, "*p != '\"'");
        return 0;
    }

    p++;    // 跳过'"'
    // calculate approximate size of the output (overestimate)
    while(p < last && *p != '\"') {
        // is escape sequence
        if (*p == '\\') {
            if (p + 1 >= last) {
                isshe_log_error(pbuf->log, " p + 1 >= last");
                return 0;
            }
            skb++;
            p++;
        }
        p++;
    }

    if (p >= last || (*p != '\"')) {
        isshe_log_error(pbuf->log, "string ended unexpectedly");
        return 0;
    }

    return (isshe_size_t)(p - (pbuf->content + pbuf->offset) - 1);
}


// Parse the input text into an unescaped cinput, and populate item.
static isshe_bool_t
string_parse(isshe_json_t * const item,
    isshe_json_parse_buffer_t * const pbuf)
{
    isshe_size_t        skipped_bytes = 0;
    isshe_uchar_t       sequence_length = 0;
    isshe_uchar_t       *vstr = NULL;
    isshe_size_t        vstr_len = 0;
    isshe_uchar_t       *p = NULL;
    isshe_size_t        cstr_len = 0;       // content string length
    isshe_uchar_t       *cstr = NULL;
    isshe_size_t        i = 0;


    // get output string length
    cstr_len = string_length_get(pbuf, &skipped_bytes);
    if (cstr_len == 0) {
        isshe_log_error(pbuf->log, "get string length failed");
        return ISSHE_FALSE;
    }

    // This is at most how much we need for the string
    vstr_len = cstr_len - skipped_bytes  + sizeof("");
    vstr = (isshe_uchar_t*)isshe_mpalloc(pbuf->mempool, vstr_len);
    if (!vstr)
    {
        isshe_log_error(pbuf->log, "alloc vstring failed");
        return ISSHE_FALSE;
    }

    p = vstr;
    cstr = pbuf->content + pbuf->offset + 1; // +1: skip '"'
    for (i = 0; i < cstr_len; i++) {
        if (cstr[i] != '\\') {
            *p++ = cstr[i];
            continue;
        }

        // else: escape sequence
        sequence_length = 2;
        if (cstr_len - i < 1) {
            isshe_log_error(pbuf->log, "invalid content string");
            isshe_mpfree(pbuf->mempool, vstr, vstr_len);
            return ISSHE_FALSE;
        }

        switch (cstr[i + 1])
        {
        case 'b':
            *p++ = '\b';
            break;
        case 'f':
            *p++ = '\f';
            break;
        case 'n':
            *p++ = '\n';
            break;
        case 'r':
            *p++ = '\r';
            break;
        case 't':
            *p++ = '\t';
            break;
        case '\"':
        case '\\':
        case '/':
            *p++ = cstr[i + 1];
            break;
        // UTF-16 literal：UTF-16字面量
        case 'u':
            sequence_length = isshe_json_utf16_to_utf8(
                cstr, cstr + cstr_len, &p);
            if (sequence_length == 0)
            {
                isshe_log_error(pbuf->log,
                    "failed to convert UTF16-literal to UTF-8");
                isshe_mpfree(pbuf->mempool, vstr, vstr_len);
                return ISSHE_FALSE;
            }
            break;
        default:
            break;
        }
    }
    *p++ = '\0';

    item->type = ISSHE_JSON_STRING;
    item->vstring = (isshe_char_t *)vstr;
    pbuf->offset += cstr_len + 2;   // 2: tow '"'

    return ISSHE_TRUE;
}


// Build an array from input text.
static isshe_bool_t
array_parse(isshe_json_t * const item,
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
        if (!value_parse(current, input_buffer))
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
object_parse(isshe_json_t * const item,
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
        if (string_parse(current, input_buffer) != ISSHE_TRUE)
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
        if (!value_parse(current, input_buffer))
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
value_parse(isshe_json_t * const item,
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
        return string_parse(item, input_buffer);
    }
    // number
    if (parse_buffer_can_access(input_buffer, 0)
    && ((parse_buffer_at_offset(input_buffer)[0] == '-')
        || ((parse_buffer_at_offset(input_buffer)[0] >= '0')
        && (parse_buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return number_parse(item, input_buffer);
    }
    // array
    if (parse_buffer_can_access(input_buffer, 0)
    && (parse_buffer_at_offset(input_buffer)[0] == '['))
    {
        return array_parse(item, input_buffer);
    }
    // object
    if (parse_buffer_can_access(input_buffer, 0)
    && (parse_buffer_at_offset(input_buffer)[0] == '{'))
    {
        return object_parse(item, input_buffer);
    }

    return ISSHE_FALSE;
}


// Parse an object - create a new root, and populate.
isshe_json_t *
isshe_json_parse_with_opts(
    const isshe_char_t *value,
    const isshe_char_t **return_parse_end,
    isshe_bool_t require_null_terminated,
    isshe_mempool_t *mempool)
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
    buffer.content = (isshe_uchar_t *)value;
    buffer.length = isshe_strlen((const char*)value) + sizeof("");
    buffer.offset = 0;
    buffer.mempool = mempool;
    buffer.log = NULL;          // TODO

    item = isshe_json_new_item(mempool);
    if (item == NULL)
    {
        goto json_parse_fail;
    }

    if (!value_parse(item,
    buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        // parse failure. ep is set.
        goto json_parse_fail;
    }

    // if we require null-terminated JSON without appended garbage,
    // skip and then check for a null terminator
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


// Default options for isshe_json_parse
isshe_json_t *
isshe_json_parse(const isshe_char_t *value, isshe_mempool_t *mempool)
{
    return isshe_json_parse_with_opts(value, 0, 0, mempool);
}