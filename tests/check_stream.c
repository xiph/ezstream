#include <check.h>

#include "cfg.h"
#include "mdata.h"
#include "stream.h"

Suite * stream_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_stream)
{
	stream_t	 s1;
	stream_t	 s2;
	mdata_t 	 m;
	char		*m_str;

	s1 = stream_get("test-stream");
	ck_assert_ptr_ne(s1, NULL);
	s2 = stream_get("test-stream");
	ck_assert_ptr_eq(s1, s2);
	s2 = stream_get("test2-stream");
	ck_assert_ptr_ne(s2, NULL);
	ck_assert_ptr_ne(s1, s2);

	ck_assert_int_ne(stream_connect(s1), 0);
	stream_disconnect(s1);
	stream_sync(s1);
	ck_assert_int_ne(stream_send(s1, NULL, 0), 0);

	ck_assert_int_ne(stream_setup(s1), 0);
	cfg_set_server_hostname("localhost", NULL);
	ck_assert_int_ne(stream_setup(s1), 0);
	cfg_set_server_password("test", NULL);
	ck_assert_int_ne(stream_setup(s1), 0);
	cfg_set_server_tls("required", NULL);
	ck_assert_int_ne(stream_setup(s1), 0);
	cfg_set_server_tls("may", NULL);
	ck_assert_int_ne(stream_setup(s1), 0);
	cfg_set_server_tls("none", NULL);
	ck_assert_int_ne(stream_setup(s1), 0);
	cfg_set_stream_mountpoint("/test.ogg", NULL);
	ck_assert_int_ne(stream_setup(s1), 0);
	cfg_set_stream_format("mp3", NULL);
	ck_assert_int_eq(stream_setup(s1), 0);
	cfg_set_stream_format("vorbis", NULL);
	ck_assert_int_eq(stream_setup(s1), 0);
	cfg_set_stream_name("test", NULL);
	cfg_set_stream_url("test", NULL);
	cfg_set_stream_genre("test", NULL);
	cfg_set_stream_description("test", NULL);
	cfg_set_stream_quality("test", NULL);
	cfg_set_stream_bitrate("test", NULL);
	cfg_set_stream_samplerate("test", NULL);
	cfg_set_stream_channels("test", NULL);
	cfg_set_stream_server_public("test", NULL);
	ck_assert_int_eq(stream_setup(s1), 0);

	ck_assert_int_eq(stream_get_connected(s1), NULL);

	m = mdata_create();

	ck_assert_int_eq(mdata_parse_file(m, SRCDIR "/test01-artist+album+title.ogg"), 0);

	cfg_set_metadata_no_updates("yes", NULL);
	ck_assert_int_eq(stream_set_metadata(s1, NULL, NULL), 0);
	cfg_set_metadata_no_updates("no", NULL);
	ck_assert_int_ne(stream_set_metadata(s1, NULL, NULL), 0);

	m_str = NULL;
	ck_assert_int_ne(stream_set_metadata(s1, m, &m_str), 0);
	ck_assert_ptr_eq(m_str, NULL);
	ck_assert_int_eq(mdata_parse_file(m, SRCDIR "/test15-title.ogg"), 0);
	ck_assert_int_ne(stream_set_metadata(s1, m, &m_str), 0);
	ck_assert_ptr_eq(m_str, NULL);
	ck_assert_int_eq(mdata_parse_file(m, SRCDIR "/test16-nometa.ogg"), 0);
	ck_assert_int_ne(stream_set_metadata(s1, m, &m_str), 0);
	ck_assert_ptr_eq(m_str, NULL);
	cfg_set_metadata_format_str("test", NULL);
	ck_assert_int_eq(mdata_parse_file(m, SRCDIR "/test01-artist+album+title.ogg"), 0);
	ck_assert_int_ne(stream_set_metadata(s1, m, &m_str), 0);
	ck_assert_ptr_eq(m_str, NULL);

	mdata_destroy(&m);
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
	if (0 < cfg_init() ||
	    0 < stream_init())
		ck_abort_msg("setup_checked failed");
}

void
teardown_checked(void)
{
	stream_exit();
	cfg_exit();
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
