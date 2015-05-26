#include <check.h>

#include "playlist.h"

Suite * playlist_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_read)
{
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
	tcase_add_test(tc_playlist, test_read);
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
