#ifndef _ISSHE_UNISTD_H_
#define _ISSHE_UNISTD_H_

#include "isshe_common.h"

#ifdef INFTIM
#define ISSHE_INFTIM    INFTIM
#else
#define ISSHE_INFTIM    (-1)    /* infinite poll timeout */
#endif

isshe_long_t isshe_pathconf(
    const isshe_char_t *pathname, isshe_int_t name);

isshe_long_t isshe_sysconf(isshe_int_t name);

isshe_int_t isshe_fcntl(isshe_int_t fd,
    isshe_int_t cmd, isshe_void_t *arg);

isshe_int_t isshe_getopt(isshe_int_t argc,
    isshe_char_t *const *argv, const isshe_char_t *str);

isshe_void_t *isshe_mmap(
    isshe_void_t *addr, isshe_size_t len, isshe_int_t prot,
    isshe_int_t flags, isshe_fd_t fd, off_t offset);

isshe_void_t isshe_munmap(isshe_void_t *addr, isshe_size_t len);
isshe_void_t isshe_sleep_us(isshe_uint_t nusecs);

isshe_int_t isshe_select(isshe_int_t nfds, fd_set *readfds,
    fd_set *writefds,  fd_set *exceptfds, struct timeval *timeout);

isshe_int_t isshe_poll(struct pollfd *fdarray,
    isshe_ulong_t nfds, isshe_int_t timeout);

#ifdef ISSHE_LINUX
isshe_int_t isshe_epoll_create(isshe_int_t flags);
isshe_int_t isshe_epoll_ctl(isshe_fd_t epfd, isshe_int_t op,
    isshe_fd_t fd, struct epoll_event *event);

isshe_int_t isshe_epoll_wait(isshe_fd_t epfd,
    struct epoll_event *events, isshe_int_t maxevents,
    isshe_int_t timeout, const sigset_t *sigmask);
#endif

#if defined(ISSHE_BSD) || defined(ISSHE_APPLE)
isshe_int_t isshe_kqueue(isshe_void_t);

isshe_int_t isshe_kevent(isshe_int_t kq,
    const struct kevent *changelist, isshe_int_t nchanges,
    struct kevent *eventlist, isshe_int_t nevents,
    const struct timespec *timeout);

isshe_int_t isshe_kevent64(isshe_int_t kq,
    const struct kevent64_s *changelist, isshe_int_t nchanges,
    struct kevent64_s *eventlist, isshe_int_t nevents,
    isshe_uint_t flags, const struct timespec *timeout);

#endif

#endif  //_ISSHE_UNISTD_H_