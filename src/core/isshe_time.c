#include "isshe_common.h"

isshe_int_t
isshe_gettimeofday(
    isshe_timeval_t *tv, isshe_timezone_t *tz)
{
    isshe_int_t rc;

    if ((rc = gettimeofday(tv, tz)) < ISSHE_SUCCESS) {
        isshe_sys_error_exit("gettimeofday error");
    }

    return rc;
}

isshe_char_t *isshe_gf_time(isshe_void_t)
{
    isshe_timeval_t  tv;
    static isshe_char_t     str[30];
    isshe_char_t            *ptr;

    isshe_gettimeofday(&tv, NULL);

    ptr = ctime(&tv.tv_sec);
    strcpy(str, &ptr[11]);
        /* Fri Sep 13 00:00:00 1986\n\0 */
        /* 0123456789012345678901234 5  */
    snprintf(str+8, sizeof(str)-8, ".%06ld", (long)tv.tv_usec);

    return(str);
}