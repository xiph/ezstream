#include <check.h>
#include <limits.h>
#include <netdb.h>

#include "cfg_private.h"
#include "log.h"

#define TEST_EMPTYSTR(s, g)     do {                    \
	const char	*errstr;			\
							\
	errstr = NULL;					\
	ck_assert_int_eq(s(NULL, &errstr), -1); 	\
	ck_assert_str_eq(errstr, "empty");		\
							\
	errstr = NULL;					\
	ck_assert_int_eq(s("", &errstr), -1);		\
	ck_assert_str_eq(errstr, "empty");		\
							\
} while (0)

#define TEST_XSTRDUP(s, g)	do {			\
	TEST_EMPTYSTR(s, g);				\
							\
	ck_assert_int_eq(s("check_cfg", NULL), 0);	\
	ck_assert_str_eq(g(), "check_cfg");		\
} while (0)

#define TEST_STRLCPY(s, g, l)	do {			\
	char		 buf[l + 1];			\
	const char	*errstr2;			\
							\
	TEST_XSTRDUP(s, g);				\
							\
	errstr2 = NULL;					\
	memset(buf, 'A', sizeof(buf) - 1);		\
	buf[sizeof(buf) - 1] = '\0';			\
	ck_assert_int_eq(s(buf, &errstr2), -1);		\
	ck_assert_ptr_ne(errstr2, NULL); 		\
} while (0)

#define TEST_BOOLEAN(s, g)	do {			\
	const char	*errstr2;			\
							\
	TEST_EMPTYSTR(s, g);				\
							\
	errstr2 = NULL;					\
	ck_assert_int_eq(s("BOGUS", &errstr2), -1);	\
	ck_assert_str_eq(errstr2, "invalid");		\
							\
	ck_assert_int_eq(s("tRuE", NULL), 0);		\
	ck_assert_int_eq(g(), 1);			\
	ck_assert_int_eq(s("YeS", NULL), 0);		\
	ck_assert_int_eq(g(), 1);			\
	ck_assert_int_eq(s("1", NULL), 0);		\
	ck_assert_int_eq(g(), 1);			\
							\
	ck_assert_int_eq(s("FaLsE", NULL), 0);		\
	ck_assert_int_eq(g(), 0);			\
	ck_assert_int_eq(s("nO", NULL), 0);		\
	ck_assert_int_eq(g(), 0);			\
	ck_assert_int_eq(s("0", NULL), 0);		\
	ck_assert_int_eq(g(), 0);			\
} while (0)

#define TEST_UINTNUM(s, g)	do {				\
	const char	*errstr2;				\
								\
	TEST_EMPTYSTR(s, g);					\
								\
	errstr2 = NULL; 					\
	ck_assert_int_eq(s("-1", &errstr2), -1);		\
	ck_assert_ptr_ne(errstr2, NULL);			\
								\
	errstr2 = NULL; 					\
	ck_assert_int_eq(s("4294967296", &errstr2), -1);	\
	ck_assert_ptr_ne(errstr2, NULL);			\
								\
	ck_assert_int_eq(s("20", NULL), 0);			\
	ck_assert_uint_eq(g(), 20);				\
} while (0)

#define TEST_INTNUM(s, g)	do {				\
	const char	*errstr2;				\
								\
	TEST_EMPTYSTR(s, g);					\
								\
	errstr2 = NULL; 					\
	ck_assert_int_eq(s("-2147483649", &errstr2), -1);		\
	ck_assert_ptr_ne(errstr2, NULL);			\
								\
	errstr2 = NULL; 					\
	ck_assert_int_eq(s("2147483648", &errstr2), -1);	\
	ck_assert_ptr_ne(errstr2, NULL);			\
								\
	ck_assert_int_eq(s("20", NULL), 0);			\
	ck_assert_int_eq(g(), 20);				\
} while (0)

Suite * cfg_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_reload)
{
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-ok.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_reload(), 0);
	ck_assert_int_eq(cfg_reload(), 0);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_reload(), -1);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad2.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_reload(), -1);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad3.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_reload(), -1);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad4.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_reload(), -1);
}
END_TEST

START_TEST(test_stream_str2fmt)
{
	enum cfg_stream_format	fmt;

	ck_assert_int_eq(cfg_stream_str2fmt(CFG_SFMT_VORBIS, &fmt), 0);
	ck_assert_int_eq(fmt, CFG_STREAM_VORBIS);
	ck_assert_int_eq(cfg_stream_str2fmt(CFG_SFMT_MP3, &fmt), 0);
	ck_assert_int_eq(fmt, CFG_STREAM_MP3);
	ck_assert_int_eq(cfg_stream_str2fmt(CFG_SFMT_THEORA, &fmt), 0);
	ck_assert_int_eq(fmt, CFG_STREAM_THEORA);
	ck_assert_int_eq(cfg_stream_str2fmt("<something else>", &fmt), -1);
}
END_TEST

START_TEST(test_stream_fmt2str)
{
	ck_assert_str_eq(cfg_stream_fmt2str(CFG_STREAM_VORBIS),
	    CFG_SFMT_VORBIS);
	ck_assert_str_eq(cfg_stream_fmt2str(CFG_STREAM_MP3),
	    CFG_SFMT_MP3);
	ck_assert_str_eq(cfg_stream_fmt2str(CFG_STREAM_THEORA),
	    CFG_SFMT_THEORA);
	ck_assert_ptr_eq(cfg_stream_fmt2str(CFG_STREAM_INVALID), NULL);
}
END_TEST

START_TEST(test_file_check)
{
	ck_assert_int_eq(cfg_file_check(""), -1);
	ck_assert_int_eq(cfg_file_check(SRCDIR "/check_cfg.c"), 0);
}
END_TEST

START_TEST(test_program_name)
{
	TEST_STRLCPY(cfg_set_program_name, cfg_get_program_name, PATH_MAX);
}
END_TEST

START_TEST(test_program_config_type)
{
	const char	*errstr = NULL;

	ck_assert_int_eq(cfg_set_program_config_type(-2, &errstr), -1);
	ck_assert_str_eq(errstr, "invalid");
	ck_assert_int_eq(cfg_set_program_config_type(CFG_TYPE_XMLFILE, NULL),
	    0);
	ck_assert_int_eq(cfg_get_program_config_type(), CFG_TYPE_XMLFILE);
}
END_TEST

START_TEST(test_program_config_file)
{
	TEST_STRLCPY(cfg_set_program_config_file, cfg_get_program_config_file,
	    PATH_MAX);
}
END_TEST

START_TEST(test_program_quiet_stderr)
{
	ck_assert_int_eq(cfg_set_program_quiet_stderr(-1, NULL), 0);
	ck_assert_int_ne(cfg_get_program_quiet_stderr(), 0);
}
END_TEST

START_TEST(test_program_rtstatus_output)
{
	ck_assert_int_eq(cfg_set_program_rtstatus_output(-1, NULL), 0);
	ck_assert_int_ne(cfg_get_program_rtstatus_output(), 0);
}
END_TEST

START_TEST(test_program_verbosity)
{
	ck_assert_int_eq(cfg_set_program_verbosity(2000, NULL), 0);
	ck_assert_int_eq(cfg_get_program_verbosity(), 2000);
}
END_TEST

START_TEST(test_server_protocol)
{
	const char	*errstr2;

	TEST_EMPTYSTR(cfg_set_server_protocol, cfg_get_server_protocol);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_set_server_protocol("invalid", &errstr2), -1);
	ck_assert_str_eq(errstr2, "unsupported");

	ck_assert_int_eq(cfg_set_server_protocol("hTtP", NULL), 0);
	ck_assert_int_eq(cfg_get_server_protocol(), CFG_PROTO_HTTP);
	ck_assert_str_eq(cfg_get_server_protocol_str(), "http");
	ck_assert_int_eq(cfg_set_server_protocol("HtTpS", NULL), 0);
	ck_assert_int_eq(cfg_get_server_protocol(), CFG_PROTO_HTTPS);
	ck_assert_str_eq(cfg_get_server_protocol_str(), "https");
}
END_TEST

START_TEST(test_server_hostname)
{
	TEST_STRLCPY(cfg_set_server_hostname, cfg_get_server_hostname,
	    NI_MAXHOST);
}
END_TEST

START_TEST(test_server_port)
{
	const char	*errstr2;

	TEST_EMPTYSTR(cfg_set_server_port, cfg_get_server_port);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_set_server_port("0", &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_set_server_port("65536", &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);

	ck_assert_int_eq(cfg_set_server_port("8000", NULL), 0);
	ck_assert_uint_eq(cfg_get_server_port(), 8000);
}
END_TEST

START_TEST(test_server_user)
{
	TEST_STRLCPY(cfg_set_server_user, cfg_get_server_user, UCREDS_SIZE);
}
END_TEST

START_TEST(test_server_password)
{
	TEST_STRLCPY(cfg_set_server_password, cfg_get_server_password,
	    UCREDS_SIZE);
}
END_TEST

START_TEST(test_server_ca_dir)
{
	TEST_STRLCPY(cfg_set_server_ca_dir, cfg_get_server_ca_dir, PATH_MAX);
}
END_TEST

START_TEST(test_server_ca_file)
{
	TEST_STRLCPY(cfg_set_server_ca_file, cfg_get_server_ca_file, PATH_MAX);
}
END_TEST

START_TEST(test_server_client_cert)
{
	TEST_STRLCPY(cfg_set_server_client_cert, cfg_get_server_client_cert,
	    PATH_MAX);
}
END_TEST

START_TEST(test_server_client_key)
{
	TEST_STRLCPY(cfg_set_server_client_key, cfg_get_server_client_key,
	    PATH_MAX);
}
END_TEST

START_TEST(test_server_reconnect_attempts)
{
	TEST_UINTNUM(cfg_set_server_reconnect_attempts,
	    cfg_get_server_reconnect_attempts);
}
END_TEST

START_TEST(test_stream_mountpoint)
{
	TEST_XSTRDUP(cfg_set_stream_mountpoint, cfg_get_stream_mountpoint);
}
END_TEST

START_TEST(test_stream_name)
{
	TEST_XSTRDUP(cfg_set_stream_name, cfg_get_stream_name);
}
END_TEST

START_TEST(test_stream_url)
{
	TEST_XSTRDUP(cfg_set_stream_url, cfg_get_stream_url);
}
END_TEST

START_TEST(test_stream_genre)
{
	TEST_XSTRDUP(cfg_set_stream_genre, cfg_get_stream_genre);
}
END_TEST

START_TEST(test_stream_description)
{
	TEST_XSTRDUP(cfg_set_stream_description, cfg_get_stream_description);
}
END_TEST

START_TEST(test_stream_quality)
{
	TEST_XSTRDUP(cfg_set_stream_quality, cfg_get_stream_quality);
}
END_TEST

START_TEST(test_stream_bitrate)
{
	TEST_XSTRDUP(cfg_set_stream_bitrate, cfg_get_stream_bitrate);
}
END_TEST

START_TEST(test_stream_samplerate)
{
	TEST_XSTRDUP(cfg_set_stream_samplerate, cfg_get_stream_samplerate);
}
END_TEST

START_TEST(test_stream_channels)
{
	TEST_XSTRDUP(cfg_set_stream_channels, cfg_get_stream_channels);
}
END_TEST

START_TEST(test_stream_server_public)
{
	TEST_BOOLEAN(cfg_set_stream_server_public,
	    cfg_get_stream_server_public);
}
END_TEST

START_TEST(test_stream_format)
{
	const char	*errstr2;

	TEST_EMPTYSTR(cfg_set_stream_format, cfg_get_stream_format);

	ck_assert_int_eq(cfg_set_stream_format("<something else>", &errstr2),
	    -1);
	ck_assert_str_eq(errstr2, "unsupported stream format");

	ck_assert_int_eq(cfg_set_stream_format(CFG_SFMT_VORBIS, NULL), 0);
	ck_assert_int_eq(cfg_get_stream_format(), CFG_STREAM_VORBIS);
}
END_TEST

START_TEST(test_stream_encoder)
{
	TEST_XSTRDUP(cfg_set_stream_encoder, cfg_get_stream_encoder);
}
END_TEST

START_TEST(test_media_type)
{
	const char	*errstr2;

	TEST_EMPTYSTR(cfg_set_media_type, cfg_get_media_type);

	ck_assert_int_eq(cfg_set_media_type("<something else>", &errstr2), -1);
	ck_assert_str_eq(errstr2, "unsupported");

	ck_assert_int_eq(cfg_set_media_type("aUtOdEtEcT", NULL), 0);
	ck_assert_int_eq(cfg_get_media_type(), CFG_MEDIA_AUTODETECT);
	ck_assert_int_eq(cfg_set_media_type("FiLe", NULL), 0);
	ck_assert_int_eq(cfg_get_media_type(), CFG_MEDIA_FILE);
	ck_assert_int_eq(cfg_set_media_type("pLaYlIsT", NULL), 0);
	ck_assert_int_eq(cfg_get_media_type(), CFG_MEDIA_PLAYLIST);
	ck_assert_int_eq(cfg_set_media_type("PrOgRaM", NULL), 0);
	ck_assert_int_eq(cfg_get_media_type(), CFG_MEDIA_PROGRAM);
	ck_assert_int_eq(cfg_set_media_type("sTdIn", NULL), 0);
	ck_assert_int_eq(cfg_get_media_type(), CFG_MEDIA_STDIN);
}
END_TEST

START_TEST(test_media_filename)
{
	TEST_STRLCPY(cfg_set_media_filename, cfg_get_media_filename, PATH_MAX);
}
END_TEST

START_TEST(test_media_shuffle)
{
	TEST_BOOLEAN(cfg_set_media_shuffle, cfg_get_media_shuffle);
}
END_TEST

START_TEST(test_media_stream_once)
{
	TEST_BOOLEAN(cfg_set_media_stream_once, cfg_get_media_stream_once);
}
END_TEST

START_TEST(test_metadata_program)
{
	TEST_STRLCPY(cfg_set_metadata_program, cfg_get_metadata_program,
	    PATH_MAX);
}
END_TEST

START_TEST(test_metadata_format_str)
{
	TEST_XSTRDUP(cfg_set_metadata_format_str,
	    cfg_get_metadata_format_str);

	/* XXX: Missing CHECKPH tests */
}
END_TEST

START_TEST(test_metadata_refresh_interval)
{
	ck_assert_int_eq(cfg_get_metadata_refresh_interval(), -1);

	TEST_INTNUM(cfg_set_metadata_refresh_interval,
	    cfg_get_metadata_refresh_interval);
}
END_TEST

START_TEST(test_metadata_normalize_strings)
{
	TEST_BOOLEAN(cfg_set_metadata_normalize_strings,
	    cfg_get_metadata_normalize_strings);
}
END_TEST

START_TEST(test_metadata_no_updates)
{
	TEST_BOOLEAN(cfg_set_metadata_no_updates,
	    cfg_get_metadata_no_updates);
}
END_TEST

START_TEST(test_decoder_get)
{
	cfg_decoder_t	dec, dec2;

	ck_assert_ptr_eq(cfg_decoder_get(NULL), NULL);
	ck_assert_ptr_eq(cfg_decoder_get(""), NULL);

	dec = cfg_decoder_get("TeSt");
	dec2 = cfg_decoder_get("test");
	ck_assert_ptr_eq(dec, dec2);
}
END_TEST

#define TEST_DEC_XSTRDUP(s, g)	do {			\
	cfg_decoder_t	 dec = cfg_decoder_get(#s);	\
	const char	*errstr;			\
							\
	errstr = NULL;					\
	ck_assert_int_ne(s(dec, NULL, &errstr), 0);	\
	ck_assert_str_eq(errstr, "empty");		\
	ck_assert_int_ne(s(dec, "", NULL), 0);		\
							\
	ck_assert_int_eq(s(dec, "test", NULL), 0);	\
	ck_assert_str_eq(g(dec), "test");		\
							\
	ck_assert_int_eq(s(dec, #s, NULL), 0);		\
	ck_assert_str_eq(g(dec), #s);			\
} while (0)

START_TEST(test_decoder_set_name)
{
	TEST_DEC_XSTRDUP(cfg_decoder_set_name, cfg_decoder_get_name);
}
END_TEST

START_TEST(test_decoder_set_program)
{
	TEST_DEC_XSTRDUP(cfg_decoder_set_program, cfg_decoder_get_program);
}
END_TEST

START_TEST(test_encoder_get)
{
	cfg_encoder_t	enc, enc2;

	ck_assert_ptr_eq(cfg_encoder_get(NULL), NULL);
	ck_assert_ptr_eq(cfg_encoder_get(""), NULL);

	enc = cfg_encoder_get("TeSt");
	enc2 = cfg_encoder_get("test");
	ck_assert_ptr_eq(enc, enc2);
}
END_TEST

#define TEST_ENC_XSTRDUP(s, g)	do {			\
	cfg_encoder_t	 enc = cfg_encoder_get(#s);	\
	const char	*errstr;			\
							\
	errstr = NULL;					\
	ck_assert_int_ne(s(enc, NULL, &errstr), 0);	\
	ck_assert_str_eq(errstr, "empty");		\
	ck_assert_int_ne(s(enc, "", NULL), 0);		\
							\
	ck_assert_int_eq(s(enc, "test", NULL), 0);	\
	ck_assert_str_eq(g(enc), "test");		\
							\
	ck_assert_int_eq(s(enc, #s, NULL), 0);		\
	ck_assert_str_eq(g(enc), #s);			\
} while (0)

START_TEST(test_encoder_set_name)
{
	TEST_ENC_XSTRDUP(cfg_encoder_set_name, cfg_encoder_get_name);
}
END_TEST

START_TEST(test_encoder_set_program)
{
	TEST_ENC_XSTRDUP(cfg_encoder_set_program, cfg_encoder_get_program);
}
END_TEST

Suite *
cfg_suite(void)
{
	Suite	*s;
	TCase	*tc_core;
	TCase	*tc_program;
	TCase	*tc_server;
	TCase	*tc_stream;
	TCase	*tc_media;
	TCase	*tc_metadata;
	TCase	*tc_decoder;
	TCase	*tc_encoder;

	s = suite_create("Config");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup_checked, teardown_checked);
	tcase_add_test(tc_core, test_reload);
	tcase_add_test(tc_core, test_stream_str2fmt);
	tcase_add_test(tc_core, test_stream_fmt2str);
	tcase_add_test(tc_core, test_file_check);
	suite_add_tcase(s, tc_core);

	tc_program = tcase_create("Program");
	tcase_add_checked_fixture(tc_program, setup_checked,
	    teardown_checked);
	tcase_add_test(tc_program, test_program_name);
	tcase_add_test(tc_program, test_program_config_type);
	tcase_add_test(tc_program, test_program_config_file);
	tcase_add_test(tc_program, test_program_quiet_stderr);
	tcase_add_test(tc_program, test_program_rtstatus_output);
	tcase_add_test(tc_program, test_program_verbosity);
	suite_add_tcase(s, tc_program);

	tc_server = tcase_create("Server");
	tcase_add_checked_fixture(tc_server, setup_checked, teardown_checked);
	tcase_add_test(tc_server, test_server_protocol);
	tcase_add_test(tc_server, test_server_hostname);
	tcase_add_test(tc_server, test_server_port);
	tcase_add_test(tc_server, test_server_user);
	tcase_add_test(tc_server, test_server_password);
	tcase_add_test(tc_server, test_server_ca_dir);
	tcase_add_test(tc_server, test_server_ca_file);
	tcase_add_test(tc_server, test_server_client_cert);
	tcase_add_test(tc_server, test_server_client_key);
	tcase_add_test(tc_server, test_server_reconnect_attempts);
	suite_add_tcase(s, tc_server);

	tc_stream = tcase_create("Stream");
	tcase_add_checked_fixture(tc_stream, setup_checked, teardown_checked);
	tcase_add_test(tc_stream, test_stream_mountpoint);
	tcase_add_test(tc_stream, test_stream_name);
	tcase_add_test(tc_stream, test_stream_url);
	tcase_add_test(tc_stream, test_stream_genre);
	tcase_add_test(tc_stream, test_stream_description);
	tcase_add_test(tc_stream, test_stream_quality);
	tcase_add_test(tc_stream, test_stream_bitrate);
	tcase_add_test(tc_stream, test_stream_samplerate);
	tcase_add_test(tc_stream, test_stream_channels);
	tcase_add_test(tc_stream, test_stream_server_public);
	tcase_add_test(tc_stream, test_stream_format);
	tcase_add_test(tc_stream, test_stream_encoder);
	suite_add_tcase(s, tc_stream);

	tc_media = tcase_create("Media");
	tcase_add_checked_fixture(tc_media, setup_checked, teardown_checked);
	tcase_add_test(tc_media, test_media_type);
	tcase_add_test(tc_media, test_media_filename);
	tcase_add_test(tc_media, test_media_shuffle);
	tcase_add_test(tc_media, test_media_stream_once);
	suite_add_tcase(s, tc_media);

	tc_metadata = tcase_create("Metadata");
	tcase_add_checked_fixture(tc_metadata, setup_checked,
	    teardown_checked);
	tcase_add_test(tc_metadata, test_metadata_program);
	tcase_add_test(tc_metadata, test_metadata_format_str);
	tcase_add_test(tc_metadata, test_metadata_refresh_interval);
	tcase_add_test(tc_metadata, test_metadata_normalize_strings);
	tcase_add_test(tc_metadata, test_metadata_no_updates);
	suite_add_tcase(s, tc_metadata);

	tc_decoder = tcase_create("Decoder");
	tcase_add_checked_fixture(tc_decoder, setup_checked,
	    teardown_checked);
	tcase_add_test(tc_decoder, test_decoder_get);
	tcase_add_test(tc_decoder, test_decoder_set_name);
	tcase_add_test(tc_decoder, test_decoder_set_program);
	suite_add_tcase(s, tc_decoder);

	tc_encoder = tcase_create("Encoder");
	tcase_add_checked_fixture(tc_encoder, setup_checked,
	    teardown_checked);
	tcase_add_test(tc_encoder, test_encoder_get);
	tcase_add_test(tc_decoder, test_encoder_set_name);
	tcase_add_test(tc_decoder, test_encoder_set_program);
	suite_add_tcase(s, tc_encoder);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < cfg_decoder_init() ||
	    0 < cfg_encoder_init() ||
	    0 < log_init())
		ck_abort_msg("setup_checked failed");
}

void
teardown_checked(void)
{
	log_exit();
	cfg_encoder_exit();
	cfg_decoder_exit();
	cfg_exit();
}

int
main(void)
{
	unsigned int	 num_failed;
	Suite           *s;
	SRunner         *sr;

	s = cfg_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
