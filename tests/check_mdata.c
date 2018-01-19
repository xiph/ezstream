#include <check.h>

#include <stdio.h>

#include "cfg.h"
#include "log.h"
#include "mdata.h"

Suite * mdata_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

mdata_t md;

START_TEST(test_mdata_md)
{
	ck_assert_ptr_eq(mdata_get_filename(md), NULL);
	ck_assert_ptr_eq(mdata_get_name(md), NULL);
	ck_assert_ptr_eq(mdata_get_artist(md), NULL);
	ck_assert_ptr_eq(mdata_get_album(md), NULL);
	ck_assert_ptr_eq(mdata_get_title(md), NULL);
	ck_assert_ptr_eq(mdata_get_songinfo(md), NULL);
	ck_assert_int_lt(mdata_get_length(md), 0);
}
END_TEST

START_TEST(test_mdata_parse_file)
{
	ck_assert_int_ne(mdata_parse_file(md, SRCDIR "/nonexistent"), 0);
	ck_assert_ptr_eq(mdata_get_filename(md), NULL);
	ck_assert_ptr_eq(mdata_get_name(md), NULL);

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test01-artist+album+title.ogg"), 0);
	ck_assert_str_eq(mdata_get_filename(md), SRCDIR "/test01-artist+album+title.ogg");
	ck_assert_str_eq(mdata_get_name(md), "test01-artist+album+title");
	ck_assert_str_eq(mdata_get_artist(md), "test artist");
	ck_assert_str_eq(mdata_get_album(md), "test album");
	ck_assert_str_eq(mdata_get_title(md), "test title");
	ck_assert_str_eq(mdata_get_songinfo(md), "test artist - test title - test album");
	ck_assert_int_eq(mdata_get_length(md), 1);

	mdata_set_normalize_strings(md, 1);
	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test02-whitespace.ogg"), 0);
	ck_assert_str_eq(mdata_get_artist(md), "test artist");
	ck_assert_str_eq(mdata_get_album(md), "test album");
	ck_assert_str_eq(mdata_get_title(md), "test title");
	mdata_set_normalize_strings(md, 0);
	ck_assert_int_eq(mdata_refresh(md), 0);
	ck_assert_str_eq(mdata_get_artist(md), "  test  artist  ");
	ck_assert_str_eq(mdata_get_album(md), "  test  album  ");
	ck_assert_str_eq(mdata_get_title(md), "  test  title  ");

	mdata_set_normalize_strings(md, 1);

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test11-artist+album.ogg"), 0);
	ck_assert_str_eq(mdata_get_artist(md), "test artist");
	ck_assert_str_eq(mdata_get_album(md), "test album");
	ck_assert_ptr_eq(mdata_get_title(md), NULL);
	ck_assert_str_eq(mdata_get_songinfo(md), "test artist - test album");

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test13-album+title.ogg"), 0);
	ck_assert_ptr_eq(mdata_get_artist(md), NULL);
	ck_assert_str_eq(mdata_get_album(md), "test album");
	ck_assert_str_eq(mdata_get_title(md), "test title");
	ck_assert_str_eq(mdata_get_songinfo(md), "test title - test album");

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test12-artist.ogg"), 0);
	ck_assert_str_eq(mdata_get_artist(md), "test artist");
	ck_assert_ptr_eq(mdata_get_album(md), NULL);
	ck_assert_ptr_eq(mdata_get_title(md), NULL);
	ck_assert_str_eq(mdata_get_songinfo(md), "test artist");

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test14-album.ogg"), 0);
	ck_assert_ptr_eq(mdata_get_artist(md), NULL);
	ck_assert_str_eq(mdata_get_album(md), "test album");
	ck_assert_ptr_eq(mdata_get_title(md), NULL);
	ck_assert_str_eq(mdata_get_songinfo(md), "test album");

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test15-title.ogg"), 0);
	ck_assert_ptr_eq(mdata_get_artist(md), NULL);
	ck_assert_ptr_eq(mdata_get_album(md), NULL);
	ck_assert_str_eq(mdata_get_title(md), "test title");
	ck_assert_str_eq(mdata_get_songinfo(md), "test title");

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test16-nometa.ogg"), 0);
	ck_assert_ptr_eq(mdata_get_artist(md), NULL);
	ck_assert_ptr_eq(mdata_get_album(md), NULL);
	ck_assert_ptr_eq(mdata_get_title(md), NULL);
	ck_assert_ptr_eq(mdata_get_songinfo(md), NULL);

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/.test17ogg"), 0);
	ck_assert_str_eq(mdata_get_name(md), "[unknown]");

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test18-emptymeta.ogg"), 0);
	ck_assert_ptr_eq(mdata_get_artist(md), NULL);
	ck_assert_ptr_eq(mdata_get_album(md), NULL);
	ck_assert_ptr_eq(mdata_get_title(md), NULL);
	ck_assert_ptr_eq(mdata_get_songinfo(md), NULL);

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test19-onlywhitespace.ogg"), 0);
	ck_assert_str_eq(mdata_get_artist(md), "");
	ck_assert_str_eq(mdata_get_album(md), "");
	ck_assert_str_eq(mdata_get_title(md), "");
	ck_assert_ptr_eq(mdata_get_songinfo(md), NULL);
}
END_TEST

START_TEST(test_mdata_run_program)
{
	ck_assert_int_ne(mdata_run_program(md, SRCDIR "/nonexistent"), 0);
	ck_assert_ptr_eq(mdata_get_filename(md), NULL);

	ck_assert_int_ne(mdata_run_program(md, SRCDIR "/test01-artist+album+title.ogg"), 0);
	ck_assert_ptr_eq(mdata_get_filename(md), NULL);

	mdata_set_normalize_strings(md, 1);
	ck_assert_int_eq(mdata_run_program(md, SRCDIR "/test-meta01.sh"), 0);
	ck_assert_str_eq(mdata_get_filename(md), SRCDIR "/test-meta01.sh");
	ck_assert_str_eq(mdata_get_name(md), "[unknown]");
	ck_assert_str_eq(mdata_get_artist(md), "artist");
	ck_assert_str_eq(mdata_get_album(md), "album");
	ck_assert_str_eq(mdata_get_title(md), "title");
	ck_assert_str_eq(mdata_get_songinfo(md), "songinfo");
	mdata_set_normalize_strings(md, 0);
	ck_assert_int_eq(mdata_refresh(md), 0);
	ck_assert_str_eq(mdata_get_artist(md), "  artist  ");
	ck_assert_str_eq(mdata_get_album(md), "  album  ");
	ck_assert_str_eq(mdata_get_title(md), "  title  ");
	ck_assert_str_eq(mdata_get_songinfo(md), "  songinfo  ");

	mdata_set_normalize_strings(md, 1);

	ck_assert_int_ne(mdata_run_program(md, SRCDIR "/test-meta02-error.sh"), 0);
	ck_assert_str_eq(mdata_get_filename(md), SRCDIR "/test-meta01.sh");

	ck_assert_int_eq(mdata_run_program(md, SRCDIR "/test-meta03-huge.sh"), 0);

	ck_assert_int_ne(mdata_run_program(md, SRCDIR "/test-meta04-kill.sh"), 0);

	ck_assert_int_eq(mdata_run_program(md, SRCDIR "/test-meta05-empty.sh"), 0);
}
END_TEST

START_TEST(test_mdata_strformat)
{
	char	buf[BUFSIZ];
	int	ret;

	ck_assert_int_eq(mdata_parse_file(md, SRCDIR "/test01-artist+album+title.ogg"), 0);

	ck_assert_int_lt(mdata_strformat(md, buf, sizeof(buf), NULL), 0);

	ret = mdata_strformat(md, buf, sizeof(buf), "@a@/@b@/@t@/@T@/@s@");
	ck_assert_int_eq(ret, (int)strlen(buf));
	ck_assert_str_eq(buf,
	    "test artist/test album/test title"
	    "/" SRCDIR "/test01-artist+album+title.ogg"
	    "/test artist - test title - test album");
}
END_TEST

Suite *
mdata_suite(void)
{
	Suite	*s;
	TCase	*tc_mdata;

	s = suite_create("MData");

	tc_mdata = tcase_create("MData");
	tcase_add_checked_fixture(tc_mdata, setup_checked, teardown_checked);
	tcase_add_test(tc_mdata, test_mdata_md);
	tcase_add_test(tc_mdata, test_mdata_parse_file);
	tcase_add_test(tc_mdata, test_mdata_run_program);
	tcase_add_test(tc_mdata, test_mdata_strformat);
	suite_add_tcase(s, tc_mdata);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < cfg_set_program_name("check_mdata", NULL) ||
	    0 < log_init(cfg_get_program_name()))
		ck_abort_msg("setup_checked failed");

	md = mdata_create();
	ck_assert_ptr_ne(md, NULL);
}

void
teardown_checked(void)
{
	mdata_destroy(&md);
	ck_assert_ptr_eq(md, NULL);

	log_exit();
	cfg_exit();
}

int
main(void)
{
	int	 num_failed;
	Suite	*s;
	SRunner *sr;

	s = mdata_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
