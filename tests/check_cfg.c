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
	ck_assert_int_eq(cfg_check(NULL), 0);
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
	ck_assert_ptr_eq(cfg_get_program_name(), NULL);
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
	TCase	*tc_metadata;

	s = suite_create("Config");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup_checked, teardown_checked);
	tcase_add_test(tc_core, test_check);
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
	    0 < log_init(NULL))
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
