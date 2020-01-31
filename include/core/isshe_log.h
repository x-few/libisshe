#ifndef _ISSHE_LOG_H_
#define _ISSHE_LOG_H_

#include "isshe_common.h"

#define ISSHE_MAX_LOG_STR   2048
#define ISSHE_MAX_TIME_STR  32
#define ISSHE_MAX_ERRNO_STR 64

//#define ISSHE_LOG_STDERR    0
#define ISSHE_LOG_EMERG     0
#define ISSHE_LOG_ALERT     1
#define ISSHE_LOG_CRIT      2
#define ISSHE_LOG_ERROR     3
#define ISSHE_LOG_WARNING   4
#define ISSHE_LOG_NOTICE    5
#define ISSHE_LOG_INFO      6
#define ISSHE_LOG_DEBUG     7

#define isshe_log_emerg(log, fmt, ...)      isshe_log(ISSHE_LOG_EMERG, log, fmt, ##__VA_ARGS__)
#define isshe_log_alert(log, fmt, ...)      isshe_log(ISSHE_LOG_ALERT, log, fmt, ##__VA_ARGS__)
#define isshe_log_crit(log, fmt, ...)       isshe_log(ISSHE_LOG_CRIT, log, fmt, ##__VA_ARGS__)
#define isshe_log_error(log, fmt, ...)      isshe_log(ISSHE_LOG_ERROR, log, fmt, ##__VA_ARGS__)
#define isshe_log_warning(log, fmt, ...)    isshe_log(ISSHE_LOG_WARNING, log, fmt, ##__VA_ARGS__)
#define isshe_log_notice(log, fmt, ...)     isshe_log(ISSHE_LOG_NOTICE, log, fmt, ##__VA_ARGS__)
#define isshe_log_info(log, fmt, ...)       isshe_log(ISSHE_LOG_INFO, log, fmt, ##__VA_ARGS__)
#define isshe_log_debug(log, fmt, ...)      isshe_log(ISSHE_LOG_DEBUG, log, fmt, ##__VA_ARGS__)

#define isshe_log_emerg_errno(log, ecode, fmt, ...)      isshe_log_errno(ISSHE_LOG_EMERG, log, ecode, fmt, ##__VA_ARGS__)
#define isshe_log_alert_errno(log, ecode, fmt, ...)      isshe_log_errno(ISSHE_LOG_ALERT, log, ecode, fmt, ##__VA_ARGS__)
#define isshe_log_crit_errno(log, ecode, fmt, ...)       isshe_log_errno(ISSHE_LOG_CRIT, log, ecode, fmt, ##__VA_ARGS__)
#define isshe_log_error_errno(log, ecode, fmt, ...)      isshe_log_errno(ISSHE_LOG_ERROR, log, ecode, fmt, ##__VA_ARGS__)
#define isshe_log_warning_errno(log, ecode, fmt, ...)    isshe_log_errno(ISSHE_LOG_WARNING, log, ecode, fmt, ##__VA_ARGS__)
#define isshe_log_notice_errno(log, ecode, fmt, ...)     isshe_log_errno(ISSHE_LOG_NOTICE, log, ecode, fmt, ##__VA_ARGS__)
#define isshe_log_info_errno(log, ecode, fmt, ...)       isshe_log_errno(ISSHE_LOG_INFO, log, ecode, fmt, ##__VA_ARGS__)
#define isshe_log_debug_errno(log, ecode, fmt, ...)      isshe_log_errno(ISSHE_LOG_DEBUG, log, ecode, fmt, ##__VA_ARGS__)

typedef void (*isshe_log_writer_pt) (isshe_log_t *log, isshe_uint_t level,
    isshe_char_t *buf, isshe_size_t len);


struct isshe_log_s {
    isshe_uint_t            level;
    isshe_file_t            *file;
    isshe_log_writer_pt     writer;
    void                   *wdata;
};

isshe_char_t *isshe_log_level_to_string(isshe_int_t level);

isshe_int_t isshe_log_level_to_number(const isshe_char_t *level);

isshe_log_t * isshe_log_create(isshe_uint_t level, isshe_char_t *filename);

void isshe_log_destroy(isshe_log_t *log);

isshe_log_t *isshe_log_instance_get(
    isshe_uint_t level, isshe_char_t *filename);

void isshe_log_instance_free();

void isshe_log(isshe_uint_t level,
    isshe_log_t *log, const char *fmt, ...);

void isshe_log_errno(isshe_uint_t level, isshe_log_t *log,
    isshe_errno_t errcode, const char *fmt, ...);


#endif