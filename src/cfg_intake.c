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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "compat.h"

#include <sys/queue.h>

#include <string.h>

#include "cfg_private.h"
#include "cfg_intake.h"
#include "xalloc.h"

struct cfg_intake {
	TAILQ_ENTRY(cfg_intake)  entry;
	char			*name;
	enum cfg_intake_type	 type;
	char			 filename[PATH_MAX];
	int			 shuffle;
	int			 stream_once;
};

TAILQ_HEAD(cfg_intake_list, cfg_intake);

struct cfg_intake_list *
cfg_intake_list_create(void)
{
	struct cfg_intake_list	*il;

	il = xcalloc(1UL, sizeof(*il));
	TAILQ_INIT(il);

	return (il);
}

void
cfg_intake_list_destroy(cfg_intake_list_t *il_p)
{
	struct cfg_intake_list	*il = *il_p;
	struct cfg_intake	*i;

	if (!il)
		return;

	while (NULL != (i = TAILQ_FIRST(il))) {
		TAILQ_REMOVE(il, i, entry);
		cfg_intake_destroy(&i);
	}

	xfree(il);
	*il_p = NULL;
}

struct cfg_intake *
cfg_intake_list_find(struct cfg_intake_list *il, const char *name)
{
	struct cfg_intake	*i;

	TAILQ_FOREACH(i, il, entry) {
		if (0 == strcasecmp(i->name, name))
			return (i);
	}

	return (NULL);
}

struct cfg_intake *
cfg_intake_list_get(struct cfg_intake_list *il, const char *name)
{
	struct cfg_intake	*i;

	i = cfg_intake_list_find(il, name);
	if (i)
		return (i);
	i = cfg_intake_create(name);
	if (!i)
		return (NULL);

	TAILQ_INSERT_TAIL(il, i, entry);

	return (i);
}

struct cfg_intake *
cfg_intake_create(const char *name)
{
	struct cfg_intake	*i;

	if (!name || !name[0])
		return (NULL);

	i = xcalloc(1UL, sizeof(*i));
	i->name = xstrdup(name);

	return (i);
}

void
cfg_intake_destroy(struct cfg_intake **i_p)
{
	struct cfg_intake	*i = *i_p;

	xfree(i->name);
	xfree(i);
	*i_p = NULL;
}

int
cfg_intake_set_name(struct cfg_intake *i, struct cfg_intake_list *il,
    const char *name, const char **errstrp)
{
	struct cfg_intake	*i2;

	if (!name || !name[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	i2 = cfg_intake_list_find(il, name);
	if (i2 && i2 != i) {
		if (errstrp)
			*errstrp = "already exists";
		return (-1);
	}

	SET_XSTRDUP(i->name, name, errstrp);

	return (0);
}

int
cfg_intake_set_type(struct cfg_intake *i, struct cfg_intake_list *not_used,
    const char *type, const char **errstrp)
{
	(void)not_used;

	if (!type || !type[0]) {
		if (errstrp)
			*errstrp = "empty";
		return (-1);
	}

	if (0 == strcasecmp("autodetect", type))
		i->type = CFG_INTAKE_AUTODETECT;
	else if (0 == strcasecmp("file", type))
		i->type = CFG_INTAKE_FILE;
	else if (0 == strcasecmp("playlist", type))
		i->type = CFG_INTAKE_PLAYLIST;
	else if (0 == strcasecmp("program", type))
		i->type = CFG_INTAKE_PROGRAM;
	else if (0 == strcasecmp("stdin", type))
		i->type = CFG_INTAKE_STDIN;
	else {
		if (errstrp)
			*errstrp = "unsupported";
		return (-1);
	}
	return (0);
}

int
cfg_intake_set_filename(struct cfg_intake *i, struct cfg_intake_list *not_used,
    const char *filename, const char **errstrp)
{
	(void)not_used;
	SET_STRLCPY(i->filename, filename, errstrp);
	return (0);
}

int
cfg_intake_set_shuffle(struct cfg_intake *i, struct cfg_intake_list *not_used,
    const char *shuffle, const char **errstrp)
{
	(void)not_used;
	SET_BOOLEAN(i->shuffle, shuffle, errstrp);
	return (0);
}

int
cfg_intake_set_stream_once(struct cfg_intake *i, struct cfg_intake_list *not_used,
    const char *stream_once, const char **errstrp)
{
	(void)not_used;
	SET_BOOLEAN(i->stream_once, stream_once, errstrp);
	return (0);
}

int
cfg_intake_validate(struct cfg_intake *i, const char **errstrp)
{
	if (!cfg_intake_get_filename(i) &&
	    CFG_INTAKE_STDIN != cfg_intake_get_type(i)) {
		if (NULL != errstrp)
			*errstrp = "intake filename missing";
		return (-1);
	}

	return (0);
}

const char *
cfg_intake_get_name(struct cfg_intake *i)
{
	return (i->name);
}

enum cfg_intake_type
cfg_intake_get_type(struct cfg_intake *i)
{
	return (i->type);
}

const char *
cfg_intake_get_filename(struct cfg_intake *i)
{
	return (i->filename[0] ? i->filename : NULL);
}

int
cfg_intake_get_shuffle(struct cfg_intake *i)
{
	return (i->shuffle);
}

int
cfg_intake_get_stream_once(struct cfg_intake *i)
{
	return (i->stream_once);
}
