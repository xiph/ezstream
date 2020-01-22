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

#ifndef __CFG_SERVER_H__
#define __CFG_SERVER_H__

#define CFG_SERVER_DEFAULT_PORT	8000
#define CFG_SERVER_DEFAULT_USER	"source"

enum cfg_server_protocol {
	CFG_PROTO_HTTP = 0,
	CFG_PROTO_HTTPS,
	CFG_PROTO_ICY,
	CFG_PROTO_ROARAUDIO,
	CFG_PROTO_MIN = CFG_PROTO_HTTP,
	CFG_PROTO_MAX = CFG_PROTO_ROARAUDIO,
};

enum cfg_server_tls {
	CFG_TLS_MAY = 0,
	CFG_TLS_NONE,
	CFG_TLS_REQUIRED,
	CFG_TLS_MIN = CFG_TLS_MAY,
	CFG_TLS_MAX = CFG_TLS_REQUIRED,
};

typedef struct cfg_server *		cfg_server_t;
typedef struct cfg_server_list *	cfg_server_list_t;

cfg_server_list_t
	cfg_server_list_create(void);
void	cfg_server_list_destroy(cfg_server_list_t *);
unsigned int
	cfg_server_list_nentries(cfg_server_list_t);

cfg_server_t
	cfg_server_list_find(cfg_server_list_t, const char *);
cfg_server_t
	cfg_server_list_get(cfg_server_list_t, const char *);
void	cfg_server_list_foreach(cfg_server_list_t, void (*)(cfg_server_t,
	    void *), void *);

cfg_server_t
	cfg_server_create(const char *);
void	cfg_server_destroy(cfg_server_t *);

int	cfg_server_set_name(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_protocol(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_hostname(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_port(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_user(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_password(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_tls(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_tls_cipher_suite(cfg_server_t, cfg_server_list_t,
	    const char *, const char **);
int	cfg_server_set_ca_dir(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_ca_file(cfg_server_t, cfg_server_list_t, const char *,
	    const char **);
int	cfg_server_set_client_cert(cfg_server_t, cfg_server_list_t,
	    const char *, const char **);
int	cfg_server_set_reconnect_attempts(cfg_server_t, cfg_server_list_t,
	    const char *, const char **);

int	cfg_server_validate(cfg_server_t, const char **);

const char *
	cfg_server_get_name(cfg_server_t);
enum cfg_server_protocol
	cfg_server_get_protocol(cfg_server_t);
const char *
	cfg_server_get_protocol_str(cfg_server_t);
const char *
	cfg_server_get_hostname(cfg_server_t);
unsigned int
	cfg_server_get_port(cfg_server_t);
const char *
	cfg_server_get_user(cfg_server_t);
const char *
	cfg_server_get_password(cfg_server_t);
enum cfg_server_tls
	cfg_server_get_tls(cfg_server_t);
const char *
	cfg_server_get_tls_str(cfg_server_t);
const char *
	cfg_server_get_tls_cipher_suite(cfg_server_t);
const char *
	cfg_server_get_ca_dir(cfg_server_t);
const char *
	cfg_server_get_ca_file(cfg_server_t);
const char *
	cfg_server_get_client_cert(cfg_server_t);
unsigned int
	cfg_server_get_reconnect_attempts(cfg_server_t);

#endif /* __CFG_SERVER_H__ */
