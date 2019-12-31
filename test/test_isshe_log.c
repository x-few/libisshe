#include "isshe_common.h"

int main()
{
    isshe_log_t *log;

    log = isshe_log_instance_get(NULL);
    if (!log) {
        printf("error: log == NULL!!!\n");
    }
    int a = 100;
    isshe_log_debug(log, "hahah, %d", a);
    isshe_log_error(log, "hahah, %d", a);

    isshe_log_free(log);

    //isshe_log_t hint;

}