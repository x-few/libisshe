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

#if (defined __linux__)
#define ISSHE_LINUX
#endif

#if (defined __APPLE__)
#define ISSHE_APPLE
#endif

#if (defined __NetBSD__)
#define ISSHE_NETBSD
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN__
#define ISSHE_LITTLE_ENDIAN
#else
#define ISSHE_BIG_ENDIAN
#endif


#endif