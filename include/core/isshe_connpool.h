#ifndef _ISSHE_CONNPOOL_H_
#define _ISSHE_CONNPOOL_H_

#include "isshe_common.h"

#define ISSHE_CONNPOOL_DEFAULT_SIZE     1024

typedef struct isshe_connection_s isshe_connection_t;
typedef struct isshe_connpool_s isshe_connpool_t;

struct isshe_connection_s
{
    isshe_socket_t      fd;
    isshe_uint16_t      port;
    isshe_sockaddr_t    sockaddr;
    isshe_socklen_t     socklen;
    isshe_int_t         protocol;
    isshe_char_t        *addr_text;
    isshe_char_t        *protocol_text;
    isshe_int_t         status;
    isshe_mempool_t     *mempool;
    void                *data;
    isshe_connection_t  *next;

};

struct isshe_connpool_s
{
    isshe_int_t         nconn;
    isshe_connection_t  *conns;         // array
    isshe_int_t         nfree_conn;
    isshe_connection_t  *free_conn;     // pointer
    isshe_mempool_t     *mempool;
    isshe_log_t         *log;
};


isshe_connpool_t *isshe_connpool_create(isshe_int_t n,
    isshe_mempool_t *mempool, isshe_log_t *log);


void isshe_connpool_destroy(isshe_connpool_t *connpool);

isshe_connection_t *isshe_connection_get(
    isshe_connpool_t *connpool);

void isshe_connection_free(isshe_connpool_t *connpool,
    isshe_connection_t *conn);

#endif