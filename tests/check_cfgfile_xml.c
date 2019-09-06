#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <check.h>

#include "cfg.h"
#include "cfgfile_xml.h"
#include "log.h"
#include "stream.h"

static void	_test_cfgfile_rw(const char *cfgfile);

Suite * cfgfile_xml_suite(void);
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
	assert(0 <= fd);
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
	stream_t	test_stream;

	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-file_template.xml");
	ck_assert_int_eq(cfg_set_program_config_file(EXAMPLESDIR "/ezstream-file_template.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);
	test_stream = stream_create(CFG_DEFAULT);
	ck_assert_int_eq(stream_configure(test_stream), 0);
	stream_destroy(&test_stream);
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-full.xml");
	ck_assert_int_eq(cfg_set_program_config_file(EXAMPLESDIR "/ezstream-full.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);
	test_stream = stream_create(CFG_DEFAULT);
	ck_assert_int_eq(stream_configure(test_stream), 0);
	stream_destroy(&test_stream);
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-metadata.xml");
	ck_assert_int_eq(cfg_set_program_config_file(EXAMPLESDIR "/ezstream-metadata.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);
	test_stream = stream_create(CFG_DEFAULT);
	ck_assert_int_eq(stream_configure(test_stream), 0);
	stream_destroy(&test_stream);
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-minimal.xml");
	ck_assert_int_eq(cfg_set_program_config_file(EXAMPLESDIR "/ezstream-minimal.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);
	test_stream = stream_create(CFG_DEFAULT);
	ck_assert_int_eq(stream_configure(test_stream), 0);
	stream_destroy(&test_stream);
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-stdin.xml");
	ck_assert_int_eq(cfg_set_program_config_file(EXAMPLESDIR "/ezstream-stdin.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);
	test_stream = stream_create(CFG_DEFAULT);
	ck_assert_int_eq(stream_configure(test_stream), 0);
	stream_destroy(&test_stream);
	_test_cfgfile_rw(EXAMPLESDIR "/ezstream-video.xml");
	ck_assert_int_eq(cfg_set_program_config_file(EXAMPLESDIR "/ezstream-video.xml",
	    NULL), 0);
	ck_assert_int_eq(cfg_file_reload(), 0);
	test_stream = stream_create(CFG_DEFAULT);
	ck_assert_int_eq(stream_configure(test_stream), 0);
	stream_destroy(&test_stream);
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
cfgfile_xml_suite(void)
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
	    0 < cfg_set_program_name("check_cfgfile_xml", NULL) ||
	    0 < log_init(cfg_get_program_name()) ||
	    0 < stream_init())
		ck_abort_msg("setup_checked failed");
}

void
teardown_checked(void)
{
	stream_exit();
	log_exit();
	cfg_exit();
}

int
main(void)
{
	int	 num_failed;
	Suite	*s;
	SRunner *sr;

	s = cfgfile_xml_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
