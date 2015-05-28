#include <check.h>

#include "playlist.h"

Suite * playlist_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_playlist_file)
{
	playlist_t	p;

	ck_assert_ptr_eq(playlist_read("nonexistent.txt"), NULL);
	p = playlist_read(SRCDIR "/playlist.txt");
	ck_assert_ptr_ne(p, NULL);
	ck_assert_uint_gt(playlist_get_num_items(p), 0);
	ck_assert_str_eq(playlist_get_next(p), "1.ogg");
	playlist_skip_next(p);
	ck_assert_uint_eq(playlist_get_position(p), 2);
	ck_assert_str_eq(playlist_get_next(p), "3.ogg");
	ck_assert_int_eq(playlist_goto_entry(p, "2.ogg"), 1);
	ck_assert_str_eq(playlist_get_next(p), "2.ogg");
	playlist_rewind(p);
	ck_assert_str_eq(playlist_get_next(p), "1.ogg");
	ck_assert_int_eq(playlist_goto_entry(p, "5.ogg"), 1);
	ck_assert_int_eq(playlist_goto_entry(p, "6.ogg"), 0);
	ck_assert_str_eq(playlist_get_next(p), "5.ogg");
	ck_assert_ptr_eq(playlist_get_next(p), NULL);
	ck_assert_int_eq(playlist_reread(&p), 1);
	ck_assert_str_eq(playlist_get_next(p), "1.ogg");
	playlist_shuffle(p);
	playlist_free(&p);
}
END_TEST

START_TEST(test_playlist_program)
{
	playlist_t	p;

	ck_assert_ptr_eq(playlist_program("nonexistent.sh"), NULL);
	p = playlist_program(EXAMPLESDIR "/play.sh");
	ck_assert_ptr_ne(p, NULL);
	ck_assert_str_eq(playlist_get_next(p),
	    "Great_Artist_-_Great_Song.ogg");
	ck_assert_uint_eq(playlist_get_num_items(p), 0);
	ck_assert_uint_eq(playlist_get_position(p), 0);
	ck_assert_int_eq(playlist_goto_entry(p,
	    "Great_Artist_-_Great_Song.ogg"), 0);
	ck_assert_int_eq(playlist_reread(p), 0);
	playlist_free(&p);
}
END_TEST

Suite *
playlist_suite(void)
{
	Suite	*s;
	TCase	*tc_playlist;

	s = suite_create("Playlist");

	tc_playlist = tcase_create("Playlist");
	tcase_add_checked_fixture(tc_playlist, setup_checked, teardown_checked);
	tcase_add_test(tc_playlist, test_playlist_file);
	tcase_add_test(tc_playlist, test_playlist_program);
	suite_add_tcase(s, tc_playlist);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < log_init() ||
	    0 < playlist_init())
		ck_abort_msg("setup_checked failed");
}

void
teardown_checked(void)
{
	playlist_exit();
	log_exit();
	cfg_exit();
}

int
main(void)
{
	unsigned int	 num_failed;
	Suite           *s;
	SRunner         *sr;

	s = playlist_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
