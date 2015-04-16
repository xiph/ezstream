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

#include <limits.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include "cfg.h"

#define OPTSTRING	"c:hmnqs:Vv"
enum opt_vals {
	OPT_CONFIGFILE		= 'c',
	OPT_HELP		= 'h',
	OPT_NOMETADATAUPDATE	= 'm',
	OPT_NORMALIZESTRINGS	= 'n',
	OPT_QUIETSTDERR 	= 'q',
	OPT_SHUFFLEFILE 	= 's',
	OPT_VERSION		= 'V',
	OPT_VERBOSE		= 'v',
	OPT_INVALID		= '?'
};

static struct cfg {
	char		progname[PATH_MAX];
	char		config_file[PATH_MAX];
	int		no_metadata_updates;
	int		normalize_strings;
	int		quiet_stderr;
	char		shuffle_file[PATH_MAX];
	unsigned int	verbosity;
} cfg;

static void	usage(void);
static void	usage_help(void);

static void
_set_progname(const char *argv0)
{
#ifdef HAVE___PROGNAME
	extern char	*__progname;
	(void)argv0;
	snprintf(cfg.progname, sizeof(cfg.progname), "%s", __progname);
#else
	if (argv0 == NULL) {
		snprintf(cfg.progname, sizeof(cfg.progname), "ezstream");
	} else {
		const char	*p = strrchr(argv0, '/');
		if (p == NULL)
			p = argv0;
		else
			p++;
		snprintf(cfg.progname, sizeof(cfg.progname), "%s", p);
	}
#endif /* HAVE___PROGNAME */
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-ghmnqVv] -c cfgfile\n",
	    cfg.progname);
	fprintf(stderr, "       %s [-ghV] -s file\n",
	    cfg.progname);
}

static void
usage_help(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "    -c cfgfile  use XML configuration in cfgfile\n");
	fprintf(stderr, "    -h          print this help and exit\n");
	fprintf(stderr, "    -m          disable metadata updates\n");
	fprintf(stderr, "    -n          normalize metadata strings\n");
	fprintf(stderr, "    -q          suppress STDERR output from external en-/decoders\n");
	fprintf(stderr, "    -s file     read lines from file, shuffle, print to STDOUT, then exit\n");
	fprintf(stderr, "    -V          print the version number and exit\n");
	fprintf(stderr, "    -v          verbose output (use twice for more effect)\n");
}

int
cfg_cmdline_parse(int argc, char *argv[], int *ret_p)
{
	int	ch;

	memset(&cfg, 0, sizeof(cfg));

	_set_progname(argv[0]);

	for (;;) {
		ch = getopt(argc, argv, OPTSTRING);
		if (0 > ch)
			break;

		switch (ch) {
		case OPT_CONFIGFILE:
			if (cfg.config_file[0]) {
				fprintf(stderr,
				    "option -%c may only be given once\n",
				    OPT_CONFIGFILE);
				usage();
				*ret_p = 2;
				return (-1);
			}
			(void)snprintf(cfg.config_file,
			    sizeof(cfg.config_file), "%s", optarg);
			break;
		case OPT_HELP:
			usage();
			usage_help();
			*ret_p = 0;
			return (-1);
		case OPT_NOMETADATAUPDATE:
			cfg.no_metadata_updates = 1;
			break;
		case OPT_NORMALIZESTRINGS:
			cfg.normalize_strings = 1;
			break;
		case OPT_QUIETSTDERR:
			cfg.quiet_stderr = 1;
			break;
		case OPT_SHUFFLEFILE:
			if (cfg.shuffle_file[0]) {
				fprintf(stderr,
				    "option -%c may only be given once\n",
				    OPT_SHUFFLEFILE);
				usage();
				*ret_p = 2;
				return (-1);
			}
			(void)snprintf(cfg.shuffle_file,
			    sizeof(cfg.shuffle_file), "%s", optarg);
			break;
		case OPT_VERSION:
			fprintf(stdout, "%s version %s\n",
			    PACKAGE_NAME, PACKAGE_VERSION);
			*ret_p = 0;
			return (-1);
		case OPT_VERBOSE:
			cfg.verbosity++;
			break;
		case OPT_INVALID:
		default:
			usage();
			*ret_p = 2;
			return (-1);
		}
	}
	argc -= optind;
	argv += optind;

	if ((!cfg.config_file[0] && !cfg.shuffle_file[0]) ||
	    (cfg.config_file[0] && cfg.shuffle_file[0])) {
		fprintf(stderr, "either -%c or -%c must be provided\n",
		    OPT_CONFIGFILE, OPT_SHUFFLEFILE);
		usage();
		*ret_p = 2;
		return (-1);
	}

	return (0);
}

const char *
cfg_progname(void)
{
	return (cfg.progname);
}

const char *
cfg_config_file(void)
{
	return (cfg.config_file);
}

int
cfg_no_metadata_updates(void)
{
	return (cfg.no_metadata_updates);
}

int
cfg_normalize_strings(void)
{
	return (cfg.normalize_strings);
}

int
cfg_quiet_stderr(void)
{
	return (cfg.quiet_stderr);
}

const char *
cfg_shuffle_file(void)
{
	return (cfg.shuffle_file[0] ? cfg.shuffle_file : NULL);
}

unsigned int
cfg_verbosity(void)
{
	return (cfg.verbosity);
}
