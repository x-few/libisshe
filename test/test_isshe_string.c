#include "isshe_common.h"

void test_isshe_snprintf()
{
    isshe_float_t   f = 123.547;
    isshe_float_t   f2 = 123.447;
    isshe_int_t     d = 123;
    isshe_pid_t     P = 12345;
    isshe_time_t    t = time(NULL);
    isshe_char_t    buf[1024];
    isshe_string_t  str = {10, "0123456789"};
    isshe_int_t     rc = 0;

    isshe_memzero(buf, 1024);

    rc = isshe_snprintf(buf, 1024,
        "%d, %.1f, %.0f, %f, %.1f, %.0f, %f, %P, %t, %S\n",
        d, f, f, f, f2, f2, f2, P, t, &str);
    printf("%.0f\n", f);
    printf("rc = %d, buf = (%lu)%s", rc, strlen(buf), buf);
}


int main(void)
{
    test_isshe_snprintf();
}