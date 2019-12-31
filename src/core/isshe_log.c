#include "isshe_common.h"

// 单例模式
static isshe_log_t *isshe_log = NULL;
static isshe_filt_t isshe_log_file;

//
static isshe_char_t* 
log_levels[] = {
    "emerg",
    "alert",
    "crit",
    "error",
    "warning",
    "notice",
    "info",
    "debug"
};

isshe_int_t
ngx_log_errno(isshe_uchar_t *buf, isshe_int_t len, isshe_errno_t errcode)
{
    isshe_int_t n = 0;
    n += isshe_snprintf(buf + n, len, " (%d %s)", errcode, strerror(errcode));
    if (n >= len) {
        buf[len - 1] = '.';
        buf[len - 2] = '.';
        buf[len - 3] = '.'
    }

    return n;
}


void
isshe_log_stderr(isshe_errno_t errcode, const char *fmt, ...)
{
    isshe_uchar_t   *p;
    va_list         args;
    isshe_uchar_t   logstr[ISSHE_MAX_LOG_STR];
    isshe_int_t     n;

    n = 0;
    va_start(args, fmt);
    n += isshe_vsnprintf(logstr + n, ISSHE_MAX_LOG_STR - n, fmt, args);
    va_end(args);

    if (errcode) {
        n += ngx_log_errno(logstr + n, ISSHE_MAX_LOG_STR - n, errcode);
    }

    if (n > ISSHE_MAX_LOG_STR - ISSHE_LINEFEED_SIZE) {
        n = ISSHE_MAX_LOG_STR - ISSHE_LINEFEED_SIZE;
    }

    p = logstr + n;
    isshe_linefeed(p);  // p++

    write(isshe_stderr, logstr, p - logstr);
}


isshe_log_t *
isshe_log_instance_get(isshe_log_t *hint)
{
    if (isshe_log) {
        return isshe_log;
    }

    isshe_log = (isshe_log_t *)isshe_malloc(sizeof(isshe_log_t));
    if (!isshe_log) {
        return NULL;
    }

    // 初始化log
    isshe_log->file = &isshe_log_file;
    isshe_log->level = ISSHE_LOG_NOTICE;
    if (hint->level) {
        isshe_log->level = hint->level;
    }

    if (!hint->file || hint->file.name.len == 0) {
        isshe_log->file.fd = isshe_stderr;
        return isshe_log;
    }

    isshe_log_file.name.data = (isshe_char_t *)isshe_malloc(hint->file.name.len + 1);
    isshe_log_file.name.len = hint->file.name;
    isshe_memcpy(isshe_log_file.name.data, hint->file.name.data, isshe_log_file.name.len);
    isshe_log_file.name.data[isshe_log_file.name.len] = '\0';

    // 打开文件
    isshe_log_file.fd = isshe_open(isshe_log_file.name.data, ISSHE_FILE_APPEND, 
                                ISSHE_FILE_CREATE_OR_OPEN,
                                ISSHE_FILE_DEFAULT_ACCESS);
    if (isshe_log_file.fd == ISSHE_INVALID_FILE) {
        isshe_log_stderr(errno, "[alert] could not open log file: \"%s\"", isshe_log_file.name.data);
    }

    return isshe_log;
}

void isshe_log_free(isshe_log_t *log)
{
    // 关闭打开的资源
    if (log->file) {
        if (log->file.fd != ISSHE_INVALID_FILE) {
            isshe_close(log->file.fd);
            log->file.fd = ISSHE_INVALID_FILE;
        }
        if (log->file.name.data && log->file.name.len) {
            isshe_free(log->file.name.data);
            log->file.name.data = NULL;
            log->file.name.len = 0;
        }
    }
    // 释放内存
    isshe_free(log);
}

static isshe_uint_t *
isshe_log_time(isshe_uchar_t *logstr)
{
    isshe_time_t    raw_time;
    isshe_tm_t      *tm;

    time(&raw_time);
    tm = localtime(&raw_time);

    isshe_sprintf(logstr, "%4d/%02d/%02d %02d:%02d:%02d",
                    tm.tm_year, tm.tm_mon,
                    tm.tm_mday, tm.tm_hour,
                    tm.tm_min, tm.tm_sec);
    return isshe_strlen(logstr);
}

void 
isshe_log_core(isshe_uint_t level, isshe_log_t *log, 
    ngx_err_t errcode, const char *fmt, va_list args)
{
    isshe_uchar_t   logstr[ISSHE_MAX_LOG_STR];
    isshe_uchar_t   *p;
    isshe_int_t     n;

    n = 0;
    isshe_memzero(logstr, ISSHE_MAX_LOG_STR);

    n += isshe_log_time(logstr);

    n += isshe_snprintf(logstr + n, ISSHE_MAX_LOG_STR - n, " [%s] ", log_levels[level]);

    n += isshe_snprintf(logstr + n, ISSHE_MAX_LOG_STR - n, " [PID=%d] [TID=%d] ", isshe_getpid(), isshe_gettid());


    n += vsnprintf(logstr + n, ISSHE_MAX_LOG_STR - n, fmt, args);

    if (n < ISSHE_MAX_LOG_STR && errcode) {
        n += isshe_snprintf(logstr + n, ISSHE_MAX_LOG_STR - n, ": %s", strerror(errcode));
    }

    if (n > ISSHE_MAX_LOG_STR - ISSHE_LINEFEED_SIZE) {
        n = ISSHE_MAX_LOG_STR - ISSHE_LINEFEED_SIZE;
    }

    p = logstr + n;
    isshe_linefeed(p);  // p++

    if (log->writer) {
        log->writer(log, level, logstr, p - logstr);
    } else {
        n = isshe_write_fd(log->file->fd, logstr, p - logstr);
    }
}
