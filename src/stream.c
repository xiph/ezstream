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

#include <sys/queue.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <shout/shout.h>

#include "cfg.h"
#include "log.h"
#include "stream.h"
#include "util.h"
#include "xalloc.h"

struct stream {
	TAILQ_ENTRY(stream)	 entry;
	char			*name;
	shout_t 		*shout;
};
TAILQ_HEAD(stream_list, stream);

static struct stream_list	streams;

static int	_stream_cfg_server(struct stream *);
static int	_stream_cfg_tls(struct stream *);
static int	_stream_cfg_stream(struct stream *);

static int
_stream_cfg_server(struct stream *s)
{
	switch (cfg_get_server_protocol()) {
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
		    s->name, cfg_get_server_protocol_str());
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_host(s->shout, cfg_get_server_hostname())) {
		log_error("%s: hostname: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_port(s->shout, (unsigned short)cfg_get_server_port())) {
		log_error("%s: port: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_user(s->shout, cfg_get_server_user())) {
		log_error("%s: user: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_password(s->shout, cfg_get_server_password())) {
		log_error("%s: password: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}

	return (0);
}

static int
_stream_cfg_tls(struct stream *s)
{
#ifdef SHOUT_TLS_AUTO
	int	tls_req;

	switch (cfg_get_server_tls()) {
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
	if (cfg_get_server_ca_dir() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_ca_directory(s->shout, cfg_get_server_ca_dir())) {
		log_error("%s: ca_dir: %s", s->name,
		    shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_server_ca_file() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_ca_file(s->shout, cfg_get_server_ca_file())) {
		log_error("%s: ca_file: %s", s->name,
		    shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_server_tls_cipher_suite() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_allowed_ciphers(s->shout, cfg_get_server_tls_cipher_suite())) {
		log_error("%s: tls_cipher_suite: %s", s->name,
		    shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_server_client_cert() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_client_certificate(s->shout, cfg_get_server_client_cert())) {
		log_error("%s: client_cert: %s", s->name,
		    shout_get_error(s->shout));
		return (-1);
	}
#else /* SHOUT_TLS_AUTO */
# warning "libshout library does not support TLS"
	switch (cfg_get_server_tls()) {
	case CFG_TLS_MAY:
		log_warning("%s: TLS optional but not supported by libshout",
		    s->name);
		break;
	case CFG_TLS_REQUIRED:
		log_error("%s: TLS required by not supported by libshout",
		    s->name);
		return (-1);
	default:
		break;
	}
#endif /* SHOUT_TLS_AUTO */

	return (0);
}

static int
_stream_cfg_stream(struct stream *s)
{
	if (SHOUTERR_SUCCESS !=
	    shout_set_mount(s->shout, cfg_get_stream_mountpoint())) {
		log_error("%s: mountpoint: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	switch (cfg_get_stream_format()) {
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
		    s->name, cfg_get_stream_format_str());
		return (-1);
	}
	if (cfg_get_stream_name() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_name(s->shout, cfg_get_stream_name())) {
		log_error("%s: name: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_stream_url() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_url(s->shout, cfg_get_stream_url())) {
		log_error("%s: url: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_stream_genre() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_genre(s->shout, cfg_get_stream_genre())) {
		log_error("%s: genre: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_stream_description() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_description(s->shout, cfg_get_stream_description())) {
		log_error("%s: description: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_stream_quality() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(s->shout, SHOUT_AI_QUALITY, cfg_get_stream_quality())) {
		log_error("%s: ai_quality: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_stream_bitrate() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(s->shout, SHOUT_AI_BITRATE, cfg_get_stream_bitrate())) {
		log_error("%s: ai_bitrate: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_stream_samplerate() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(s->shout, SHOUT_AI_SAMPLERATE, cfg_get_stream_samplerate())) {
		log_error("%s: ai_samplerate: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (cfg_get_stream_channels() &&
	    SHOUTERR_SUCCESS !=
	    shout_set_audio_info(s->shout, SHOUT_AI_CHANNELS, cfg_get_stream_channels())) {
		log_error("%s: ai_channels: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}
	if (SHOUTERR_SUCCESS !=
	    shout_set_public(s->shout, (unsigned int)cfg_get_stream_server_public())) {
		log_error("%s: public: %s",
		    s->name, shout_get_error(s->shout));
		return (-1);
	}

	return (0);
}

int
stream_init(void)
{
	TAILQ_INIT(&streams);
	shout_init();
	return (0);
}

void
stream_exit(void)
{
	struct stream	*e;

	while (NULL != (e = TAILQ_FIRST(&streams))) {
		TAILQ_REMOVE(&streams, e, entry);
		xfree(e->name);
		shout_free(e->shout);
		xfree(e);
	}
	shout_shutdown();
}

struct stream *
stream_get(const char *name)
{
	struct stream	*e;

	if (!name || !name[0]) {
		log_alert("stream_get: empty name");
		exit(1);
	}

	TAILQ_FOREACH(e, &streams, entry) {
		if (0 == strcasecmp(e->name, name))
			return (e);
	}

	e = xcalloc(1UL, sizeof(*e));
	e->name = xstrdup(name);
	e->shout = shout_new();
	if (NULL == e->shout) {
		log_syserr(ALERT, ENOMEM, "shout_new");
		exit(1);
	}

	TAILQ_INSERT_TAIL(&streams, e, entry);

	return (e);
}

int
stream_setup(struct stream *s)
{
	if (0 != _stream_cfg_server(s) ||
	    0 != _stream_cfg_tls(s) ||
	    0 != _stream_cfg_stream(s)) {
		/* Reset handle on error */
		shout_free(s->shout);
		s->shout = shout_new();
		if (NULL == s->shout) {
			log_syserr(ALERT, ENOMEM, "shout_new");
			exit(1);
		}
		return (-1);
	}

	return (0);
}

int
stream_set_metadata(struct stream *s, metadata_t md, char **md_str)
{
	shout_metadata_t	*shout_md = NULL;
	char			*songInfo;
	const char		*artist, *title;
	int			 ret = SHOUTERR_SUCCESS;

	if (cfg_get_metadata_no_updates())
		return (0);

	if (md == NULL)
		return (-1);

	if ((shout_md = shout_metadata_new()) == NULL) {
		log_syserr(ALERT, ENOMEM, "shout_metadata_new");
		exit(1);
	}

	artist = metadata_get_artist(md);
	title = metadata_get_title(md);

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

	songInfo = stream_get_metadata_str(cfg_get_metadata_format_str(), md);
	if (songInfo == NULL) {
		if (artist[0] == '\0' && title[0] == '\0')
			songInfo = xstrdup(metadata_get_string(md));
		else
			songInfo = metadata_assemble_string(md);
		if (artist[0] != '\0' && title[0] != '\0') {
			if (shout_metadata_add(shout_md, "artist", artist) != SHOUTERR_SUCCESS) {
				log_syserr(ALERT, ENOMEM,
				    "shout_metadata_add");
				exit(1);
			}
			if (shout_metadata_add(shout_md, "title", title) != SHOUTERR_SUCCESS) {
				log_syserr(ALERT, ENOMEM,
				    "shout_metadata_add");
				exit(1);
			}
		} else {
			if (shout_metadata_add(shout_md, "song", songInfo) != SHOUTERR_SUCCESS) {
				log_syserr(ALERT, ENOMEM,
				    "shout_metadata_add");
				exit(1);
			}
		}
	} else if (shout_metadata_add(shout_md, "song", songInfo) != SHOUTERR_SUCCESS) {
		log_syserr(ALERT, ENOMEM, "shout_metadata_add");
		exit(1);
	}

	if ((ret = shout_set_metadata(s->shout, shout_md)) != SHOUTERR_SUCCESS)
		log_warning("shout_set_metadata: %s", shout_get_error(s->shout));

	shout_metadata_free(shout_md);

	if (ret == SHOUTERR_SUCCESS) {
		if (md_str != NULL && *md_str == NULL)
			*md_str = xstrdup(songInfo);
	}

	xfree(songInfo);
	return (ret == SHOUTERR_SUCCESS ? 0 : -1);
}

char *
stream_get_metadata_str(const char *format, metadata_t mdata)
{
	char	*tmp, *str;

	if (format == NULL)
		return (NULL);

	str = xstrdup(format);

	if (strstr(format, PLACEHOLDER_ARTIST) != NULL) {
		tmp = replaceString(str, PLACEHOLDER_ARTIST,
		    metadata_get_artist(mdata));
		xfree(str);
		str = tmp;
	}
	if (strstr(format, PLACEHOLDER_TITLE) != NULL) {
		tmp = replaceString(str, PLACEHOLDER_TITLE,
		    metadata_get_title(mdata));
		xfree(str);
		str = tmp;
	}
	if (strstr(format, PLACEHOLDER_STRING) != NULL) {
		tmp = replaceString(str, PLACEHOLDER_STRING,
		    metadata_get_string(mdata));
		xfree(str);
		str = tmp;
	}
	if (strstr(format, PLACEHOLDER_TRACK) != NULL) {
		tmp = replaceString(str, PLACEHOLDER_TRACK,
		    metadata_get_filename(mdata));
		xfree(str);
		str = tmp;
	}

	return (str);
}

int
stream_get_connected(struct stream *s)
{
	return (shout_get_connected(s->shout) == SHOUTERR_CONNECTED ? 1 : 0);
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
