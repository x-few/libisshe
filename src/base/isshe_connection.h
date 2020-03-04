#ifndef _ISSHE_CONNECTION_H_
#define _ISSHE_CONNECTION_H_

#include "isshe_common.h"

#define ISSHE_IPV4_ADDR_LEN         4
#define ISSHE_IPV6_ADDR_LEN         16

// 对应sock5
#define ISSHE_ADDR_TYPE_UNKNOWN     0x0
#define ISSHE_ADDR_TYPE_IPV4        0x1
#define ISSHE_ADDR_TYPE_DOMAIN      0x3
#define ISSHE_ADDR_TYPE_IPV6        0x4
#define ISSHE_ADDR_TYPE_IPV4_TEXT   0x5
#define ISSHE_ADDR_TYPE_IPV6_TEXT   0x6

// protocol
//#define ISSHE_PROTOCOL_SOCKS_V4     0x4
#define ISSHE_PROTOCOL_INVALID      0

struct isshe_address_s
{
    isshe_char_t            *addr;
    isshe_uint8_t           addr_type;
    isshe_uint8_t           addr_len;
    isshe_uint16_t          port;
    isshe_sockaddr_t        *sockaddr;
    isshe_socklen_t         socklen;
    isshe_addrinfo_t        *info;
};

struct isshe_connection_s
{
    isshe_fd_t          fd;
    isshe_address_t     *addr;
    isshe_char_t        *protocol_text;
    isshe_uint8_t       protocol;
    isshe_int_t         status;
    isshe_mempool_t     *mempool;
    isshe_void_t        *data;
    isshe_connection_t  *next;
};

isshe_int_t isshe_address_type_get(
    const isshe_char_t *addr,isshe_uint8_t addr_len);

isshe_int_t isshe_address_pton(
    const isshe_char_t *addr_text,
    isshe_uint8_t addr_len,
    isshe_uint8_t addr_type,
    isshe_sockaddr_t *res_sockaddr,
    isshe_socklen_t *res_socklen,
    isshe_log_t *log);

isshe_address_t * isshe_address_create(
    isshe_char_t *addr, isshe_uint8_t addr_len,
    isshe_uint8_t addr_type, isshe_mempool_t *mempool,
    isshe_log_t *log);

isshe_sockaddr_t * isshe_address_sockaddr_create(
    isshe_address_t *address, isshe_mempool_t *mempool, isshe_log_t *log);

isshe_int_t isshe_sockaddr_port_set(
    isshe_sockaddr_t *sockaddr, isshe_uint16_t port);

isshe_int_t isshe_address_port_set(
    isshe_address_t *address, isshe_uint16_t port);

void isshe_address_print(
    isshe_address_t *addr, isshe_log_t *log);

#endif