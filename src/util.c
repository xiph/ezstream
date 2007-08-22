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

/*
 * This file contains utility functions, as well as a few other unexciting
 * but verbose functions outsourced from ezstream.c to make it more readable.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <shout/shout.h>

#include "util.h"
#include "configfile.h"
#include "xalloc.h"

extern EZCONFIG *pezConfig;
extern char	*__progname;

int
strrcmp(const char *s, const char *sub)
{
	size_t	slen = strlen(s);
	size_t	sublen = strlen(sub);

	if (sublen > slen)
		return (1);

	return (memcmp(s + slen - sublen, sub, sublen));
}

int
strrcasecmp(const char *s, const char *sub)
{
	char	*s_cpy = xstrdup(s);
	char	*sub_cpy = xstrdup(sub);
	char	*p;
	int	 ret;

	for (p = s_cpy; *p != '\0'; p++)
		*p = tolower((int)*p);

	for (p = sub_cpy; *p != '\0'; p++)
		*p = tolower((int)*p);

	ret = strrcmp(s_cpy, sub_cpy);

	xfree(s_cpy);
	xfree(sub_cpy);

	return (ret);
}

shout_t *
stream_setup(const char *host, const int port, const char *mount)
{
	shout_t *shout = NULL;

	if ((shout = shout_new()) == NULL) {
		printf("%s: shout_new(): %s", __progname, strerror(ENOMEM));
		return (NULL);
	}

	if (shout_set_host(shout, host) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_host(): %s\n", __progname,
		       shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_protocol(): %s\n", __progname,
			shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_port(shout, port) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_port: %s\n", __progname,
			shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_password(shout, pezConfig->password) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_password(): %s\n", __progname,
			shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_mount(shout, mount) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_mount(): %s\n", __progname,
			shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (shout_set_user(shout, "source") != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_user(): %s\n", __progname,
			shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}

	if (!strcmp(pezConfig->format, MP3_FORMAT) &&
	    shout_set_format(shout, SHOUT_FORMAT_MP3) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_format(MP3): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if ((!strcmp(pezConfig->format, VORBIS_FORMAT) ||
	     !strcmp(pezConfig->format, THEORA_FORMAT)) &&
	    shout_set_format(shout, SHOUT_FORMAT_OGG) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_format(OGG): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}

	if (pezConfig->serverName &&
	    shout_set_name(shout, pezConfig->serverName) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_name(): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (pezConfig->serverURL &&
	    shout_set_url(shout, pezConfig->serverURL) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_url(): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (pezConfig->serverGenre &&
	    shout_set_genre(shout, pezConfig->serverGenre) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_genre(): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (pezConfig->serverDescription &&
	    shout_set_description(shout, pezConfig->serverDescription) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_description(): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (pezConfig->serverBitrate &&
	    shout_set_audio_info(shout, SHOUT_AI_BITRATE, pezConfig->serverBitrate) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_audio_info(AI_BITRATE): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (pezConfig->serverChannels &&
	    shout_set_audio_info(shout, SHOUT_AI_CHANNELS, pezConfig->serverChannels) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_audio_info(AI_CHANNELS): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (pezConfig->serverSamplerate &&
	    shout_set_audio_info(shout, SHOUT_AI_SAMPLERATE, pezConfig->serverSamplerate) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_audio_info(AI_SAMPLERATE): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}
	if (pezConfig->serverQuality &&
	    shout_set_audio_info(shout, SHOUT_AI_QUALITY, pezConfig->serverQuality) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_audio_info(AI_QUALITY): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}

	if (shout_set_public(shout, pezConfig->serverPublic) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_public(): %s\n",
		       __progname, shout_get_error(shout));
		shout_free(shout);
		return (NULL);
	}

	return (shout);
}
