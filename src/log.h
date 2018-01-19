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

#ifndef __LOG_H__
#define __LOG_H__

#include "attributes.h"

enum log_levels {
	ALERT,
	ERROR,
	WARNING,
	NOTICE,
	INFO,
	DEBUG
};

int	log_init(const char *);
void	log_exit(void);

void	log_set_verbosity(unsigned int);

int	log_syserr(enum log_levels, int, const char *);

int	log_alert(const char *, ...)
    ATTRIBUTE_NONNULL(1)
    ATTRIBUTE_FORMAT(printf, 1, 2);
int	log_error(const char *, ...)
    ATTRIBUTE_NONNULL(1)
    ATTRIBUTE_FORMAT(printf, 1, 2);
int	log_warning(const char *, ...)
    ATTRIBUTE_NONNULL(1)
    ATTRIBUTE_FORMAT(printf, 1, 2);
int	log_notice(const char *, ...)
    ATTRIBUTE_NONNULL(1)
    ATTRIBUTE_FORMAT(printf, 1, 2);
int	log_info(const char *, ...)
    ATTRIBUTE_NONNULL(1)
    ATTRIBUTE_FORMAT(printf, 1, 2);
int	log_debug(const char *, ...)
    ATTRIBUTE_NONNULL(1)
    ATTRIBUTE_FORMAT(printf, 1, 2);

#endif /* __LOG_H__ */
