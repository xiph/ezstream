/*
 * Copyright (c) 2015, 2017 Moritz Grimm <mgrimm@mrsserver.net>
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

#include <sys/stat.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "cfg_private.h"
#include "cfg_xmlfile.h"
#include "log.h"
#include "xalloc.h"

static struct cfg_program	cfg_program;
static struct cfg		cfg;
static struct cfg		cfg_tmp;

static void	_cfg_reset(struct cfg *);
static void	_cfg_switch(struct cfg *, struct cfg *);
static int	_cfg_load(void);
static void	_cfg_save(void);
static void	_cfg_restore(void);
static void	_cfg_commit(void);

static void
_cfg_reset(struct cfg *c)
{
	cfg_server_list_destroy(&c->servers);
	cfg_stream_list_destroy(&c->streams);
	cfg_decoder_list_destroy(&c->decoders);
	cfg_encoder_list_destroy(&c->encoders);

	xfree(c->metadata.format_str);

	memset(c, 0, sizeof(*c));

	c->metadata.refresh_interval = -1;
}

static void
_cfg_switch(struct cfg *a, struct cfg *b)
{
	struct cfg	tmp;

	memcpy(&tmp, a, sizeof(tmp));
	memcpy(a, b, sizeof(*a));
	memcpy(b, &tmp, sizeof(*b));
}

static int
_cfg_load(void)
{
	switch (cfg_program.config_type) {
	case CFG_TYPE_XMLFILE:
		if (0 > cfg_xmlfile_parse(cfg_program.config_file))
			return (-1);
		break;
	default:
		log_alert("unsupported config type %u",
		    cfg_program.config_type);
		abort();
	}
	return (0);
}

static void
_cfg_save(void)
{
	_cfg_reset(&cfg_tmp);
	_cfg_switch(&cfg_tmp, &cfg);
	_cfg_reset(&cfg);
}

static void
_cfg_restore(void)
{
	_cfg_reset(&cfg);
	_cfg_switch(&cfg, &cfg_tmp);
	_cfg_reset(&cfg_tmp);
}

static void
_cfg_commit(void)
{
	_cfg_reset(&cfg_tmp);
}

int
cfg_init(void)
{
	_cfg_reset(&cfg);
	_cfg_commit();
	return (0);
}

void
cfg_exit(void)
{
	(void)cfg_init();
}

int
cfg_file_reload(void)
{
	_cfg_save();
	if (0 > _cfg_load()) {
		_cfg_restore();
		return (-1);
	}
	_cfg_commit();

	return (0);
}

int
cfg_check(const char **errstrp)
{
	if (!cfg_get_media_filename() &&
	    CFG_MEDIA_STDIN != cfg_get_media_type()) {
		if (NULL != errstrp)
			*errstrp = "media filename missing";
		return (-1);
	}

	return (0);
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
cfg_file_check(const char *file)
{
	struct stat	st;

	if (0 > stat(file, &st)) {
		log_error("%s: %s", file, strerror(errno));
		return (-1);
	}

	if (st.st_mode & S_IROTH)
		log_warning("%s: world readable", file);
	else if (st.st_mode & S_IRGRP)
		log_notice("%s: group readable", file);

	if (st.st_mode & S_IWOTH) {
		log_error("%s: world writeable", file);
		return (-1);
	}

	return (0);
}

cfg_decoder_list_t
cfg_get_decoders(void)
{
	if (!cfg.decoders)
		cfg.decoders = cfg_decoder_list_create();

	return (cfg.decoders);
}

cfg_encoder_list_t
cfg_get_encoders(void)
{
	if (!cfg.encoders)
		cfg.encoders = cfg_encoder_list_create();

	return (cfg.encoders);
}

cfg_server_list_t
cfg_get_servers(void)
{
	if (!cfg.servers)
		cfg.servers = cfg_server_list_create();

	return (cfg.servers);
}

cfg_stream_list_t
cfg_get_streams(void)
{
	if (!cfg.streams)
		cfg.streams = cfg_stream_list_create();

	return (cfg.streams);
}

int
cfg_set_program_name(const char *progname, const char **errstrp)
{
	SET_STRLCPY(cfg_program.name, progname, errstrp);
	return (0);
}

int
cfg_set_program_config_type(enum cfg_config_type type, const char **errstrp)
{
	if (CFG_TYPE_MIN > type || CFG_TYPE_MAX < type) {
		if (NULL != errstrp)
			*errstrp = "invalid";
		return (-1);
	}
	cfg_program.config_type = type;
	return (0);
}

int
cfg_set_program_config_file(const char *file, const char **errstrp)
{
	SET_STRLCPY(cfg_program.config_file, file, errstrp);
	return (0);
}

int
cfg_set_program_pid_file(const char *file, const char **errstrp)
{
	SET_STRLCPY(cfg_program.pid_file, file, errstrp);
	return (0);
}

int
cfg_set_program_quiet_stderr(int quiet_stderr, const char **not_used)
{
	(void)not_used;
	cfg_program.quiet_stderr = quiet_stderr ? 1 : 0;
	return (0);
}

int
cfg_set_program_rtstatus_output(int rtstatus_output, const char **not_used)
{
	(void)not_used;
	cfg_program.rtstatus_output = rtstatus_output ? 1 : 0;
	return (0);
}

int
cfg_set_program_verbosity(unsigned int verbosity, const char **not_used)
{
	(void)not_used;
	cfg_program.verbosity = verbosity;
	return (0);
}

int
cfg_set_media_type(const char *type, const char **errstrp)
{
	if (!type || !type[0]) {
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
	SET_INTNUM(cfg.metadata.refresh_interval, num_str, errstrp);
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
	return (cfg_program.name);
}

enum cfg_config_type
cfg_get_program_config_type(void)
{
	return (cfg_program.config_type);
}

const char *
cfg_get_program_config_file(void)
{
	return (cfg_program.config_file[0] ? cfg_program.config_file : NULL);
}

const char *
cfg_get_program_pid_file(void)
{
	return (cfg_program.pid_file[0] ? cfg_program.pid_file : NULL);
}

int
cfg_get_program_quiet_stderr(void)
{
	return (cfg_program.quiet_stderr);
}

int
cfg_get_program_rtstatus_output(void)
{
	return (cfg_program.rtstatus_output);
}

unsigned int
cfg_get_program_verbosity(void)
{
	return (cfg_program.verbosity);
}

enum cfg_media_type
cfg_get_media_type(void)
{
	return (cfg.media.type);
}

const char *
cfg_get_media_filename(void)
{
	return (cfg.media.filename[0] ? cfg.media.filename : NULL);
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
	return (cfg.metadata.program[0] ? cfg.metadata.program : NULL);
}

const char *
cfg_get_metadata_format_str(void)
{
	return (cfg.metadata.format_str);
}

int
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
