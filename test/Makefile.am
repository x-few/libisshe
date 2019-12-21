AM_CPPFLAGS = \
	-I$(top_srcdir)/src/core \
	-I$(top_srcdir)/src/crypto

LDADD = \
	${top_builddir}/src/libisshe.a

bin_PROGRAMS = \
	test_isshe_crypto \
	test_isshe_error \
	test_isshe_hmac \
	test_isshe_md5 \
	test_isshe_rand \
	test_isshe_sha2

test_isshe_crypto_SOURCE    = test_isshe_crypto.c

test_isshe_error_SOURCE     = test_isshe_error.c

test_isshe_hmac_SOURCE      = test_isshe_hmac.c

test_isshe_md5_SOURCE       = test_isshe_md5.c

test_isshe_rand_SOURCE      = test_isshe_rand.c

test_isshe_sha2_SOURCE      = test_isshe_sha2.c
