#include "isshe_common.h"

isshe_int_t
isshe_conn_addr_type_get(const isshe_char_t *addr_str)
{
    // TODO 根据addr字符串识别是什么地址
    return ISSHE_CONN_ADDR_TYPE_IPV4;
}


isshe_int_t
isshe_conn_addr_pton(const isshe_char_t *addr_str,
    isshe_int_t type, void *res_addr, isshe_socklen_t *socklen)
{
    struct sockaddr_in  *in4;
    struct sockaddr_in6 *in6;

    switch (type)
    {
    case ISSHE_CONN_ADDR_TYPE_IPV4:
        *socklen = sizeof(struct sockaddr_in);
        in4 = (struct sockaddr_in *)res_addr;
        in4->sin_family = AF_INET;
        if (inet_pton(in4->sin_family, addr_str, (void *)&in4->sin_addr) != 1) {
            return ISSHE_FAILURE;
        }
        break;
    case ISSHE_CONN_ADDR_TYPE_IPV6:
        *socklen = sizeof(struct sockaddr_in6);
        in6 = (struct sockaddr_in6 *)res_addr;
        in6->sin6_family = AF_INET6;
        if (inet_pton(in6->sin6_family, addr_str, (void *)&in6->sin6_addr) != 1) {
            return ISSHE_FAILURE;
        }
        break;
    case ISSHE_CONN_ADDR_TYPE_DOMAIN:
        // TODO 域名解析后存在到addr，然后直接return
        break;
    }

    return ISSHE_SUCCESS;
}
