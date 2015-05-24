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

static int	_log(enum log_levels, const char *, ...)
    ATTRIBUTE_NONNULL(2)
    ATTRIBUTE_FORMAT(printf, 2, 3);
static int	_vlog(enum log_levels, const char *, va_list)
    ATTRIBUTE_NONNULL(2);

static int
_log(enum log_levels lvl, const char *fmt, ...)
{
	va_list ap;
	int	ret;

	va_start(ap, fmt);
	ret = _vlog(lvl, fmt, ap);
	va_end(ap);

	return (ret);
}

static int
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
			return (0);
		p = LOG_NOTICE;
		break;
	case INFO:
		if (cfg_get_program_verbosity() < 2)
			return (0);
		p = LOG_INFO;
		break;
	case DEBUG:
	default:
		if (cfg_get_program_verbosity() < 3)
			return (0);
		p = LOG_DEBUG;
		break;
	};

	va_copy(ap2, ap);
	vsyslog(p, fmt, ap2);
	va_end(ap2);

	return (1);
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

int
log_syserr(enum log_levels lvl, int error, const char *pfx)
{
	char	errbuf[1024];
	int	ret;

	if (0 != strerror_r(error, errbuf, sizeof(errbuf)))
		abort();
	ret = _log(lvl, "%s%s%s",
	    pfx ? pfx : "",
	    pfx ? ": " : "",
	    errbuf);

	return (ret);
}

int
log_alert(const char *fmt, ...)
{
	va_list ap;
	int	ret;

	va_start(ap, fmt);
	ret = _vlog(ALERT, fmt, ap);
	va_end(ap);

	return (ret);
}

int
log_error(const char *fmt, ...)
{
	va_list ap;
	int	ret;

	va_start(ap, fmt);
	ret = _vlog(ERROR, fmt, ap);
	va_end(ap);

	return (ret);
}

int
log_warning(const char *fmt, ...)
{
	va_list ap;
	int	ret;

	va_start(ap, fmt);
	ret = _vlog(WARNING, fmt, ap);
	va_end(ap);

	return (ret);
}

int
log_notice(const char *fmt, ...)
{
	va_list ap;
	int	ret;

	va_start(ap, fmt);
	ret = _vlog(NOTICE, fmt, ap);
	va_end(ap);

	return (ret);
}

int
log_info(const char *fmt, ...)
{
	va_list ap;
	int	ret;

	va_start(ap, fmt);
	ret = _vlog(INFO, fmt, ap);
	va_end(ap);

	return (ret);
}

int
log_debug(const char *fmt, ...)
{
	va_list ap;
	int	ret;

	va_start(ap, fmt);
	ret = _vlog(DEBUG, fmt, ap);
	va_end(ap);

	return (ret);
}
