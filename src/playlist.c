/*
 * Copyright (c) 2007 Moritz Grimm <gtgbr@gmx.net>
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

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "compat.h"
#include "playlist.h"
#include "util.h"

extern char	*__progname;

struct playlist {
	char	 *filename;
	char	**list;
	size_t	  size;
	size_t	  num;
	size_t	  index;
	int	  program;
	char	 *prog_track;
};

playlist_t *	playlist_create(const char *);
int		playlist_add(playlist_t *, const char *);
unsigned int	playlist_random(void);
const char *	playlist_run_program(playlist_t *);

playlist_t *
playlist_create(const char *filename)
{
	playlist_t	*pl;

	pl = xcalloc(1, sizeof(playlist_t));
	pl->filename = xstrdup(filename);

	return (pl);
}

int
playlist_add(playlist_t *pl, const char *entry)
{
	size_t	num;

	if (pl == NULL || entry == NULL) {
		printf("%s: playlist_add(): Internal error: Bad arguments\n",
		       __progname);
		exit(1);
	}

	num = pl->num + 1;

	if (pl->size == 0) {
		pl->list = xcalloc(2, sizeof(char *));
		pl->size = 2 * sizeof(char *);
	}

	if (pl->size / sizeof(char *) <= num) {
		unsigned long	i;

		pl->list = xrealloc(pl->list, 2, pl->size);
		pl->size = 2 * pl->size;

		for (i = num; i < pl->size / sizeof(char *); i++)
			pl->list[i] = NULL;
	}

	pl->list[num - 1] = xstrdup(entry);
	pl->num = num;

	return (1);
}

unsigned int
playlist_random(void)
{
	unsigned int	ret = 0;

#ifdef HAVE_ARC4RANDOM
	ret = arc4random();
#elif HAVE_RANDOM
	ret = (unsigned int)random();
#else
	ret = (unsigned int)rand();
#endif

	return (ret);
}

void
playlist_init(void)
{
#ifdef HAVE_RANDOM
# ifdef HAVE_SRANDOMDEV
	srandomdev();
# else
	srandom((unsigned int)time(NULL));
# endif /* HAVE_SRANDOMDEV */
#else
	srand((unsigned int)time(NULL));
#endif /* HAVE_RANDOM */
}

void playlist_shutdown(void) {}

playlist_t *
playlist_read(const char *filename)
{
	playlist_t	*pl;
	unsigned long	 line;
	FILE		*filep;
	char		 buf[PATH_MAX + 1];

	pl = playlist_create(filename);

	if ((filep = fopen(filename, "r")) == NULL) {
		printf("%s: %s: %s\n", __progname, filename, strerror(errno));
		playlist_free(pl);
		return (NULL);
	}

	line = 0;
	while (fgets(buf, sizeof(buf), filep) != NULL) {
		line++;

		if (strlen(buf) == sizeof(buf) - 1) {
			char skip_buf[2];

			printf("%s[%lu]: File or path name too long\n",
			       filename, line);
			/* Discard any excess chars in that line. */
			while (fgets(skip_buf, sizeof(skip_buf), filep) != NULL) {
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
		if (buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		if (buf[0] != '\0' && buf[strlen(buf) - 1] == '\r')
			buf[strlen(buf) - 1] = '\0';

		/* Skip lines that are empty after the trimming. */
		if (buf[0] == '\0')
			continue;

		/* We got one. */
		if (!playlist_add(pl, buf)) {
			fclose(filep);
			playlist_free(pl);
			return (NULL);
		}
	}
	if (ferror(filep)) {
		printf("%s: playlist_read(): Error while reading %s: %s\n",
		       __progname, filename, strerror(errno));
		fclose(filep);
		playlist_free(pl);
		return (NULL);
	}

	fclose(filep);
	return (pl);
}

playlist_t *
playlist_program(const char *filename)
{
	playlist_t	*pl;
#ifdef HAVE_STAT
	struct stat	 st;
#else
	FILE		*filep;
#endif

	pl = playlist_create(filename);
	pl->program = 1;

#ifdef HAVE_STAT
	if (stat(filename, &st) == -1) {
		printf("%s: %s: %s\n", __progname, filename, strerror(errno));
		playlist_free(pl);
		return (NULL);
	}
	if (!(st.st_mode & (S_IEXEC | S_IXGRP | S_IXOTH))) {
		printf("%s: %s: Not an executable program\n", __progname, filename);
		playlist_free(pl);
		return (NULL);
	}
#else
	if ((filep = fopen(filename, "r")) == NULL) {
		printf("%s: %s: %s\n", __progname, filename, strerror(errno));
		playlist_free(pl);
		return (NULL);
	}
	fclose(filep);
#endif /* HAVE_STAT */

	return (pl);
}

void
playlist_free(playlist_t *pl)
{
	size_t	i;

	if (pl != NULL) {
		if (pl->filename != NULL) {
			xfree(pl->filename);
			pl->filename = NULL;
		}

		if (pl->list != NULL) {
			if (pl->size > 0) {
				for (i = 0; i < pl->size / sizeof(char *); i++) {
					if (pl->list[i] != NULL) {
						xfree(pl->list[i]);
						pl->list[i] = NULL;
					} else
						break;
				}
			}

			xfree(pl->list);
			pl->list = NULL;
		}

		if (pl->prog_track != NULL)
			xfree(pl->prog_track);

		xfree(pl);
		pl = NULL;
	}
}

const char *
playlist_get_next(playlist_t *pl)
{
	if (pl == NULL) {
		printf("%s: playlist_get_next(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (pl->program)
		return (playlist_run_program(pl));

	return ((const char *)pl->list[pl->index++]);
}

const char *
playlist_peek_next(playlist_t *pl)
{
	if (pl == NULL) {
		printf("%s: playlist_peek_next(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (pl->program)
		return (NULL);

	return ((const char *)pl->list[pl->index]);
}

void
playlist_skip_next(playlist_t *pl)
{
	if (pl == NULL) {
		printf("%s: playlist_skip_next(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (pl->program)
		return;

	if (pl->list[pl->index] != NULL)
		pl->index++;
}

unsigned long
playlist_get_num_items(playlist_t *pl)
{
	if (pl == NULL) {
		printf("%s: playlist_get_position(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (pl->program)
		return (0);

	return ((unsigned long)pl->num);
}

unsigned long
playlist_get_position(playlist_t *pl)
{
	if (pl == NULL) {
		printf("%s: playlist_get_position(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (pl->program)
		return (0);

	return ((unsigned long)pl->index);
}

int
playlist_set_position(playlist_t *pl, unsigned long index)
{
	if (pl == NULL) {
		printf("%s: playlist_set_position(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (pl->program || index > pl->num - 1)
		return (0);

	pl->index = (size_t)index;

	return (1);
}

int
playlist_goto_entry(playlist_t *pl, const char *entry)
{
	unsigned long	i;

	if (pl == NULL || entry == NULL) {
		printf("%s: playlist_goto_entry(): Internal error: Bad arguments\n",
		       __progname);
		exit(1);
	}

	if (pl->program || pl->num == 0)
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
playlist_rewind(playlist_t *pl)
{
	if (pl == NULL) {
		printf("%s: playlist_rewind(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (pl->program)
		return;

	pl->index = 0;
}

int
playlist_reread(playlist_t **plist)
{
	playlist_t	*new_pl, *pl;

	if (plist == NULL || *plist == NULL) {
		printf("%s: playlist_reread(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	pl = *plist;

	if (pl->program)
		return (0);

	if ((new_pl = playlist_read(pl->filename)) == NULL)
		return (0);

	playlist_free(pl);
	*plist = new_pl;

	return (1);
}

/*
 * Yet another implementation of the "Knuth Shuffle":
 */
void
playlist_shuffle(playlist_t *pl)
{
	unsigned long	 d, i, range;
	char		*temp;

	if (pl == NULL) {
		printf("%s: playlist_shuffle(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (pl->program || pl->num < 2)
		return;

	for (i = 0; i < pl->num; i++) {
		range = pl->num - i;

		/*
		 * Only accept a random number if it is smaller than the
		 * largest multiple of our range. This reduces PRNG bias.
		 */
		do {
			d = (unsigned long)playlist_random();
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

const char *
playlist_run_program(playlist_t *pl)
{
	FILE	*filep;
	char	 buf[PATH_MAX + 1];

	if (pl == NULL) {
		printf("%s: playlist_run_program(): Internal error: NULL argument\n",
		       __progname);
		exit(1);
	}

	if (!pl->program)
		return (NULL);

	fflush(NULL);
	errno = 0;
	if ((filep = popen(pl->filename, "r")) == NULL) {
		printf("%s: playlist_run_program(): Error while executing '%s'",
		       __progname, pl->filename);
		/* popen() does not set errno reliably ... */
		if (errno)
			printf(": %s\n", strerror(errno));
		else
			printf("\n");
		return (NULL);
	}

	if (fgets(buf, sizeof(buf), filep) == NULL) {
		if (ferror(filep))
			printf("%s: Error while reading output from program '%s': %s\n",
			       __progname, pl->filename, strerror(errno));
		pclose(filep);
		printf("%s: FATAL: External program '%s' not (or no longer) usable.\n",
		       __progname, pl->filename);
		exit(1);
	}

	pclose(filep);

	if (strlen(buf) == sizeof(buf) - 1) {
		printf("%s: Output from program '%s' too long\n", __progname,
		       pl->filename);
		return (NULL);
	}

	if (buf[0] != '\0' && buf[strlen(buf) - 1] == '\n')
		buf[strlen(buf) - 1] = '\0';
	if (buf[0] != '\0' && buf[strlen(buf) - 1] == '\r')
		buf[strlen(buf) - 1] = '\0';
	if (buf[0] == '\0') {
		printf("%s: Empty line received from program '%s'\n",
		       __progname, pl->filename);
		return (NULL);
	}

	if (pl->prog_track != NULL)
		xfree(pl->prog_track);
	pl->prog_track = xstrdup(buf);

	return ((const char *)pl->prog_track);
}
