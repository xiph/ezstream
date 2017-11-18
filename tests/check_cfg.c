#include <check.h>
#include <limits.h>
#include <netdb.h>

#include "cfg_private.h"
#include "log.h"

#include "check_cfg.h"

Suite * cfg_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_check)
{
	const char	*errstr = NULL;

	ck_assert_int_eq(cfg_check(NULL), -1);
	ck_assert_int_eq(cfg_check(&errstr), -1);
	ck_assert_str_eq(errstr, "media filename missing");
	ck_assert_int_eq(cfg_set_media_type("stdin", NULL), 0);

	ck_assert_int_eq(cfg_check(NULL), 0);

	ck_assert_int_eq(cfg_set_media_type("autodetect", NULL), 0);
	ck_assert_int_eq(cfg_check(NULL), -1);
	ck_assert_int_eq(cfg_check(&errstr), -1);
	ck_assert_str_eq(errstr, "media filename missing");
	ck_assert_int_eq(cfg_set_media_filename(SRCDIR "/playlist.txt", NULL),
	    0);

	ck_assert_int_eq(cfg_check(NULL), 0);
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
	ck_assert_ptr_eq(cfg_get_program_config_file(), NULL);
	TEST_STRLCPY(cfg_set_program_config_file, cfg_get_program_config_file,
	    PATH_MAX);
}
END_TEST

START_TEST(test_program_pid_file)
{
	ck_assert_ptr_eq(cfg_get_program_pid_file(), NULL);
	TEST_STRLCPY(cfg_set_program_pid_file, cfg_get_program_pid_file,
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

START_TEST(test_media_type)
{
	const char	*errstr2;

	TEST_EMPTYSTR(cfg_set_media_type);

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
	ck_assert_ptr_eq(cfg_get_metadata_program(), NULL);
	TEST_STRLCPY(cfg_set_metadata_program, cfg_get_metadata_program,
	    PATH_MAX);
}
END_TEST

START_TEST(test_metadata_format_str)
{
	const char	*errstr2;

	TEST_XSTRDUP(cfg_set_metadata_format_str,
	    cfg_get_metadata_format_str);

	ck_assert_int_eq(cfg_set_metadata_format_str("test", NULL), 0);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_set_metadata_format_str(PLACEHOLDER_METADATA,
	    &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);
	ck_assert_str_eq(errstr2,
	    "prohibited placeholder " PLACEHOLDER_METADATA);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_set_metadata_format_str(
	    PLACEHOLDER_TRACK PLACEHOLDER_TRACK, &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);
	ck_assert_str_eq(errstr2, "duplicate placeholder " PLACEHOLDER_TRACK);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_set_metadata_format_str(
	    PLACEHOLDER_STRING PLACEHOLDER_STRING, &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);
	ck_assert_str_eq(errstr2, "duplicate placeholder " PLACEHOLDER_STRING);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_set_metadata_format_str(
	    PLACEHOLDER_ARTIST PLACEHOLDER_ARTIST, &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);
	ck_assert_str_eq(errstr2, "duplicate placeholder " PLACEHOLDER_ARTIST);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_set_metadata_format_str(
	    PLACEHOLDER_TITLE PLACEHOLDER_TITLE, &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);
	ck_assert_str_eq(errstr2, "duplicate placeholder " PLACEHOLDER_TITLE);
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

Suite *
cfg_suite(void)
{
	Suite	*s;
	TCase	*tc_core;
	TCase	*tc_program;
	TCase	*tc_media;
	TCase	*tc_metadata;

	s = suite_create("Config");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup_checked, teardown_checked);
	tcase_add_test(tc_core, test_check);
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
	tcase_add_test(tc_program, test_program_pid_file);
	tcase_add_test(tc_program, test_program_quiet_stderr);
	tcase_add_test(tc_program, test_program_rtstatus_output);
	tcase_add_test(tc_program, test_program_verbosity);
	suite_add_tcase(s, tc_program);

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

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < log_init())
		ck_abort_msg("setup_checked failed");
}

void
teardown_checked(void)
{
	log_exit();
	cfg_exit();
}

int
main(void)
{
	int	 num_failed;
	Suite	*s;
	SRunner *sr;

	s = cfg_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
