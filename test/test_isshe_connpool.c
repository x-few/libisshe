#include "isshe_common.h"


void print_connpool(isshe_connpool_t *connpool, isshe_int_t n)
{
    isshe_int_t i;
    isshe_connection_t *conn = connpool->free_conn;

    for (i = 0; i < n; i++) {
        printf("%d ", (int)conn->data);
        conn = conn->next;
    }
    printf("\n");
}

void test()
{
    isshe_connpool_t *connpool;
    isshe_mempool_t *mempool;
    isshe_log_t *log;
    
    log = isshe_log_instance_get(ISSHE_LOG_DEBUG, NULL);
    if (!log) {
        printf("error: log == NULL!!!\n");
        return;
    }

    mempool = isshe_mempool_create(8 * 1024, log);
    if (!mempool) {
        isshe_log_alert(log, "create memory pool failed");
        return;
    }

    connpool = isshe_connpool_create(1024, mempool, log);
    if (!connpool) {
        isshe_log_alert(log, "create connection pool failed");
        return;
    }

    isshe_connection_t *conn, *conn2, *conn3;
    conn = isshe_connection_get(connpool);
    conn2 = isshe_connection_get(connpool);
    conn3 = isshe_connection_get(connpool);
    
    if (!conn || !conn2 || !conn3) {
        isshe_log_alert(log, "get connection failed");
        return;
    }

    isshe_log_debug(log, "conn1 = %d, conn2 = %d, conn3 = %d",
        (int)conn->data, (int)conn2->data, (int)conn3->data);
    print_connpool(connpool, 10);

    isshe_connection_free(connpool, conn);
    print_connpool(connpool, 10);

    isshe_connection_free(connpool, conn2);
    print_connpool(connpool, 10);

    isshe_connection_free(connpool, conn3);
    print_connpool(connpool, 10);

    isshe_connpool_destroy(connpool);
    isshe_mempool_destroy(mempool);
    isshe_log_instance_free();
}

int main(void)
{
    test();
}