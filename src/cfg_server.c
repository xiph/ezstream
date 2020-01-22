/*
 * Copyright (c) 2017 Moritz Grimm <mgrimm@mrsserver.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "compat.h"

#include <sys/queue.h>

#include <stdint.h>
#include <string.h>

#include "cfg_private.h"
#include "cfg_server.h"
#include "xalloc.h"

struct cfg_server {
	TAILQ_ENTRY(cfg_server)  entry;
	char			*name;
	enum cfg_server_protocol protocol;
	char			 hostname[NI_MAXHOST];
	unsigned int		 port;
	char			 user[UCREDS_SIZE];
	char			 password[UCREDS_SIZE];
	enum cfg_server_tls	 tls;
	char			 tls_cipher_suite[CSUITE_SIZE];
	char			 ca_dir[PATH_MAX];
	char			 ca_file[PATH_MAX];
	char			 client_cert[PATH_MAX];
	unsigned int		 reconnect_attempts;
};

TAILQ_HEAD(cfg_server_list, cfg_server);

struct cfg_server_list *
cfg_server_list_create(void)
{
	struct cfg_server_list *sl;

	sl = xcalloc(1UL, sizeof(*sl));
	TAILQ_INIT(sl);

	return (sl);
}

void
cfg_server_list_destroy(cfg_server_list_t *sl_p)
{
	struct cfg_server_list	*sl = *sl_p;
	struct cfg_server	*s;

	if (!sl)
		return;

	while (NULL != (s = TAILQ_FIRST(sl))) {
		TAILQ_REMOVE(sl, s, entry);
		cfg_server_destroy(&s);
	}

	xfree(sl);
	*sl_p = NULL;
}

unsigned int
cfg_server_list_nentries(struct cfg_server_list *sl)
{
	struct cfg_server	*s;
	unsigned int		 n = 0;

	TAILQ_FOREACH(s, sl, entry) {
		n++;
	}

	return (n);
}

struct cfg_server *
cfg_server_list_find(struct cfg_server_list *sl, const char *name)
{
	struct cfg_server	*s;

	TAILQ_FOREACH(s, sl, entry) {
		if (0 == strcasecmp(s->name, name))
			return (s);
	}

	return (NULL);
}

struct cfg_server *
cfg_server_list_get(struct cfg_server_list *sl, const char *name)
{
	struct cfg_server	*s;

	s = cfg_server_list_find(sl, name);
	if (s)
		return (s);
	s = cfg_server_create(name);
	if (!s)
		return (NULL);

	TAILQ_INSERT_TAIL(sl, s, entry);

	return (s);
}

void
cfg_server_list_foreach(struct cfg_server_list *sl,
    void (*cb)(cfg_server_t, void *), void *cb_arg)
{
	struct cfg_server	*s;

	TAILQ_FOREACH(s, sl, entry) {
		cb(s, cb_arg);
	}
}

struct cfg_server *
cfg_server_create(const char *name)
{
	struct cfg_server	*s;

	if (!name || !name[0])
		return (NULL);

	s = xcalloc(1UL, sizeof(*s));
	s->name = xstrdup(name);

	return (s);
}

void
cfg_server_destroy(struct cfg_server **s_p)
{
	struct cfg_server	*s = *s_p;

	xfree(s->name);
	xfree(s);
	*s_p = NULL;
}

int
cfg_server_set_name(struct cfg_server *s, struct cfg_server_list *sl,
    const char *name, const char **errstrp)
{
	struct cfg_server	*s2;

	if (!name || !name[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	s2 = cfg_server_list_find(sl, name);
	if (s2 && s2 != s) {
		if (errstrp)
			*errstrp = "already exists";
		return (-1);
	}

	SET_XSTRDUP(s->name, name, errstrp);

	return (0);
}

int
cfg_server_set_protocol(struct cfg_server *s, struct cfg_server_list *not_used,
    const char *protocol, const char **errstrp)
{
	(void)not_used;

	if (!protocol || !protocol[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (0 == strcasecmp("http", protocol))
		s->protocol = CFG_PROTO_HTTP;
	else if (0 == strcasecmp("https", protocol))
		s->protocol = CFG_PROTO_HTTPS;
	else if (0 == strcasecmp("icy", protocol))
		s->protocol = CFG_PROTO_ICY;
	else if (0 == strcasecmp("roaraudio", protocol))
		s->protocol = CFG_PROTO_ROARAUDIO;
	else {
		if (NULL != errstrp)
			*errstrp = "unsupported";
		return (-1);
	}
	return (0);
}

int
cfg_server_set_hostname(struct cfg_server *s, struct cfg_server_list *not_used,
    const char *hostname, const char **errstrp)
{
	(void)not_used;
	SET_STRLCPY(s->hostname, hostname, errstrp);
	return (0);
}

int
cfg_server_set_port(struct cfg_server *s, struct cfg_server_list *not_used,
    const char *port_str, const char **errstrp)
{
	const char	*errstr;
	unsigned int	 port;

	(void)not_used;

	if (!port_str || !port_str[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	port = (unsigned int)strtonum(port_str, 1, UINT16_MAX, &errstr);
	if (errstr) {
		if (errstrp)
			*errstrp = errstr;
		return (-1);
	}
	s->port = port;

	return (0);
}

int
cfg_server_set_user(struct cfg_server *s, struct cfg_server_list *not_used,
    const char *user, const char **errstrp)
{
	(void)not_used;
	SET_STRLCPY(s->user, user, errstrp);
	return (0);
}

int
cfg_server_set_password(struct cfg_server *s, struct cfg_server_list *not_used,
    const char *password, const char **errstrp)
{
	(void)not_used;
	SET_STRLCPY(s->password, password, errstrp);
	return (0);
}

int
cfg_server_set_tls(struct cfg_server *s, struct cfg_server_list *not_used,
    const char *tls, const char **errstrp)
{
	(void)not_used;

	if (!tls || !tls[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (0 == strcasecmp("may", tls))
		s->tls = CFG_TLS_MAY;
	else if (0 == strcasecmp("none", tls))
		s->tls = CFG_TLS_NONE;
	else if (0 == strcasecmp("required", tls))
		s->tls = CFG_TLS_REQUIRED;
	else {
		if (NULL != errstrp)
			*errstrp = "invalid";
		return (-1);
	}
	return (0);
}

int
cfg_server_set_tls_cipher_suite(struct cfg_server *s,
    struct cfg_server_list *not_used,const char *suite, const char **errstrp)
{
	(void)not_used;
	SET_STRLCPY(s->tls_cipher_suite, suite, errstrp);
	return (0);
}

int
cfg_server_set_ca_dir(struct cfg_server *s, struct cfg_server_list *not_used,
    const char *ca_dir, const char **errstrp)
{
	(void)not_used;
	SET_STRLCPY(s->ca_dir, ca_dir, errstrp);
	return (0);
}

int
cfg_server_set_ca_file(struct cfg_server *s, struct cfg_server_list *not_used,
    const char *ca_file, const char **errstrp)
{
	(void)not_used;
	SET_STRLCPY(s->ca_file, ca_file, errstrp);
	return (0);
}

int
cfg_server_set_client_cert(struct cfg_server *s,
    struct cfg_server_list *not_used, const char *client_cert,
    const char **errstrp)
{
	(void)not_used;
	SET_STRLCPY(s->client_cert, client_cert, errstrp);
	return (0);
}

int
cfg_server_set_reconnect_attempts(struct cfg_server *s,
    struct cfg_server_list *not_used,const char *num_str,
    const char **errstrp)
{
	(void)not_used;
	SET_UINTNUM(s->reconnect_attempts, num_str, errstrp);
	return (0);
}

int
cfg_server_validate(struct cfg_server *s, const char **errstrp)
{
	if (!cfg_server_get_hostname(s)) {
		if (NULL != errstrp)
			*errstrp = "hostname missing";
		return (-1);
	}

	if (!cfg_server_get_password(s)) {
		if (NULL != errstrp)
			*errstrp = "password missing";
		return (-1);
	}

	return (0);
}

const char *
cfg_server_get_name(struct cfg_server *s)
{
	return (s->name);
}

enum cfg_server_protocol
cfg_server_get_protocol(struct cfg_server *s)
{
	return (s->protocol);
}

const char *
cfg_server_get_protocol_str(struct cfg_server *s)
{
	switch (s->protocol) {
	case CFG_PROTO_HTTPS:
		return ("https");
	case CFG_PROTO_ICY:
		return ("icy");
	case CFG_PROTO_ROARAUDIO:
		return ("roaraudio");
	case CFG_PROTO_HTTP:
	default:
		return ("http");
	}
}

const char *
cfg_server_get_hostname(struct cfg_server *s)
{
	return (s->hostname[0] ? s->hostname : NULL);
}

unsigned int
cfg_server_get_port(struct cfg_server *s)
{
	return (s->port ? s->port : CFG_SERVER_DEFAULT_PORT);
}

const char *
cfg_server_get_user(struct cfg_server *s)
{
	return (s->user[0] ? s->user : CFG_SERVER_DEFAULT_USER);
}

const char *
cfg_server_get_password(struct cfg_server *s)
{
	return (s->password[0] ? s->password : NULL);
}

enum cfg_server_tls
cfg_server_get_tls(struct cfg_server *s)
{
	if (CFG_PROTO_HTTPS == s->protocol)
		return (CFG_TLS_REQUIRED);
	return (s->tls);
}

const char *
cfg_server_get_tls_str(struct cfg_server *s)
{
	if (CFG_PROTO_HTTPS == s->protocol)
		return ("required");
	switch (s->tls) {
	case CFG_TLS_NONE:
		return ("none");
	case CFG_TLS_REQUIRED:
		return ("required");
	case CFG_TLS_MAY:
	default:
		return ("may");
	}
}

const char *
cfg_server_get_tls_cipher_suite(struct cfg_server *s)
{
	return (s->tls_cipher_suite[0]
	    ? s->tls_cipher_suite
	    : NULL);
}

const char *
cfg_server_get_ca_dir(struct cfg_server *s)
{
	return (s->ca_dir[0] ? s->ca_dir : NULL);
}

const char *
cfg_server_get_ca_file(struct cfg_server *s)
{
	return (s->ca_file[0] ? s->ca_file : NULL);
}

const char *
cfg_server_get_client_cert(struct cfg_server *s)
{
	return (s->client_cert[0] ? s->client_cert : NULL);
}

unsigned int
cfg_server_get_reconnect_attempts(struct cfg_server *s)
{
	return (s->reconnect_attempts);
}
