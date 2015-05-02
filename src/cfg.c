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

#include "compat.h"

#include <stdint.h>
#include <stdlib.h>

#include "cfg_private.h"
#include "cfg_xmlfile.h"
#include "log.h"
#include "xalloc.h"

static struct cfg	cfg;
static struct cfg	cfg_tmp;

static void	_cfg_reset(struct cfg *);
static void	_cfg_copy(struct cfg *, struct cfg *);
static int	_cfg_load(void);

static void
_cfg_reset(struct cfg *c)
{
	xfree(c->stream.mountpoint);
	xfree(c->stream.name);
	xfree(c->stream.url);
	xfree(c->stream.genre);
	xfree(c->stream.description);
	xfree(c->stream.quality);
	xfree(c->stream.bitrate);
	xfree(c->stream.samplerate);
	xfree(c->stream.channels);
	xfree(c->stream.encoder);

	xfree(c->metadata.format_str);

	memset(c, 0, sizeof(*c));
}

static void
_cfg_copy(struct cfg *dst, struct cfg *src)
{
	memcpy(&dst->program, &src->program, sizeof(dst->program));

	memcpy(&dst->server, &src->server, sizeof(dst->server));

	if (src->stream.mountpoint)
		dst->stream.mountpoint = xstrdup(src->stream.mountpoint);
	if (src->stream.name)
		dst->stream.name = xstrdup(src->stream.name);
	if (src->stream.url)
		dst->stream.url = xstrdup(src->stream.url);
	if (src->stream.genre)
		dst->stream.genre = xstrdup(src->stream.genre);
	if (src->stream.description)
		dst->stream.description = xstrdup(src->stream.description);
	if (src->stream.quality)
		dst->stream.quality = xstrdup(src->stream.quality);
	if (src->stream.bitrate)
		dst->stream.bitrate = xstrdup(src->stream.bitrate);
	if (src->stream.samplerate)
		dst->stream.samplerate = xstrdup(src->stream.samplerate);
	if (src->stream.channels)
		dst->stream.channels = xstrdup(src->stream.channels);
	dst->stream.server_public = src->stream.server_public;
	dst->stream.format = src->stream.format;
	if (src->stream.encoder)
		dst->stream.encoder = xstrdup(src->stream.encoder);

	memcpy(&dst->media, &src->media, sizeof(dst->media));

	strlcpy(dst->metadata.program, src->metadata.program,
	    sizeof(dst->metadata.program));
	if (src->metadata.format_str)
		dst->metadata.format_str = xstrdup(src->metadata.format_str);
	dst->metadata.refresh_interval = src->metadata.refresh_interval;
	dst->metadata.normalize_strings = src->metadata.normalize_strings;
	dst->metadata.no_updates = src->metadata.no_updates;
}

static int
_cfg_load(void)
{
	switch (cfg.program.config_type) {
	case CFG_TYPE_XMLFILE:
		if (0 > cfg_xmlfile_parse(cfg.program.config_file))
			return (-1);
		break;
	default:
		log_alert("unsupported config type %u",
		    cfg.program.config_type);
		abort();
	}
	return (0);
}

int
cfg_reload(void)
{
	_cfg_copy(&cfg_tmp, &cfg);
	if (0 > _cfg_load()) {
		/* roll back */
		_cfg_reset(&cfg);
		_cfg_copy(&cfg, &cfg_tmp);
		return (-1);
	}
	_cfg_reset(&cfg_tmp);

	return (0);
}

void
cfg_exit(void)
{
	_cfg_reset(&cfg);
}

int
cfg_stream_str2fmt(const char *str, enum cfg_stream_format *fmt_p)
{
	if (0 == strcasecmp(str, CFG_SFMT_VORBIS)) {
		*fmt_p = CFG_STREAM_VORBIS;
	} else if (0 == strcasecmp(str, CFG_SFMT_MP3)) {
		*fmt_p = CFG_STREAM_MP3;
	} else if (0 == strcasecmp(str, CFG_SFMT_THEORA)) {
		*fmt_p = CFG_STREAM_THEORA;
	} else
		return (-1);
	return (0);
}

const char *
cfg_stream_fmt2str(enum cfg_stream_format fmt)
{
	switch (fmt) {
	case CFG_STREAM_VORBIS:
		return (CFG_SFMT_VORBIS);
	case CFG_STREAM_MP3:
		return (CFG_SFMT_MP3);
	case CFG_STREAM_THEORA:
		return (CFG_SFMT_THEORA);
	default:
		return (NULL);
	}
}

int
cfg_set_program_name(const char *progname, const char **errstrp)
{
	SET_STRLCPY(cfg.program.name, progname, errstrp);
	return (0);
}

int
cfg_set_program_config_type(enum cfg_config_type type, const char **errstrp)
{
	if (type >= CFG_TYPE_MAX) {
		if (NULL != errstrp)
			*errstrp = "invalid";
		return (-1);
	}
	cfg.program.config_type = type;
	return (0);
}

int
cfg_set_program_config_file(const char *file, const char **errstrp)
{
	SET_STRLCPY(cfg.program.config_file, file, errstrp);
	return (0);
}

int
cfg_set_program_quiet_stderr(int quiet_stderr, const char **not_used)
{
	(void)not_used;
	cfg.program.quiet_stderr = quiet_stderr ? 1 : 0;
	return (0);
}

int
cfg_set_program_verbosity(unsigned int verbosity, const char **not_used)
{
	(void)not_used;
	cfg.program.verbosity = verbosity;
	return (0);
}

int
cfg_set_server_protocol(const char *protocol, const char **errstrp)
{
	if (!protocol) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (0 == strcasecmp("http", protocol))
		cfg.server.protocol = CFG_PROTO_HTTP;
	else if (0 == strcasecmp("https", protocol))
		cfg.server.protocol = CFG_PROTO_HTTPS;
	else {
		if (NULL != errstrp)
			*errstrp = "unsupported";
		return (-1);
	}
	return (0);
}

int
cfg_set_server_hostname(const char *hostname, const char **errstrp)
{
	SET_STRLCPY(cfg.server.hostname, hostname, errstrp);
	return (0);
}

int
cfg_set_server_port(const char *port_str, const char **errstrp)
{
	const char	*errstr;
	unsigned int	 port;

	if (!port_str) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	port = strtonum(port_str, 1, UINT16_MAX, &errstr);
	if (errstr) {
		if (errstrp)
			*errstrp = errstr;
		return (-1);
	}
	cfg.server.port = port;

	return (0);
}

int
cfg_set_server_user(const char *user, const char **errstrp)
{
	SET_STRLCPY(cfg.server.user, user, errstrp);
	return (0);
}

int
cfg_set_server_password(const char *password, const char **errstrp)
{
	SET_STRLCPY(cfg.server.password, password, errstrp);
	return (0);
}

int
cfg_set_server_ca_dir(const char *ca_dir, const char **errstrp)
{
	SET_STRLCPY(cfg.server.ca_dir, ca_dir, errstrp);
	return (0);
}

int
cfg_set_server_ca_file(const char *ca_file, const char **errstrp)
{
	SET_STRLCPY(cfg.server.ca_file, ca_file, errstrp);
	return (0);
}

int
cfg_set_server_client_cert(const char *client_cert, const char **errstrp)
{
	SET_STRLCPY(cfg.server.client_cert, client_cert, errstrp);
	return (0);
}

int
cfg_set_server_client_key(const char *client_key, const char **errstrp)
{
	SET_STRLCPY(cfg.server.client_key, client_key, errstrp);
	return (0);
}

int
cfg_set_server_reconnect_attempts(const char *num_str, const char **errstrp)
{
	const char	*errstr;
	unsigned int	 num;

	if (!num_str) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	num = strtonum(num_str, 0, UINT_MAX, &errstr);
	if (errstr) {
		if (errstrp)
			*errstrp = errstr;
		return (-1);
	}
	cfg.server.reconnect_attempts = num;

	return (0);
}

int
cfg_set_stream_mountpoint(const char *mountpoint, const char **errstrp)
{
	if (!mountpoint || !mountpoint[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.mountpoint)
		xfree(cfg.stream.mountpoint);
	cfg.stream.mountpoint = xstrdup(mountpoint);

	return (0);
}

int
cfg_set_stream_name(const char *name, const char **errstrp)
{
	if (!name || !name[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.name)
		xfree(cfg.stream.name);
	cfg.stream.name = xstrdup(name);

	return (0);
}

int
cfg_set_stream_url(const char *url, const char **errstrp)
{
	if (!url || !url[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.url)
		xfree(cfg.stream.url);
	cfg.stream.url = xstrdup(url);

	return (0);
}

int
cfg_set_stream_genre(const char *genre, const char **errstrp)
{
	if (!genre || !genre[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.genre)
		xfree(cfg.stream.genre);
	cfg.stream.genre = xstrdup(genre);

	return (0);
}

int
cfg_set_stream_description(const char *description, const char **errstrp)
{
	if (!description || !description[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.description)
		xfree(cfg.stream.description);
	cfg.stream.description = xstrdup(description);

	return (0);
}

int
cfg_set_stream_quality(const char *quality, const char **errstrp)
{
	if (!quality || !quality[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.quality)
		xfree(cfg.stream.quality);
	cfg.stream.quality = xstrdup(quality);

	return (0);
}

int
cfg_set_stream_bitrate(const char *bitrate, const char **errstrp)
{
	if (!bitrate || !bitrate[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.bitrate)
		xfree(cfg.stream.bitrate);
	cfg.stream.bitrate = xstrdup(bitrate);

	return (0);
}

int
cfg_set_stream_samplerate(const char *samplerate, const char **errstrp)
{
	if (!samplerate || !samplerate[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.samplerate)
		xfree(cfg.stream.samplerate);
	cfg.stream.samplerate = xstrdup(samplerate);

	return (0);
}

int
cfg_set_stream_channels(const char *channels, const char **errstrp)
{
	if (!channels || !channels[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.channels)
		xfree(cfg.stream.channels);
	cfg.stream.channels = xstrdup(channels);

	return (0);
}

int
cfg_set_stream_server_public(const char *server_public, const char **errstrp)
{
	SET_BOOLEAN(cfg.stream.server_public, server_public, errstrp);
	return (0);
}

int
cfg_set_stream_format(const char *fmt_str, const char **errstrp)
{
	enum cfg_stream_format	fmt;

	if (!fmt_str) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (0 > cfg_stream_str2fmt(fmt_str, &fmt)) {
		if (errstrp)
			*errstrp = "unsupported stream format";
		return (-1);
	}

	cfg.stream.format = fmt;

	return (0);
}

int
cfg_set_stream_encoder(const char *encoder, const char **errstrp)
{
	if (!encoder || !encoder[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (cfg.stream.encoder)
		xfree(cfg.stream.encoder);
	cfg.stream.encoder = xstrdup(encoder);

	return (0);
}

int
cfg_set_media_type(const char *type, const char **errstrp)
{
	if (!type) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (0 == strcasecmp("autodetect", type))
		cfg.media.type = CFG_MEDIA_AUTODETECT;
	else if (0 == strcasecmp("file", type))
		cfg.media.type = CFG_MEDIA_FILE;
	else if (0 == strcasecmp("playlist", type))
		cfg.media.type = CFG_MEDIA_PLAYLIST;
	else if (0 == strcasecmp("program", type))
		cfg.media.type = CFG_MEDIA_PROGRAM;
	else if (0 == strcasecmp("stdin", type))
		cfg.media.type = CFG_MEDIA_STDIN;
	else {
		if (errstrp)
			*errstrp = "unsupported";
		return (-1);
	}
	return (0);
}

int
cfg_set_media_filename(const char *filename, const char **errstrp)
{
	SET_STRLCPY(cfg.media.filename, filename, errstrp);
	return (0);
}

int
cfg_set_media_shuffle(const char *shuffle, const char **errstrp)
{
	SET_BOOLEAN(cfg.media.shuffle, shuffle, errstrp);
	return (0);
}

int
cfg_set_media_stream_once(const char *stream_once, const char **errstrp)
{
	SET_BOOLEAN(cfg.media.stream_once, stream_once, errstrp);
	return (0);
}

int
cfg_set_metadata_program(const char *program, const char **errstrp)
{
	SET_STRLCPY(cfg.metadata.program, program, errstrp);
	return (0);
}

int
cfg_set_metadata_format_str(const char *format_str, const char **errstrp)
{
	if (!format_str || !format_str[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	CHECKPH_PROHIBITED(format_str, PLACEHOLDER_METADATA);
	CHECKPH_DUPLICATE(format_str, PLACEHOLDER_TRACK);
	CHECKPH_DUPLICATE(format_str, PLACEHOLDER_STRING);
	CHECKPH_DUPLICATE(format_str, PLACEHOLDER_ARTIST);
	CHECKPH_DUPLICATE(format_str, PLACEHOLDER_TITLE);

	if (cfg.metadata.format_str)
		xfree(cfg.metadata.format_str);
	cfg.metadata.format_str = xstrdup(format_str);

	return (0);
}

int
cfg_set_metadata_refresh_interval(const char *num_str, const char **errstrp)
{
	const char	*errstr;
	unsigned int	 num;

	if (!num_str) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	num = strtonum(num_str, 0, UINT_MAX, &errstr);
	if (errstr) {
		if (errstrp)
			*errstrp = errstr;
		return (-1);
	}
	cfg.metadata.refresh_interval = num;

	return (0);
}

int
cfg_set_metadata_normalize_strings(const char *normalize_strings,
    const char **errstrp)
{
	SET_BOOLEAN(cfg.metadata.normalize_strings, normalize_strings,
	    errstrp);
	return (0);
}

int
cfg_set_metadata_no_updates(const char *no_updates, const char **errstrp)
{
	SET_BOOLEAN(cfg.metadata.no_updates, no_updates,
	    errstrp);
	return (0);
}

const char *
cfg_get_program_name(void)
{
	return (cfg.program.name);
}

enum cfg_config_type
cfg_get_program_config_type(void)
{
	return (cfg.program.config_type);
}

const char *
cfg_get_program_config_file(void)
{
	return (cfg.program.config_file);
}

int
cfg_get_program_quiet_stderr(void)
{
	return (cfg.program.quiet_stderr);
}

unsigned int
cfg_get_program_verbosity(void)
{
	return (cfg.program.verbosity);
}

enum cfg_server_protocol
cfg_get_server_protocol(void)
{
	return (cfg.server.protocol);
}

const char *
cfg_get_server_protocol_str(void)
{
	switch (cfg.server.protocol) {
	case CFG_PROTO_HTTP:
		return ("http");
	case CFG_PROTO_HTTPS:
		return ("https");
	default:
		log_alert("unsupported protocol %u", cfg.server.protocol);
		abort();
	}
}

const char *
cfg_get_server_hostname(void)
{
	return (cfg.server.hostname);
}

unsigned int
cfg_get_server_port(void)
{
	return (cfg.server.port);
}

const char *
cfg_get_server_user(void)
{
	return (cfg.server.user[0] ? cfg.server.user : DEFAULT_USER);
}

const char *
cfg_get_server_password(void)
{
	return (cfg.server.password);
}

const char *
cfg_get_server_ca_dir(void)
{
	return (cfg.server.ca_dir);
}

const char *
cfg_get_server_ca_file(void)
{
	return (cfg.server.ca_file);
}

const char *
cfg_get_server_client_cert(void)
{
	return (cfg.server.client_cert);
}

const char *
cfg_get_server_client_key(void)
{
	return (cfg.server.client_key);
}

unsigned int
cfg_get_server_reconnect_attempts(void)
{
	return (cfg.server.reconnect_attempts);
}

const char *
cfg_get_stream_mountpoint(void)
{
	return (cfg.stream.mountpoint);
}

const char *
cfg_get_stream_name(void)
{
	return (cfg.stream.name);
}

const char *
cfg_get_stream_url(void)
{
	return (cfg.stream.url);
}

const char *
cfg_get_stream_genre(void)
{
	return (cfg.stream.genre);
}

const char *
cfg_get_stream_description(void)
{
	return (cfg.stream.description);
}

const char *
cfg_get_stream_quality(void)
{
	return (cfg.stream.quality);
}

const char *
cfg_get_stream_bitrate(void)
{
	return (cfg.stream.bitrate);
}

const char *
cfg_get_stream_samplerate(void)
{
	return (cfg.stream.samplerate);
}

const char *
cfg_get_stream_channels(void)
{
	return (cfg.stream.channels);
}

int
cfg_get_stream_server_public(void)
{
	return (cfg.stream.server_public);
}

enum cfg_stream_format
cfg_get_stream_format(void)
{
	return (cfg.stream.format);
}

const char *
cfg_get_stream_encoder(void)
{
	return (cfg.stream.encoder);
}

enum cfg_media_type
cfg_get_media_type(void)
{
	return (cfg.media.type);
}

const char *
cfg_get_media_filename(void)
{
	return (cfg.media.filename);
}

int
cfg_get_media_shuffle(void)
{
	return (cfg.media.shuffle);
}

int
cfg_get_media_stream_once(void)
{
	return (cfg.media.stream_once);
}

const char *
cfg_get_metadata_program(void)
{
	return (cfg.metadata.program);
}

const char *
cfg_get_metadata_format_str(void)
{
	return (cfg.metadata.format_str);
}

unsigned int
cfg_get_metadata_refresh_interval(void)
{
	return (cfg.metadata.refresh_interval);
}

int
cfg_get_metadata_normalize_strings(void)
{
	return (cfg.metadata.normalize_strings);
}

int
cfg_get_metadata_no_updates(void)
{
	return (cfg.metadata.no_updates);
}
