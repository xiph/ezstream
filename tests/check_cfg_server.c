#include <check.h>
#include <limits.h>
#include <netdb.h>

#include "cfg_private.h"
#include "log.h"

#include "check_cfg.h"

Suite * cfg_suite(void);
void	setup_checked(void);
void	teardown_checked(void);

cfg_server_list_t	servers;

START_TEST(test_server_list_get)
{
	cfg_server_t	srv, srv2;

	ck_assert_ptr_eq(cfg_server_list_get(servers, NULL), NULL);
	ck_assert_ptr_eq(cfg_server_list_get(servers, ""), NULL);

	srv = cfg_server_list_get(servers, "TeSt");
	srv2 = cfg_server_list_get(servers, "test");
	ck_assert_ptr_eq(srv, srv2);
}
END_TEST

START_TEST(test_server_protocol)
{
	cfg_server_t	 srv = cfg_server_list_get(servers, "test_server_protocol");
	const char	*errstr2;

	TEST_EMPTYSTR_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_protocol);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_server_set_protocol(srv, servers, "invalid",
	    NULL), -1);
	ck_assert_int_eq(cfg_server_set_protocol(srv, servers, "invalid",
	    &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);
	ck_assert_str_eq(errstr2, "unsupported");

	ck_assert_int_eq(cfg_server_set_protocol(srv, servers, "hTtP", NULL),
	    0);
	ck_assert_int_eq(cfg_server_get_protocol(srv), CFG_PROTO_HTTP);
	ck_assert_str_eq(cfg_server_get_protocol_str(srv), "http");
	ck_assert_int_eq(cfg_server_set_protocol(srv, servers, "HtTpS", NULL),
	    0);
	ck_assert_int_eq(cfg_server_get_protocol(srv), CFG_PROTO_HTTPS);
	ck_assert_str_eq(cfg_server_get_protocol_str(srv), "https");
}
END_TEST

START_TEST(test_server_hostname)
{
	TEST_STRLCPY_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_hostname, cfg_server_get_hostname, NI_MAXHOST);
}
END_TEST

START_TEST(test_server_port)
{
	cfg_server_t	 srv = cfg_server_list_get(servers, "test_server_port");
	const char	*errstr2;

	ck_assert_uint_eq(cfg_server_get_port(srv), DEFAULT_PORT);

	TEST_EMPTYSTR_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_port);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_server_set_port(srv, servers, "0", NULL), -1);
	ck_assert_int_eq(cfg_server_set_port(srv, servers, "0", &errstr2), -1);
	ck_assert_ptr_ne(errstr2, NULL);

	errstr2 = NULL;
	ck_assert_int_eq(cfg_server_set_port(srv, servers, "65536", &errstr2),
	    -1);
	ck_assert_ptr_ne(errstr2, NULL);

	ck_assert_int_eq(cfg_server_set_port(srv, servers, "8008", NULL), 0);
	ck_assert_uint_eq(cfg_server_get_port(srv), 8008);
}
END_TEST

START_TEST(test_server_user)
{
	cfg_server_t	 srv = cfg_server_list_get(servers, "test_server_user");

	ck_assert_str_eq(cfg_server_get_user(srv), DEFAULT_USER);
	TEST_STRLCPY_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_user, cfg_server_get_user, UCREDS_SIZE);
}
END_TEST

START_TEST(test_server_password)
{
	TEST_STRLCPY_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_password, cfg_server_get_password, UCREDS_SIZE);
}
END_TEST

START_TEST(test_server_ca_dir)
{
	TEST_STRLCPY_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_ca_dir, cfg_server_get_ca_dir, PATH_MAX);
}
END_TEST

START_TEST(test_server_tls)
{
	cfg_server_t	 srv = cfg_server_list_get(servers, "test_server_tls");
	const char	*errstr = NULL;

	ck_assert_int_eq(cfg_server_set_tls(srv, servers, "", NULL), -1);
	ck_assert_int_eq(cfg_server_set_tls(srv, servers, "", &errstr), -1);
	ck_assert_str_eq(errstr, "empty");
	ck_assert_int_eq(cfg_server_set_tls(srv, servers, "test", &errstr),
	    -1);
	ck_assert_str_eq(errstr, "invalid");
	ck_assert_int_eq(cfg_server_get_tls(srv), CFG_TLS_MAY);
	ck_assert_int_eq(cfg_server_set_tls(srv, servers, "None", NULL), 0);
	ck_assert_int_eq(cfg_server_get_tls(srv), CFG_TLS_NONE);
	ck_assert_int_eq(cfg_server_set_tls(srv, servers, "Required", NULL),
	    0);
	ck_assert_int_eq(cfg_server_get_tls(srv), CFG_TLS_REQUIRED);
	ck_assert_int_eq(cfg_server_set_tls(srv, servers, "May", NULL), 0);
	ck_assert_int_eq(cfg_server_get_tls(srv), CFG_TLS_MAY);
}
END_TEST

START_TEST(test_server_tls_cipher_suite)
{
	TEST_STRLCPY_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_tls_cipher_suite, cfg_server_get_tls_cipher_suite,
	    CSUITE_SIZE);
}
END_TEST

START_TEST(test_server_ca_file)
{
	TEST_STRLCPY_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_ca_file, cfg_server_get_ca_file, PATH_MAX);
}
END_TEST

START_TEST(test_server_client_cert)
{
	TEST_STRLCPY_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_client_cert, cfg_server_get_client_cert, PATH_MAX);
}
END_TEST

START_TEST(test_server_reconnect_attempts)
{
	TEST_UINTNUM_T(cfg_server_t, cfg_server_list_get, servers,
	    cfg_server_set_reconnect_attempts,
	    cfg_server_get_reconnect_attempts);
}
END_TEST

START_TEST(test_server_validate)
{
	cfg_server_t	 srv = cfg_server_list_get(servers, "test_server_validate");
	const char	*errstr = NULL;

	ck_assert_int_ne(cfg_server_validate(srv, NULL), 0);
	ck_assert_int_ne(cfg_server_validate(srv, &errstr), 0);
	ck_assert_str_eq(errstr, "hostname missing");
	ck_assert_int_eq(cfg_server_set_hostname(srv, servers, "localhost",
	    NULL), 0);

	ck_assert_int_ne(cfg_server_validate(srv, &errstr), 0);
	ck_assert_str_eq(errstr, "password missing");
	ck_assert_int_eq(cfg_server_set_password(srv, servers, "secret", NULL),
	    0);

	ck_assert_int_eq(cfg_server_validate(srv, NULL), 0);
}
END_TEST

Suite *
cfg_suite(void)
{
	Suite	*s;
	TCase	*tc_server;

	s = suite_create("Config");

	tc_server = tcase_create("Server");
	tcase_add_checked_fixture(tc_server, setup_checked, teardown_checked);
	tcase_add_test(tc_server, test_server_list_get);
	tcase_add_test(tc_server, test_server_protocol);
	tcase_add_test(tc_server, test_server_hostname);
	tcase_add_test(tc_server, test_server_port);
	tcase_add_test(tc_server, test_server_user);
	tcase_add_test(tc_server, test_server_password);
	tcase_add_test(tc_server, test_server_tls);
	tcase_add_test(tc_server, test_server_tls_cipher_suite);
	tcase_add_test(tc_server, test_server_ca_dir);
	tcase_add_test(tc_server, test_server_ca_file);
	tcase_add_test(tc_server, test_server_client_cert);
	tcase_add_test(tc_server, test_server_reconnect_attempts);
	tcase_add_test(tc_server, test_server_validate);
	suite_add_tcase(s, tc_server);

	return (s);
}

void
setup_checked(void)
{
	if (0 < cfg_init() ||
	    0 < cfg_set_program_name("check_cfg_server", NULL) ||
	    0 < log_init(cfg_get_program_name()))
		ck_abort_msg("setup_checked failed");

	servers = cfg_server_list_create();
}

void
teardown_checked(void)
{
	cfg_server_list_destroy(&servers);

	log_exit();
	cfg_exit();
}

int
main(void)
{
	int	 num_failed;
	Suite	*s;
	SRunner *sr;

	s = cfg_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	num_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	if (num_failed)
		return (1);
	return (0);
}
