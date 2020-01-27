#include "isshe_common.h"


isshe_connpool_t *
isshe_connpool_create(isshe_int_t n,
    isshe_mempool_t *mempool, isshe_log_t *log)
{
    isshe_connpool_t *connpool;
    isshe_int_t size, i;

    size = n * sizeof(isshe_connection_t) + sizeof(isshe_connpool_t);
    if (mempool) {
        connpool = (isshe_connpool_t *)isshe_mpalloc(mempool, size);
    } else {
        connpool = (isshe_connpool_t *)isshe_malloc(size, NULL);
    }
    if (!connpool) {
        if (log) {
            isshe_log_alert(log, "malloc connection pool failed");
        } else {
            printf("malloc connection pool failed\n");
        }
        return NULL;
    }

    isshe_memzero(connpool, size);
    connpool->nconn = n;
    connpool->nfree_conn = n;
    connpool->conns = (isshe_connection_t *)
        ((isshe_char_t *)connpool + sizeof(isshe_connpool_t));
    connpool->free_conn = connpool->conns;      // 这个是指针，指向空闲的节点
    connpool->log = log;
    connpool->mempool = mempool;

    // 连起来，方便获取
    for (i = 0; i < n - 1; i++) {
        connpool->conns[i].fd = ISSHE_INVALID_FILE;
        connpool->conns[i].next = &connpool->conns[i + 1];
        //connpool->conns[i].data = (void *)i;      // TODO ONLY DEBUG!
    }
    connpool->conns[i].next = NULL;
    //connpool->conns[i].data = (void *)i;          // TODO ONLY DEBUG!

    return connpool;
}


void
isshe_connpool_destroy(isshe_connpool_t *connpool)
{
    // TODO 如何保证连接全部被关闭呢？
    if (connpool->mempool) {
        isshe_mpfree(connpool->mempool, connpool,
            connpool->nconn * sizeof(isshe_connection_t)
            + sizeof(isshe_connpool_t));
    } else {
        isshe_free(connpool, NULL);
    }
}

// 获取到的连接，记得不能使用memset对整个结构进行清零。（next指针需要保持）
isshe_connection_t *
isshe_connection_get(isshe_connpool_t *connpool)
{
    // 从free中取一个
    isshe_connection_t *conn;

    if (!connpool) {
        return NULL;
    }

    if (connpool->nfree_conn <= 0
    || connpool->free_conn == NULL) {
        isshe_log_alert(connpool->log, 
            "%u connections are not enough", connpool->nconn);
        return NULL;
    }

    conn = connpool->free_conn;
    connpool->free_conn = connpool->free_conn->next;
    connpool->nfree_conn--;

    return conn;
}


void
isshe_connection_free(
    isshe_connpool_t *connpool, isshe_connection_t *conn)
{
    // TODO free all resources
    //isshe_memzero(conn, sizeof(isshe_connection_t));
    conn->fd = ISSHE_INVALID_FILE;
    conn->next = connpool->free_conn;
    connpool->free_conn = conn;
}