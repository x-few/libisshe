#include "isshe_common.h"

isshe_int_t
isshe_conn_addr_type_get(const isshe_char_t *addr_str)
{
    // TODO 根据addr字符串识别是什么地址
    return ISSHE_CONN_ADDR_TYPE_IPV4;
}

isshe_int_t
isshe_conn_dns(
    const isshe_char_t *domain_name,
    isshe_addrinfo_t **res)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;  // 协议族
    hints.ai_flags = AI_CANONNAME;

    // 记得使用freeaddrinfo进行释放！
    return getaddrinfo(domain_name, NULL, &hints, res);
}

isshe_addrinfo_t *
isshe_conn_select_addrinfo(
    isshe_addrinfo_t *ai)
{
    while(ai) {
        if (!ai->ai_addr) {
            ai = ai->ai_next;
            continue;
        }

        return ai;
    }

    return NULL;
}


isshe_int_t
isshe_conn_addr_pton(const isshe_char_t *addr_str,
    isshe_int_t type, void *res_addr,
    isshe_socklen_t *socklen)
{
    struct sockaddr_in  *in4;
    struct sockaddr_in6 *in6;
    isshe_addrinfo_t    *ais;
    isshe_addrinfo_t    *ai;

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
        isshe_conn_dns(addr_str, &ais);
        ai = isshe_conn_select_addrinfo(ais);
        if (!ai) {
            freeaddrinfo(ais);
            return ISSHE_FAILURE;
        }

        *socklen = ai->ai_addrlen;
        isshe_memcpy(res_addr, ai->ai_addr, ai->ai_addrlen);
        freeaddrinfo(ais);
        break;
    }

    return ISSHE_SUCCESS;
}

isshe_int_t
isshe_conn_port_set(isshe_sockaddr_t *sockaddr, isshe_uint16_t port)
{
    struct sockaddr_in  *in4;

    if (!sockaddr || port == 0) {
        return ISSHE_FAILURE;
    }

    in4 = (struct sockaddr_in *)sockaddr;
    in4->sin_port = htons(port);

    return ISSHE_SUCCESS;
}
