

#include "isshe_common.h"


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
lock_reg(isshe_fd_t fd, isshe_int_t cmd,
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

isshe_void_t
isshe_lock_reg(isshe_fd_t fd, isshe_int_t cmd,
    isshe_int_t type, isshe_off_t offset,
    isshe_int_t whence, isshe_off_t len)
{
    if (lock_reg(fd, cmd, type, offset, whence, len) == ISSHE_FAILURE) {
        isshe_sys_error_exit("lock_reg error");
    }
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
        if ( (fd = open(pathname, oflag, mode)) == ISSHE_FAILURE ) {
            return ISSHE_INVALID_FILE;
        }
        va_end(ap);
    } else {
        if ( (fd = open(pathname, oflag)) == ISSHE_FAILURE ) {
            return ISSHE_INVALID_FILE;
        }
    }

    return fd;
}

isshe_void_t isshe_close(isshe_fd_t fd)
{
    if (close(fd) == ISSHE_FAILURE) {
        isshe_sys_error_exit("close error");
    }
}

isshe_ssize_t
isshe_read(isshe_fd_t fd, isshe_void_t *ptr, isshe_size_t nbytes)
{
    isshe_ssize_t n;

    if ( (n = read(fd, ptr, nbytes)) == ISSHE_FAILURE) {
        isshe_sys_error_exit("read error");
    }

    return(n);
}

isshe_void_t
isshe_write(isshe_fd_t fd, isshe_void_t *ptr, isshe_size_t nbytes)
{
    if (write(fd, ptr, nbytes) != nbytes){
        isshe_sys_error_exit("write error");
    }
}

isshe_void_t
isshe_unlink(const isshe_char_t *pathname)
{
    if (unlink(pathname) == ISSHE_FAILURE) {
        isshe_sys_error_exit("unlink error for %s", pathname);
    }
}

static isshe_ssize_t
restart_read(isshe_fd_t fd, isshe_char_t *ptr)
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

static isshe_ssize_t
readline(isshe_fd_t fd, isshe_void_t *vptr, isshe_size_t maxlen)
{
    isshe_int_t n, rc;
    isshe_char_t c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = restart_read(fd, &c)) == 1) {
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

isshe_ssize_t
isshe_readline(isshe_fd_t fd,
    isshe_void_t *ptr, isshe_size_t maxlen)
{
    isshe_ssize_t n;

    if ( (n = readline(fd, ptr, maxlen)) < 0) {
        isshe_sys_error_exit("readline error");
    }

    return(n);
}

isshe_off_t
isshe_lseek(isshe_fd_t fd, isshe_off_t offset, isshe_int_t whence)
{
    isshe_off_t pos;

    if ( (pos = lseek(fd, offset, whence)) == (isshe_off_t) ISSHE_FAILURE) {
        isshe_sys_error_exit("lseek error");
    }

    return(pos);
}

isshe_void_t
isshe_ftruncate(isshe_fd_t fd, isshe_off_t length)
{
    if (ftruncate(fd, length) == ISSHE_FAILURE) {
        isshe_sys_error_exit("ftruncate error");
    }
}

isshe_void_t
isshe_fstat(isshe_fd_t fd, struct stat *ptr)
{
    if (fstat(fd, ptr) == -1) {
        isshe_sys_error_exit("fstat error");
    }
}

isshe_char_t *
isshe_read_all(isshe_fd_t fd, isshe_ssize_t *reslen)
{
    isshe_finfo_t info;
    isshe_char_t *buf;
    isshe_ssize_t len;

    isshe_fstat(fd, &info);
    if (info.st_size > 0) {
        buf = isshe_malloc(info.st_size, NULL);           // remember to free
        len = isshe_read(fd, buf, info.st_size);
        *reslen = len;
        if (len != info.st_size) {
            return NULL;
        }
        return buf;
    }

    return NULL;
}

