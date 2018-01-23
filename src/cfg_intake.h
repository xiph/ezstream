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

#ifndef __CFG_INTAKE_H__
#define __CFG_INTAKE_H__

enum cfg_intake_type {
	CFG_INTAKE_AUTODETECT = 0,
	CFG_INTAKE_FILE,
	CFG_INTAKE_PLAYLIST,
	CFG_INTAKE_PROGRAM,
	CFG_INTAKE_STDIN,
	CFG_INTAKE_MIN = CFG_INTAKE_AUTODETECT,
	CFG_INTAKE_MAX = CFG_INTAKE_STDIN,
};

typedef struct cfg_intake *		cfg_intake_t;
typedef struct cfg_intake_list *	cfg_intake_list_t;

cfg_intake_list_t
	cfg_intake_list_create(void);
void	cfg_intake_list_destroy(cfg_intake_list_t *);
unsigned int
	cfg_intake_list_nentries(cfg_intake_list_t);

cfg_intake_t
	cfg_intake_list_find(cfg_intake_list_t, const char *);
cfg_intake_t
	cfg_intake_list_get(cfg_intake_list_t, const char *);
void	cfg_intake_list_foreach(cfg_intake_list_t, void (*)(cfg_intake_t,
	    void *), void *);

cfg_intake_t
	cfg_intake_create(const char *);
void	cfg_intake_destroy(cfg_intake_t *);

int	cfg_intake_set_name(cfg_intake_t, cfg_intake_list_t, const char *,
	    const char **);
int	cfg_intake_set_type(cfg_intake_t, cfg_intake_list_t, const char *,
	    const char **);
int	cfg_intake_set_filename(cfg_intake_t, cfg_intake_list_t, const char *,
	    const char **);
int	cfg_intake_set_shuffle(cfg_intake_t, cfg_intake_list_t, const char *,
	    const char **);
int	cfg_intake_set_stream_once(cfg_intake_t, cfg_intake_list_t,
	    const char *, const char **);

int	cfg_intake_validate(cfg_intake_t, const char **);

const char *
	cfg_intake_get_name(cfg_intake_t);
enum cfg_intake_type
	cfg_intake_get_type(cfg_intake_t);
const char *
	cfg_intake_get_type_str(cfg_intake_t);
const char *
	cfg_intake_get_filename(cfg_intake_t);
int	cfg_intake_get_shuffle(cfg_intake_t);
int	cfg_intake_get_stream_once(cfg_intake_t);

#endif /* __CFG_INTAKE_H__ */
