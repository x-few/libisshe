#include "isshe_common.h"

void test()
{
    isshe_mempool_t *pool;
    isshe_log_t *log;
    
    log = isshe_log_instance_get(ISSHE_LOG_DEBUG, NULL);
    if (!log) {
        printf("error: log == NULL!!!\n");
        return;
    }
    pool = isshe_mempool_create(3 * 1024, log);
    if (!pool) {
        isshe_log_alert(log, "create memory pool failed");
        return;
    }

    isshe_uchar_t *tmp;
    char *tmp_data;
    isshe_size_t  size = 1024;

    tmp = (isshe_uchar_t *)isshe_mpalloc(pool, size);
    tmp_data = "isshe_tmp_data...1...";
    isshe_memcpy(tmp, tmp_data, strlen(tmp_data) + 1);
    isshe_log_debug(log, "tmp = %s", tmp);
    //isshe_mpfree(pool, tmp, 0);
    isshe_mpfree(pool, tmp, size);

    tmp = (isshe_uchar_t *)isshe_mpalloc(pool, size);
    isshe_mpfree(pool, tmp, size);
    tmp = (isshe_uchar_t *)isshe_mpalloc(pool, size);
    isshe_mpfree(pool, tmp, size);

    size = 8 * 1024;
    tmp = (isshe_uchar_t *)isshe_mpalloc(pool, size);
    tmp_data = "isshe_tmp_data...2...";
    isshe_memcpy(tmp, tmp_data, strlen(tmp_data) + 1);
    isshe_log_debug(log, "tmp = %s", tmp);
    isshe_mpfree(pool, tmp, size);

    isshe_mempool_destroy(pool);
}

int main(void)
{
    test();
}