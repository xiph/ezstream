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

struct cfg_encoder_list *
cfg_encoder_list_create(void)
{
	struct cfg_encoder_list *el;

	el = xcalloc(1UL, sizeof(*el));
	TAILQ_INIT(el);

	return (el);
}

void
cfg_encoder_list_destroy(struct cfg_encoder_list **el_p)
{
	struct cfg_encoder_list *el = *el_p;
	struct cfg_encoder	*e;

	if (!el)
		return;

	while (NULL != (e = TAILQ_FIRST(el)))
		cfg_encoder_list_remove(el, &e);

	xfree(el);
	*el_p = NULL;
}

unsigned int
cfg_encoder_list_nentries(struct cfg_encoder_list *el)
{
	struct cfg_encoder	*e;
	unsigned int		 n = 0;

	TAILQ_FOREACH(e, el, entry) {
		n++;
	}

	return (n);
}

struct cfg_encoder *
cfg_encoder_list_find(struct cfg_encoder_list *el, const char *name)
{
	struct cfg_encoder	*e;

	TAILQ_FOREACH(e, el, entry) {
		if (0 == strcasecmp(e->name, name))
			return (e);
	}

	return (NULL);
}

struct cfg_encoder *
cfg_encoder_list_get(struct cfg_encoder_list *el, const char *name)
{
	struct cfg_encoder	*e;

	e = cfg_encoder_list_find(el, name);
	if (e)
		return (e);
	e = cfg_encoder_create(name);
	if (!e)
		return (NULL);

	TAILQ_INSERT_TAIL(el, e, entry);

	return (e);
}

void
cfg_encoder_list_foreach(struct cfg_encoder_list *el,
    void (*cb)(cfg_encoder_t, void *), void *cb_arg)
{
	struct cfg_encoder	*e;

	TAILQ_FOREACH(e, el, entry) {
		cb(e, cb_arg);
	}
}

void
cfg_encoder_list_remove(struct cfg_encoder_list *el, struct cfg_encoder **e_p)
{
	TAILQ_REMOVE(el, *e_p, entry);
	cfg_encoder_destroy(e_p);
}

struct cfg_encoder *
cfg_encoder_create(const char *name)
{
	struct cfg_encoder	*e;

	if (!name || !name[0])
		return (NULL);

	e = xcalloc(1UL, sizeof(*e));
	e->name = xstrdup(name);

	return (e);
}

void
cfg_encoder_destroy(struct cfg_encoder **e_p)
{
	struct cfg_encoder	*e = *e_p;

	xfree(e->name);
	xfree(e->program);
	xfree(e);
	*e_p = NULL;
}

int
cfg_encoder_set_format(struct cfg_encoder *e, enum cfg_stream_format fmt)
{
	assert(CFG_STREAM_MIN <= fmt && CFG_STREAM_MAX >= fmt);
	e->format = fmt;
	return (0);
}

int
cfg_encoder_set_name(struct cfg_encoder *e, struct cfg_encoder_list *el,
    const char *name, const char **errstrp)
{
	struct cfg_encoder	*e2;

	if (!name || !name[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	e2 = cfg_encoder_list_find(el, name);
	if (e2 && e2 != e) {
		if (errstrp)
			*errstrp = "already exists";
		return (-1);
	}

	SET_XSTRDUP(e->name, name, errstrp);

	return (0);
}

int
cfg_encoder_set_format_str(struct cfg_encoder *e,
    struct cfg_encoder_list *not_used, const char *fmt_str,
    const char **errstrp)
{
	enum cfg_stream_format	fmt;

	(void)not_used;

	if (!fmt_str || !fmt_str[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (0 > cfg_stream_str2fmt(fmt_str, &fmt)) {
		if (errstrp)
			*errstrp = "unsupported stream format";
		return (-1);
	}

	cfg_encoder_set_format(e, fmt);

	return (0);
}

int
cfg_encoder_set_program(struct cfg_encoder *e,
    struct cfg_encoder_list *not_used, const char *program,
    const char **errstrp)
{
	(void)not_used;

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
	if (CFG_STREAM_INVALID == e->format) {
		if (errstrp)
			*errstrp = "format not set";
		return (-1);
	}

	if (!e->program) {
		if (errstrp)
			*errstrp = "program not set";
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
