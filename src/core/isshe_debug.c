#include "isshe_common.h"


void isshe_debug_print_addr(struct sockaddr *sockaddr, isshe_log_t *log)
{
    char                    addr[INET6_ADDRSTRLEN];
    struct sockaddr_in      *in4;
    struct sockaddr_in6     *in6;
    int                     port;

    isshe_memzero(addr, INET6_ADDRSTRLEN);
    if (sockaddr->sa_family == AF_INET6) {
        in6 = (struct sockaddr_in6 *)sockaddr;
        inet_ntop(AF_INET6, &in6->sin6_addr, addr, sizeof(struct sockaddr_in6));
        port = in6->sin6_port;
    } else {
        in4 = (struct sockaddr_in *)sockaddr;
        inet_ntop(AF_INET, &in4->sin_addr, addr, sizeof(struct sockaddr_in));
        port = in4->sin_port;
    }

    // 打印对端的信息
    if (log) {
        isshe_log_debug(log, "addr: %s, port: %d", addr, ntohs(port));
    } else {
        printf("addr:%s, port:%d\n", addr, ntohs(port));
    }
}

void
isshe_debug_print_buffer(char *buf, int buf_len, int print_len)
{
    size_t n;
    size_t i;

    n = buf_len > print_len ? print_len : buf_len;
    for (i=0; i<n; ++i) {
        printf("%u(%x), ", buf[i], buf[i]);
    }
}
