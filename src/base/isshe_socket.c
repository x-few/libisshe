#include "isshe_common.h"

isshe_fd_t
isshe_socket(isshe_int_t domain,
    isshe_int_t type, isshe_int_t protocol)
{
    isshe_int_t rc;

    if ((rc = socket(domain, type, protocol)) < 0) {
        isshe_sys_error_exit("socket error");
    }

    return rc;
}

isshe_int_t
isshe_setsockopt(isshe_fd_t fd,
    isshe_int_t level, isshe_int_t optname,
    const isshe_void_t *optval, isshe_int_t optlen)
{
    isshe_int_t rc;

    if ((rc = setsockopt(fd, level, optname, optval, optlen)) < 0) {
        isshe_sys_error_exit("setsockopt error");
    }

    return rc;
}

isshe_int_t
isshe_bind(isshe_fd_t fd,
    isshe_sa_t *my_addr, isshe_socklen_t addrlen)
{
    isshe_int_t rc;

    if ((rc = bind(fd, my_addr, addrlen)) < 0) {
        isshe_sys_error_exit("bind error");
    }

    return rc;
}

isshe_int_t
isshe_listen(isshe_fd_t fd, isshe_int_t backlog)
{
    isshe_int_t rc;

    if ((rc = listen(fd,  backlog)) < 0) {
        isshe_sys_error_exit("listen error");
    }

    return rc;
}

isshe_int_t
isshe_accept(isshe_fd_t fd,
    isshe_sa_t *addr, isshe_socklen_t *addrlen)
{
    isshe_int_t rc;

    if ((rc = accept(fd, addr, addrlen)) < 0) {
        isshe_sys_error_exit("accept error");
    }

    return rc;
}

isshe_int_t
isshe_connect(isshe_fd_t fd,
    isshe_sa_t *serv_addr, isshe_socklen_t addrlen)
{
    isshe_int_t rc;

    if ((rc = connect(fd, serv_addr, addrlen)) < 0) {
        isshe_sys_error_exit("connect error");
    }

    return rc;
}

/*
 * 协议无关包裹函数
 */
isshe_int_t
isshe_addrinfo_get(
    const isshe_char_t *node, const isshe_char_t *service,
    const struct addrinfo *hints, struct addrinfo **res)
{
    isshe_int_t rc;

    if ((rc = getaddrinfo(node, service, hints, res)) != 0) {
        isshe_gai_error_exit(rc, "getaddrinfo error");
    }

    return rc;
}

isshe_int_t
isshe_nameinfo_get(
    const isshe_sa_t *sa, isshe_socklen_t salen,
    isshe_char_t *host, isshe_size_t hostlen,
    isshe_char_t *serv, isshe_size_t servlen, isshe_int_t flags)
{
    isshe_int_t rc;

    if ((rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)) != 0) {
        isshe_gai_error_exit(rc, "getnameinfo error");
    }

    return rc;
}

isshe_void_t
isshe_addrinfo_free(struct addrinfo *res)
{
    freeaddrinfo(res);
}

const isshe_char_t *
isshe_inet_ntop(isshe_sa_family_t af, const isshe_void_t *src,
    isshe_char_t *dst, isshe_socklen_t size)
{
    const isshe_char_t *res;

    if ((res = inet_ntop(af, src, dst, size)) == NULL) {
        isshe_sys_error_exit("inet_ntop error");
    }

    return res;
}

isshe_int_t
isshe_inet_pton(isshe_sa_family_t af,
    const isshe_char_t *src, isshe_void_t *dst)
{
    isshe_int_t rc;

    rc = inet_pton(af, src, dst);
    if (rc == 0) {
        isshe_app_error_exit(
            "inet_pton error: invalid dotted-decimal address");
    }
    else if (rc < 0) {
        isshe_sys_error_exit("inet_pton error");
    }

    return rc;
}

/*
 * 一些方便使用的包裹函数
 */
isshe_fd_t
open_client_fd(isshe_char_t *hostname, isshe_char_t *port)
{
    isshe_int_t clientfd, rc;
    struct addrinfo hints, *listp, *p;

    /* 获取潜在服务器地址列表 */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;  /* Open a connection */
    hints.ai_flags = AI_NUMERICSERV;  /* 数字化的地址/端口字符串，不能是域名 */
    hints.ai_flags |= AI_ADDRCONFIG;  /* 仅当本地系统配置了IPv4/IPv6地址，listp才返回IPv4/IPv6地址 */
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n",
            hostname, port, gai_strerror(rc));
        return -2;
    }

    /* 遍历列表，找一个可以成功连接上的服务器 */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) {
            break; /* Success */
        }

        /* Connect failed, try another */  //line:netp:openclientfd:closefd
        if (close(clientfd) < 0) { 
            fprintf(stderr, "open_client_fd: close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) {   /* All connects failed */
        return -1;
    }

    /* The last connect succeeded */
    return clientfd;
}

isshe_fd_t open_listen_fd(isshe_char_t *port)
{
    struct addrinfo hints, *listp, *p;
    isshe_int_t listenfd, rc, optval=1;

    /* 获取潜在服务器地址列表 */
    /* AI_PASSIVE: 如果getaddrinfo第一个参数为NULL，则返回可以用于bind和accept的地址 */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* 流套接字：TCP... */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;            /* 数字化的地址/端口字符串，不能是域名 */
    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -2;
    }

    /* 遍历列表，绑定一个可以绑定的 */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            continue;  /* Socket failed, try the next */
        }

        /* 消除bind的"Address already in use"错误 */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,    //line:netp:csapp:setsockopt
                   (const isshe_void_t *)&optval , sizeof(isshe_int_t));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        if (close(listenfd) < 0) { /* Bind failed, try the next */
            fprintf(stderr, "open_listen_fd close failed: %s\n", strerror(errno));
            return -1;
        }
    }

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) {   /* No address worked */
        return -1;
    }

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, ISSHE_DEFAULT_LISTEN_BACKLOG) < 0) {
        close(listenfd);
        return -1;
    }

    return listenfd;
}

isshe_fd_t
isshe_open_client_fd(isshe_char_t *hostname, isshe_char_t *port)
{
    isshe_fd_t fd;

    if ((fd = open_client_fd(hostname, port)) < 0) {
        isshe_sys_error_exit("open_client_fd error");
    }

    return fd;
}

isshe_fd_t
isshe_open_listen_fd(isshe_char_t *port)
{
    isshe_fd_t rc;

    if ((rc = open_listen_fd(port)) < 0) {
        isshe_sys_error_exit("open_listen_fd error");
    }

    return rc;
}

isshe_ssize_t
isshe_sendto(isshe_fd_t fd, const isshe_void_t *buf,
    isshe_size_t len, isshe_int_t flags,
    const isshe_sa_t *dest_addr, isshe_socklen_t addrlen)
{
    isshe_ssize_t rc;

    if ((rc = sendto(fd, buf, len, flags, dest_addr, addrlen)) < 0) {
        isshe_sys_error_exit("sendto error");
    }

    return rc;
}

isshe_ssize_t isshe_recvfrom(isshe_fd_t fd, isshe_void_t *buf,
    isshe_size_t len, isshe_int_t flags,
    isshe_sa_t *src_addr, isshe_socklen_t *addrlen)
{
    isshe_ssize_t rc;

    if ((rc = recvfrom(fd, buf, len, flags, src_addr, addrlen)) < 0) {
        isshe_sys_error_exit("recvfrom error");
    }

    return rc;
}

