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

#include "compat.h"

#include <assert.h>
#include <stdio.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <string.h>

#include "cfg.h"
#include "cmdline.h"
#include "playlist.h"

#define OPTSTRING	"c:hqrs:Vv"
enum opt_vals {
	OPT_CONFIGFILE		= 'c',
	OPT_HELP		= 'h',
	OPT_QUIETSTDERR 	= 'q',
	OPT_RTSTATUS		= 'r',
	OPT_SHUFFLEFILE 	= 's',
	OPT_VERSION		= 'V',
	OPT_VERBOSE		= 'v',
	OPT_INVALID		= '?'
};

static void	_usage(void);
static void	_usage_help(void);
static void	_set_program_name(const char *);

static void
_usage(void)
{
	fprintf(stderr, "usage: %s [-hqrVv] -c cfgfile\n",
	    cfg_get_program_name());
	fprintf(stderr, "       %s -s file\n",
	    cfg_get_program_name());
}

static void
_usage_help(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "    -c cfgfile  use XML configuration in cfgfile\n");
	fprintf(stderr, "    -h          print this help and exit\n");
	fprintf(stderr, "    -q          suppress STDERR output from external en-/decoders\n");
	fprintf(stderr, "    -r          show real-time stream information on stdout\n");
	fprintf(stderr, "    -s file     read lines from file, shuffle, print to STDOUT, then exit\n");
	fprintf(stderr, "    -V          print the version number and exit\n");
	fprintf(stderr, "    -v          increase logging verbosity\n");
}

static void
_set_program_name(const char *argv0)
{
#ifdef HAVE___PROGNAME
	extern char	*__progname;
	(void)argv0;
	cfg_set_program_name(__progname, NULL);
#else
	if (argv0 == NULL) {
		cfg_set_program_name("ezstream", NULL);
	} else {
		const char	*p = strrchr(argv0, '/');
		if (p == NULL)
			p = argv0;
		else
			p++;
		cfg_set_program_name(p, NULL);
	}
#endif /* HAVE___PROGNAME */
}

int
cmdline_parse(int argc, char *argv[], int *ret_p)
{
	const char	*playlistFile = NULL;
	unsigned int	 verbosity = 0;
	const char	*err_str;

	_set_program_name(argv[0]);

	optind = 1;
	for (;;) {
		int	 ch;

		ch = getopt(argc, argv, OPTSTRING);
		if (0 > ch)
			break;

		switch (ch) {
		case OPT_CONFIGFILE:
			assert(0 == cfg_set_program_config_type(CFG_TYPE_XMLFILE,
				   NULL));
			if (0 > cfg_set_program_config_file(optarg, &err_str)) {
				fprintf(stderr, "-%c: argument %s\n",
				    OPT_CONFIGFILE, err_str);
				_usage();
				*ret_p = 2;
				return (-1);
			}
			break;
		case OPT_HELP:
			_usage();
			_usage_help();
			*ret_p = 0;
			return (-1);
		case OPT_RTSTATUS:
			cfg_set_program_rtstatus_output(1, NULL);
			/* FALLTHROUGH */
		case OPT_QUIETSTDERR:
			cfg_set_program_quiet_stderr(1, NULL);
			break;
		case OPT_SHUFFLEFILE:
			playlistFile = optarg;
			break;
		case OPT_VERSION:
			fprintf(stdout, "%s version %s\n",
			    PACKAGE_NAME, PACKAGE_VERSION);
			*ret_p = 0;
			return (-1);
		case OPT_VERBOSE:
			verbosity++;
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

	cfg_set_program_verbosity(verbosity, NULL);

	if (playlistFile) {
		playlist_t	 pl;

		if (0 > playlist_init()) {
			*ret_p = 1;
			return (-1);
		}
		if (0 == strcmp(playlistFile, "-")) {
			pl = playlist_read(NULL);
		} else {
			pl = playlist_read(playlistFile);
		}
		if (pl == NULL) {
			*ret_p = 1;
		} else {
			const char	*entry;

			playlist_shuffle(pl);
			while (NULL != (entry = playlist_get_next(pl)))
				printf("%s\n", entry);
			playlist_free(&pl);
			*ret_p = 0;
		}
		playlist_exit();
		return (-1);
	}

	if (!cfg_get_program_config_file()) {
		fprintf(stderr, "either -%c or -%c must be provided\n",
		    OPT_CONFIGFILE, OPT_SHUFFLEFILE);
		_usage();
		*ret_p = 2;
		return (-1);
	}

	return (0);
}
