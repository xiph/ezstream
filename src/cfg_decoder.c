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

#include <string.h>

#include "cfg_private.h"
#include "cfg_decoder.h"
#include "log.h"
#include "xalloc.h"

struct file_ext {
	TAILQ_ENTRY(file_ext)	 entry;
	char			*ext;
};
TAILQ_HEAD(file_ext_list, file_ext);

struct cfg_decoder {
	TAILQ_ENTRY(cfg_decoder) entry;
	char			*name;
	char			*program;
	struct file_ext_list	 exts;
};
TAILQ_HEAD(cfg_decoder_list, cfg_decoder);

static struct cfg_decoder_list	cfg_decoders;

int
cfg_decoder_init(void)
{
	TAILQ_INIT(&cfg_decoders);
	return (0);
}

void
cfg_decoder_exit(void)
{
	struct cfg_decoder	*d;

	while (NULL != (d = TAILQ_FIRST(&cfg_decoders))) {
		struct file_ext *e;

		TAILQ_REMOVE(&cfg_decoders, d, entry);
		xfree(d->name);
		xfree(d->program);
		while (NULL != (e = TAILQ_FIRST(&d->exts))) {
			TAILQ_REMOVE(&d->exts, e, entry);
			xfree(e->ext);
			xfree(e);
		}
		xfree(d);
	}
}

struct cfg_decoder *
cfg_decoder_get(const char *name)
{
	struct cfg_decoder	*d;

	if (!name || !name[0])
		return (NULL);

	TAILQ_FOREACH(d, &cfg_decoders, entry) {
		if (0 == strcasecmp(d->name, name))
			return (d);
	}

	d = xcalloc(1UL, sizeof(*d));
	d->name = xstrdup(name);
	TAILQ_INIT(&d->exts);

	return (d);
}

int
cfg_decoder_set_name(struct cfg_decoder *d, const char *name,
    const char **errstrp)
{
	if (!name || !name[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	xfree(d->name);
	d->name = xstrdup(name);

	return (0);
}

int
cfg_decoder_set_program(struct cfg_decoder *d, const char *program,
    const char **errstrp)
{
	if (!program || !program[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	xfree(d->program);
	d->program = xstrdup(program);

	return (0);
}

int
cfg_decoder_add_match(struct cfg_decoder *d, const char *ext,
    const char **errstrp)
{
	struct cfg_decoder	*d2;
	struct file_ext 	*e, *e2;

	if (!ext || !ext[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	d2 = cfg_decoder_find(ext);
	e = NULL;
	if (d2) {
		while (NULL != (e2 = TAILQ_FIRST(&d2->exts))) {
			if (0 == strcasecmp(e2->ext, ext)) {
				log_notice("%s: relocating match from %s to %s",
				    ext, d2->name, d->name);
				TAILQ_REMOVE(&d2->exts, e2, entry);
				e = e2;
				break;
			}
		}
	}
	if (!e) {
		e = xcalloc(1UL, sizeof(*e));
		e->ext = xstrdup(ext);
	}
	TAILQ_INSERT_TAIL(&d->exts, e, entry);

	return (0);
}

int
cfg_decoder_validate(struct cfg_decoder *d, const char **errstrp)
{
	struct file_ext *e;
	unsigned int	 num_exts;

	if (!d->program) {
		if (errstrp)
			*errstrp = "program not set";
		return (-1);
	}

	num_exts = 0;
	TAILQ_FOREACH(e, &d->exts, entry) {
		num_exts++;
	}
	if (!num_exts) {
		if (errstrp)
			*errstrp = "no file extensions registered";
		return (-1);
	}

	CHECKPH_PROHIBITED(d->program, PLACEHOLDER_STRING);
	CHECKPH_DUPLICATE(d->program, PLACEHOLDER_TRACK);
	CHECKPH_DUPLICATE(d->program, PLACEHOLDER_METADATA);
	CHECKPH_DUPLICATE(d->program, PLACEHOLDER_ARTIST);
	CHECKPH_DUPLICATE(d->program, PLACEHOLDER_TITLE);
	CHECKPH_REQUIRED(d->program, PLACEHOLDER_TRACK);

	return (0);
}

struct cfg_decoder *
cfg_decoder_find(const char *ext)
{
	struct cfg_decoder	*d;

	TAILQ_FOREACH(d, &cfg_decoders, entry) {
		struct file_ext *e;

		TAILQ_FOREACH(e, &d->exts, entry) {
			if (0 == strcasecmp(e->ext, ext))
				return (d);
		}
	}

	return (NULL);
}

const char *
cfg_decoder_get_name(struct cfg_decoder *d)
{
	return (d->name);
}

const char *
cfg_decoder_get_program(struct cfg_decoder *d)
{
	return (d->program);
}
