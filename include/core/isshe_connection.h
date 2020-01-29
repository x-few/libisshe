#ifndef _ISSHE_CONNECTION_H_
#define _ISSHE_CONNECTION_H_

#include "isshe_common.h"

#define ISSHE_IPV4_ADDR_LEN         4
#define ISSHE_IPV6_ADDR_LEN         16

typedef struct isshe_connection_s isshe_connection_t;
typedef enum isshe_conn_addr_type_e isshe_conn_addr_type_t;

enum isshe_conn_addr_type_e
{
    ISSHE_CONN_ADDR_TYPE_IPV4 = 0,
    ISSHE_CONN_ADDR_TYPE_IPV6 = 1,
    ISSHE_CONN_ADDR_TYPE_DOMAIN = 2,
};


struct isshe_connection_s
{
    isshe_socket_t      fd;
    isshe_uint16_t      port;
    isshe_sockaddr_t    *sockaddr;
    isshe_socklen_t     socklen;
    isshe_int_t         protocol;
    isshe_char_t        *addr_text;
    isshe_char_t        *protocol_text;
    isshe_int_t         status;
    isshe_mempool_t     *mempool;
    void                *data;
    isshe_connection_t  *next;
};


isshe_int_t isshe_conn_addr_type_get(const isshe_char_t *addr_str);

isshe_int_t isshe_conn_addr_pton(const isshe_char_t *addr_str,
                            isshe_int_t type, void *res_addr
                            , isshe_socklen_t *socklen);

#endif