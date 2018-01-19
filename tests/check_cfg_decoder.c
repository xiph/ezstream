#include <check.h>
#include <limits.h>
#include <netdb.h>

#include "cfg_private.h"
#include "log.h"

#include "check_cfg.h"

Suite * cfg_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

cfg_decoder_list_t	decoders;

START_TEST(test_decoder_list_get)
{
	cfg_decoder_t	dec, dec2;

	ck_assert_ptr_eq(cfg_decoder_list_get(decoders, NULL), NULL);
	ck_assert_ptr_eq(cfg_decoder_list_get(decoders, ""), NULL);

	dec = cfg_decoder_list_get(decoders, "TeSt");
	dec2 = cfg_decoder_list_get(decoders, "test");
	ck_assert_ptr_eq(dec, dec2);
}
END_TEST

START_TEST(test_decoder_set_name)
{
	TEST_XSTRDUP_T(cfg_decoder_t, cfg_decoder_list_get, decoders,
	    cfg_decoder_set_name, cfg_decoder_get_name);
}
END_TEST

START_TEST(test_decoder_set_program)
{
	TEST_XSTRDUP_T(cfg_decoder_t, cfg_decoder_list_get, decoders,
	    cfg_decoder_set_program, cfg_decoder_get_program);
}
END_TEST

START_TEST(test_decoder_add_match)
{
	cfg_decoder_t	 dec = cfg_decoder_list_get(decoders, "test_decoder_add_match");
	cfg_decoder_t	 dec2 = cfg_decoder_list_get(decoders, "test_decoder_add_match_2");
	const char	*errstr;

	errstr = NULL;
	ck_assert_int_ne(cfg_decoder_add_match(dec, decoders, NULL, NULL), 0);
	ck_assert_int_ne(cfg_decoder_add_match(dec, decoders, NULL, &errstr),
	    0);
	ck_assert_str_eq(errstr, "empty");
	ck_assert_int_ne(cfg_decoder_add_match(dec, decoders, "", NULL), 0);

	ck_assert_int_eq(cfg_decoder_add_match(dec, decoders, ".test", NULL),
	    0);
	ck_assert_int_eq(cfg_decoder_add_match(dec, decoders, ".test2", NULL), 0);
	ck_assert_ptr_eq(cfg_decoder_list_findext(decoders, ".test"), dec);
	ck_assert_ptr_eq(cfg_decoder_list_findext(decoders, ".test2"), dec);

	ck_assert_int_eq(cfg_decoder_add_match(dec2, decoders, ".test2", NULL),
	    0);
	ck_assert_ptr_eq(cfg_decoder_list_findext(decoders, ".test"), dec);
	ck_assert_ptr_eq(cfg_decoder_list_findext(decoders, ".test2"), dec2);
}
END_TEST

START_TEST(test_decoder_validate)
{
	cfg_decoder_t	 dec = cfg_decoder_list_get(decoders, "test_decoder_validate");
	const char	*errstr;

	errstr = NULL;
	ck_assert_int_ne(cfg_decoder_validate(dec, NULL), 0);
	ck_assert_int_ne(cfg_decoder_validate(dec, &errstr), 0);
	ck_assert_str_eq(errstr, "program not set");

	ck_assert_int_eq(cfg_decoder_set_program(dec, decoders, "test", NULL),
	    0);
	ck_assert_int_ne(cfg_decoder_validate(dec, &errstr), 0);
	ck_assert_str_eq(errstr, "no file extensions registered");

	ck_assert_int_eq(cfg_decoder_add_match(dec, decoders, ".test", NULL),
	    0);

	ck_assert_int_eq(cfg_decoder_set_program(dec, decoders,
	    PLACEHOLDER_STRING, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_decoder_validate(dec, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "prohibited placeholder " PLACEHOLDER_STRING);

	ck_assert_int_eq(cfg_decoder_set_program(dec, decoders,
	    PLACEHOLDER_TRACK PLACEHOLDER_TRACK, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_decoder_validate(dec, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "duplicate placeholder " PLACEHOLDER_TRACK);

	ck_assert_int_eq(cfg_decoder_set_program(dec, decoders,
	    PLACEHOLDER_METADATA PLACEHOLDER_METADATA, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_decoder_validate(dec, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "duplicate placeholder " PLACEHOLDER_METADATA);

	ck_assert_int_eq(cfg_decoder_set_program(dec, decoders,
	    PLACEHOLDER_ARTIST PLACEHOLDER_ARTIST, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_decoder_validate(dec, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "duplicate placeholder " PLACEHOLDER_ARTIST);

	ck_assert_int_eq(cfg_decoder_set_program(dec, decoders,
	    PLACEHOLDER_TITLE PLACEHOLDER_TITLE, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_decoder_validate(dec, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "duplicate placeholder " PLACEHOLDER_TITLE);

	ck_assert_int_eq(cfg_decoder_set_program(dec, decoders,
	    "test", NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_decoder_validate(dec, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "missing placeholder " PLACEHOLDER_TRACK);

	ck_assert_int_eq(cfg_decoder_set_program(dec, decoders,
	    PLACEHOLDER_TRACK, NULL), 0);
	ck_assert_int_eq(cfg_decoder_validate(dec, &errstr), 0);
}
END_TEST

Suite *
cfg_suite(void)
{
	Suite	*s;
	TCase	*tc_decoder;

	s = suite_create("Config");

	tc_decoder = tcase_create("Decoder");
	tcase_add_checked_fixture(tc_decoder, setup_checked,
	    teardown_checked);
	tcase_add_test(tc_decoder, test_decoder_list_get);
	tcase_add_test(tc_decoder, test_decoder_set_name);
	tcase_add_test(tc_decoder, test_decoder_set_program);
	tcase_add_test(tc_decoder, test_decoder_add_match);
	tcase_add_test(tc_decoder, test_decoder_validate);
	suite_add_tcase(s, tc_decoder);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < cfg_set_program_name("check_cfg_decoder", NULL) ||
	    0 < log_init(cfg_get_program_name()))
		ck_abort_msg("setup_checked failed");

	decoders = cfg_decoder_list_create();
}

void
teardown_checked(void)
{
	cfg_decoder_list_destroy(&decoders);

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
