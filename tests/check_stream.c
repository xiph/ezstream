#include <check.h>

#include "stream.h"

Suite * stream_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_stream)
{
}
END_TEST

Suite *
stream_suite(void)
{
	Suite	*s;
	TCase	*tc_stream;

	s = suite_create("Stream");

	tc_stream = tcase_create("Stream");
	tcase_add_checked_fixture(tc_stream, setup_checked, teardown_checked);
	tcase_add_test(tc_stream, test_stream);
	suite_add_tcase(s, tc_stream);

	return (s);
}

void
setup_checked(void)
{
	if (0 < stream_init())
		ck_abort_msg("setup_checked failed");
}

void
teardown_checked(void)
{
	stream_exit();
}

int
main(void)
{
	unsigned int	 num_failed;
	Suite           *s;
	SRunner         *sr;

	s = stream_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
