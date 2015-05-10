#include <check.h>
#include <limits.h>
#include <netdb.h>

#include "cfg.h"

Suite * cfg_suite(void);

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
	ck_assert_int_eq(cfg_file_check(NULL), -1);
}
END_TEST

START_TEST(test_program_name)
{
	char		 buf[PATH_MAX * 2];
	const char	*errstr;

	errstr = NULL;
	ck_assert_int_eq(cfg_set_program_name(NULL, &errstr), -1);
	ck_assert_ptr_ne(errstr, NULL);

	errstr = NULL;
	memset(buf, 'A', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	ck_assert_int_eq(cfg_set_program_name(buf, &errstr), -1);
	ck_assert_ptr_ne(errstr, NULL);

	ck_assert_int_eq(cfg_set_program_name("check_cfg", NULL), 0);
	ck_assert_str_eq(cfg_get_program_name(), "check_cfg");
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
	char		 buf[PATH_MAX + 1];
	const char	*errstr;

	errstr = NULL;
	ck_assert_int_eq(cfg_set_program_config_file(NULL, &errstr), -1);
	ck_assert_ptr_ne(errstr, NULL);

	errstr = NULL;
	memset(buf, 'A', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	ck_assert_int_eq(cfg_set_program_config_file(buf, &errstr), -1);
	ck_assert_ptr_ne(errstr, NULL);

	ck_assert_int_eq(cfg_set_program_config_file("/path/to/somewhere.cfg",
	    NULL), 0);
	ck_assert_str_eq(cfg_get_program_config_file(),
	    "/path/to/somewhere.cfg");
}
END_TEST

START_TEST(test_program_quiet_stderr)
{
	ck_assert_int_eq(cfg_set_program_quiet_stderr(-1, NULL), 0);
	ck_assert_int_ne(cfg_get_program_quiet_stderr(), 0);
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
	const char	*errstr = NULL;

	ck_assert_int_eq(cfg_set_server_protocol(NULL, &errstr), -1);
	ck_assert_str_eq(errstr, "empty");
	ck_assert_int_eq(cfg_set_server_protocol("invalid", &errstr), -1);
	ck_assert_str_eq(errstr, "unsupported");
	ck_assert_int_eq(cfg_set_server_protocol("hTtP", NULL), 0);
	ck_assert_int_eq(cfg_get_server_protocol(), CFG_PROTO_HTTP);
	ck_assert_int_eq(cfg_set_server_protocol("HtTpS", NULL), 0);
	ck_assert_int_eq(cfg_get_server_protocol(), CFG_PROTO_HTTPS);
}
END_TEST

START_TEST(test_server_hostname)
{
	char		 buf[NI_MAXHOST + 1];
	const char	*errstr;

	errstr = NULL;
	ck_assert_int_eq(cfg_set_server_hostname(NULL, &errstr), -1);
	ck_assert_ptr_ne(errstr, NULL);

	errstr = NULL;
	memset(buf, 'A', sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	ck_assert_int_eq(cfg_set_server_hostname(buf, &errstr), -1);
	ck_assert_ptr_ne(errstr, NULL);

	ck_assert_int_eq(cfg_set_server_hostname("check_cfg", NULL), 0);
	ck_assert_str_eq(cfg_get_server_hostname(), "check_cfg");
}
END_TEST

START_TEST(test_server_port)
{
	const char	*errstr;

	errstr = NULL;
	ck_assert_int_eq(cfg_set_server_port(NULL, &errstr), -1);
	ck_assert_str_eq(errstr, "empty");

	errstr = NULL;
	ck_assert_int_eq(cfg_set_server_port("0", &errstr), -1);
	ck_assert_ptr_ne(errstr, NULL);

	errstr = NULL;
	ck_assert_int_eq(cfg_set_server_port("65536", &errstr), -1);
	ck_assert_ptr_ne(errstr, NULL);

	ck_assert_int_eq(cfg_set_server_port("8000", NULL), 0);
	ck_assert_uint_eq(cfg_get_server_port(), 8000);
}
END_TEST

Suite *
cfg_suite(void)
{
	Suite   *s;
	TCase   *tc_core;

	s = suite_create("Config");

	tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_stream_str2fmt);
	tcase_add_test(tc_core, test_stream_fmt2str);
	tcase_add_test(tc_core, test_file_check);
	tcase_add_test(tc_core, test_program_name);
	tcase_add_test(tc_core, test_program_config_type);
	tcase_add_test(tc_core, test_program_config_file);
	tcase_add_test(tc_core, test_program_quiet_stderr);
	tcase_add_test(tc_core, test_program_verbosity);
	tcase_add_test(tc_core, test_server_protocol);
	tcase_add_test(tc_core, test_server_hostname);
	tcase_add_test(tc_core, test_server_port);
	suite_add_tcase(s, tc_core);

	return (s);
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

	cfg_exit();

	if (num_failed)
		return (1);
	return (0);
}
