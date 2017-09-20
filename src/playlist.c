/*
 * Copyright (c) 2007, 2009 Moritz Grimm <mgrimm@mrsserver.net>
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
#endif

#include <sys/stat.h>
#ifdef HAVE_SYS_RANDOM_H
# include <sys/random.h>
#endif

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "log.h"
#include "playlist.h"
#include "xalloc.h"

/* Usually defined in <sys/stat.h>. */
#ifndef S_IEXEC
# define S_IEXEC	S_IXUSR
#endif /* !S_IEXEC */

struct playlist {
	char	 *filename;
	char	**list;
	size_t	  size;
	size_t	  num;
	size_t	  index;
	int	  program;
	char	 *prog_track;
};

static struct playlist * _playlist_create(const char *);
static int		_playlist_add(struct playlist *, const char *);
static unsigned int	_playlist_random(void);
static const char *	_playlist_run_program(struct playlist *);

static struct playlist *
_playlist_create(const char *filename)
{
	struct playlist *pl;

	pl = xcalloc(1UL, sizeof(*pl));
	pl->filename = xstrdup(filename);

	return (pl);
}

static int
_playlist_add(struct playlist *pl, const char *entry)
{
	size_t	num;

	num = pl->num + 1;

	if (pl->size == 0) {
		pl->list = xcalloc(2UL, sizeof(char *));
		pl->size = 2 * sizeof(char *);
	}

	if (pl->size / sizeof(char *) <= num) {
		size_t	i;

		pl->list = xreallocarray(pl->list, 2UL, pl->size);
		pl->size = 2 * pl->size;

		for (i = num; i < pl->size / sizeof(char *); i++)
			pl->list[i] = NULL;
	}

	pl->list[num - 1] = xstrdup(entry);
	pl->num = num;

	return (1);
}

static unsigned int
_playlist_random(void)
{
	unsigned int	ret = 0;

#ifdef HAVE_ARC4RANDOM
	ret = arc4random();
#elif HAVE_GETRANDOM
	if (sizeof(ret) != getrandom(&ret, sizeof(ret), 0)) {
		log_alert("getrandom: %s", strerror(errno));
		exit(1);
	}
#else
# warning "using deterministic randomness for shuffling playlists"
	ret = (unsigned int)random();
#endif

	return (ret);
}

static const char *
_playlist_run_program(struct playlist *pl)
{
	FILE	*filep;
	char	 buf[PATH_MAX + 1];

	fflush(NULL);
	errno = 0;
	log_debug("running command: %s", pl->filename);
	if ((filep = popen(pl->filename, "r")) == NULL) {
		/* popen() does not set errno reliably ... */
		if (errno)
			log_error("execution error: %s: %s", pl->filename,
			    strerror(errno));
		else
			log_error("execution error: %s", pl->filename);
		return (NULL);
	}

	if (NULL == fgets(buf, (int)sizeof(buf), filep)) {
		if (ferror(filep))
			log_error("%s: output read error: %s", pl->filename,
			    strerror(ferror(filep)));
		else
			log_error("%s: output read error: EOF", pl->filename);
		pclose(filep);
		return (NULL);
	}
	pclose(filep);

	if (strlen(buf) == sizeof(buf) - 1) {
		log_error("%s: output too long", pl->filename);
		return (NULL);
	}

	buf[strcspn(buf, "\n")] = '\0';
	buf[strcspn(buf, "\r")] = '\0';
	if (buf[0] == '\0')
		/* Empty line (end of playlist.) */
		return (NULL);

	xfree(pl->prog_track);
	pl->prog_track = xstrdup(buf);

	return ((const char *)pl->prog_track);
}

int
playlist_init(void)
{
	srandom((unsigned int)time(NULL));

	return (0);
}

void playlist_exit(void) {}

struct playlist *
playlist_read(const char *filename)
{
	struct playlist *pl;
	unsigned long	 line;
	FILE		*filep;
	char		 buf[PATH_MAX];

	if (filename != NULL) {
		pl = _playlist_create(filename);

		if ((filep = fopen(filename, "r")) == NULL) {
			log_error("%s: %s", filename, strerror(errno));
			playlist_free(&pl);
			return (NULL);
		}
	} else {
		pl = _playlist_create("stdin");

		if ((filep = fdopen(STDIN_FILENO, "r")) == NULL) {
			log_error("stdin: %s", strerror(errno));
			playlist_free(&pl);
			return (NULL);
		}
	}

	line = 0;
	while (fgets(buf, (int)sizeof(buf), filep) != NULL) {
		line++;

		if (strlen(buf) == sizeof(buf) - 1) {
			char skip_buf[2];

			log_error("%s[%lu]: file or path name too long",
			    pl->filename, line);
			/* Discard any excess chars in that line. */
			while (fgets(skip_buf, (int)sizeof(skip_buf), filep) != NULL) {
				if (skip_buf[0] == '\n')
					break;
			}
			continue;
		}

		/*
		 * fgets() would happily fill a buffer with
		 * { '\0', ..., '\n', '\0' }. Also skip lines that begin with
		 * a '#', which is considered a comment.
		 */
		if (buf[0] == '\0' || buf[0] == '#')
			continue;

		/* Trim any trailing newlines and carriage returns. */
		buf[strcspn(buf, "\n")] = '\0';
		buf[strcspn(buf, "\r")] = '\0';

		/* Skip lines that are empty after the trimming. */
		if (buf[0] == '\0')
			continue;

		/* We got one. */
		if (!_playlist_add(pl, buf)) {
			fclose(filep);
			playlist_free(&pl);
			return (NULL);
		}
	}
	if (ferror(filep)) {
		log_error("playlist_read: %s: %s", pl->filename,
		    strerror(errno));
		fclose(filep);
		playlist_free(&pl);
		return (NULL);
	}

	fclose(filep);
	return (pl);
}

struct playlist *
playlist_program(const char *filename)
{
	struct playlist *pl;
	struct stat	 st;

	if (stat(filename, &st) == -1) {
		log_error("%s: %s", filename, strerror(errno));
		return (NULL);
	}
	if (st.st_mode & S_IWOTH) {
		log_error("%s: world writeable", filename);
		return (NULL);
	}
	if (!(st.st_mode & (S_IEXEC | S_IXGRP | S_IXOTH))) {
		log_error("%s: not an executable program", filename);
		return (NULL);
	}

	pl = _playlist_create(filename);
	pl->program = 1;

	return (pl);
}

void
playlist_free(struct playlist **pl_p)
{
	struct playlist *pl;

	if (pl_p == NULL || (pl = *pl_p) == NULL)
		return;

	if (pl->filename != NULL) {
		xfree(pl->filename);
		pl->filename = NULL;
	}

	if (pl->list != NULL) {
		if (pl->size > 0) {
			size_t	i;

			for (i = 0; i < pl->size / sizeof(char *); i++) {
				if (pl->list[i] != NULL) {
					xfree(pl->list[i]);
					pl->list[i] = NULL;
				} else
					break;
			}
		}

		xfree(pl->list);
	}

	if (pl->prog_track != NULL) {
		xfree(pl->prog_track);
		pl->prog_track = NULL;
	}

	xfree(*pl_p);
	*pl_p = NULL;
}

const char *
playlist_get_next(struct playlist *pl)
{
	if (pl->program)
		return (_playlist_run_program(pl));

	if (pl->num == 0)
		return (NULL);

	return ((const char *)pl->list[pl->index++]);
}

void
playlist_skip_next(struct playlist *pl)
{
	if (pl->program || pl->num == 0)
		return;

	if (pl->list[pl->index] != NULL)
		pl->index++;
}

unsigned long
playlist_get_num_items(struct playlist *pl)
{
	if (pl->program)
		return (0);

	return ((unsigned long)pl->num);
}

unsigned long
playlist_get_position(struct playlist *pl)
{
	if (pl->program)
		return (0);

	return ((unsigned long)pl->index);
}

int
playlist_goto_entry(struct playlist *pl, const char *entry)
{
	unsigned long	i;

	if (pl->program)
		return (0);

	for (i = 0; i < pl->num; i++) {
		if (strcmp(pl->list[i], entry) == 0) {
			pl->index = (size_t)i;
			return (1);
		}
	}

	return (0);
}

void
playlist_rewind(struct playlist *pl)
{
	if (pl->program)
		return;

	pl->index = 0;
}

int
playlist_reread(struct playlist **plist)
{
	struct playlist *new_pl, *pl;

	pl = *plist;

	if (pl->program)
		return (0);

	if ((new_pl = playlist_read(pl->filename)) == NULL)
		return (0);

	playlist_free(&pl);
	*plist = new_pl;

	return (1);
}

/*
 * Yet another implementation of the "Knuth Shuffle":
 */
void
playlist_shuffle(struct playlist *pl)
{
	size_t	 d, i;
	char	*temp;

	if (pl->program || pl->num < 2)
		return;

	for (i = 0; i < pl->num; i++) {
		size_t	range;

		range = pl->num - i;

		/*
		 * Only accept a random number if it is smaller than the
		 * largest multiple of our range. This reduces PRNG bias.
		 */
		do {
			d = (unsigned long)_playlist_random();
		} while (d > RAND_MAX - (RAND_MAX % range));

		/*
		 * The range starts at the item we want to shuffle, excluding
		 * already shuffled items.
		 */
		d = i + (d % range);

		temp = pl->list[d];
		pl->list[d] = pl->list[i];
		pl->list[i] = temp;
	}
}
