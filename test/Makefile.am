AM_CPPFLAGS = -I$(top_srcdir)/include/core \
    -I$(top_srcdir)/include/crypto

LDADD = \
	${top_builddir}/src/libisshe.a

bin_PROGRAMS = \
	test_isshe_crypto.out \
	test_isshe_error.out \
	test_isshe_hmac.out \
	test_isshe_md5.out \
	test_isshe_rand.out \
	test_isshe_sha2.out \
	test_isshe_json.out

test_isshe_crypto_out_SOURCE    = test_isshe_crypto.c

test_isshe_error_out_SOURCE     = test_isshe_error.c

test_isshe_hmac_out_SOURCE      = test_isshe_hmac.c

test_isshe_md5_out_SOURCE       = test_isshe_md5.c

test_isshe_rand_out_SOURCE      = test_isshe_rand.c

test_isshe_sha2_out_SOURCE      = test_isshe_sha2.c

test_isshe_json_out_SOURCE      = test_isshe_json.c
