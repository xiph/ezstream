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

#ifndef __CFG_H__
#define __CFG_H__

#define PLACEHOLDER_METADATA	"@M@"
#define PLACEHOLDER_ARTIST	"@a@"
#define PLACEHOLDER_ALBUM	"@b@"
#define PLACEHOLDER_TITLE	"@t@"
#define PLACEHOLDER_TRACK	"@T@"
#define PLACEHOLDER_STRING	"@s@"
#define PLACEHOLDER_START_TIMESTAMP "@u@"

#define CFG_DEFAULT		"default"

enum cfg_config_type {
	CFG_TYPE_XMLFILE = 0,
	CFG_TYPE_MIN = CFG_TYPE_XMLFILE,
	CFG_TYPE_MAX = CFG_TYPE_XMLFILE,
};

#include "cfg_decoder.h"
#include "cfg_encoder.h"
#include "cfg_intake.h"
#include "cfg_server.h"
#include "cfg_stream.h"

int	cfg_init(void);
void	cfg_exit(void);

int	cfg_file_reload(void);

int	cfg_check(const char **);

int	cfg_file_check(const char *);

cfg_decoder_list_t
	cfg_get_decoders(void);
cfg_encoder_list_t
	cfg_get_encoders(void);
cfg_intake_list_t
	cfg_get_intakes(void);
cfg_server_list_t
	cfg_get_servers(void);
cfg_stream_list_t
	cfg_get_streams(void);

int	cfg_set_program_name(const char *, const char **);
int	cfg_set_program_config_type(enum cfg_config_type, const char **);
int	cfg_set_program_config_file(const char *, const char **);
int	cfg_set_program_pid_file(const char *, const char **);
int	cfg_set_program_quiet_stderr(int, const char **);
int	cfg_set_program_rtstatus_output(int, const char **);
int	cfg_set_program_verbosity(unsigned int, const char **);

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
const char *
	cfg_get_program_pid_file(void);
int	cfg_get_program_quiet_stderr(void);
int	cfg_get_program_rtstatus_output(void);
unsigned int
	cfg_get_program_verbosity(void);

const char *
	cfg_get_metadata_program(void);
const char *
	cfg_get_metadata_format_str(void);
int	cfg_get_metadata_refresh_interval(void);
int	cfg_get_metadata_normalize_strings(void);
int	cfg_get_metadata_no_updates(void);

#endif /* __CFG_H__ */
