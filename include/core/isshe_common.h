#ifndef _ISSHE_COMMON_H_
#define _ISSHE_COMMON_H_
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>        // For bool type
//#include <endian.h>
#include <syslog.h>     // for syslog()
#include <sys/msg.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>    // system v
#include <sys/shm.h>    // system v
#include <stddef.h>
#include <pthread.h>
#include <rpc/rpc.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <sys/mman.h>
#include <poll.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>
#include <sys/wait.h>
#include <assert.h>
#include <getopt.h>
#include <execinfo.h>   // for backtrace

#include "isshe_define.h"

#ifdef ISSHE_LINUX
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifdef ISSHE_APPLE
#include <sys/event.h>
#include <sys/syslimits.h>  // for OPEN_MAX
#define va_mode_t   int
#else
#define va_mode_t   mode_t
#endif

#include "isshe_types.h"
#include "isshe_error.h"
#include "isshe_ipc.h"
#include "isshe_json.h"
#include "isshe_string.h"
#include "isshe_file.h"
#include "isshe_log.h"
#include "isshe_debug.h"
#include "isshe_memory.h"
#include "isshe_process.h"
#include "isshe_pthread.h"
#include "isshe_rio.h"
#include "isshe_rpc.h"
#include "isshe_sbuf.h"
#include "isshe_signal.h"
#include "isshe_socket.h"
#include "isshe_stdio.h"
#include "isshe_time.h"
#include "isshe_unistd.h"
#include "isshe_process_title.h"
#include "isshe_connection.h"
#include "isshe_mempool.h"
#include "isshe_connpool.h"

// crypto
#include "isshe_md5.h"
#include "isshe_hmac.h"
#include "isshe_sha2.h"
#include "isshe_aes.h"
#include "isshe_aes_cfg128.h"
#include "isshe_rand.h"

#ifndef OPEN_MAX
#define OPEN_MAX FOPEN_MAX
#endif

#define ISSHE_SUCCESS   0
#define ISSHE_FAILURE   (-1)
#define ISSHE_TRUE      1
#define ISSHE_FALSE     0
#define MAXLINE         4096
#define ISSHE_MAXLINE   MAXLINE

#ifndef	PATH_MAX                /* should be in <limits.h> */
#define	PATH_MAX        1024    /* max # of characters in a pathname */
#endif

#define	min(a,b)    ((a) < (b) ? (a) : (b))
#define	max(a,b)    ((a) > (b) ? (a) : (b))

#endif
