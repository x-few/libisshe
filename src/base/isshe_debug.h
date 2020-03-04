#ifndef _ISSHE_DEBUG_H_
#define _ISSHE_DEBUG_H_

#include "isshe_common.h"

isshe_void_t isshe_debug_print_addr(
    isshe_sa_t *sockaddr, isshe_log_t *log);
isshe_void_t isshe_debug_print_buffer(
    isshe_char_t *buf, isshe_int_t buf_len, isshe_int_t print_len);

#endif