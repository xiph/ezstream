/*
 *  ezstream - source client for Icecast with external en-/decoder support
 *  Copyright (C) 2003, 2004, 2005, 2006  Ed Zaleski <oddsock@oddsock.org>
 *  Copyright (C) 2007                    Moritz Grimm <mdgrimm@gmx.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#define ICONV_REPLACE		0
#define ICONV_TRANSLIT		1
#define ICONV_IGNORE		2

int		strrcmp(const char *, const char *);
int		strrcasecmp(const char *, const char *);
shout_t *	stream_setup(const char *, const int, const char *);
char *		CHARtoUTF8(const char *, int);
char *		UTF8toCHAR(const char *, int);

#endif /* __UTIL_H__ */
