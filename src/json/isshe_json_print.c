#include "isshe_json.h"

// length
static isshe_size_t number_print_length(isshe_double_t num,
    isshe_json_print_buffer_t *pbuf);
static isshe_size_t string_print_length(const isshe_uchar_t *str,
    isshe_json_print_buffer_t *pbuf);
static isshe_size_t array_print_length(
    const isshe_json_t * const item,
    isshe_json_print_buffer_t * const pbuf);
static isshe_size_t object_print_length(
    const isshe_json_t * const item,
    isshe_json_print_buffer_t * const pbuf);
static isshe_size_t value_print_length(
    const isshe_json_t * const item,
    isshe_json_print_buffer_t * const pbuf);
static isshe_size_t json_print_length(
    const isshe_json_t * const item, isshe_bool_t format);

static isshe_int_t number_print(isshe_double_t num,
    isshe_json_print_buffer_t *pbuf);
static isshe_int_t string_print(const isshe_uchar_t *str,
    isshe_json_print_buffer_t *pbuf);
static isshe_int_t array_print(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf);
static isshe_int_t object_print(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf);
static isshe_int_t value_print(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf);

static isshe_size_t
number_print_length(isshe_double_t num,
    isshe_json_print_buffer_t *pbuf)
{
    isshe_int_t len = 0;
    isshe_char_t number_buffer[ISSHE_JSON_NUMBER_LEN_MAX] = {0};
    isshe_double_t test = 0.0;

    if (!isshe_json_is_equal_double(num * 0, 0)) {
        return 4;   // "null"
    }

    len = sprintf(number_buffer, "%1.15g", num);

    if ((sscanf(number_buffer, "%lg", &test) != 1)
    || !isshe_json_is_equal_double(test, num)) {
        len = sprintf(number_buffer, "%1.17g", num);
    }

    if ((len < 0) || (len > (isshe_int_t)(sizeof(number_buffer) - 1)))
    {
        len = 4;   // "null"
    }

    return (isshe_size_t)len;
}

static isshe_size_t
string_print_length(const isshe_uchar_t *str,
    isshe_json_print_buffer_t *pbuf)
{
    isshe_size_t len = 0;
    isshe_size_t i = 0;
    isshe_size_t escape_characters = 0;

    if (!str) {
        return 2;   // "\"\""
    }

    for (i = 0; str[i]; i++) {
        switch (str[i])
        {
            case '\"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                // one character escape sequence
                escape_characters++;
                break;
            default:
                if (str[i] < 32)
                {
                    // UTF-16 escape sequence uXXXX
                    escape_characters += 5;
                }
                break;
        }
    }

    len = i + escape_characters + strlen("\"\"");  // "\"\""

    return len;
}

static isshe_size_t
array_print_length(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf)
{
    isshe_size_t len = 0;
    isshe_json_t *current = item->child;
    isshe_size_t depth = 0;

    pbuf->depth++;
    depth = pbuf->depth;

    len += 1;   // '['
    while(current) {
        len += value_print_length(current, pbuf);

        if (current->next) {
            len += pbuf->format ? 2 : 1;  // ", "、","
        }

        current = current->next;
    }

    len += 1;   // ']'
    pbuf->depth--;

    return len;
}

static isshe_size_t
object_print_length(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf)
{
    isshe_size_t len = 0;
    isshe_json_t *current = item->child;
    isshe_bool_t format = pbuf->format;
    isshe_size_t depth = 0;
    isshe_size_t rc = 0;

    pbuf->depth++;
    depth = pbuf->depth;

    len += format ? 2 : 1;    // "{\n"、"{"
    while (current) {
        if (format) {
            len += depth;
        }
        rc = string_print_length(
            (isshe_uchar_t *)current->kstring, pbuf);   // for key
        if (rc == 0) {
            return 0;   // error
        }
        len += rc;
        len += format ? 2 : 1;  // ":\t"、":"
        rc = value_print_length(current, pbuf);
        if (rc == 0) {
            return 0;
        }
        len += rc;
        len += format ? 1 : 0;  // '\n'
        len += current->next ? 1 : 0;  // ','

        current = current->next;
    }

    len += format ? depth : 1;

    pbuf->depth--;

    return len;
}

static isshe_size_t
value_print_length(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf)
{
    isshe_size_t len = 0;
    isshe_size_t tmp_len = 0;

    if (!item) {
        return 0;
    }

    switch ((item->type) & 0xFF)
    {
    case ISSHE_JSON_NULL:
        len += 4;           // "null"
        break;
    case ISSHE_JSON_FALSE:
        len += 5;           // "false"
        break;
    case ISSHE_JSON_TRUE:
        len += 4;           // "true"
        break;
    case ISSHE_JSON_NUMBER:
        len += number_print_length(item->vnumber, pbuf);
        break;
    case ISSHE_JSON_STRING:
        len += string_print_length((isshe_uchar_t *)item->vstring, pbuf);
        break;
    case ISSHE_JSON_ARRAY:
        len += array_print_length(item, pbuf);
        break;
    case ISSHE_JSON_OBJECT:
        len += object_print_length(item, pbuf);
        break;
    case ISSHE_JSON_RAW:
        len += isshe_strlen(item->vstring);
        break;
    default:
        break;
    }

    return len;
}

static isshe_size_t
json_print_length(
    const isshe_json_t * const item,
    isshe_bool_t format)
{
    isshe_json_print_buffer_t pbuf[1];
    isshe_size_t len = 0;

    isshe_memzero(pbuf, sizeof(pbuf));

    pbuf->format = format;

    len = value_print_length(item, pbuf) + 1;

    return len;
}

static isshe_int_t
number_print(isshe_double_t num,
    isshe_json_print_buffer_t *pbuf)
{
    //isshe_char_t offset = pbuf->offset;
    isshe_uchar_t *p = pbuf->buffer + pbuf->offset;
    isshe_int_t len = 0;
    isshe_char_t nbuf[ISSHE_JSON_NUMBER_LEN_MAX] = {0};
    isshe_double_t test = 0.0;
    isshe_int_t i = 0;
    isshe_uchar_t decimal_point = isshe_json_decimal_point_get();

    if (!isshe_json_is_equal_double(num * 0, 0)) {
        len = sprintf(nbuf, "null");
    } else {
        len = sprintf(nbuf, "%1.15g", num);

        if ((sscanf(nbuf, "%lg", &test) != 1)
        || !isshe_json_is_equal_double(test, num)) {
            len = sprintf(nbuf, "%1.17g", num);
        }
    }

    if ((len < 0) || (len > (isshe_int_t)(sizeof(nbuf) - 1)))
    {
        printf("---isshe---: error---\n");
        return ISSHE_ERROR;
    }

    for (i = 0; i < len; i++)
    {
        if (nbuf[i] == decimal_point)
        {
            p[i] = '.';
            continue;
        }

        p[i] = nbuf[i];
    }

    pbuf->offset += len;

    return ISSHE_OK;
}

static isshe_int_t
string_print(const isshe_uchar_t *str,
    isshe_json_print_buffer_t *pbuf)
{
    isshe_uchar_t *p = pbuf->buffer + pbuf->offset;
    isshe_size_t len = 0;
    isshe_size_t i = 0;
    isshe_size_t escape_characters = 0;
    isshe_size_t pi = 0;

    if (!str) {
        len = strlen("\"\"");
        isshe_memcpy(p, "\"\"", len);
        pbuf->offset += len;
        return ISSHE_OK;
    }

    for (i = 0; str[i]; i++) {
        switch (str[i])
        {
            case '\"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                // one character escape sequence
                escape_characters++;
                break;
            default:
                if (str[i] < 32)
                {
                    // UTF-16 escape sequence uXXXX
                    escape_characters += 5;
                }
                break;
        }
    }
    len = i + escape_characters + strlen("\"\"");

    pi = 0;
    p[pi++] = '\"';
    if (escape_characters == 0) {
        isshe_memcpy(p + 1, str, len - strlen("\"\""));
    } else {
        for (i = 0; str[i]; i++) {
            if (str[i] > 31 && str[i] != '\"' && str[i] != '\\') {
                // 普通字符，直接复制
                p[pi++] = str[i];
                continue;
            }
            // else
            p[pi++] = '\\';
            switch (str[i])
            {
                case '\"':
                    p[pi++] = '\"';
                    break;
                case '\\':
                    p[pi++] = '\\';
                    break;
                case '\b':
                    p[pi++] = 'b';
                    break;
                case '\f':
                    p[pi++] = 'f';
                    break;
                case '\n':
                    p[pi++] = 'n';
                    break;
                case '\r':
                    p[pi++] = 'r';
                    break;
                case '\t':
                    p[pi++] = 't';
                    break;
                default:
                    // str[i] < 32
                    // UTF-16 escape sequence uXXXX
                    // pi += sprintf(p + pi, "u%04x", str + i);
                    printf("---isshe---: u%04x\n", (isshe_uchar_t)str[i]);
                    sprintf((isshe_char_t *)(p + pi), "u%04x", *(str + i));
                    pi += 5;
                    break;
            }
        }
    }
    p[len - 1] = '\"';

    pbuf->offset += len;

    return ISSHE_OK;
}

static isshe_int_t
array_print(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf)
{
    isshe_uchar_t *p = NULL;
    isshe_size_t pi = 0;
    isshe_size_t len = 0;
    isshe_json_t *current = NULL;

    if (!item || !pbuf) {
        return ISSHE_ERROR;
    }

    pbuf->depth++;
    p = pbuf->buffer + pbuf->offset;
    current = item->child;

    p[pi++] = '[';
    pbuf->offset++;

    while(current) {
        if (value_print(current, pbuf) != ISSHE_OK) {
            return ISSHE_ERROR;
        }
        // update offset
        p = pbuf->buffer + pbuf->offset;
        pi = 0;

        if (current->next) {
            p[pi++] = ',';
            pbuf->offset++;
            if (pbuf->format) {
                p[pi++] = ' ';
                pbuf->offset++;
            }
        }
        current = current->next;
    }

    p[pi++] = ']';
    pbuf->offset++;
    pbuf->depth--;

    return ISSHE_OK;
}

static isshe_int_t
object_print(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf)
{
    isshe_uchar_t *p = NULL;
    isshe_size_t pi = 0;
    isshe_size_t i = 0;
    isshe_json_t *current = NULL;

    if (!item || !pbuf) {
        return ISSHE_ERROR;
    }

    pbuf->depth++;
    p = pbuf->buffer + pbuf->offset;
    current = item->child;

    p[pi++] = '{';
    pbuf->offset++;
    if (pbuf->format) {
        p[pi++] = '\n';
        pbuf->offset++;
    }

    while(current) {
        if (pbuf->format) {
            //len = pbuf->depth;
            for (i = 0; i < pbuf->depth; i++) {
                p[pi++] = '\t';
            }
            pbuf->offset += pbuf->depth;
        }

        // print key
        if (string_print((isshe_uchar_t *)current->kstring, pbuf) != ISSHE_OK) {
            return ISSHE_ERROR;
        }
        // update offset
        p = pbuf->buffer + pbuf->offset;
        pi = 0;

        p[pi++] = ':';
        pbuf->offset++;
        if (pbuf->format) {
            p[pi++] = '\t';
            pbuf->offset++;
        }

        if (value_print(current, pbuf) != ISSHE_OK) {
            return ISSHE_ERROR;
        }
        // update offset
        p = pbuf->buffer + pbuf->offset;
        pi = 0;

        if (current->next) {
            p[pi++] = ',';
            pbuf->offset++;
        }

        if (pbuf->format) {
            p[pi++] = '\n';
            pbuf->offset++;
        }

        current = current->next;
    }

    if (pbuf->format) {
        for (i = 0; i < (pbuf->depth - 1); i++) {
            p[pi++] = '\t';
        }
        pbuf->offset += pbuf->depth - 1;
    }

    p[pi++] = '}';
    pbuf->offset++;
    pbuf->depth--;

    return ISSHE_OK;
}

static isshe_int_t
value_print(const isshe_json_t * const item,
    isshe_json_print_buffer_t *pbuf)
{
    //isshe_char_t *buffer = pbuf->buffer;
    //isshe_char_t offset = pbuf->offset;
    isshe_uchar_t *p = pbuf->buffer + pbuf->offset;
    isshe_size_t pi = 0;
    isshe_size_t len = 0;

    if (!item) {
        return ISSHE_ERROR;
    }

    switch ((item->type) & 0xFF)
    {
    case ISSHE_JSON_NULL:
        isshe_memcpy(p, "null", 4);
        pbuf->offset += 4;
        break;
    case ISSHE_JSON_FALSE:
        isshe_memcpy(p, "false", 5);
        pbuf->offset += 5;
        break;
    case ISSHE_JSON_TRUE:
        isshe_memcpy(p, "true", 4);
        pbuf->offset += 4;
        break;
    case ISSHE_JSON_NUMBER:
        number_print(item->vnumber, pbuf);
        break;
    case ISSHE_JSON_STRING:
        string_print((isshe_uchar_t *)item->vstring, pbuf);
        break;
    case ISSHE_JSON_ARRAY:
        array_print(item, pbuf);
        break;
    case ISSHE_JSON_OBJECT:
        object_print(item, pbuf);
        break;
    case ISSHE_JSON_RAW:
        len = isshe_strlen(item->vstring);
        isshe_memcpy(p, item->vstring, len);
        pbuf->offset += len;
        break;
    default:
        break;
    }

    return ISSHE_OK;
}

static isshe_int_t
json_print(const isshe_json_t * const item,
    isshe_bool_t format, isshe_log_t *log)
{
    isshe_json_print_buffer_t pbuf[1];
    isshe_size_t len = 0;

    len = json_print_length(item, format);
    if (len == 0) {
        return ISSHE_ERROR;
    }
    isshe_memzero(pbuf, sizeof(pbuf));

    pbuf->format = format;
    pbuf->length = len;
    pbuf->log = log;

    // 这里不使用内存池
    pbuf->buffer = isshe_malloc(len);
    if (!pbuf->buffer) {
        printf("alloc print buffer failed");
        return ISSHE_ERROR;
    }

    if (value_print(item, pbuf) != ISSHE_OK) {
        isshe_free(pbuf->buffer);
        return ISSHE_ERROR;
    }

    pbuf->buffer[pbuf->offset] = '\0';

    if (log) {
        isshe_log_info(log, "%s", pbuf->buffer);
    } else {
        printf("%s\n", pbuf->buffer);
    }

    isshe_free(pbuf->buffer);

    return ISSHE_OK;
}

isshe_size_t
isshe_json_print_length(const isshe_json_t * const item)
{
    return json_print_length(item, ISSHE_FALSE);
}

isshe_size_t
isshe_json_print_format_length(const isshe_json_t * const item)
{
    return json_print_length(item, ISSHE_TRUE);
}

isshe_int_t
isshe_json_print(
    const isshe_json_t * const item, isshe_log_t *log)
{
    return json_print(item, ISSHE_FALSE, log);
}

isshe_int_t
isshe_json_print_format(
    const isshe_json_t * const item, isshe_log_t *log)
{
    return json_print(item, ISSHE_TRUE, log);
}