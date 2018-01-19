#include <check.h>
#include <limits.h>
#include <netdb.h>

#include "cfg_private.h"
#include "log.h"

#include "check_cfg.h"

Suite * cfg_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

cfg_encoder_list_t	encoders;

START_TEST(test_encoder_list_get)
{
	cfg_encoder_t	enc, enc2;

	ck_assert_ptr_eq(cfg_encoder_list_get(encoders, NULL), NULL);
	ck_assert_ptr_eq(cfg_encoder_list_get(encoders, ""), NULL);

	enc = cfg_encoder_list_get(encoders, "TeSt");
	enc2 = cfg_encoder_list_get(encoders, "test");
	ck_assert_ptr_eq(enc, enc2);
}
END_TEST

START_TEST(test_encoder_set_name)
{
	TEST_XSTRDUP_T(cfg_encoder_t, cfg_encoder_list_get, encoders,
	    cfg_encoder_set_name, cfg_encoder_get_name);
}
END_TEST

START_TEST(test_encoder_set_program)
{
	cfg_encoder_t	 enc = cfg_encoder_list_get(encoders, "test_encoder_set_program");

	ck_assert_int_eq(cfg_encoder_set_program(enc, encoders, NULL, NULL), 0);
	ck_assert_ptr_eq(cfg_encoder_get_program(enc), NULL);

	ck_assert_int_eq(cfg_encoder_set_program(enc, encoders, "test", NULL), 0);
	ck_assert_str_eq(cfg_encoder_get_program(enc), "test");
}
END_TEST

START_TEST(test_encoder_set_format_str)
{
	cfg_encoder_t	 enc = cfg_encoder_list_get(encoders, "test_encoder_set_format_str");
	const char	*errstr;

	errstr = NULL;
	ck_assert_int_ne(cfg_encoder_set_format_str(enc, encoders, NULL,
	    NULL), 0);
	ck_assert_int_ne(cfg_encoder_set_format_str(enc, encoders, NULL,
	    &errstr), 0);
	ck_assert_str_eq(errstr, "empty");
	ck_assert_int_ne(cfg_encoder_set_format_str(enc, encoders, "", NULL),
	    0);

	errstr = NULL;
	ck_assert_int_ne(cfg_encoder_set_format_str(enc, encoders, "test",
	    &errstr), 0);
	ck_assert_str_eq(errstr, "unsupported stream format");

	ck_assert_int_eq(cfg_encoder_set_format_str(enc, encoders,
	    CFG_SFMT_VORBIS, NULL), 0);
	ck_assert_int_eq(cfg_encoder_set_format_str(enc, encoders,
	    CFG_SFMT_MP3, NULL), 0);
	ck_assert_int_eq(cfg_encoder_set_format_str(enc, encoders,
	    CFG_SFMT_THEORA, NULL), 0);
	ck_assert_uint_eq(cfg_encoder_get_format(enc), CFG_STREAM_THEORA);
}
END_TEST

START_TEST(test_encoder_validate)
{
	cfg_encoder_t	 enc = cfg_encoder_list_get(encoders, "test_encoder_validate");
	const char	*errstr;

	ck_assert_int_ne(cfg_encoder_validate(enc, NULL), 0);
	ck_assert_int_ne(cfg_encoder_validate(enc, &errstr), 0);
	ck_assert_str_eq(errstr, "format not set");

	ck_assert_int_eq(cfg_encoder_set_format(enc, CFG_STREAM_VORBIS), 0);

	ck_assert_int_eq(cfg_encoder_validate(enc, NULL), 0);

	ck_assert_int_eq(cfg_encoder_set_program(enc, encoders,
	    PLACEHOLDER_TRACK, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_encoder_validate(enc, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "prohibited placeholder " PLACEHOLDER_TRACK);

	ck_assert_int_eq(cfg_encoder_set_program(enc, encoders,
	    PLACEHOLDER_STRING, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_encoder_validate(enc, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "prohibited placeholder " PLACEHOLDER_STRING);

	ck_assert_int_eq(cfg_encoder_set_program(enc, encoders,
	    PLACEHOLDER_METADATA PLACEHOLDER_METADATA, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_encoder_validate(enc, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "duplicate placeholder " PLACEHOLDER_METADATA);

	ck_assert_int_eq(cfg_encoder_set_program(enc, encoders,
	    PLACEHOLDER_ARTIST PLACEHOLDER_ARTIST, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_encoder_validate(enc, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "duplicate placeholder " PLACEHOLDER_ARTIST);

	ck_assert_int_eq(cfg_encoder_set_program(enc, encoders,
	    PLACEHOLDER_TITLE PLACEHOLDER_TITLE, NULL), 0);
	errstr = NULL;
	ck_assert_int_ne(cfg_encoder_validate(enc, &errstr), 0);
	ck_assert_str_eq(errstr,
	    "duplicate placeholder " PLACEHOLDER_TITLE);
}
END_TEST

Suite *
cfg_suite(void)
{
	Suite	*s;
	TCase	*tc_encoder;

	s = suite_create("Config");

	tc_encoder = tcase_create("Encoder");
	tcase_add_checked_fixture(tc_encoder, setup_checked,
	    teardown_checked);
	tcase_add_test(tc_encoder, test_encoder_list_get);
	tcase_add_test(tc_encoder, test_encoder_set_name);
	tcase_add_test(tc_encoder, test_encoder_set_program);
	tcase_add_test(tc_encoder, test_encoder_set_format_str);
	tcase_add_test(tc_encoder, test_encoder_validate);
	suite_add_tcase(s, tc_encoder);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < cfg_set_program_name("check_cfg_encoder", NULL) ||
	    0 < log_init(cfg_get_program_name()))
		ck_abort_msg("setup_checked failed");

	encoders = cfg_encoder_list_create();
}

void
teardown_checked(void)
{
	cfg_encoder_list_destroy(&encoders);

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
