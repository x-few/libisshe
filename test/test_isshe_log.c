#include "isshe_common.h"


void test1()
{
    isshe_log_t *log;

    log = isshe_log_instance_get(ISSHE_LOG_ERROR, NULL);
    if (!log) {
        printf("error: log == NULL!!!\n");
    }
    int a = 100;
    isshe_log_debug(log, "hahah, a = %d", a);
    isshe_log_error(log, "hahah, a = %d", a);
    isshe_log_free();
}

void test2()
{
    isshe_log_t *log;

    log = isshe_log_instance_get(ISSHE_LOG_DEBUG, NULL);
    if (!log) {
        printf("error: log == NULL!!!\n");
    }
    char b[] = "abc";
    isshe_log_debug(log, "hahah, b = %s", b);
    isshe_log_error(log, "hahah, b = %s", b);

    isshe_log_t *log2 = isshe_log_instance_get(0, NULL);
    isshe_log(ISSHE_LOG_CRIT, log2, "b = %s", b);
    isshe_log_free();
}

void test3()
{
    isshe_log_t *log;

    log = isshe_log_instance_get(ISSHE_LOG_DEBUG, "log.tmp");
    isshe_char_t *b = "fjadlkdfjklasjfklds";
    //b[1] = '2';
    isshe_log(ISSHE_LOG_CRIT, log, "b = %s", b);
    isshe_log_free();
}

int main()
{
    test1();
    test2();
    test3();
}