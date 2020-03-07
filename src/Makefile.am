AM_CPPFLAGS = -I$(top_srcdir)/src/base \
    -I$(top_srcdir)/src/crypto \
    -I$(top_srcdir)/src/json

noinst_LIBRARIES = libisshe.a

ISSHE_CORE_SRCS = \
    base/isshe_debug.c \
    base/isshe_error.c \
    base/isshe_process.c \
    base/isshe_signal.c \
    base/isshe_stdio.c \
    base/isshe_unistd.c \
    base/isshe_pthread.c \
    base/isshe_time.c \
    base/isshe_ipc.c \
    base/isshe_rpc.c \
    base/isshe_socket.c \
    base/isshe_rio.c \
    base/isshe_sbuf.c \
    base/isshe_log.c \
    base/isshe_memory.c \
    base/isshe_process_title.c \
    base/isshe_mempool.c \
    base/isshe_string.c \
    base/isshe_file.c \
    base/isshe_connection.c \
    base/isshe_connpool.c

ISSHE_CRYPTO_SRCS = \
    crypto/isshe_aes.c \
    crypto/isshe_aes_cfb128.c \
    crypto/isshe_md5.c \
    crypto/isshe_sha2.c \
    crypto/isshe_hmac.c \
    crypto/isshe_rand.c

ISSHE_JSON_SRCS = \
    json/isshe_json.c \
    json/isshe_json_print.c \
    json/isshe_json_parse.c

libisshe_a_SOURCES = \
    $(ISSHE_CORE_SRCS) \
    $(ISSHE_CRYPTO_SRCS) \
    $(ISSHE_JSON_SRCS)

