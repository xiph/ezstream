/*
 * Copyright (c) 2015 Moritz Grimm <mgrimm@mrsserver.net>
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <shout/shout.h>

#include "cfg.h"
#include "log.h"
#include "mdata.h"
#include "stream.h"
#include "util.h"
#include "xalloc.h"

struct stream {
	char	*name;
	shout_t *shout;
};

static int	_stream_cfg_server(struct stream *, cfg_server_t);
static int	_stream_cfg_tls(struct stream *, cfg_server_t);
static int	_stream_cfg_stream(struct stream *, cfg_stream_t);
static void	_stream_reset(struct stream *);

static int
_stream_cfg_server(struct stream *s, cfg_server_t cfg_server)
{
	switch (cfg_server_get_protocol(cfg_server)) {
	case CFG_PROTO_HTTP:
		if (SHOUTERR_SUCCESS !=
		    shout_set_protocol(s->shout, SHOUT_PROTOCOL_HTTP)) {
			log_error("%s: protocol: %s",
			    s->name, shout_get_error(s->shout));
			return (-1);
		}
		break;
	default:
		log_error("%s: protocol: unsupported: %s",
		    s->name, cfg_server_get_protocol_str(cfg_server));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_host(s->shout, cfg_server_get_hostname(cfg_server))) {
		log_error("%s: hostname: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_port(s->shout, (unsigned short)cfg_server_get_port(cfg_server))) {
		log_error("%s: port: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_user(s->shout, cfg_server_get_user(cfg_server))) {
		log_error("%s: user: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_password(s->shout, cfg_server_get_password(cfg_server))) {
		log_error("%s: password: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}

	return (0);
}

static int
_stream_cfg_tls(struct stream *s, cfg_server_t cfg_server)
{
#ifdef SHOUT_TLS_AUTO
	int	tls_req;

	switch (cfg_server_get_tls(cfg_server)) {
	case CFG_TLS_NONE:
		tls_req = SHOUT_TLS_DISABLED;
		break;
	case CFG_TLS_MAY:
		tls_req = SHOUT_TLS_AUTO;
		break;
	case CFG_TLS_REQUIRED:
		tls_req = SHOUT_TLS_AUTO_NO_PLAIN;
		break;
	default:
		log_error("%s: tls: invalid", s->name);
		return (-1);
	}
	if (SHOUTERR_SUCCESS != shout_set_tls(s->shout, tls_req)) {
		log_error("%s: tls: %s", s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_server_get_ca_dir(cfg_server) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_ca_directory(s->shout, cfg_server_get_ca_dir(cfg_server))) {
		log_error("%s: ca_dir: %s", s->name,
		    shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_server_get_ca_file(cfg_server) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_ca_file(s->shout, cfg_server_get_ca_file(cfg_server))) {
		log_error("%s: ca_file: %s", s->name,
		    shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_server_get_tls_cipher_suite(cfg_server) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_allowed_ciphers(s->shout, cfg_server_get_tls_cipher_suite(cfg_server))) {
		log_error("%s: tls_cipher_suite: %s", s->name,
		    shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_server_get_client_cert(cfg_server) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_client_certificate(s->shout, cfg_server_get_client_cert(cfg_server))) {
		log_error("%s: client_cert: %s", s->name,
		    shout_get_error(s->shout));
		return (-1);
	}
#else /* SHOUT_TLS_AUTO */
# warning "libshout library does not support TLS"
	switch (cfg_server_get_tls(cfg_server)) {
	case CFG_TLS_MAY:
		log_warning("%s: TLS optional but not supported by libshout",
		    s->name);
		break;
	case CFG_TLS_REQUIRED:
		log_error("%s: TLS required but not supported by libshout",
		    s->name);
		return (-1);
	default:
		break;
	}
#endif /* SHOUT_TLS_AUTO */

	return (0);
}

static int
_stream_cfg_stream(struct stream *s, cfg_stream_t cfg_stream)
{
	if (SHOUTERR_SUCCESS !=
	    shout_set_mount(s->shout, cfg_stream_get_mountpoint(cfg_stream))) {
		log_error("%s: mountpoint: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	switch (cfg_stream_get_format(cfg_stream)) {
	case CFG_STREAM_VORBIS:
	case CFG_STREAM_THEORA:
		if (SHOUTERR_SUCCESS !=
		    shout_set_format(s->shout, SHOUT_FORMAT_OGG)) {
			log_error("%s: format_ogg: %s",
			    s->name, shout_get_error(s->shout));
			return (-1);
		}
		break;
	case CFG_STREAM_MP3:
		if (SHOUTERR_SUCCESS !=
		    shout_set_format(s->shout, SHOUT_FORMAT_MP3)) {
			log_error("%s: format_mp3: %s",
			    s->name, shout_get_error(s->shout));
			return (-1);
		}
		break;
	default:
		log_error("%s: format: unsupported: %s",
		    s->name, cfg_stream_get_format_str(cfg_stream));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_public(s->shout, (unsigned int)cfg_stream_get_public(cfg_stream))) {
		log_error("%s: public: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_stream_get_stream_name(cfg_stream) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_name(s->shout, cfg_stream_get_stream_name(cfg_stream))) {
		log_error("%s: name: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_stream_get_stream_url(cfg_stream) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_url(s->shout, cfg_stream_get_stream_url(cfg_stream))) {
		log_error("%s: url: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_stream_get_stream_genre(cfg_stream) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_genre(s->shout, cfg_stream_get_stream_genre(cfg_stream))) {
		log_error("%s: genre: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_stream_get_stream_description(cfg_stream) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_description(s->shout, cfg_stream_get_stream_description(cfg_stream))) {
		log_error("%s: description: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_stream_get_stream_quality(cfg_stream) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(s->shout, SHOUT_AI_QUALITY, cfg_stream_get_stream_quality(cfg_stream))) {
		log_error("%s: ai_quality: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_stream_get_stream_bitrate(cfg_stream) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(s->shout, SHOUT_AI_BITRATE, cfg_stream_get_stream_bitrate(cfg_stream))) {
		log_error("%s: ai_bitrate: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_stream_get_stream_samplerate(cfg_stream) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(s->shout, SHOUT_AI_SAMPLERATE, cfg_stream_get_stream_samplerate(cfg_stream))) {
		log_error("%s: ai_samplerate: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_stream_get_stream_channels(cfg_stream) &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(s->shout, SHOUT_AI_CHANNELS, cfg_stream_get_stream_channels(cfg_stream))) {
		log_error("%s: ai_channels: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}

	return (0);
}

void
_stream_reset(struct stream *s)
{
	if (!s->shout)
		return;
	shout_free(s->shout);
	s->shout = shout_new();
	if (NULL == s->shout) {
		log_syserr(ALERT, ENOMEM, "shout_new");
		exit(1);
	}
}

int
stream_init(void)
{
	shout_init();
	return (0);
}

void
stream_exit(void)
{
	shout_shutdown();
}

struct stream *
stream_create(const char *name)
{
	struct stream	*s;

	s = xcalloc(1UL, sizeof(*s));
	s->name = xstrdup(name);
	s->shout = shout_new();
	if (NULL == s->shout) {
		log_syserr(ALERT, ENOMEM, "shout_new");
		exit(1);
	}

	return (s);
}

void
stream_destroy(struct stream **s_p)
{
	struct stream	*s = *s_p;

	shout_free(s->shout);
	xfree(s->name);
	xfree(s);
	*s_p = NULL;
}

int
stream_configure(struct stream *s)
{
	cfg_stream_list_t	 streams;
	cfg_server_list_t	 servers;
	cfg_stream_t		 cfg_stream;
	cfg_server_t		 cfg_server;
	const char		*server;

	streams = cfg_get_streams();
	cfg_stream = cfg_stream_list_find(streams, s->name);
	if (!cfg_stream) {
		log_error("%s: stream has no configuration", s->name);
		return (-1);
	}
	servers = cfg_get_servers();
	server = cfg_stream_get_server(cfg_stream);
	if (!server)
		server = CFG_DEFAULT;
	cfg_server = cfg_server_list_find(servers, server);
	if (!cfg_server) {
		log_error("%s: stream server has no configuration: %s",
		    s->name, cfg_stream_get_server(cfg_stream));
		return (-1);
	}

	if (0 != _stream_cfg_server(s, cfg_server) ||
	    0 != _stream_cfg_tls(s, cfg_server) ||
	    0 != _stream_cfg_stream(s, cfg_stream)) {
		_stream_reset(s);
		return (-1);
	}

	return (0);
}

int
stream_set_metadata(struct stream *s, mdata_t md, char **md_str)
{
	shout_metadata_t	*shout_md = NULL;
	int			 ret;

	if (cfg_get_metadata_no_updates())
		return (0);

	if (md == NULL)
		return (-1);

	if ((shout_md = shout_metadata_new()) == NULL) {
		log_syserr(ALERT, ENOMEM, "shout_metadata_new");
		exit(1);
	}

	/*
	 * We can do this, because we know how libshout works. This adds
	 * "charset=UTF-8" to the HTTP metadata update request and has the
	 * desired effect of letting newer-than-2.3.1 versions of Icecast know
	 * which encoding we're using.
	 */
	if (shout_metadata_add(shout_md, "charset", "UTF-8") != SHOUTERR_SUCCESS) {
		/* Assume SHOUTERR_MALLOC */
		log_syserr(ALERT, ENOMEM, "shout_metadata_add");
		exit(1);
	}

	if (cfg_get_metadata_format_str()) {
		char	buf[BUFSIZ];

		mdata_strformat(md, buf, sizeof(buf),
		    cfg_get_metadata_format_str());
		if (SHOUTERR_SUCCESS !=
		    shout_metadata_add(shout_md, "song", buf)) {
			log_syserr(ALERT, ENOMEM, "shout_metadata_add");
			exit(1);
		}
		log_info("stream metadata: formatted: %s", buf);
	} else {
		if (mdata_get_artist(md) && mdata_get_title(md)) {
			if (SHOUTERR_SUCCESS != shout_metadata_add(shout_md,
				"artist", mdata_get_artist(md)) ||
			    SHOUTERR_SUCCESS != shout_metadata_add(shout_md,
				"title", mdata_get_title(md))) {
				log_syserr(ALERT, ENOMEM,
				    "shout_metadata_add");
				exit(1);
			}
			log_info("stream metadata: artist=\"%s\" title=\"%s\"",
			    mdata_get_artist(md),
			    mdata_get_title(md));
		} else if (mdata_get_songinfo(md)) {
			if (SHOUTERR_SUCCESS != shout_metadata_add(shout_md,
			        "song", mdata_get_songinfo(md))) {
				log_syserr(ALERT, ENOMEM,
				    "shout_metadata_add");
				exit(1);
			}
			log_info("stream metadata: songinfo: %s",
			    mdata_get_songinfo(md));
		} else {
			if (SHOUTERR_SUCCESS != shout_metadata_add(shout_md,
			        "song", mdata_get_name(md))) {
				log_syserr(ALERT, ENOMEM,
				    "shout_metadata_add");
				exit(1);
			}
			log_info("stream metadata: name: %s",
			    mdata_get_name(md));
		}
	}

	if ((ret = shout_set_metadata(s->shout, shout_md)) != SHOUTERR_SUCCESS)
		log_warning("shout_set_metadata: %s", shout_get_error(s->shout));

	shout_metadata_free(shout_md);

	if (ret == SHOUTERR_SUCCESS) {
		if (md_str != NULL && *md_str == NULL)
			*md_str = mdata_get_songinfo(md) ?
			    xstrdup(mdata_get_songinfo(md)) :
			    xstrdup(mdata_get_name(md));
	}

	return (ret == SHOUTERR_SUCCESS ? 0 : -1);
}

int
stream_get_connected(struct stream *s)
{
	return (shout_get_connected(s->shout) == SHOUTERR_CONNECTED ? 1 : 0);
}

cfg_stream_t
stream_get_cfg_stream(struct stream *s)
{
	return (cfg_stream_list_find(cfg_get_streams(), s->name));
}

cfg_intake_t
stream_get_cfg_intake(struct stream *s)
{
	cfg_stream_t	 cfg_stream;
	const char	*intake;

	cfg_stream = cfg_stream_list_get(cfg_get_streams(), s->name);
	intake = cfg_stream_get_intake(cfg_stream);
	if (!intake)
		intake = CFG_DEFAULT;
	return (cfg_intake_list_get(cfg_get_intakes(), intake));
}

cfg_server_t
stream_get_cfg_server(struct stream *s)
{
	cfg_stream_t	 cfg_stream;
	const char	*server;

	cfg_stream = cfg_stream_list_get(cfg_get_streams(), s->name);
	server = cfg_stream_get_server(cfg_stream);
	if (!server)
		server = CFG_DEFAULT;
	return (cfg_server_list_get(cfg_get_servers(), server));
}

int
stream_connect(struct stream *s)
{
	if (shout_open(s->shout) == SHOUTERR_SUCCESS)
		return (0);

	log_warning("%s: connect: %s: error %d: %s", s->name,
	    shout_get_host(s->shout), shout_get_errno(s->shout),
	    shout_get_error(s->shout));

	return (-1);
}

void
stream_disconnect(struct stream *s)
{
	if (!stream_get_connected(s))
		return;
	shout_close(s->shout);
}

void
stream_sync(struct stream *s)
{
	shout_sync(s->shout);
}

int
stream_send(struct stream *s, const char *data, size_t len)
{
	if (shout_send(s->shout, (const unsigned char *)data, len)
	    == SHOUTERR_SUCCESS)
		return (0);

	log_warning("%s: send: %s: error %d: %s", s->name,
	    shout_get_host(s->shout), shout_get_errno(s->shout),
	    shout_get_error(s->shout));

	stream_disconnect(s);

	return (-1);
}
