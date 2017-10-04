#include <check.h>

#include "cfg.h"
#include "cmdline.h"

Suite * cmdline_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_configfile)
{
	char	*argv[] =
	{
		"check_cmdline", "-c", EXAMPLESDIR "/ezstream-full.xml" , NULL
	};
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ret = 0;
	ck_assert_int_eq(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 0);
}
END_TEST

START_TEST(test_help)
{
	char	*argv[] = { "check_cmdline", "-h", NULL };
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 0);
}
END_TEST

START_TEST(test_pidfile)
{
	char	*argv[] =
	{
		"check_cmdline", "-p", "PIDFILE-TEST" , NULL
	};
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ck_assert_ptr_eq(cfg_get_program_pid_file(), NULL);
	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 2);
	ck_assert_str_eq(cfg_get_program_pid_file(), "PIDFILE-TEST");
}
END_TEST

START_TEST(test_quiet_stderr)
{
	char	*argv[] = { "check_cmdline", "-q", NULL };
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ck_assert_int_eq(cfg_get_program_quiet_stderr(), 0);
	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 2);
	ck_assert_int_ne(cfg_get_program_quiet_stderr(), 0);
}
END_TEST

START_TEST(test_rtstatus_output)
{
	char	*argv[] = { "check_cmdline", "-r", NULL };
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ck_assert_int_eq(cfg_get_program_rtstatus_output(), 0);
	ck_assert_int_eq(cfg_get_program_quiet_stderr(), 0);
	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 2);
	ck_assert_int_ne(cfg_get_program_rtstatus_output(), 0);
	ck_assert_int_ne(cfg_get_program_quiet_stderr(), 0);
}
END_TEST

START_TEST(test_shuffle)
{
	char	*argv[] =
	{
		"check_cmdline", "-s", SRCDIR "/playlist.txt", NULL
	};
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 0);
}
END_TEST

START_TEST(test_version)
{
	char	*argv[] = { "check_cmdline", "-V", NULL };
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 0);
}
END_TEST

START_TEST(test_verbose)
{
	char	*argv[] = { "check_cmdline", "-v", NULL };
	char	*argv2[] = { "check_cmdline", "-v", "-v", NULL };
	char	*argv3[] = { "check_cmdline", "-v", "-v", "-v", NULL };
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 argc2 = (int)(sizeof(argv2) / sizeof(argv2[0])) - 1;
	int	 argc3 = (int)(sizeof(argv3) / sizeof(argv3[0])) - 1;
	int	 ret;

	ck_assert_uint_eq(cfg_get_program_verbosity(), 0);
	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_uint_eq(cfg_get_program_verbosity(), 1);

	ck_assert_int_ne(cmdline_parse(argc2, argv2, &ret), 0);
	ck_assert_uint_eq(cfg_get_program_verbosity(), 2);

	ck_assert_int_ne(cmdline_parse(argc3, argv3, &ret), 0);
	ck_assert_uint_eq(cfg_get_program_verbosity(), 3);

	ck_assert_int_eq(ret, 2);
}
END_TEST

START_TEST(test_invalid)
{
	char	*argv[] = { "check_cmdline", "-x", NULL };
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 2);
}
END_TEST

Suite *
cmdline_suite(void)
{
	Suite	*s;
	TCase	*tc_cmdline;

	s = suite_create("CmdLine");

	tc_cmdline = tcase_create("CmdLine");
	tcase_add_checked_fixture(tc_cmdline, setup_checked, teardown_checked);
	tcase_add_test(tc_cmdline, test_configfile);
	tcase_add_test(tc_cmdline, test_help);
	tcase_add_test(tc_cmdline, test_pidfile);
	tcase_add_test(tc_cmdline, test_quiet_stderr);
	tcase_add_test(tc_cmdline, test_rtstatus_output);
	tcase_add_test(tc_cmdline, test_shuffle);
	tcase_add_test(tc_cmdline, test_version);
	tcase_add_test(tc_cmdline, test_verbose);
	tcase_add_test(tc_cmdline, test_invalid);
	suite_add_tcase(s, tc_cmdline);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init())
		ck_abort_msg("setup_checked failed");
}

void
teardown_checked(void)
{
	cfg_exit();
}

int
main(void)
{
	int	 num_failed;
	Suite	*s;
	SRunner *sr;

	s = cmdline_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
