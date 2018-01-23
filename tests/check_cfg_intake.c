#include <check.h>
#include <limits.h>
#include <netdb.h>

#include "cfg_private.h"
#include "log.h"

#include "check_cfg.h"

static void	_il_cb(cfg_intake_t, void *);

Suite * cfg_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

cfg_intake_list_t	intakes;

static void
_il_cb(cfg_intake_t in, void *arg)
{
	int	*count = (int *)arg;

	ck_assert_ptr_ne(cfg_intake_get_name(in), NULL);
	(*count)++;
}

START_TEST(test_intake_list_get)
{
	cfg_intake_t	in, in2;
	int		count = 0;

	ck_assert_ptr_eq(cfg_intake_list_get(intakes, NULL), NULL);
	ck_assert_ptr_eq(cfg_intake_list_get(intakes, ""), NULL);

	in = cfg_intake_list_get(intakes, "TeSt");
	in2 = cfg_intake_list_get(intakes, "test");
	ck_assert_ptr_eq(in, in2);

	(void)cfg_intake_list_get(intakes, "test2");
	ck_assert_uint_eq(cfg_intake_list_nentries(intakes), 2);
	cfg_intake_list_foreach(intakes, _il_cb, &count);
	ck_assert_int_eq(count, 2);
}
END_TEST

START_TEST(test_intake_set_name)
{
	TEST_XSTRDUP_T(cfg_intake_t, cfg_intake_list_get, intakes,
	    cfg_intake_set_name, cfg_intake_get_name);
}
END_TEST

START_TEST(test_intake_set_type)
{
	const char	*errstr2;
	cfg_intake_t	 in = cfg_intake_list_get(intakes, "test_intake_set_type");

	TEST_EMPTYSTR_T(cfg_intake_t, cfg_intake_list_get, intakes,
	    cfg_intake_set_type);

	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "<something else>",
	    NULL), -1);
	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "<something else>",
	    &errstr2), -1);
	ck_assert_str_eq(errstr2, "unsupported");

	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "aUtOdEtEcT", NULL),
	    0);
	ck_assert_int_eq(cfg_intake_get_type(in), CFG_INTAKE_AUTODETECT);
	ck_assert_str_eq(cfg_intake_get_type_str(in), "autodetect");
	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "FiLe", NULL), 0);
	ck_assert_int_eq(cfg_intake_get_type(in), CFG_INTAKE_FILE);
	ck_assert_str_eq(cfg_intake_get_type_str(in), "file");
	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "pLaYlIsT", NULL),
	    0);
	ck_assert_int_eq(cfg_intake_get_type(in), CFG_INTAKE_PLAYLIST);
	ck_assert_str_eq(cfg_intake_get_type_str(in), "playlist");
	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "PrOgRaM", NULL), 0);
	ck_assert_int_eq(cfg_intake_get_type(in), CFG_INTAKE_PROGRAM);
	ck_assert_str_eq(cfg_intake_get_type_str(in), "program");
	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "sTdIn", NULL), 0);
	ck_assert_int_eq(cfg_intake_get_type(in), CFG_INTAKE_STDIN);
	ck_assert_str_eq(cfg_intake_get_type_str(in), "stdin");
}
END_TEST

START_TEST(test_intake_set_filename)
{
	TEST_STRLCPY_T(cfg_intake_t, cfg_intake_list_get, intakes,
	    cfg_intake_set_filename, cfg_intake_get_filename, PATH_MAX);
}
END_TEST

START_TEST(test_intake_set_shuffle)
{
	TEST_BOOLEAN_T(cfg_intake_t, cfg_intake_list_get, intakes,
	    cfg_intake_set_shuffle, cfg_intake_get_shuffle);
}
END_TEST

START_TEST(test_intake_set_stream_once)
{
	TEST_BOOLEAN_T(cfg_intake_t, cfg_intake_list_get, intakes,
	    cfg_intake_set_stream_once, cfg_intake_get_stream_once);
}
END_TEST

START_TEST(test_intake_validate)
{
	cfg_intake_t	 in = cfg_intake_list_get(intakes, "test_intake_validate");
	const char	*errstr;

	ck_assert_int_ne(cfg_intake_validate(in, NULL), 0);
	ck_assert_int_ne(cfg_intake_validate(in, &errstr), 0);
	ck_assert_str_eq(errstr, "intake filename missing");

	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "stdin", NULL), 0);

	ck_assert_int_eq(cfg_intake_validate(in, NULL), 0);

	ck_assert_int_eq(cfg_intake_set_type(in, intakes, "autodetect", NULL),
	    0);
	ck_assert_int_ne(cfg_intake_validate(in, NULL), 0);
	ck_assert_int_ne(cfg_intake_validate(in, &errstr), 0);
	ck_assert_str_eq(errstr, "intake filename missing");
	ck_assert_int_eq(cfg_intake_set_filename(in, intakes,
	    SRCDIR "/playlist.txt", NULL), 0);

	ck_assert_int_eq(cfg_intake_validate(in, NULL), 0);
}
END_TEST

Suite *
cfg_suite(void)
{
	Suite	*s;
	TCase	*tc_intake;

	s = suite_create("Config");

	tc_intake = tcase_create("Intake");
	tcase_add_checked_fixture(tc_intake, setup_checked,
	    teardown_checked);
	tcase_add_test(tc_intake, test_intake_list_get);
	tcase_add_test(tc_intake, test_intake_set_name);
	tcase_add_test(tc_intake, test_intake_set_type);
	tcase_add_test(tc_intake, test_intake_set_filename);
	tcase_add_test(tc_intake, test_intake_set_shuffle);
	tcase_add_test(tc_intake, test_intake_set_stream_once);
	tcase_add_test(tc_intake, test_intake_validate);
	suite_add_tcase(s, tc_intake);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < cfg_set_program_name("check_cfg_intake", NULL) ||
	    0 < log_init(cfg_get_program_name()))
		ck_abort_msg("setup_checked failed");

	intakes = cfg_intake_list_create();
}

void
teardown_checked(void)
{
	cfg_intake_list_destroy(&intakes);

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
