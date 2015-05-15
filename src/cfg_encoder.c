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

#include <assert.h>
#include <string.h>

#include "cfg_private.h"
#include "cfg_encoder.h"
#include "xalloc.h"

struct cfg_encoder {
	TAILQ_ENTRY(cfg_encoder) entry;
	char			*name;
	enum cfg_stream_format	 format;
	char			*program;
};
TAILQ_HEAD(cfg_encoder_list, cfg_encoder);

static struct cfg_encoder_list	cfg_encoders;

int
cfg_encoder_init(void)
{
	TAILQ_INIT(&cfg_encoders);
	return (0);
}

void
cfg_encoder_exit(void)
{
	struct cfg_encoder	*e;

	while (NULL != (e = TAILQ_FIRST(&cfg_encoders))) {
		TAILQ_REMOVE(&cfg_encoders, e, entry);
		xfree(e->name);
		xfree(e->program);
		xfree(e);
	}
}

struct cfg_encoder *
cfg_encoder_get(const char *name)
{
	struct cfg_encoder	*e;

	if (!name || !name[0])
		return (NULL);

	TAILQ_FOREACH(e, &cfg_encoders, entry) {
		if (0 == strcasecmp(e->name, name))
			return (e);
	}

	e = xcalloc(1UL, sizeof(*e));
	e->name = xstrdup(name);

	TAILQ_INSERT_TAIL(&cfg_encoders, e, entry);

	return (e);
}

int
cfg_encoder_set_name(struct cfg_encoder *e, const char *name,
    const char **errstrp)
{
	if (!name || !name[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	xfree(e->name);
	e->name = xstrdup(name);

	return (0);
}

int
cfg_encoder_set_format(struct cfg_encoder *e, enum cfg_stream_format fmt,
    const char **not_used)
{
	(void)not_used;
	assert(CFG_STREAM_MIN <= fmt && CFG_STREAM_MAX >= fmt);
	e->format = fmt;
	return (0);
}

int
cfg_encoder_set_format_str(struct cfg_encoder *e, const char *fmt_str,
    const char **errstrp)
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

	cfg_encoder_set_format(e, fmt, errstrp);

	return (0);
}

int
cfg_encoder_set_program(struct cfg_encoder *e, const char *program,
    const char **errstrp)
{
	if (!program || !program[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	xfree(e->program);
	e->program = xstrdup(program);

	return (0);
}

int
cfg_encoder_validate(struct cfg_encoder *e, const char **errstrp)
{
	if (!e->program) {
		if (errstrp)
			*errstrp = "program not set";
		return (-1);
	}

	if (CFG_STREAM_INVALID == e->format) {
		if (errstrp)
			*errstrp = "format not set";
		return (-1);
	}

	CHECKPH_PROHIBITED(e->program, PLACEHOLDER_TRACK);
	CHECKPH_PROHIBITED(e->program, PLACEHOLDER_STRING);
	CHECKPH_DUPLICATE(e->program, PLACEHOLDER_METADATA);
	CHECKPH_DUPLICATE(e->program, PLACEHOLDER_ARTIST);
	CHECKPH_DUPLICATE(e->program, PLACEHOLDER_TITLE);

	return (0);
}

const char *
cfg_encoder_get_name(struct cfg_encoder *e)
{
	return (e->name);
}

enum cfg_stream_format
cfg_encoder_get_format(struct cfg_encoder *e)
{
	return (e->format);
}

const char *
cfg_encoder_get_program(struct cfg_encoder *e)
{
	return (e->program);
}
