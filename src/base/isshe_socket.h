#ifndef _ISSHE_SOCKET_H_
#define _ISSHE_SOCKET_H_

#include "isshe_common.h"

#define ISSHE_DEFAULT_LISTEN_BACKLOG    1024  /* Second argument to listen() */

isshe_fd_t isshe_socket(isshe_int_t domain,
    isshe_int_t type, isshe_int_t protocol);

isshe_int_t isshe_setsockopt(isshe_fd_t fd,
    isshe_int_t level, isshe_int_t optname,
    const isshe_void_t *optval, isshe_int_t optlen);

isshe_int_t isshe_bind(isshe_fd_t fd,
    isshe_sa_t *my_addr, isshe_socklen_t addrlen);

isshe_int_t isshe_listen(isshe_fd_t fd,isshe_int_t backlog);

isshe_int_t isshe_accept(isshe_fd_t fd,
    isshe_sa_t *addr, isshe_socklen_t *addrlen);

isshe_int_t isshe_connect(isshe_fd_t fd,
    isshe_sa_t *serv_addr, isshe_socklen_t addrlen);

/*
 * 一些协议无关包裹函数
 */
isshe_int_t isshe_addrinfo_get(
    const isshe_char_t *node, const isshe_char_t *service,
    const isshe_addrinfo_t *hints, isshe_addrinfo_t **res);

isshe_int_t isshe_nameinfo_get(
    const isshe_sa_t *sa, isshe_socklen_t salen,
    isshe_char_t *host, isshe_size_t hostlen,
    isshe_char_t *serv, isshe_size_t servlen, isshe_int_t flags);

isshe_void_t isshe_addrinfo_free(isshe_addrinfo_t *res);

const isshe_char_t *isshe_inet_ntop(
    isshe_sa_family_t af, const isshe_void_t *src,
    isshe_char_t *dst, isshe_socklen_t size);

isshe_int_t isshe_inet_pton(isshe_sa_family_t af,
    const isshe_char_t *src, isshe_void_t *dst);

/*
 * open_client_fd - 在<hostname，port>处打开与服务器的连接，并返回准备读取和写入的套接字描述符。
 *                  这个函数是可重入和协议无关的。
 *
 *     On error, returns:
 *       -2 for getaddrinfo error
 *       -1 with errno set for other errors.
 */
isshe_fd_t open_client_fd(isshe_char_t *hostname, isshe_char_t *port);

/*
 * open_listen_fd - 打开并返回port上的listening套接字; 这个函数是可重入和协议无关的
 * 错误返回:
 *  -2： getaddrinfo error
 *  -1： with errno set for other errors.
 */
isshe_fd_t open_listen_fd(isshe_char_t *port);

isshe_fd_t isshe_open_client_fd(
    isshe_char_t *hostname, isshe_char_t *port);
isshe_fd_t isshe_open_listen_fd(isshe_char_t *port);

ssize_t isshe_sendto(
    isshe_fd_t sockfd, const isshe_void_t *buf, isshe_size_t len,
    isshe_int_t flags, const isshe_sa_t *dest_addr, isshe_socklen_t addrlen);

ssize_t isshe_recvfrom(
    isshe_fd_t sockfd, isshe_void_t *buf, isshe_size_t len,
    isshe_int_t flags, isshe_sa_t *src_addr, isshe_socklen_t *addrlen);

#endif