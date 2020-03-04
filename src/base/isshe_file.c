

#include "isshe_common.h"

// TODO isshe_write_file/isshe_read_file: 参考ngx实现相关的文件操作

static isshe_int_t
isshe_lock_unlock(isshe_fd_t fd, isshe_int16_t type) {
    struct flock fl;

    fl.l_type = type;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return(fcntl(fd, F_SETLK, &fl));
}


isshe_int_t isshe_lock_file(isshe_fd_t fd)
{
    return isshe_lock_unlock(fd, F_WRLCK);
}


isshe_int_t isshe_unlock_file(isshe_fd_t fd)
{
    return isshe_lock_unlock(fd, F_UNLCK);
}


isshe_int_t
isshe_lock_op(isshe_fd_t fd, isshe_int_t cmd,
    isshe_int_t type, isshe_off_t offset,
    isshe_int_t whence, isshe_off_t len)
{
    struct flock    lock;

    lock.l_type = type;     /* F_RDLCK, F_WRLCK, F_UNLCK */
    lock.l_start = offset;  /* byte offset, relative to l_whence */
    lock.l_whence = whence; /* SEEK_SET, SEEK_CUR, SEEK_END */
    lock.l_len = len;       /* #bytes (0 means to EOF) */

    return( fcntl(fd, cmd, &lock) );    /* -1 upon error */
}


isshe_pid_t
isshe_lock_test(isshe_int_t fd, isshe_int_t type,
    isshe_off_t offset, isshe_int_t whence, isshe_off_t len)
{
    struct flock lock;

    lock.l_type = type;		/* F_RDLCK or F_WRLCK */
    lock.l_start = offset;	/* byte offset, relative to l_whence */
    lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END */
    lock.l_len = len;		/* #bytes (0 means to EOF) */

    if (fcntl(fd, F_GETLK, &lock) == ISSHE_ERROR)
        return(ISSHE_ERROR);			/* unexpected error */

    if (lock.l_type == F_UNLCK)
        return(0);			/* false, region not locked by another proc */
    return(lock.l_pid);		/* true, return positive PID of lock owner */
}


isshe_int_t
isshe_open(const isshe_char_t *pathname, isshe_int_t oflag, ...)
{
    isshe_fd_t      fd;
    va_list         ap;
    isshe_mode_t    mode;

    if (oflag & O_CREAT) {
        va_start(ap, oflag);
        mode = va_arg(ap, isshe_mode_t);
        if ( (fd = open(pathname, oflag, mode)) == ISSHE_ERROR ) {
            return ISSHE_INVALID_FILE;
        }
        va_end(ap);
    } else {
        if ( (fd = open(pathname, oflag)) == ISSHE_ERROR ) {
            return ISSHE_INVALID_FILE;
        }
    }

    return fd;
}

isshe_int_t isshe_close(isshe_fd_t fd)
{
    if (fd != ISSHE_INVALID_FD) {
        return close(fd);
    }

    return ISSHE_OK;
}


static isshe_ssize_t
isshe_restart_read(isshe_fd_t fd, isshe_char_t *ptr)
{
    static isshe_int_t	read_cnt = 0;
    static isshe_char_t	*read_ptr;
    static isshe_char_t	read_buf[ISSHE_MAXLINE];

    if (read_cnt <= 0) {
        while(1) {
            if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
                if (errno == EINTR) {
                    continue;
                }
                return(-1);
            } else if (read_cnt == 0) {
                return(0);
            }
            read_ptr = read_buf;
            break;
        }
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return(1);
}

isshe_ssize_t
isshe_read_line(isshe_fd_t fd, isshe_void_t *vptr, isshe_size_t maxlen)
{
    isshe_int_t n, rc;
    isshe_char_t c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = isshe_restart_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;      /* newline is stored, like fgets() */
        } else if (rc == 0) {
            if (n == 1)
                return(0);  /* EOF, no data read */
            else
                break;      /* EOF, some data was read */
        } else
            return(-1);     /* error, errno set by read() */
    }

    *ptr = 0;               /* null terminate like fgets() */
    return(n);
}
/* end readline */


isshe_char_t *
isshe_read_all(isshe_fd_t fd, isshe_ssize_t *reslen)
{
    isshe_finfo_t info;
    isshe_char_t *buf;
    isshe_ssize_t len;

    isshe_fstat(fd, &info);
    if (info.st_size > 0) {
        buf = isshe_malloc(info.st_size);           // remember to free
        len = isshe_read(fd, buf, info.st_size);
        *reslen = len;
        if (len != info.st_size) {
            return NULL;
        }
        return buf;
    }

    return NULL;
}

