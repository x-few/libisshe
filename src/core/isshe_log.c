#include "isshe_common.h"

// 单例模式
static isshe_log_t *isshe_log_instance = NULL;

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

static isshe_char_t* 
log_levels_color[] = {
    "\033[31memerg\033[0m",
    "\033[31malert\033[0m", 
    "\033[31mcrit\033[0m", 
    "\033[31merror\033[0m", 
    "\033[33mwarning\033[0m", 
    "\033[33mnotice\033[0m", 
    "\033[32minfo\033[0m", 
    "\033[35mdebug\033[0m", 
};

isshe_char_t *
isshe_log_level_to_string(isshe_int_t level)
{
    return (isshe_char_t *)log_levels[level];
}

isshe_int_t
isshe_log_level_to_number(const isshe_char_t *level)
{
    isshe_int_t i;
    isshe_int_t len;

    len = strlen(level);
    for (i = 0; i < sizeof(log_levels); i++) {
        if (strlen(log_levels[i]) == len 
        && isshe_memcmp(log_levels[i], level, len) == 0) {
            return i;
        }
    }

    return ISSHE_FAILURE;
}

isshe_int_t
ngx_log_errno(isshe_char_t *buf, isshe_int_t len, isshe_errno_t errcode)
{
    isshe_int_t n = 0;
    n += isshe_snprintf(buf + n, len, " (%d %s)", errcode, strerror(errcode));
    if (n >= len) {
        buf[len - 1] = '.';
        buf[len - 2] = '.';
        buf[len - 3] = '.';
    }

    return n;
}


void
isshe_log_stderr(isshe_errno_t errcode, const char *fmt, ...)
{
    isshe_char_t   *p;
    va_list         args;
    isshe_char_t   logstr[ISSHE_MAX_LOG_STR];
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

    isshe_write(isshe_stderr, logstr, p - logstr);
}

isshe_log_t *
isshe_log_create(isshe_uint_t level, isshe_char_t *filename)
{
    isshe_log_t *log;
    isshe_file_t *file;

    log = (isshe_log_t *)isshe_malloc(sizeof(isshe_log_t), NULL);
    if (!log) {
        return NULL;
    }

    file = (isshe_file_t *)isshe_malloc(sizeof(isshe_file_t), NULL);
    if (!file) {
        isshe_free(log, NULL);
        return NULL;
    }

    isshe_memzero(log, sizeof(isshe_log_t));
    isshe_memzero(file, sizeof(isshe_file_t));

    // 初始化log
    log->file = file;
    log->level = level;

    if (!filename) {
        log->file->fd = isshe_stderr;
        isshe_log_stderr(0, "[warning] set log file fd to stderr");
        return log;
    }

    file->name.len = strlen(filename) + 1;
    file->name.data = (isshe_char_t *)isshe_malloc(file->name.len, NULL);
    isshe_memcpy(file->name.data, filename, file->name.len);
    file->name.data[file->name.len] = '\0';

    // 打开文件
    file->fd = isshe_open(filename,
        ISSHE_FILE_APPEND | ISSHE_FILE_CREATE_OR_OPEN,
        ISSHE_FILE_DEFAULT_ACCESS);
    if (file->fd == ISSHE_INVALID_FILE) {
        isshe_log_stderr(errno, "[alert] could not open log file: \"%s\"", file->name.data);
    }

    return log;
}


void isshe_log_destroy(isshe_log_t *log)
{
    if (!log) {
        isshe_log_warning(log, "isshe_log_destroy: log == NULL");
        return;
    }

    // 关闭打开的资源
    if (log->file) {
        if (log->file->fd != ISSHE_INVALID_FILE
            && log->file->fd != isshe_stderr
            && log->file->fd != isshe_stdout) {
            isshe_close(log->file->fd);
            log->file->fd = ISSHE_INVALID_FILE;
        }
        if (log->file->name.data && log->file->name.len) {
            isshe_free(log->file->name.data, NULL);
            log->file->name.data = NULL;
            log->file->name.len = 0;
        }

        isshe_free(log->file, NULL);
        log->file = NULL;
    }

    // 释放内存
    isshe_free(log, NULL);
}


isshe_log_t *
isshe_log_instance_get(isshe_uint_t level, isshe_char_t *filename)
{
    if (isshe_log_instance) {
        return isshe_log_instance;
    }

    isshe_log_instance = isshe_log_create(level, filename);

    return isshe_log_instance;
}


void
isshe_log_instance_free()
{
    isshe_log_destroy(isshe_log_instance);
    isshe_log_instance = NULL;
}



static isshe_uint_t
isshe_log_time(isshe_char_t *logstr)
{
    isshe_tm_t      *tm;
    isshe_timeval_t tv;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    isshe_sprintf(logstr, "%4d/%02d/%02d %02d:%02d:%02d.%06ld",
                    tm->tm_year + 1900, tm->tm_mon,
                    tm->tm_mday, tm->tm_hour,
                    tm->tm_min, tm->tm_sec, (long)tv.tv_usec);
    return strlen(logstr);
}

static void 
isshe_log_core(isshe_uint_t level, isshe_log_t *log, 
    isshe_errno_t errcode, const char *fmt, va_list args)
{
    isshe_char_t   logstr[ISSHE_MAX_LOG_STR];
    isshe_char_t   *p;
    isshe_int_t     n;

    n = 0;
    isshe_memzero(logstr, ISSHE_MAX_LOG_STR);
    n += isshe_log_time(logstr);
    if (log->file->fd == isshe_stderr) {
        n += isshe_snprintf(logstr + n, ISSHE_MAX_LOG_STR - n, " [%s]", log_levels_color[level]);
    } else {
        n += isshe_snprintf(logstr + n, ISSHE_MAX_LOG_STR - n, " [%s]", log_levels[level]);
    }
    

    n += isshe_snprintf(logstr + n, ISSHE_MAX_LOG_STR - n, " %d#%d: ", isshe_log_pid, isshe_log_tid);

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
        isshe_write(log->file->fd, logstr, p - logstr);
    }
}

void isshe_log(isshe_uint_t level,
    isshe_log_t *log, const char *fmt, ...)
{
    va_list args;
    if (log->level >= level) {
        va_start(args, fmt);
        isshe_log_core(level, log, 0, fmt, args);
        va_end(args);
    }
}

void isshe_log_errno(isshe_uint_t level, isshe_log_t *log,
    isshe_errno_t errcode, const char *fmt, ...)
{
    va_list args;
    if (log->level >= level) {
        va_start(args, fmt);
        isshe_log_core(level, log, errcode, fmt, args);
        va_end(args);
    }
}
