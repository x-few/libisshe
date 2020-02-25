#include "isshe_common.h"

extern isshe_int_t daemon_proc;    // defined in isshe_error.c

isshe_pid_t isshe_fork(isshe_void_t)
{
    isshe_pid_t pid;
    if ((pid = fork()) < 0) {
        isshe_sys_error_exit("Fork error");
    }

    return pid;
}

isshe_void_t isshe_print_exit_status(isshe_int_t status)
{
    if (WIFEXITED(status)) {
        printf("normal termination, exit status = %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status),
#ifdef WCOREDUMP
        WCOREDUMP(status) ? "(core file generated)" : "");
#else
        "");
#endif
    } else if (WIFSTOPPED(status)) {
        printf("child stopped, signal number = %d\n", WSTOPSIG(status));
    }
}

isshe_void_t
isshe_daemonize(
    const isshe_char_t *pname, isshe_int_t facility)
{
    isshe_int_t i, fd0, fd1, fd2;
    isshe_pid_t pid;
    struct rlimit rl;

    // 清除文件模式创建屏蔽字
    umask(0);

    // 获取最大文件描述符
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        printf("%s: can't get file limit\n", pname);
        exit(0);
    }

    // 成为会话leader，没有控制终端
    pid = isshe_fork();
    if (pid < 0) {
        printf("fork failed\n");
        exit(0);
    } else if (pid != 0) {
        exit(0);    // 终止父进程
    }

    setsid();

    // 确保之后的open不会分配控制终端
    isshe_signal(SIGHUP, SIG_IGN);

    pid = isshe_fork();
    if (pid != 0) {
        exit(0);    // 终止第一个子进程
    }

    daemon_proc = 1;    /* for our err_XXX() functions */

    // 改工作目录
    if (chdir("/") < 0) {
        printf("%s: can't change directory to /", pname);
        exit(0);
    }

    // close all open file descriptors
    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++) {
        close(i);
    }

    // 把0、1、2整到/dev/null
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    // 初始化log文件
    openlog(pname, LOG_PID, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR,
            "unexpected file descriptors %d %d %d\n",
            fd0, fd1, fd2);
        exit(1);
    }
}

isshe_pid_t
isshe_waitpid(isshe_pid_t pid,
    isshe_int_t *iptr, isshe_int_t options)
{
    isshe_pid_t ret_pid;

    if ( (ret_pid = waitpid(pid, iptr, options)) == ISSHE_FAILURE ) {
        isshe_sys_error_exit("waitpid error");
    }

    return ret_pid;
}
