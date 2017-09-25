/*
 *  ezstream - source client for Icecast with external en-/decoder support
 *  Copyright (C) 2003, 2004, 2005, 2006  Ed Zaleski <oddsock@oddsock.org>
 *  Copyright (C) 2007, 2017              Moritz Grimm <mgrimm@mrsserver.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef __UTIL_H__
#define __UTIL_H__

struct util_dict {
	const char	*from;
	const char	*to;
};

int	util_write_pid_file(const char *);
int	util_strrcmp(const char *, const char *);
int	util_strrcasecmp(const char *, const char *);
char *	util_char2utf8(const char *);
char *	util_utf82char(const char *);
char *	util_expand_words(const char *, struct util_dict[]);
char *	util_shellquote(const char *, size_t);
int	util_urlparse(const char *, char **, unsigned short *, char **);

#endif /* __UTIL_H__ */
