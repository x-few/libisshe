#include "isshe_common.h"

isshe_long_t isshe_pathconf(const isshe_char_t *pathname, isshe_int_t name)
{
    isshe_long_t    val;

    errno = 0;  /* in case pathconf() does not change this */
    if ( (val = pathconf(pathname, name)) == ISSHE_FAILURE) {
        if (errno != 0) {
            isshe_sys_error_exit("pathconf error");
        }
        else {
            isshe_sys_error_exit("pathconf: %d not defined", name);
        }
    }
    return(val);
}

isshe_long_t isshe_sysconf(isshe_int_t name)
{
    isshe_long_t    val;

    errno = 0;  /* in case sysconf() does not change this */
    if ( (val = sysconf(name)) == ISSHE_FAILURE) {
        if (errno != 0) {
            isshe_sys_error_exit("sysconf error");
        }
        else {
            isshe_sys_error_exit("sysconf: %d not defined", name);
        }
    }
    return(val);
}

isshe_int_t
isshe_fcntl(isshe_int_t fd, isshe_int_t cmd, isshe_void_t *arg)
{
    isshe_int_t	n;

    if ( (n = fcntl(fd, cmd, arg)) == ISSHE_FAILURE) {
        isshe_sys_error_exit("fcntl error");
    }

    return(n);
}

isshe_pid_t
lock_test(isshe_int_t fd, isshe_int_t type,
    isshe_off_t offset, isshe_int_t whence, isshe_off_t len)
{
    struct flock lock;

    lock.l_type = type;		/* F_RDLCK or F_WRLCK */
    lock.l_start = offset;	/* byte offset, relative to l_whence */
    lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END */
    lock.l_len = len;		/* #bytes (0 means to EOF) */

    if (fcntl(fd, F_GETLK, &lock) == ISSHE_FAILURE)
        return(ISSHE_FAILURE);			/* unexpected error */

    if (lock.l_type == F_UNLCK)
        return(0);			/* false, region not locked by another proc */
    return(lock.l_pid);		/* true, return positive PID of lock owner */
}

isshe_pid_t
isshe_lock_test(isshe_int_t fd, isshe_int_t type,
    isshe_off_t offset, isshe_int_t whence, isshe_off_t len)
{
    isshe_pid_t pid;

    if ( (pid = lock_test(fd, type, offset, whence, len)) == ISSHE_FAILURE) {
        isshe_sys_error_exit("lock_test error");
    }

    return(pid);
}

isshe_int_t
isshe_getopt(isshe_int_t argc,
    isshe_char_t *const *argv, const isshe_char_t *str)
{
    isshe_int_t opt;

    if ( ( opt = getopt(argc, argv, str)) == '?') {
        exit(1);        /* getopt() has already written to stderr */
    }

    return(opt);
}

isshe_void_t *
isshe_mmap(isshe_void_t *addr, isshe_size_t len,
    isshe_int_t prot, isshe_int_t flags,
    isshe_int_t fd, isshe_off_t offset)
{
    isshe_void_t    *ptr;

    if ( (ptr = mmap(addr, len, prot, flags, fd, offset)) == ISSHE_MAP_FAILED ) {
        isshe_sys_error_exit("mmap error");
    }
    return(ptr);
}

isshe_void_t isshe_munmap(isshe_void_t *addr, isshe_size_t len)
{
    if (munmap(addr, len) == -1) {
        isshe_sys_error_exit("munmap error");
    }
}


isshe_int_t sleep_us(isshe_uint_t nusecs)
{
    struct timeval  tval;

    if (nusecs == 0) {
        return(0);
    }

    for ( ; ; ) {
        tval.tv_sec = nusecs / 1000000;
        tval.tv_usec = nusecs % 1000000;
        if (select(0, NULL, NULL, NULL, &tval) == 0) {
            return(0);      /* all OK */
        }

        /*
         * Note that on an interrupted system call there's not
         * much we can do, since the timeval{} isn't updated with the time
         * remaining.  We could obtain the clock time before the call, and
         * then obtain the clock time here, subtracting them to determine
         * how isshe_long_t select() blocked before it was interrupted, but that
         * seems like too much work :-)
         */
        if (errno != EINTR)
            return(-1);
        /* else go around again */
    }
}

isshe_void_t isshe_sleep_us(isshe_uint_t nusecs)
{
    if (sleep_us(nusecs) == -1) {
        isshe_sys_error_exit("sleep_us error");
    }
}

isshe_int_t
isshe_select(isshe_int_t nfds,
    fd_set *readfds, fd_set *writefds,
    fd_set *exceptfds, struct timeval *timeout)
{
    isshe_int_t n;

again_select:
    if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0) {
        if (errno == EINTR) {
            goto again_select;
        } else {
            isshe_sys_error_exit("select error");
        }
    } else if (n == 0 && timeout == NULL) {
        isshe_error_exit("select returned 0 with no timeout");
    }

    return(n);  /* can return 0 on timeout */
}

isshe_int_t
isshe_poll(struct pollfd *fdarray,
    isshe_ulong_t nfds, isshe_int_t timeout)
{
    isshe_int_t n;

    if ( (n = poll(fdarray, nfds, timeout)) < 0) {
        isshe_error_exit("poll error");
    }

    return(n);
}

#ifdef ISSHE_LINUX
isshe_fd_t isshe_epoll_create(isshe_int_t flags)
{
    isshe_fd_t rc;

    if ( (rc = epoll_create1(flags)) < 0 ) {
        isshe_sys_error_exit("epoll_create1 error");
    }

    return(rc);
}

isshe_int_t
isshe_epoll_ctl(isshe_fd_t epfd, isshe_int_t op,
    isshe_int_t fd, struct epoll_event *event)
{
    isshe_int_t rc;

    if ( (rc = epoll_ctl(epfd, op, fd, event)) < 0 ) {
        isshe_sys_error_exit("epoll_ctl error");
    }

    return(rc);
}

isshe_int_t
isshe_epoll_wait(isshe_fd_t epfd,
struct epoll_event *events, isshe_int_t maxevents,
isshe_int_t timeout, const sigset_t *sigmask)
{
    isshe_int_t rc = 0;

    if (sigmask) {
        rc = epoll_pwait(epfd, events, maxevents, timeout, sigmask);
    } else {
        rc = epoll_wait(epfd, events, maxevents, timeout);
    }

    if ( rc < 0 ) {
        isshe_sys_error_exit("epoll_pwait error");
    }

    return(rc);
}
#endif

#if defined(ISSHE_BSD) || defined(ISSHE_APPLE)
isshe_int_t isshe_kqueue(isshe_void_t)
{
    isshe_int_t rc;

    if ((rc = kqueue()) == ISSHE_FAILURE){
        isshe_sys_error_exit("kqueue error");
    }

    return rc;
}

isshe_int_t
isshe_kevent(isshe_int_t kq, const struct kevent *changelist,
    isshe_int_t nchanges, struct kevent *eventlist,
    isshe_int_t nevents, const struct timespec *timeout)
{
    isshe_int_t rc;

    rc = kevent(kq, changelist, nchanges, eventlist, nevents, timeout);
    if (rc == ISSHE_FAILURE){
        isshe_sys_error_exit("kevent error");
    }

    return rc;
}

isshe_int_t
isshe_kevent64(isshe_int_t kq,
    const struct kevent64_s *changelist, isshe_int_t nchanges,
    struct kevent64_s *eventlist, isshe_int_t nevents,
    isshe_uint_t flags, const struct timespec *timeout)
{
    isshe_int_t rc;

    rc = kevent64(kq, changelist, nchanges, eventlist, nevents, flags, timeout);
    if (rc == ISSHE_FAILURE){
        isshe_sys_error_exit("kevent64 error");
    }

    return rc;
}
#endif