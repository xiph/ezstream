#include <sys/file.h>

#include <stdio.h>
#include <unistd.h>

#include <check.h>

#include "util.h"
#include "xalloc.h"

Suite * util_suite(void);

START_TEST(test_util_write_pid_file)
{
	FILE	*pidfile_2;

	ck_assert_int_eq(util_write_pid_file(NULL), 0);
	ck_assert_int_ne(util_write_pid_file("/nonexistent/path/check_util-write_pid_file.pid"), 0);

	pidfile_2 = fopen(BUILDDIR "/check_util-write_pid_file2.pid", "w");
	ck_assert_ptr_ne(pidfile_2, NULL);
	ck_assert_int_eq(flock(fileno(pidfile_2), LOCK_EX), 0);
	ck_assert_int_ne(util_write_pid_file(BUILDDIR "/check_util-write_pid_file2.pid"), 0);
	ck_assert_int_eq(flock(fileno(pidfile_2), LOCK_UN), 0);
	ck_assert_int_eq(fclose(pidfile_2), 0);

	unlink(BUILDDIR "/check_util-write_pid_file.pid");
	ck_assert_int_eq(util_write_pid_file(BUILDDIR "/check_util-write_pid_file.pid"), 0);
	ck_assert_int_eq(util_write_pid_file(BUILDDIR "/check_util-write_pid_file.pid"), 0);
	ck_assert_int_eq(unlink(BUILDDIR "/check_util-write_pid_file.pid"), 0);
	ck_assert_int_ne(access(BUILDDIR "/check_util-write_pid_file.pid", F_OK), 0);
}
END_TEST

START_TEST(test_util_strrcmp)
{
	ck_assert_int_ne(util_strrcmp(".m3u", "playlist.m3u"), 0);
	ck_assert_int_ne(util_strrcasecmp(".m3u", "playlist.m3u"), 0);

	ck_assert_int_ne(util_strrcmp("playlist.m3u", ".MEU"), 0);
	ck_assert_int_ne(util_strrcasecmp("playlist.m3u", ".MEU"), 0);

	ck_assert_int_ne(util_strrcmp("playlist.m3u", ".M3u"), 0);
	ck_assert_int_eq(util_strrcasecmp("playlist.m3u", ".M3u"), 0);

	ck_assert_int_eq(util_strrcmp("playlist.m3u", ".m3u"), 0);
	ck_assert_int_eq(util_strrcasecmp("playlist.m3u", ".m3u"), 0);
}
END_TEST

START_TEST(test_util_utf8sanity)
{
	const char	*test_str = "TESTing\t1234 !@#{}\n";
	char		*str;

	str = util_char2utf8(test_str);
	ck_assert_str_eq(str, test_str);
	xfree(str);

	str = util_utf82char(test_str);
	ck_assert_str_eq(str, test_str);
	xfree(str);
}
END_TEST

START_TEST(test_util_expand_words)
{
	ck_assert_ptr_eq(util_expand_words("", NULL), NULL);
}
END_TEST

START_TEST(test_util_shellquote)
{
	char	*str;

	str = util_shellquote("testing 1 2 3", 0);
	ck_assert_str_eq(str, "'testing 1 2 3'");
	xfree(str);

	str = util_shellquote("testing 1 2 3", 9);
	ck_assert_str_eq(str, "'testing'");
	xfree(str);

	str = util_shellquote("testing'1 2 3", 11);
	ck_assert_str_eq(str, "'testing'");
	xfree(str);

	str = util_shellquote("foo'bar", 0);
	ck_assert_str_eq(str, "'foo'\\''bar'");
	xfree(str);

	str = util_shellquote("foo\\bar", 0);
	ck_assert_str_eq(str, "'foo\\bar'");
	xfree(str);

	str = util_shellquote("''''", 0);
	ck_assert_str_eq(str, "''\\'''\\'''\\'''\\'''");
	xfree(str);

	str = util_shellquote("$(echo TEST)", 0);
	ck_assert_str_eq(str, "'$(echo TEST)'");
	xfree(str);

	str = util_shellquote("`echo TEST`", 1000000);
	ck_assert_str_eq(str, "'`echo TEST`'");
	xfree(str);
}
END_TEST

Suite *
util_suite(void)
{
	Suite	*s;
	TCase	*tc_util;

	s = suite_create("Util");

	tc_util = tcase_create("Util");
	tcase_add_test(tc_util, test_util_write_pid_file);
	tcase_add_test(tc_util, test_util_strrcmp);
	tcase_add_test(tc_util, test_util_utf8sanity);
	tcase_add_test(tc_util, test_util_expand_words);
	tcase_add_test(tc_util, test_util_shellquote);
	suite_add_tcase(s, tc_util);

	return (s);
}

int
main(void)
{
	unsigned int	 num_failed;
	Suite           *s;
	SRunner         *sr;

	s = util_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
