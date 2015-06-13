#include <check.h>

#include "xalloc.h"

Suite * xalloc_suite(void);

START_TEST(test_malloc)
{
	void	*p;

	p = xmalloc(0UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
	p = xmalloc(1UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
}
END_TEST

START_TEST(test_calloc)
{
	void	*p;

	p = xcalloc(0UL, 0UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
	p = xcalloc(1UL, 0UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
	p = xcalloc(0UL, 1UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
	p = xcalloc(1UL, 1UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
}
END_TEST

START_TEST(test_reallocarray)
{
	void	*p;

	p = xreallocarray(NULL, 0UL, 0UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
	p = xreallocarray(NULL, 1UL, 0UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
	p = xreallocarray(NULL, 0UL, 1UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
	p = xreallocarray(NULL, 1UL, 1UL);
	ck_assert_ptr_ne(p, NULL);
	p = xreallocarray(p, 2UL, 2UL);
	ck_assert_ptr_ne(p, NULL);
	p = xreallocarray(p, 1UL, 1UL);
	ck_assert_ptr_ne(p, NULL);
	xfree(p);
}
END_TEST

START_TEST(test_strdup)
{
	char	*s;

	s = xstrdup("test");
	ck_assert_str_eq(s, "test");
	xfree(s);
}
END_TEST

Suite *
xalloc_suite(void)
{
	Suite	*s;
	TCase	*tc_xalloc;

	s = suite_create("Xalloc");

	tc_xalloc = tcase_create("Xalloc");
	tcase_add_test(tc_xalloc, test_malloc);
	tcase_add_test(tc_xalloc, test_calloc);
	tcase_add_test(tc_xalloc, test_reallocarray);
	tcase_add_test(tc_xalloc, test_strdup);
	suite_add_tcase(s, tc_xalloc);

	return (s);
}

int
main(void)
{
	unsigned int	 num_failed;
	Suite           *s;
	SRunner         *sr;

	s = xalloc_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
