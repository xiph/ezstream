/*
 * Copyright (c) 2018 Moritz Grimm <mgrimm@mrsserver.net>
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

#include "compat.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cfg.h"
#include "cfgfile_xml.h"
#include "ezconfig0.h"
#include "log.h"
#include "util.h"
#include "xalloc.h"

static char		*v0_cfgfile;
static unsigned int	 cfg_verbosity;

#define OPTSTRING	"0:hv"
enum opt_vals {
	OPT_V0CONFIGFILE	= '0',
	OPT_HELP		= 'h',
	OPT_VERBOSE		= 'v',
	OPT_INVALID		= '?'
};

static void	_usage(void);
static void	_usage_help(void);
static int	_cmdline_parse(int, char *[], int *);
static int	_url_parse(const char *, char **, char **, char **);
static int	_parse_ezconfig0(EZCONFIG *);

static void
_usage(void)
{
	fprintf(stderr, "usage: %s [-hv] -0 v0-cfgfile\n",
	    util_get_progname(NULL));
}

static void
_usage_help(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "    -0 v0-cfgfile  migrate from v0-cfgfile (ezstream version 0.x)\n");
	fprintf(stderr, "    -h             print this help and exit\n");
	fprintf(stderr, "    -v             increase logging verbosity\n");
}

static int
_cmdline_parse(int argc, char *argv[], int *ret_p)
{
	v0_cfgfile = NULL;
	cfg_verbosity = 0;
	optind = 1;
	for (;;) {
		int	 ch;

		ch = getopt(argc, argv, OPTSTRING);
		if (0 > ch)
			break;

		switch (ch) {
		case OPT_V0CONFIGFILE:
			v0_cfgfile = optarg;
			break;
		case OPT_HELP:
			_usage();
			_usage_help();
			*ret_p = 0;
			return (-1);
		case OPT_VERBOSE:
			cfg_verbosity++;
			break;
		case OPT_INVALID:
		default:
			_usage();
			*ret_p = 2;
			return (-1);
		}
	}
	argc -= optind;
	argv += optind;

	if (!v0_cfgfile) {
		fprintf(stderr, "-%c must be provided\n",
		    OPT_V0CONFIGFILE);
		_usage();
		*ret_p = 2;
		return (-1);
	}

	return (0);
}

static int
_url_parse(const char *url, char **hostname, char **port,
    char **mountname)
{
	const char	*p1, *p2, *p3;
	size_t		 hostsiz, portsiz, mountsiz;

	if (strncasecmp(url, "http://", strlen("http://")) != 0) {
		log_error("%s: Invalid <url>: Not an HTTP address",
		    v0_cfgfile);
		return (-1);
	}

	p1 = url + strlen("http://");
	p2 = strchr(p1, ':');
	if (p2 == NULL) {
		log_error("%s: Invalid <url>: Missing port", v0_cfgfile);
		return (-1);
	}
	hostsiz = (size_t)(p2 - p1) + 1;
	*hostname = xmalloc(hostsiz);
	strlcpy(*hostname, p1, hostsiz);

	p2++;
	p3 = strchr(p2, '/');
	if (p3 == NULL) {
		log_error("%s: Invalid <url>: Missing mountpoint or too long port number",
		    v0_cfgfile);
		xfree(*hostname);
		return (-1);
	}

	portsiz = (size_t)(p3 - p2) + 1;
	*port = xmalloc(portsiz);
	strlcpy(*port, p2, portsiz);

	mountsiz = strlen(p3) + 1;
	*mountname = xmalloc(mountsiz);
	strlcpy(*mountname, p3, mountsiz);

	return (0);
}

#define ENTITY_SET(e, el, func, ctx, val)	do {			\
	const char	*err_str2;					\
									\
	if (0 > (func)((e), (el), (val), &err_str2)) {			\
		log_warning("%s: %s: %s: %s", v0_cfgfile, (ctx),	\
		    err_str2, (val));					\
		warnings++;						\
	}								\
} while (0)

static int
_parse_ezconfig0(EZCONFIG *ez)
{
	char			*hostname, *port, *mountname;
	const char		*err_str;
	int			 warnings = 0;
	cfg_server_list_t	 srv_list = cfg_get_servers();
	cfg_server_t		 srv = cfg_server_list_get(srv_list, CFG_DEFAULT);
	cfg_stream_list_t	 str_list = cfg_get_streams();
	cfg_stream_t		 str = cfg_stream_list_get(str_list, CFG_DEFAULT);
	cfg_intake_list_t	 in_list = cfg_get_intakes();
	cfg_intake_t		 in = cfg_intake_list_get(in_list, CFG_DEFAULT);
	cfg_encoder_list_t	 enc_list = cfg_get_encoders();
	cfg_decoder_list_t	 dec_list = cfg_get_decoders();;
	unsigned int		 i;
	char			 strbuf[BUFSIZ];

	if (NULL == ez->URL || 0 == strlen(ez->URL)) {
		log_error("%s: Missing <url> -- not an ezstream version 0.x configuration?",
		    v0_cfgfile);
		return (-1);
	}
	if (0 > _url_parse(ez->URL, &hostname, &port, &mountname))
		return (-1);
	ENTITY_SET(srv, srv_list, cfg_server_set_protocol, "<url>", "HTTP");
	ENTITY_SET(srv, srv_list, cfg_server_set_hostname, "<url>", hostname);
	xfree(hostname);
	ENTITY_SET(srv, srv_list, cfg_server_set_port, "<url>", port);
	xfree(port);
	ENTITY_SET(str, str_list, cfg_stream_set_mountpoint, "<url>",
	    mountname);
	xfree(mountname);

	if (ez->username)
		ENTITY_SET(srv, srv_list, cfg_server_set_user,
		    "<sourceuser>", ez->username);
	if (ez->password)
		ENTITY_SET(srv, srv_list, cfg_server_set_password,
		    "<sourcepassword>", ez->password);
	if (ez->format)
		ENTITY_SET(str, str_list, cfg_stream_set_format,
		    "<format>", ez->format);
	if (ez->fileName) {
		if (0 == strcasecmp(ez->fileName, "stdin"))
			ENTITY_SET(in, in_list, cfg_intake_set_type,
			    "<filename>", "stdin");
		else
			ENTITY_SET(in, in_list, cfg_intake_set_filename,
			    "<filename>", ez->fileName);
	}
	if (ez->metadataProgram) {
		if (0 > cfg_set_metadata_program(ez->metadataProgram,
		    &err_str)) {
			log_warning("%s: %s: %s: %s", v0_cfgfile,
			    "<metadata_progname>", err_str,
			    ez->metadataProgram);
			warnings++;
		}
	}
	if (ez->metadataFormat) {
		if (0 > cfg_set_metadata_format_str(ez->metadataFormat,
		    &err_str)) {
			log_warning("%s: %s: %s: %s", v0_cfgfile,
			    "<metadata_format>", err_str, ez->metadataFormat);
			warnings++;
		}
	}
	if (ez->metadataRefreshInterval) {
		snprintf(strbuf, sizeof(strbuf), "%d",
		    ez->metadataRefreshInterval);
		if (0 > cfg_set_metadata_refresh_interval(strbuf, &err_str)) {
			log_warning("%s: %s: %s: %s", v0_cfgfile,
			    "<metadata_refreshinterval>", err_str, strbuf);
			warnings++;
		}
	}
	if (ez->fileNameIsProgram)
		ENTITY_SET(in, in_list, cfg_intake_set_type,
		    "playlist_program", "program");
	snprintf(strbuf, sizeof(strbuf), "%d", ez->shuffle);
	ENTITY_SET(in, in_list, cfg_intake_set_shuffle,
	    "<shuffle>", strbuf);
	snprintf(strbuf, sizeof(strbuf), "%d", ez->streamOnce);
	ENTITY_SET(in, in_list, cfg_intake_set_stream_once,
	    "<stream_once>", strbuf);
	snprintf(strbuf, sizeof(strbuf), "%u", ez->reconnectAttempts);
	ENTITY_SET(srv, srv_list, cfg_server_set_reconnect_attempts,
	    "<reconnect_tries>", strbuf);
	if (ez->serverName)
		ENTITY_SET(str, str_list, cfg_stream_set_stream_name,
		    "<svrinfoname>", ez->serverName);
	if (ez->serverURL)
		ENTITY_SET(str, str_list, cfg_stream_set_stream_url,
		    "<svrinfourl>", ez->serverURL);
	if (ez->serverGenre)
		ENTITY_SET(str, str_list, cfg_stream_set_stream_genre,
		    "<svrinfogenre>", ez->serverGenre);
	if (ez->serverDescription)
		ENTITY_SET(str, str_list, cfg_stream_set_stream_description,
		    "<svrinfodescription>", ez->serverDescription);
	if (ez->serverBitrate)
		ENTITY_SET(str, str_list, cfg_stream_set_stream_bitrate,
		    "<svrinfobitrate>", ez->serverBitrate);
	if (ez->serverChannels)
		ENTITY_SET(str, str_list, cfg_stream_set_stream_channels,
		    "<svrinfochannels>", ez->serverChannels);
	if (ez->serverSamplerate)
		ENTITY_SET(str, str_list, cfg_stream_set_stream_samplerate,
		    "<svrinfosamplerate>", ez->serverSamplerate);
	if (ez->serverQuality)
		ENTITY_SET(str, str_list, cfg_stream_set_stream_quality,
		    "<svrinfoquality>", ez->serverQuality);
	ENTITY_SET(str, str_list, cfg_stream_set_public,
	    "<svrinfopublic>", ez->serverPublic ? "yes" : "no");
	if (ez->reencode)
		ENTITY_SET(str, str_list, cfg_stream_set_encoder, "<reencode>",
		    ez->format);

	for (i = 0; i < MAX_FORMAT_ENCDEC; i++) {
		FORMAT_ENCDEC	*ed = ez->encoderDecoders[i];

		if (!ed)
			continue;

		if (ed->encoder) {
			cfg_encoder_t	 enc = cfg_encoder_list_get(enc_list,
			    CFG_DEFAULT);

			ENTITY_SET(enc, enc_list, cfg_encoder_set_program,
			    "<encode>", ed->encoder);
			if (ed->format) {
				ENTITY_SET(enc, enc_list, cfg_encoder_set_name,
				    "<format> (encoder)", ed->format);
				ENTITY_SET(enc, enc_list, cfg_encoder_set_format_str,
				    "<format> (encoder)", ed->format);
			}
			if (0 > cfg_encoder_validate(enc, &err_str)) {
				log_warning("%s: %s: %s", v0_cfgfile,
				    "<encdec> (encoder)", err_str);
				cfg_encoder_list_remove(enc_list, &enc);
				warnings++;
			}
		} else {
			if (ed->format &&
			    0 == strcasecmp(ed->format, "THEORA")) {
				cfg_encoder_t	enc = NULL;

				enc = cfg_encoder_list_find(enc_list,
				    ed->format);
				if (NULL == enc)
					enc = cfg_encoder_list_get(enc_list,
					    CFG_DEFAULT);
				ENTITY_SET(enc, enc_list, cfg_encoder_set_name,
				    "<format> (encoder)", ed->format);
				ENTITY_SET(enc, enc_list, cfg_encoder_set_format_str,
				    "<format> (encoder)", ed->format);
				if (0 > cfg_encoder_validate(enc, &err_str)) {
					log_warning("%s: %s: %s", v0_cfgfile,
					    "<encdec> (encoder)", err_str);
					cfg_encoder_list_remove(enc_list, &enc);
					warnings++;
				}
			}
		}
		if (ed->decoder) {
			cfg_decoder_t	dec = NULL;

			if (ed->format)
				dec = cfg_decoder_list_find(dec_list, ed->format);
			if (NULL == dec)
				dec = cfg_decoder_list_get(dec_list, CFG_DEFAULT);

			ENTITY_SET(dec, dec_list, cfg_decoder_set_program,
			    "<decode>", ed->decoder);
			if (ed->format)
				ENTITY_SET(dec, dec_list, cfg_decoder_set_name,
				    "<format> (decoder)", ed->format);
			if (ed->match)
				ENTITY_SET(dec, dec_list, cfg_decoder_add_match,
				    "<match>", ed->match);

			if (0 > cfg_decoder_validate(dec, &err_str)) {
				log_warning("%s: %s: %s", v0_cfgfile,
				    "<encdec> (decoder)", err_str);
				cfg_decoder_list_remove(dec_list, &dec);
				warnings++;
			}
		}
	}

	if (warnings)
		log_warning("%s: %u warnings", v0_cfgfile, warnings);

	if (cfg_stream_get_encoder(str) &&
	    NULL == cfg_encoder_list_find(enc_list,
		cfg_stream_get_encoder(str))) {
		log_error("%s: %s encoder not found due to errors",
		    v0_cfgfile, cfg_stream_get_encoder(str));
		return (-1);
	}

	if (0 > cfg_server_validate(srv, &err_str) ||
	    0 > cfg_stream_validate(str, &err_str) ||
	    0 > cfg_intake_validate(in, &err_str)) {
		log_error("%s: configuration invalid: %s", v0_cfgfile,
		    err_str);
		return (-1);
	}

	return (0);
}

int
main(int argc, char *argv[])
{
	int		 ret;
	EZCONFIG	*ez = NULL;

	ret = 1;
	if (0 > cfg_init() ||
	    0 > log_init(util_get_progname(argv[0])) ||
	    0 > _cmdline_parse(argc, argv, &ret))
		return (ret);
	(void)cfg_set_program_name(util_get_progname(argv[0]), NULL);

	if (0 > parseConfig(v0_cfgfile))
		goto error;
	ez = getEZConfig();

	if (0 > _parse_ezconfig0(ez))
		goto error;

	printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n");
	printf("<!--\n");
	printf("  This ezstream version 1.x configuration file was generated by\n");
	printf("  %s.\n\n", util_get_progname(argv[0]));
	printf("  Source (ezstream version 0.x):\n");
	printf("    %s\n", basename(v0_cfgfile));
	printf("  -->\n\n");
	cfgfile_xml_print(stdout);

	freeConfig(ez);

	log_exit();
	cfg_exit();

	return (0);

error:
	if (ez)
		freeConfig(ez);

	log_exit();
	cfg_exit();

	return (1);
}
