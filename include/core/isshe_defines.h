#ifndef _ISSHE_DEFINE_H_
#define _ISSHE_DEFINE_H_

#if (defined __NetBSD__)
#define ISSHE_NETBSD
#endif

#if (defined __FreeBSD__)
#define ISSHE_FREEBSD
#endif

#if (defined __OpenBSD__)
#define ISSHE_OPENBSD
#endif

#if (defined __DragonFly__)
#define ISSHE_DRAGONFLY
#endif

#if (defined(__bsdi__) \
    || defined(ISSHE_NETBSD) \
    || defined(ISSHE_FREEBSD) \
    || defined(ISSHE_OPENBSD) \
    || defined(ISSHE_DRAGONFLY))
#define ISSHE_BSD
#endif

#if (defined __linux__)
#define ISSHE_LINUX
#endif

#if (defined __APPLE__)
#define ISSHE_APPLE
#endif

#if (defined __NetBSD__)
#define ISSHE_NETBSD
#endif

#if __BYTE_ORDER == __BIG_ENDIAN__
#define ISSHE_BIG_ENDIAN
#else
#define ISSHE_LITTLE_ENDIAN
#endif

#ifdef OPEN_MAX
#define ISSHE_OPEN_MAX      OPEN_MAX
#else
#define ISSHE_OPEN_MAX      FOPEN_MAX
#endif

#define ISSHE_FILENAME_MAX  FILENAME_MAX

#define ISSHE_AGAIN         1
#define ISSHE_OK            0
#define ISSHE_ERROR         (-1)

#define ISSHE_TRUE          1
#define ISSHE_FALSE         0

#define ISSHE_MAXLINE       4096

#ifdef PATH_MAX                     /* should be in <limits.h> */
#define ISSHE_PATH_MAX      PATH_MAX
#else
#define ISSHE_PATH_MAX      1024    /* max # of characters in a pathname */
#endif

#define	min(a, b)           ((a) < (b) ? (a) : (b))
#define	max(a, b)           ((a) > (b) ? (a) : (b))

#ifdef MAP_FAILED
#define ISSHE_MAP_FAILED    MAP_FAILED
#else
#define ISSHE_MAP_FAILED    ((void *)-1)
#endif

#endif