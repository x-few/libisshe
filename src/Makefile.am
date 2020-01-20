AM_CPPFLAGS = -I$(top_srcdir)/include/core \
    -I$(top_srcdir)/include/crypto

noinst_LIBRARIES = libisshe.a

ISSHE_CORE_SRCS = \
    core/isshe_error.c \
    core/isshe_file.c \
    core/isshe_process.c \
    core/isshe_signal.c \
    core/isshe_stdio.c \
    core/isshe_unistd.c \
    core/isshe_pthread.c \
    core/isshe_time.c \
    core/isshe_ipc.c \
    core/isshe_rpc.c \
    core/isshe_socket.c \
    core/isshe_rio.c \
    core/isshe_sbuf.c \
    core/isshe_json.c \
    core/isshe_log.c \
    core/isshe_memory.c \
    core/isshe_string.c \
    core/isshe_process_title.c \
    core/isshe_mempool.c \
    core/isshe_connpool.c

ISSHE_CRYPTO_SRCS = \
    crypto/isshe_aes.c \
    crypto/isshe_aes_cfb128.c \
    crypto/isshe_md5.c \
    crypto/isshe_sha2.c \
    crypto/isshe_hmac.c \
    crypto/isshe_rand.c

libisshe_a_SOURCES = \
    $(ISSHE_CORE_SRCS) \
    $(ISSHE_CRYPTO_SRCS)

