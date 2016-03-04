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

#ifndef __CFG_H__
#define __CFG_H__

#define CFG_SFMT_VORBIS 	"VORBIS"
#define CFG_SFMT_MP3		"MP3"
#define CFG_SFMT_THEORA 	"THEORA"

#define PLACEHOLDER_TRACK	"@T@"
#define PLACEHOLDER_METADATA	"@M@"
#define PLACEHOLDER_ARTIST	"@a@"
#define PLACEHOLDER_TITLE	"@t@"
#define PLACEHOLDER_STRING	"@s@"

enum cfg_config_type {
	CFG_TYPE_XMLFILE = 0,
	CFG_TYPE_MIN = CFG_TYPE_XMLFILE,
	CFG_TYPE_MAX = CFG_TYPE_XMLFILE,
};

enum cfg_server_protocol {
	CFG_PROTO_HTTP = 0,
	CFG_PROTO_HTTPS,
	CFG_PROTO_MIN = CFG_PROTO_HTTP,
	CFG_PROTO_MAX = CFG_PROTO_HTTPS,
};

enum cfg_server_tls {
	CFG_TLS_MAY = 0,
	CFG_TLS_NONE,
	CFG_TLS_REQUIRED,
	CFG_TLS_MIN = CFG_TLS_MAY,
	CFG_TLS_MAX = CFG_TLS_REQUIRED,
};

enum cfg_media_type {
	CFG_MEDIA_AUTODETECT = 0,
	CFG_MEDIA_FILE,
	CFG_MEDIA_PLAYLIST,
	CFG_MEDIA_PROGRAM,
	CFG_MEDIA_STDIN,
	CFG_MEDIA_MIN = CFG_MEDIA_AUTODETECT,
	CFG_MEDIA_MAX = CFG_MEDIA_STDIN,
};

enum cfg_stream_format {
	CFG_STREAM_INVALID = 0,
	CFG_STREAM_VORBIS,
	CFG_STREAM_MP3,
	CFG_STREAM_THEORA,
	CFG_STREAM_MIN = CFG_STREAM_VORBIS,
	CFG_STREAM_MAX = CFG_STREAM_THEORA,
};

#include "cfg_decoder.h"
#include "cfg_encoder.h"

int	cfg_init(void);
void	cfg_exit(void);

void	cfg_save(void);
void	cfg_pop(void);
void	cfg_clear(void);

int	cfg_check(const char **);

int	cfg_reload(void);

int	cfg_stream_str2fmt(const char *, enum cfg_stream_format *);
const char *
	cfg_stream_fmt2str(enum cfg_stream_format);

int	cfg_file_check(const char *);

int	cfg_set_program_name(const char *, const char **);
int	cfg_set_program_config_type(enum cfg_config_type, const char **);
int	cfg_set_program_config_file(const char *, const char **);
int	cfg_set_program_quiet_stderr(int, const char **);
int	cfg_set_program_rtstatus_output(int, const char **);
int	cfg_set_program_verbosity(unsigned int, const char **);

int	cfg_set_server_protocol(const char *, const char **);
int	cfg_set_server_hostname(const char *, const char **);
int	cfg_set_server_port(const char *, const char **);
int	cfg_set_server_user(const char *, const char **);
int	cfg_set_server_password(const char *, const char **);
int	cfg_set_server_tls(const char *, const char **);
int	cfg_set_server_tls_cipher_suite(const char *, const char **);
int	cfg_set_server_ca_dir(const char *, const char **);
int	cfg_set_server_ca_file(const char *, const char **);
int	cfg_set_server_client_cert(const char *, const char **);
int	cfg_set_server_reconnect_attempts(const char *, const char **);

int	cfg_set_stream_mountpoint(const char *, const char **);
int	cfg_set_stream_name(const char *, const char **);
int	cfg_set_stream_url(const char *, const char **);
int	cfg_set_stream_genre(const char *, const char **);
int	cfg_set_stream_description(const char *, const char **);
int	cfg_set_stream_quality(const char *, const char **);
int	cfg_set_stream_bitrate(const char *, const char **);
int	cfg_set_stream_samplerate(const char *, const char **);
int	cfg_set_stream_channels(const char *, const char **);
int	cfg_set_stream_server_public(const char *, const char **);
int	cfg_set_stream_format(const char *, const char **);
int	cfg_set_stream_encoder(const char *, const char **);

int	cfg_set_media_type(const char *, const char **);
int	cfg_set_media_filename(const char *, const char **);
int	cfg_set_media_shuffle(const char *, const char **);
int	cfg_set_media_stream_once(const char *, const char **);

int	cfg_set_metadata_program(const char *, const char **);
int	cfg_set_metadata_format_str(const char *, const char **);
int	cfg_set_metadata_refresh_interval(const char *, const char **);
int	cfg_set_metadata_normalize_strings(const char *, const char **);
int	cfg_set_metadata_no_updates(const char *, const char **);

const char *
	cfg_get_program_name(void);
enum cfg_config_type
	cfg_get_program_config_type(void);
const char *
	cfg_get_program_config_file(void);
int	cfg_get_program_quiet_stderr(void);
int	cfg_get_program_rtstatus_output(void);
unsigned int
	cfg_get_program_verbosity(void);

enum cfg_server_protocol
	cfg_get_server_protocol(void);
const char *
	cfg_get_server_protocol_str(void);
const char *
	cfg_get_server_hostname(void);
unsigned int
	cfg_get_server_port(void);
const char *
	cfg_get_server_user(void);
const char *
	cfg_get_server_password(void);
enum cfg_server_tls
	cfg_get_server_tls(void);
const char *
	cfg_get_server_tls_cipher_suite(void);
const char *
	cfg_get_server_ca_dir(void);
const char *
	cfg_get_server_ca_file(void);
const char *
	cfg_get_server_client_cert(void);
unsigned int
	cfg_get_server_reconnect_attempts(void);

const char *
	cfg_get_stream_mountpoint(void);
const char *
	cfg_get_stream_name(void);
const char *
	cfg_get_stream_url(void);
const char *
	cfg_get_stream_genre(void);
const char *
	cfg_get_stream_description(void);
const char *
	cfg_get_stream_quality(void);
const char *
	cfg_get_stream_bitrate(void);
const char *
	cfg_get_stream_samplerate(void);
const char *
	cfg_get_stream_channels(void);
int	cfg_get_stream_server_public(void);
enum cfg_stream_format
	cfg_get_stream_format(void);
const char *
	cfg_get_stream_format_str(void);
const char *
	cfg_get_stream_encoder(void);

enum cfg_media_type
	cfg_get_media_type(void);
const char *
	cfg_get_media_filename(void);
int	cfg_get_media_shuffle(void);
int	cfg_get_media_stream_once(void);

const char *
	cfg_get_metadata_program(void);
const char *
	cfg_get_metadata_format_str(void);
int	cfg_get_metadata_refresh_interval(void);
int	cfg_get_metadata_normalize_strings(void);
int	cfg_get_metadata_no_updates(void);

#endif /* __CFG_H__ */
