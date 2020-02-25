#ifndef _ISSHE_PROCESS_H_
#define _ISSHE_PROCESS_H_

#include "isshe_common.h"

#define isshe_log_pid            getpid()

isshe_pid_t isshe_fork(isshe_void_t);
isshe_void_t isshe_print_exit_status(isshe_int_t status);

/**
 * daemonize: 守护进程化——使一个进程"变为"守护进程
 */
isshe_void_t isshe_daemonize(
    const isshe_char_t *pname, isshe_int_t facility);

isshe_pid_t isshe_waitpid(isshe_pid_t pid,
    isshe_int_t *iptr, isshe_int_t options);

#endif