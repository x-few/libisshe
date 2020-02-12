#include "isshe_common.h"

isshe_int_t
isshe_address_type_get(
    const isshe_char_t *addr,isshe_uint8_t addr_len)
{
    isshe_sockaddr_in4_t    in4;
    isshe_sockaddr_in6_t    in6;

    if (!addr || addr_len == 0) {
        return ISSHE_FAILURE;
    }

    if (inet_pton(AF_INET, addr, (void *)&in4.sin_addr) == 1) {
        return ISSHE_ADDR_TYPE_IPV4_TEXT;
    }

    if (inet_pton(AF_INET6, addr, (void *)&in6.sin6_addr) == 1) {
        return ISSHE_ADDR_TYPE_IPV6_TEXT;
    }

    // TODO DOMAIN/IPV4/IPV6未进行判断

    return ISSHE_ADDR_TYPE_DOMAIN;
}

isshe_bool_t
isshe_address_type_is_support(isshe_uint8_t addr_type)
{
    switch (addr_type)
    {
    case ISSHE_ADDR_TYPE_DOMAIN:
    case ISSHE_ADDR_TYPE_IPV6:
    case ISSHE_ADDR_TYPE_IPV4:
    case ISSHE_ADDR_TYPE_IPV6_TEXT:
    case ISSHE_ADDR_TYPE_IPV4_TEXT:
        break;
    default:
        return ISSHE_FALSE;
    }

    return ISSHE_TRUE;
}

isshe_int_t
isshe_address_dns(
    const isshe_char_t *domain_name,
    isshe_addrinfo_t **res)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;  // 协议族
    hints.ai_flags = AI_CANONNAME;

    if (getaddrinfo(domain_name, NULL, &hints, res) != ISSHE_SUCCESS) {
        return ISSHE_FAILURE;
    }
    // 记得使用freeaddrinfo进行释放！
    return ISSHE_SUCCESS;
}

isshe_addrinfo_t *
isshe_conn_select_addrinfo(isshe_addrinfo_t *ai)
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

/*
 * addr_text: 如果是域名/IPv4/IPv6字符串，则以'\0'结尾；否则addr_len指定长度
 */
isshe_int_t
isshe_address_pton(const isshe_char_t *addr_text, isshe_uint8_t addr_len,
    isshe_uint8_t addr_type, isshe_sockaddr_t *res_sockaddr,
    isshe_socklen_t *res_socklen, isshe_log_t *log)
{
    isshe_sockaddr_in4_t    *in4;
    isshe_sockaddr_in6_t    *in6;
    isshe_addrinfo_t        *ais;
    isshe_addrinfo_t        *ai;

    if (!addr_text || !res_sockaddr) {
        isshe_log_error(log, "addr pton: invalid parameters");
        return ISSHE_FAILURE;
    }

    switch (addr_type)
    {
    case ISSHE_ADDR_TYPE_DOMAIN:
        // TODO 域名解析后存在到addr，然后直接return
        if (isshe_address_dns(addr_text, &ais) != ISSHE_SUCCESS || !ais) {
            isshe_log_error(log, "dns failed!");
            return ISSHE_FAILURE;
        }
        ai = isshe_conn_select_addrinfo(ais);
        if (!ai) {
            isshe_log_error(log, "select addrinfo failed!");
            freeaddrinfo(ais);
            return ISSHE_FAILURE;
        }

        *res_socklen = ai->ai_addrlen;
        isshe_memcpy(res_sockaddr, ai->ai_addr, ai->ai_addrlen);
        freeaddrinfo(ais);
        break;
    case ISSHE_ADDR_TYPE_IPV4_TEXT:
        *res_socklen = sizeof(isshe_sockaddr_in4_t);
        in4 = (isshe_sockaddr_in4_t *)res_sockaddr;
        in4->sin_family = AF_INET;
        if (inet_pton(in4->sin_family, addr_text, (void *)&in4->sin_addr) != 1) {
            return ISSHE_FAILURE;
        }
        break;
    case ISSHE_ADDR_TYPE_IPV6_TEXT:
        *res_socklen = sizeof(isshe_sockaddr_in6_t);
        in6 = (isshe_sockaddr_in6_t *)res_sockaddr;
        in6->sin6_family = AF_INET6;
        if (inet_pton(in6->sin6_family, addr_text, (void *)&in6->sin6_addr) != 1) {
            return ISSHE_FAILURE;
        }
        break;
    case ISSHE_ADDR_TYPE_IPV4:
        *res_socklen = sizeof(isshe_sockaddr_in4_t);
        in4 = (isshe_sockaddr_in4_t *)res_sockaddr;
        in4->sin_family = AF_INET;
        // TODO htonl ?
        in4->sin_addr.s_addr = *(in_addr_t *)addr_text;
        break;
    case ISSHE_ADDR_TYPE_IPV6:
        *res_socklen = sizeof(isshe_sockaddr_in6_t);
        in6 = (isshe_sockaddr_in6_t *)res_sockaddr;
        in6->sin6_family = AF_INET6;
        isshe_memcpy(&in6->sin6_addr,
            (isshe_char_t *)addr_text, addr_len);
        break;
    }

    return ISSHE_SUCCESS;
}


isshe_address_t *
isshe_address_create(
    isshe_char_t *addr, isshe_uint8_t addr_len,
    isshe_uint8_t addr_type, isshe_mempool_t *mempool,
    isshe_log_t *log)
{
    isshe_address_t *newaddr;

    if (!addr || addr_len == 0 || !mempool|| !log
    || !isshe_address_type_is_support(addr_type)) {
        isshe_log_error(log, "address create error: invalid parameters");
        return NULL;
    }

    newaddr = (isshe_address_t *)isshe_mpalloc(mempool, sizeof(isshe_address_t));
    if (!newaddr) {
        isshe_log_error(log, "address create error: mpalloc failed");
        return NULL;
    }

    isshe_memzero(newaddr, sizeof(isshe_address_t));

    newaddr->addr = isshe_strdup_mp(addr, addr_len, mempool);
    if (!newaddr->addr) {
        isshe_mpfree(mempool, newaddr, sizeof(isshe_address_t));
        return NULL;
    }
    newaddr->addr_len = addr_len;
    newaddr->addr_type = addr_type;

    return newaddr;
}

isshe_sockaddr_t *
isshe_address_sockaddr_create(isshe_address_t *address,
    isshe_mempool_t *mempool, isshe_log_t *log)
{
    isshe_sockaddr_t        *sockaddr;
    isshe_socklen_t         socklen;
    isshe_char_t            *addr_text;
    isshe_uint8_t           addr_len;
    isshe_sockaddr_in4_t    *in4;
    isshe_sockaddr_in6_t    *in6;

    if (!address || address->sockaddr) {
        isshe_log_error(log, "invalid parameters");
        return NULL;
    }

    sockaddr = (isshe_sockaddr_t *)isshe_mpalloc(
        mempool, sizeof(isshe_sockaddr_t));
    if (!sockaddr) {
        isshe_log_error(log, "mpalloc sockaddr failed");
        return NULL;
    }

    isshe_memzero(sockaddr, sizeof(isshe_sockaddr_t));

    switch (address->addr_type)
    {
    case ISSHE_ADDR_TYPE_DOMAIN:
    case ISSHE_ADDR_TYPE_IPV4_TEXT:
    case ISSHE_ADDR_TYPE_IPV6_TEXT:
        // 分配内存addr_text
        addr_len = address->addr_len + 1;
        addr_text = (isshe_char_t *)isshe_mpalloc(mempool, addr_len);
        if (!addr_text) {
            isshe_log_alert(log, "mpalloc addr_text failed");
            return NULL;
        }
        isshe_memcpy(addr_text, address->addr, address->addr_len);
        addr_text[address->addr_len] = '\0';
        break;
    case ISSHE_ADDR_TYPE_IPV4:
    case ISSHE_ADDR_TYPE_IPV6:
        addr_text = address->addr;
        addr_len = address->addr_len;
        break;
    default:
        isshe_log_error(log, "unsupport addr type");
        return NULL;
    }

    // 进行地址转换/域名解析
    if (isshe_address_pton(address->addr,
    address->addr_len, address->addr_type,
    sockaddr, &socklen, log) == ISSHE_FAILURE) {
        isshe_log_alert(log, "convert addr string to socksaddr failed: %s", addr_text);
        return NULL;
    }

    //isshe_mpfree(mempool, addr_text, addr_len);

    address->sockaddr = sockaddr;
    address->socklen = socklen;

    return sockaddr;
}

isshe_int_t
isshe_address_addrinfo_create()
{
    // TODO
    return ISSHE_SUCCESS;
}

isshe_int_t
isshe_sockaddr_port_set(isshe_sockaddr_t *sockaddr, isshe_uint16_t port)
{
    isshe_sockaddr_in4_t  *in4;

    if (!sockaddr || port == 0) {
        return ISSHE_FAILURE;
    }

    in4 = (isshe_sockaddr_in4_t *)sockaddr;
    in4->sin_port = htons(port);

    return ISSHE_SUCCESS;
}

isshe_int_t
isshe_address_port_set(isshe_address_t *address, isshe_uint16_t port)
{
    if (!address) {
        return ISSHE_FAILURE;
    }
    address->port = port;
    if (address->sockaddr) {
        return isshe_sockaddr_port_set(address->sockaddr, port);
    }

    return ISSHE_SUCCESS;
}

void isshe_address_print(
    isshe_address_t *addr, isshe_log_t *log)
{
    isshe_log_info(log, "--------------------------------------");
    isshe_log_info(log, "addr type      : %d", addr->addr_type);
    isshe_log_info(log, "addr text      : (%d)%s",
        addr->addr_len, addr->addr);
    isshe_log_info(log, "addr type      : %d", addr->port);
    isshe_log_info(log, "--------------------------------------");
}