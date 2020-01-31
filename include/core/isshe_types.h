
#ifndef _ISSHE_STDINT_H_
#define _ISSHE_STDINT_H_

#include "isshe_common.h"

#define ISSHE_TRUE      1
#define ISSHE_FALSE     0

typedef int             isshe_int_t;
typedef unsigned int    isshe_uint_t;
typedef uint8_t         isshe_uint8_t;
typedef uint16_t        isshe_uint16_t;
typedef uint32_t        isshe_uint32_t;
typedef uint64_t        isshe_uint64_t;


typedef char            isshe_char_t;
typedef unsigned char   isshe_uchar_t;
typedef size_t          isshe_size_t;

typedef int             isshe_errno_t;

typedef time_t          isshe_time_t;
typedef struct tm       isshe_tm_t;
typedef struct timeval  isshe_timeval_t;


typedef pid_t           isshe_pid_t;

typedef int             isshe_bool_t;

typedef struct sockaddr_storage isshe_sockaddr_t;
typedef int             isshe_socket_t;
typedef socklen_t       isshe_socklen_t;
typedef struct addrinfo isshe_addrinfo_t;

typedef int                         isshe_fd_t;
typedef struct stat                 isshe_file_info_t;
typedef struct isshe_log_s          isshe_log_t;
typedef struct isshe_file_s         isshe_file_t;

typedef struct isshe_connection_s isshe_connection_t;
typedef enum isshe_conn_addr_type_e isshe_conn_addr_type_t;

typedef struct isshe_connpool_s isshe_connpool_t;

typedef struct isshe_mempool_s isshe_mempool_t;
typedef struct isshe_mempool_data_s isshe_mempool_data_t;

typedef struct isshe_process_title_s isshe_process_title_t;


#endif