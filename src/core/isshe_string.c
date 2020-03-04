
#include "isshe_common.h"


isshe_char_t *isshe_strdup(isshe_char_t *src, isshe_mempool_t *mempool)
{
    isshe_size_t len;

    len = isshe_strlen(src) + sizeof("");
    return (isshe_char_t *)isshe_memdup((const void *)src, len, mempool);
}

isshe_int_t isshe_strcmp_case_insensitive(
    const isshe_char_t *str1,
    const isshe_char_t *str2)
{
    assert(str1);
    assert(str2);

    if (str1 == str2) {
        return 0;
    }

    for(; tolower(*str1) == tolower(*str2); str1++, str2++) {
        if (*str1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*str1) - tolower(*str2);
}


isshe_string_t *
isshe_string_create(const isshe_char_t *str,
    isshe_size_t size, isshe_mempool_t *mempool)
{
    isshe_string_t *string;

    if (mempool) {
        string = (isshe_string_t *)isshe_mpalloc(mempool, sizeof(isshe_string_t));
    } else {
        string = (isshe_string_t *)isshe_malloc(sizeof(isshe_string_t));
    }
    if (!string) {
        return NULL;
    }

    isshe_memzero(string, sizeof(isshe_string_t));
    if (!str) {
        return string;
    }

    if (mempool) {
        string->data = (isshe_char_t *)isshe_mpalloc(mempool, size);
    } else {
        string->data = (isshe_char_t *)isshe_malloc(size);
    }
    if (!string->data) {
        isshe_mpfree(mempool, string, sizeof(isshe_string_t));
        return NULL;
    }

    string->len = size;
    isshe_memcpy(string->data, str, size);

    return string;
}

isshe_void_t
isshe_string_destroy(isshe_string_t *string, isshe_mempool_t *mempool)
{
    if (string) {
        if (string->data && string->len > 0) {
            if (mempool) {
                isshe_mpfree(mempool, string->data, string->len);
            } else {
                isshe_free(string->data);
            }
            string->data = NULL;
            string->len = 0;
        }

        if(mempool) {
            isshe_mpfree(mempool, string, sizeof(isshe_string_t));
        } else {
            isshe_free(string);
        }
    }
}

isshe_int_t
isshe_string_set(isshe_string_t *string, isshe_char_t *str)
{
    if (!string || !str) {
        return ISSHE_ERROR;
    }

    string->data = str;
    string->len = strlen(str);
    return ISSHE_OK;
}

isshe_string_t *
isshe_string_dup(isshe_string_t *string, isshe_mempool_t *mempool)
{
    if (!string) {
        return NULL;
    }
    return isshe_string_create(string->data, string->len, mempool);
}


static isshe_int_t
isshe_sprintf_num(
    isshe_char_t *buf, isshe_size_t size,
    isshe_uint64_t ui64, isshe_char_t zero,
    isshe_uint_t hexadecimal, isshe_uint_t width)
{
    isshe_char_t        temp[ISSHE_MAX_INT64_STR_LEN + 1];
    isshe_char_t        *last, *p;
    isshe_size_t        len;
    isshe_uint32_t      ui32;
    static isshe_char_t hex[] = "0123456789abcdef";
    static isshe_char_t HEX[] = "0123456789ABCDEF";
    isshe_char_t        *save_buf;

    save_buf = buf;
    last = buf + size;
    p = temp + ISSHE_MAX_INT64_STR_LEN; // 指向最后一个

    if (hexadecimal == 1) {
        do {
            p--;    // 为了不用-1，先--
            *p = hex[(isshe_uint32_t)(ui64 & 0xf)];
            ui64 >>= 4;
        } while (ui64);
    } else if (hexadecimal == 2) {
        do {
            p--;
            *p = HEX[(isshe_uint32_t)(ui64 & 0xf)];
            ui64 >>= 4;
        } while (ui64);
    } else {
        do {
            p--;
            *p = (isshe_char_t) (ui64 % 10 + '0');
            ui64 /= 10;
        } while(ui64);
    }

    // 填充 zero
    len = (temp + ISSHE_MAX_INT64_STR_LEN) - p;
    while (len < width && buf < last) {
        *buf++ = zero;
        width--;
    }

    if (len > last - buf) {
        len = last - buf;
    }

    isshe_memcpy(buf, p, len);
    return (buf + len - save_buf);
}


isshe_int_t
isshe_vsnprintf(
    isshe_char_t *buf, isshe_size_t size,
    const char *fmt, va_list args)
{
    isshe_char_t    *last, *p;
    isshe_int64_t   i64;
    isshe_uint64_t  ui64, frac;
    isshe_string_t  *str;
    isshe_size_t    len, slen;
    isshe_char_t    zero;
    isshe_int_t     d;
    isshe_double_t  f;
    isshe_uint_t    width, sign, hex, frac_width, scale, n;
    isshe_char_t    *save_buf;
    
    save_buf = buf;
    last = buf + size;

    while (*fmt && buf < last) {

        if (*fmt != '%') {
            *buf++ = *fmt++;
            continue;
        }

        // *fmt == '%'

        // init
        sign = 1;
        width = 0;
        hex = 0;
        slen = 0;   // 与nginx不同
        frac_width = 0;

        i64 = 0;
        ui64 = 0;

        zero = (isshe_char_t) ((*++fmt == '0') ? '0' : ' ');

        // 处理类似02f等的情况
        while(*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        // 处理一些多重前缀
        while(ISSHE_TRUE) {
            switch (*fmt)
            {
            case 'u':
                sign = 0;
                fmt++;
                continue;
            case 'x':
                hex = 1;
                sign = 0;
                fmt++;
                continue;
            case 'X':
                hex = 2;
                sign = 0;
                fmt++;
                continue;
            case '.':
                fmt++;

                while(*fmt >= '0' && *fmt <= '9') {
                    frac_width = frac_width * 10 + (*fmt - '0');
                    fmt++;
                }
                break;
            case '*':   // for %*s
                slen = va_arg(args, isshe_size_t);
                fmt++;
                continue;
            default:
                break;
            }

            break;
        }

        switch (*fmt)
        {
        case 's':
            p = va_arg(args, isshe_char_t *);
            if (slen == 0) {
                len = isshe_strlen(p);
                len = isshe_min(((isshe_size_t)(last - buf)), len);
            } else {
                len = isshe_min(((isshe_size_t)(last - buf)), slen);
            }
            isshe_memcpy(buf, p, len);
            buf += len;
            fmt++;
            continue;   // next %x

        case 'S':       // isshe_string_t *
            str = va_arg(args, isshe_string_t *);
            len = isshe_min(((isshe_size_t)(last - buf)), str->len);
            isshe_memcpy(buf, str->data, len);
            buf += len;
            fmt++;
            continue;
        case 'c':
            d = va_arg(args, isshe_int_t);
            *buf++ = (isshe_char_t)(d & 0xff);    // 取低8位
            fmt++;
            continue;
        case '%':
            *buf++ = '%';
            fmt++;
            continue;
        case 'f':
            f = va_arg(args, isshe_double_t);

            if (f < 0) {
                *buf++ = '-';
                f = -f;
            }

            ui64 = (isshe_uint64_t) f;  // 舍弃小数点后的
            frac = 0;

            //if (frac_width) {
            scale = 1;
            for (n = frac_width; n > 0; n--) {
                scale *= 10;
            }
            // +0.5是为了进1
            frac = (isshe_uint64_t)((f - (isshe_double_t) ui64) * scale + 0.5);
            if (frac == scale) {
                ui64++;
                frac = 0;
            }
            //}

            buf += isshe_sprintf_num(buf, last - buf, ui64, zero, 0, width);

            if (frac_width) {
                if (buf < last) {
                    *buf++ = '.';
                }
                buf += isshe_sprintf_num(buf, last - buf, frac, '0', 0, frac_width);
            }
            fmt++;
            continue;
        case 'd':
            if (sign) {
                i64 = (isshe_int64_t) va_arg(args, isshe_int_t);
            } else {
                ui64 = (isshe_uint64_t) va_arg(args, isshe_uint_t);
            }
            break;      // break 都是用来处理int类型的
        case 'l':
            if (sign) {
                i64 = (isshe_int64_t) va_arg(args, isshe_long_t);
            } else {
                ui64 = (isshe_uint64_t) va_arg(args, isshe_ulong_t);
            }
            break;
        case 'D':
            if (sign) {
                i64 = (isshe_int64_t) va_arg(args, isshe_int32_t);
            } else {
                ui64 = (isshe_uint64_t) va_arg(args, isshe_uint32_t);
            }
            break;
        case 'L':
            if (sign) {
                i64 = (isshe_int64_t) va_arg(args, isshe_int64_t);
            } else {
                ui64 = (isshe_uint64_t) va_arg(args, isshe_uint64_t);
            }
            break;
        case 'p':
            ui64 = (uintptr_t) va_arg(args, void *);
            hex = 2;
            sign = 0;
            zero = '0';
            width = 2 * sizeof(void *);
            break;
        case 'P':       // isshe_pid_t
            i64 = (isshe_int64_t) va_arg(args, isshe_pid_t);
            break;
        case 't':       // isshe_time_t
            i64 = (isshe_int64_t) va_arg(args, isshe_time_t);
            break;
        default:
            *buf++ = *fmt++;
            continue;
        }

        if (sign) {
            if (i64 < 0) {
                *buf++ = '-';
                ui64 = (isshe_uint64_t) -i64;
            } else {
                ui64 = (isshe_uint64_t) i64;
            }
        }

        buf += isshe_sprintf_num(buf, last - buf, ui64, zero, hex, width);
        fmt++;
    }

    return (buf - save_buf);
}

isshe_int_t
isshe_snprintf(
    isshe_char_t *buf, isshe_size_t max,
    const isshe_char_t *fmt, ...)
{
    isshe_int_t     rc;
    va_list         args;

    va_start(args, fmt);
    rc = isshe_vsnprintf(buf, max, fmt, args);
    va_end(args);

    return rc;
}
