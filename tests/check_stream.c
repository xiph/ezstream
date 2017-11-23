#include <check.h>

#include "cfg.h"
#include "mdata.h"
#include "stream.h"
#include "xalloc.h"

Suite * stream_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

START_TEST(test_stream)
{
	stream_t		 s;
	mdata_t 		 m;
	char			*m_str;
	cfg_server_t		 srv_cfg;
	cfg_stream_t		 str_cfg;
	cfg_server_list_t	 servers = cfg_get_servers();
	cfg_stream_list_t	 streams = cfg_get_streams();

	s = stream_create("test-stream");
	ck_assert_ptr_ne(s, NULL);

	ck_assert_int_ne(stream_connect(s), 0);
	stream_disconnect(s);
	stream_sync(s);
	ck_assert_int_ne(stream_send(s, NULL, 0), 0);

	srv_cfg = stream_get_cfg_server(s);
	ck_assert_ptr_ne(srv_cfg, NULL);
	str_cfg = stream_get_cfg_stream(s);
	ck_assert_ptr_ne(str_cfg, NULL);
	ck_assert_ptr_ne(stream_get_cfg_intake(s), NULL);

	ck_assert_int_ne(stream_configure(s), 0);
	cfg_server_set_hostname(srv_cfg, servers, "localhost", NULL);
	ck_assert_int_ne(stream_configure(s), 0);
	cfg_server_set_password(srv_cfg, servers, "test", NULL);
	ck_assert_int_ne(stream_configure(s), 0);
	cfg_server_set_tls(srv_cfg, servers, "required", NULL);
	ck_assert_int_ne(stream_configure(s), 0);
	cfg_server_set_tls(srv_cfg, servers, "may", NULL);
	ck_assert_int_ne(stream_configure(s), 0);
	cfg_server_set_tls(srv_cfg, servers, "none", NULL);
	ck_assert_int_ne(stream_configure(s), 0);
	cfg_stream_set_mountpoint(str_cfg, streams, "/test.ogg", NULL);
	ck_assert_int_ne(stream_configure(s), 0);
	cfg_stream_set_format(str_cfg, streams, "mp3", NULL);
	ck_assert_int_eq(stream_configure(s), 0);
	cfg_stream_set_format(str_cfg, streams, "vorbis", NULL);
	ck_assert_int_eq(stream_configure(s), 0);
	cfg_stream_set_stream_name(str_cfg, streams, "test", NULL);
	cfg_stream_set_stream_url(str_cfg, streams, "test", NULL);
	cfg_stream_set_stream_genre(str_cfg, streams, "test", NULL);
	cfg_stream_set_stream_description(str_cfg, streams, "test", NULL);
	cfg_stream_set_stream_quality(str_cfg, streams, "test", NULL);
	cfg_stream_set_stream_bitrate(str_cfg, streams, "test", NULL);
	cfg_stream_set_stream_samplerate(str_cfg, streams, "test", NULL);
	cfg_stream_set_stream_channels(str_cfg, streams, "test", NULL);
	cfg_stream_set_public(str_cfg, streams, "test", NULL);
	ck_assert_int_eq(stream_configure(s), 0);

	ck_assert_int_eq(stream_get_connected(s), 0);

	m = mdata_create();

	ck_assert_int_eq(mdata_parse_file(m, SRCDIR "/test01-artist+album+title.ogg"), 0);

	cfg_set_metadata_no_updates("yes", NULL);
	ck_assert_int_eq(stream_set_metadata(s, NULL, NULL), 0);
	cfg_set_metadata_no_updates("no", NULL);
	ck_assert_int_ne(stream_set_metadata(s, NULL, NULL), 0);

	/*
	 * Not running stream_set_metadata checked as libshout behaves
	 * different on different systems, making these unreliable ...
	 */
	m_str = NULL;
	stream_set_metadata(s, m, &m_str);
	xfree(m_str);
	m_str = NULL;
	ck_assert_int_eq(mdata_parse_file(m, SRCDIR "/test15-title.ogg"), 0);
	stream_set_metadata(s, m, &m_str);
	xfree(m_str);
	m_str = NULL;
	ck_assert_int_eq(mdata_parse_file(m, SRCDIR "/test16-nometa.ogg"), 0);
	stream_set_metadata(s, m, &m_str);
	xfree(m_str);
	m_str = NULL;
	cfg_set_metadata_format_str("test", NULL);
	ck_assert_int_eq(mdata_parse_file(m, SRCDIR "/test01-artist+album+title.ogg"), 0);
	stream_set_metadata(s, m, &m_str);
	xfree(m_str);
	m_str = NULL;

	mdata_destroy(&m);

	stream_destroy(&s);
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
	int	 num_failed;
	Suite	*s;
	SRunner	*sr;

	s = stream_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
