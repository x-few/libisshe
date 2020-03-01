#include "isshe_common.h"


void test1()
{
    isshe_log_t *log;

    log = isshe_log_instance_get(ISSHE_LOG_ERROR, NULL, NULL);
    if (!log) {
        printf("error: log == NULL!!!\n");
    }
    int a = 100;
    isshe_log_debug(log, "hahah, a = %d", a);
    isshe_log_error(log, "hahah, a = %d", a);
    isshe_log_instance_free();
}

void test2()
{
    isshe_log_t *log;

    log = isshe_log_instance_get(ISSHE_LOG_DEBUG, NULL, NULL);
    if (!log) {
        printf("error: log == NULL!!!\n");
    }
    char b[] = "abc";
    isshe_log_debug(log, "hahah, b = %s", b);
    isshe_log_error(log, "hahah, b = %s", b);

    isshe_log_t *log2 = isshe_log_instance_get(0, NULL, NULL);
    isshe_log(ISSHE_LOG_CRIT, log2, "b = %s", b);
    isshe_log_instance_free();
}

void test3()
{
    isshe_log_t *log;

    log = isshe_log_instance_get(ISSHE_LOG_DEBUG, "log.tmp", NULL);
    //printf("pid = %d, log = %p\n", getpid(), log);
    isshe_char_t *b = "fjadlkdfjklasjfklds";
    //b[1] = '2';
    isshe_log(ISSHE_LOG_CRIT, log, "b = %s", b);
    
    isshe_log_instance_free();
}

void test4()
{
    pid_t pid;
    if ((pid = fork()) == 0) {
        // child
        test3();
    } else {
        test3();
    }
}

void test5()
{
    isshe_log_t *log;
    int i = 0;

    log = isshe_log_instance_get(ISSHE_LOG_DEBUG, "log.tmp", NULL);
    isshe_char_t *b = "fjadlkdfjklasjfklds";
    //b[1] = '2';
    fork();
    printf("pid = %d, log = %p\n", getpid(), log);
    for (i = 0; i < 10; i++) {
        isshe_log(ISSHE_LOG_CRIT, log, "b = %s", b);
    }
    
    isshe_log_instance_free();
}

void test6()
{
    isshe_log_t *log;

    log = isshe_log_create(ISSHE_LOG_DEBUG, NULL, NULL);
    isshe_log_debug(log, "test6: pid = %d, log = %p", getpid(), log);
    isshe_log_destroy(log);
}

void test7()
{
    isshe_log_t *log;

    log = isshe_log_create(ISSHE_LOG_DEBUG, "log.tmp", NULL);
    isshe_log_debug(log, "test7: pid = %d, log = %p", getpid(), log);
    isshe_log_destroy(log);
}

int main()
{
    
    //test1();
    //test2();
    //test3();
    //test4();
    //test5();
    test6();
    test7();

}