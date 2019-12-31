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

typedef void (*isshe_log_writer_pt) (isshe_log_t *log, isshe_uint_t level,
    isshe_uchar_t *buf, isshe_size_t len);


struct isshe_log_s {
    isshe_uint_t            level;
    isshe_file_t            *file;
    isshe_log_writer_pt     writer;
    void                   *wdata;
};

isshe_log_t *isshe_log_instance_get(isshe_log_t *hint);
void isshe_log_free(isshe_log_t *log);

void isshe_log(isshe_uint_t level, isshe_log_t *log, 
    isshe_errno_t errcode, const char *fmt, ...);

void isshe_log_debug(isshe_log_t *log, const char *fmt, ...);

void isshe_log_error(isshe_log_t *log, const char *fmt, ...);


#endif