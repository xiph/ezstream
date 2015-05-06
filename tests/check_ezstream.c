#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "compat.h"

#include <check.h>

START_TEST(test_ezstream)
{
}
END_TEST

Suite *
ezstream_suite(void)
{
	Suite   *s;
	TCase   *tc_core;

	s = suite_create("Ezstream");

	tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_ezstream);
	suite_add_tcase(s, tc_core);

	return (s);
}

int
main(void)
{
	unsigned int	 num_failed;
	Suite           *s;
	SRunner         *sr;

	s = ezstream_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
