/*
 * 开一块新的environ大小的空间，把environ复制过去，并重新设置environ[i]指向；
 * 然后空出来的旧environ空间，就全部用来设置标题。
 */

#include "isshe_common.h"

#if (defined ISSHE_LINUX || defined ISSHE_APPLE)
// 全局变量
extern char **environ;

static isshe_process_title_t ipt;

static isshe_size_t
environ_lenght_get()
{
    isshe_int_t i;
    isshe_size_t size = 0;

    for (i = 0; environ[i]; i++) {
        size += strlen(environ[i]) + 1;
    }
    return size;
}

static char *
argv_last_get(int argc, char *argv[])
{
    char *last;
    isshe_int_t i;

    last = argv[0];
    for (i = 0; i < argc || (i >= argc && argv[i]); i++) {
        if (last == argv[i]) {
            last = argv[i] + strlen(argv[i]) + 1;
        }
    }
    return last;
}

static char *
environ_reset(char *newenvp, char *last)
{
    isshe_size_t size;
    isshe_int_t i;

    for (i = 0; environ[i]; i++) {
        if (last == environ[i]) {
            size = strlen(environ[i]) + 1;
            last = environ[i] + size;

            isshe_memcpy(newenvp, environ[i], size);
            environ[i] = (char *)newenvp;
            newenvp += size;
        }
    }

    last--;         // 指多了一个，指回来
    return last;
}


isshe_int_t
isshe_process_title_init(int argc, char *argv[])
{
    isshe_char_t **env = environ;
    isshe_size_t size;
    isshe_char_t *p;
    isshe_int_t i;

    size = environ_lenght_get();

    p = (isshe_char_t *)malloc(size);
    if (!p) {
        return ISSHE_FAILURE;
    }

    ipt.argv_last = argv_last_get(argc, argv);
    ipt.argv_last = environ_reset(p, ipt.argv_last);

    ipt.argc = argc;
    ipt.argv = argv;

    return ISSHE_SUCCESS;
}

void
isshe_process_title_set(const char *fmt, ...)
{
    va_list     args;
    isshe_int_t len;
    isshe_int_t argv_len;

    // To change the process title in Linux
    // and Solaris we have to set argv[1] to NULL.
    ipt.argv[1] = NULL;
    argv_len = ipt.argv_last - ipt.argv[0];

    va_start(args, fmt);
    len = vsnprintf(ipt.argv[0], argv_len, fmt, args);
    va_end(args);

    if (argv_len > len) {
        isshe_memset(ipt.argv[0] + len,
                        ISSHE_PROCESS_TITLE_PAD,
                        argv_len - len);
    }

}

#endif