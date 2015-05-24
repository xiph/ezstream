#include <check.h>

#include "cfg.h"
#include "cmdline.h"

Suite * cmdline_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_help)
{
	char	*argv[] = { "check_cmdline", "-h", NULL };
	int	 argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
	int	 ret;

	ck_assert_int_ne(cmdline_parse(argc, argv, &ret), 0);
	ck_assert_int_eq(ret, 0);
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
	tcase_add_test(tc_cmdline, test_help);
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
	unsigned int	 num_failed;
	Suite           *s;
	SRunner         *sr;

	s = cmdline_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
