#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <check.h>

#include "cfg_private.h"
#include "cfgfile_xml.h"
#include "log.h"

static void	_test_cfgfile_rw(const char *cfgfile);

Suite * cfg_xmlfile_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

static void
_test_cfgfile_rw(const char *cfgfile)
{
	const char	*testfile = BUILDDIR "/check_cfgfile_xml.xml";
	FILE		*fp;
	int		 fd;
	int		 ret;

	ck_assert_int_eq(cfg_set_program_config_file(cfgfile, NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);

	fd = open(testfile, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR|S_IWUSR);
	ck_assert_int_ge(fd, 0);
	fp = fdopen(fd, "w");
	if (NULL == fp) {
		unlink(testfile);
		close(fd);
	}
	ck_assert_ptr_ne(fp, NULL);
	cfgfile_xml_print(fp);
	ck_assert_int_eq(fclose(fp), 0);

	ck_assert_int_eq(cfg_set_program_config_file(testfile, NULL), 0);
	ret = cfg_file_reload();
	ck_assert_int_eq(unlink(testfile), 0);
	ck_assert_int_eq(ret, 0);
}

START_TEST(test_reload)
{
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-file_template.xml");
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-full.xml");
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-metadata.xml");
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-minimal.xml");
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-stdin.xml");
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-video.xml");
	ck_assert_int_eq(cfg_set_program_config_file(EXAMPLESDIR "/ezstream-full.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), -1);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad2.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), -1);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad3.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), -1);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad4.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), -1);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad5.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), -1);
	ck_assert_int_eq(cfg_set_program_config_file(SRCDIR "/config-bad6.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), -1);
}
END_TEST

Suite *
cfg_xmlfile_suite(void)
{
	Suite	*s;
	TCase	*tc_xmlcfg;

	s = suite_create("Config");

	tc_xmlcfg = tcase_create("XmlConfigFile");
	tcase_add_checked_fixture(tc_xmlcfg, setup_checked, teardown_checked);
	tcase_add_test(tc_xmlcfg, test_reload);
	suite_add_tcase(s, tc_xmlcfg);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < cfg_set_program_name("check_cfg_xmlfile", NULL) ||
	    0 < log_init(cfg_get_program_name()))
		ck_abort_msg("setup_checked failed");
}

void
teardown_checked(void)
{
	log_exit();
	cfg_exit();
}

int
main(void)
{
	int	 num_failed;
	Suite	*s;
	SRunner	*sr;

	s = cfg_xmlfile_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
