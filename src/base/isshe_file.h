#ifndef _ISSHE_FILE_H_
#define _ISSHE_FILE_H_

#include "isshe_common.h"

#define ISSHE_FILE_RDONLY           O_RDONLY
#define ISSHE_FILE_WRONLY           O_WRONLY
#define ISSHE_FILE_RDWR             O_RDWR
#define ISSHE_FILE_CREATE_OR_OPEN   O_CREAT
#define ISSHE_FILE_CRWR             (O_CREAT | O_WRONLY)
#define ISSHE_FILE_CRRDWR           (O_CREAT | O_RDWR)
#define ISSHE_FILE_OPEN             0
#define ISSHE_FILE_TRUNCATE         (O_CREAT|O_TRUNC)
#define ISSHE_FILE_APPEND           (O_WRONLY|O_APPEND)
#define ISSHE_FILE_NONBLOCK         O_NONBLOCK
#define ISSHE_LOCKMODE              (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define ISSHE_FILE_DEFAULT_ACCESS   (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)     // 0644
#define ISSHE_FILE_OWNER_ACCESS     (S_IRUSR | S_IWUSR)                         // 0600
#define	ISSHE_DIR_DEFAULT_ACCESS    (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

#define ISSHE_INVALID_FILE          -1
#define ISSHE_INVALID_FD            -1

#define isshe_stdout                STDOUT_FILENO
#define isshe_stderr                STDERR_FILENO

#define isshe_read                  read
#define isshe_write                 write
#define isshe_delete(name)          unlink((const char *) name)
#define isshe_lseek                 lseek
#define isshe_ftruncate             ftruncate
#define isshe_fstat                 fstat

struct isshe_file_s
{
    isshe_fd_t                  fd;
    isshe_string_t              *name;
    isshe_finfo_t               *info;
};

isshe_int_t isshe_lock_file(isshe_fd_t fd);
isshe_int_t isshe_unlock_file(isshe_fd_t fd);

// 简化fcntl锁相关的调用
isshe_int_t isshe_lock_op(
    isshe_fd_t, isshe_int_t,isshe_int_t,
    isshe_off_t, isshe_int_t, isshe_off_t); /* {Prog lockreg} */

isshe_pid_t isshe_lock_test(
    isshe_fd_t, isshe_int_t, isshe_off_t,
    isshe_int_t, isshe_off_t); /* {Prog locktest} */

// 几个记录锁的宏
#define read_lock(fd, offset, whence, len) \
            isshe_lock_op((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define	readw_lock(fd, offset, whence, len) \
            isshe_lock_op((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define	write_lock(fd, offset, whence, len) \
            isshe_lock_op((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define	writew_lock(fd, offset, whence, len) \
            isshe_lock_op((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define	un_lock(fd, offset, whence, len) \
            isshe_lock_op((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))
#define	is_read_lockable(fd, offset, whence, len) \
            (isshe_lock_test((fd), F_RDLCK, (offset), (whence), (len)) == 0)
#define	is_write_lockable(fd, offset, whence, len) \
            (isshe_lock_test((fd), F_WRLCK, (offset), (whence), (len)) == 0)

isshe_int_t isshe_open(const isshe_char_t *pathname, isshe_int_t oflag, ...);
isshe_int_t isshe_close(isshe_fd_t fd);
isshe_ssize_t isshe_read_line(isshe_fd_t fd, isshe_void_t *ptr, isshe_size_t maxlen);
isshe_char_t *isshe_read_all(isshe_fd_t fd, isshe_ssize_t *reslen);

#endif