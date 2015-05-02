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

#include "attributes.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "cfg.h"
#include "log.h"

static void	_log(enum log_levels, const char *, ...)
    ATTRIBUTE_NONNULL(2)
    ATTRIBUTE_FORMAT(printf, 2, 3);
static void	_vlog(enum log_levels, const char *, va_list)
    ATTRIBUTE_NONNULL(2);

static void
_log(enum log_levels lvl, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_vlog(lvl, fmt, ap);
	va_end(ap);
}

static void
_vlog(enum log_levels lvl, const char *fmt, va_list ap)
{
	va_list ap2;
	int	p;

	switch (lvl) {
	case ALERT:
		p = LOG_ALERT;
		break;
	case ERROR:
		p = LOG_ERR;
		break;
	case WARNING:
		p = LOG_WARNING;
		break;
	case NOTICE:
		if (cfg_get_program_verbosity() < 1)
			return;
		p = LOG_NOTICE;
		break;
	case INFO:
		if (cfg_get_program_verbosity() < 2)
			return;
		p = LOG_INFO;
		break;
	case DEBUG:
	default:
		if (cfg_get_program_verbosity() < 3)
			return;
		p = LOG_DEBUG;
		break;
	};

	va_copy(ap2, ap);
	vsyslog(p, fmt, ap2);
	va_end(ap2);
}

int
log_init(void)
{
	openlog(cfg_get_program_name(),
	    LOG_PID|LOG_CONS|LOG_NDELAY|LOG_PERROR,
	    LOG_USER);
	return (0);
}

void
log_exit(void)
{
	closelog();
}

void
log_syserr(enum log_levels lvl, int error, const char *pfx)
{
	char	errbuf[1024];

	if (0 != strerror_r(error, errbuf, sizeof(errbuf)))
		abort();
	_log(lvl, "%s%s%s",
	    pfx ? pfx : "",
	    pfx ? ": " : "",
	    errbuf);
}

void
log_alert(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_vlog(ALERT, fmt, ap);
	va_end(ap);
}

void
log_error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_vlog(ERROR, fmt, ap);
	va_end(ap);
}

void
log_warning(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_vlog(WARNING, fmt, ap);
	va_end(ap);
}

void
log_notice(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_vlog(NOTICE, fmt, ap);
	va_end(ap);
}

void
log_info(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_vlog(INFO, fmt, ap);
	va_end(ap);
}

void
log_debug(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	_vlog(DEBUG, fmt, ap);
	va_end(ap);
}
