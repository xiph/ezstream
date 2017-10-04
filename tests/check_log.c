#include <check.h>
#include <errno.h>

#include "cfg.h"
#include "log.h"

Suite * log_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_log)
{
	unsigned int	verbosity;

	ck_assert_int_eq(log_syserr(ALERT, 0, "alert"), 0);

	verbosity = 0;
	ck_assert_int_eq(cfg_set_program_verbosity(verbosity, NULL), 0);
	ck_assert_int_ne(log_alert("alert"), 0);
	ck_assert_int_ne(log_syserr(ALERT, EINVAL, "alert"), 0);
	ck_assert_int_ne(log_syserr(ALERT, EINVAL, NULL), 0);
	ck_assert_int_ne(log_error("error"), 0);
	ck_assert_int_ne(log_syserr(ERROR, EINVAL, "error"), 0);
	ck_assert_int_ne(log_warning("warning"), 0);
	ck_assert_int_ne(log_syserr(WARNING, EINVAL, "warning"), 0);

	ck_assert_int_eq(log_notice("notice"), 0);
	ck_assert_int_eq(log_syserr(NOTICE, EINVAL, "notice"), 0);
	ck_assert_int_eq(cfg_set_program_verbosity(++verbosity, NULL), 0);
	ck_assert_int_ne(log_notice("notice"), 0);
	ck_assert_int_ne(log_syserr(NOTICE, EINVAL, "notice"), 0);

	ck_assert_int_eq(log_info("info"), 0);
	ck_assert_int_eq(log_syserr(INFO, EINVAL, "info"), 0);
	ck_assert_int_eq(cfg_set_program_verbosity(++verbosity, NULL), 0);
	ck_assert_int_ne(log_info("info"), 0);
	ck_assert_int_ne(log_syserr(INFO, EINVAL, "info"), 0);

	ck_assert_int_eq(log_debug("debug"), 0);
	ck_assert_int_eq(log_syserr(DEBUG, EINVAL, "debug"), 0);
	ck_assert_int_eq(cfg_set_program_verbosity(++verbosity, NULL), 0);
	ck_assert_int_ne(log_debug("debug"), 0);
	ck_assert_int_ne(log_syserr(DEBUG, EINVAL, "debug"), 0);
}
END_TEST

Suite *
log_suite(void)
{
	Suite	*s;
	TCase	*tc_log;

	s = suite_create("Log");

	tc_log = tcase_create("Log");
	tcase_add_checked_fixture(tc_log, setup_checked, teardown_checked);
	tcase_add_test(tc_log, test_log);
	suite_add_tcase(s, tc_log);

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

	s = log_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
