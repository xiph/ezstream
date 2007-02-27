/*
 *  ezstream - source client for Icecast with external en-/decoder support
 *  Copyright (C) 2003, 2004, 2005, 2006  Ed Zaleski <oddsock@oddsock.org>
 *  Copyright (C) 2007                    Moritz Grimm <gtgbr@gmx.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_PATHS_H
# include <paths.h>
#endif
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifdef WIN32
# include <io.h>
# include <windows.h>
#else
# include <libgen.h>
# include <unistd.h>
#endif /* WIN32 */
#include <shout/shout.h>
#include <vorbis/vorbisfile.h>

#ifndef HAVE_GETOPT
# include "getopt.h"
#endif
#if !defined(HAVE_STRLCAT) || !defined(HAVE_STRLCPY)
# include "strlfctns.h"
#endif
#include "configfile.h"
#include "ezsignals.h"
#include "playlist.h"
#include "util.h"

#ifndef PATH_MAX
# define PATH_MAX	256
#endif

/* For Solaris, possibly others (usually defined in <paths.h>.) */
#ifndef _PATH_DEVNULL
#  define _PATH_DEVNULL "/dev/null"
#endif /* _PATH_DEVNULL */

#ifdef WIN32
# define STRNCASECMP	strnicmp
# define popen		_popen
# define pclose 	_pclose
# define snprintf	_snprintf
# define stat		_stat
#else
# define STRNCASECMP	strncasecmp
#endif /* WIN32 */

#ifdef HAVE___PROGNAME
extern char		*__progname;
#else
char			*__progname;
#endif /* HAVE___PROGNAME */

int			 qFlag;
int			 vFlag;

EZCONFIG		*pezConfig = NULL;
static const char	*blankString = "";
playlist_t		*playlist = NULL;
int			 playlistMode = 0;

#ifdef HAVE_SIGNALS
const int		 ezstream_signals[] = { SIGHUP, SIGUSR1 };

volatile sig_atomic_t	 rereadPlaylist = 0;
volatile sig_atomic_t	 rereadPlaylist_notify = 0;
volatile sig_atomic_t	 skipTrack = 0;

void
sig_handler(int sig)
{
	switch (sig) {
	case SIGHUP:
		rereadPlaylist = 1;
		rereadPlaylist_notify = 1;
		break;
	case SIGUSR1:
		skipTrack = 1;
		break;
	default:
		break;
	}
}
#else
int			 rereadPlaylist = 0;
int			 rereadPlaylist_notify = 0;
int			 skipTrack = 0;
#endif /* HAVE_SIGNALS */

typedef struct tag_ID3Tag {
	char tag[3];
	char trackName[30];
	char artistName[30];
	char albumName[30];
	char year[3];
	char comment[30];
	char genre;
} ID3Tag;

#ifdef WIN32
char *	basename(const char *);
#endif
int	strrcmp(const char *, const char *);
int	urlParse(const char *, char **, int *, char **);
void	replaceString(const char *, char *, size_t, const char *, const char *);
void	setMetadata(shout_t *, const char *);
char *	buildCommandString(const char *, const char *, const char *);
char *	processMetadata(shout_t *, const char *, const char *);
FILE *	openResource(shout_t *, const char *, int *, char **, int *);
int	sendStream(shout_t *, FILE *, const char *, int, void *);
int	streamFile(shout_t *, const char *);
int	streamPlaylist(shout_t *, const char *);
char *	getProgname(const char *);
void	usage(void);
void	usageHelp(void);

#ifdef WIN32
char *
basename(const char *fileName)
{
	char	*pLast = strrchr(fileName, '\\');

	if (pLast != NULL)
		return (pLast + 1);

	return (NULL);
}
#endif /* WIN32 */

int
strrcmp(const char *s, const char *sub)
{
	int	slen = strlen(s);
	int	sublen = strlen(sub);

	if (sublen > slen)
		return (1);

	return (memcmp(s + slen - sublen, sub, sublen));
}

int
urlParse(const char *url, char **hostname, int *port, char **mountname)
{
	char	*p1, *p2, *p3;
	char	 tmpPort[25] = "";
	size_t	 hostsiz, mountsiz;

	if (hostname == NULL || port == NULL || mountname == NULL) {
		printf("%s: urlParse(): Internal error: Bad arguments\n",
		       __progname);
		exit(1);
	}

	if (strncmp(url, "http://", strlen("http://")) != 0)
		return (0);

	p1 = (char *)(url) + strlen("http://");
	p2 = strchr(p1, ':');
	if (p2 == NULL)
		return (0);
	hostsiz = (p2 - p1) + 1;
	*hostname = xmalloc(hostsiz);
	strlcpy(*hostname, p1, hostsiz);

	p2++;
	p3 = strchr(p2, '/');
	if (p3 == NULL || p3 - p2 >= (int)sizeof(tmpPort))
		return (0);

	strlcpy(tmpPort, p2, (p3 - p2) + 1);
	*port = atoi(tmpPort);

	mountsiz = strlen(p3) + 1;
	*mountname = xmalloc(mountsiz);
	strlcpy(*mountname, p3, mountsiz);

	return (1);
}

void
replaceString(const char *source, char *dest, size_t size,
	      const char *from, const char *to)
{
	char	*p1 = (char *)source;
	char	*p2;

	p2 = strstr(p1, from);
	if (p2 != NULL) {
		if (p2 - p1 >= size) {
			printf("%s: replaceString(): Internal error: p2 - p1 >= size\n",
			       __progname);
			abort();
		}
		strncat(dest, p1, p2 - p1);
		strlcat(dest, to, size);
		p1 = p2 + strlen(from);
	}
	strlcat(dest, p1, size);
}

void
setMetadata(shout_t *shout, const char *metadata)
{
	shout_metadata_t *shoutMetadata = shout_metadata_new();

	shout_metadata_add(shoutMetadata, "song", metadata); 
	shout_set_metadata(shout, shoutMetadata);
	shout_metadata_free(shoutMetadata);
}

char*
buildCommandString(const char *extension, const char *fileName,
		   const char *metadata)
{
	char	*commandString = NULL;
	size_t	 commandStringLen = 0;
	char	*encoder = NULL;
	char	*decoder = NULL;
	char	*newDecoder = NULL;
	size_t	 newDecoderLen = 0;
	char	*newEncoder = NULL;
	size_t	 newEncoderLen = 0;

	decoder = xstrdup(getFormatDecoder(extension));
	if (strlen(decoder) == 0) {
		printf("%s: Unknown extension '%s', cannot decode '%s'.\n",
		       __progname, extension, fileName);
		xfree(decoder);
		return (NULL);
	}
	newDecoderLen = strlen(decoder) + strlen(fileName) + 1;
	newDecoder = xcalloc(1, newDecoderLen);
	replaceString(decoder, newDecoder, newDecoderLen, TRACK_PLACEHOLDER,
		      fileName);
	if (strstr(decoder, METADATA_PLACEHOLDER) != NULL) {
		size_t tmpLen = strlen(newDecoder) + strlen(metadata) + 1;
		char *tmpStr = xcalloc(1, tmpLen);
		replaceString(newDecoder, tmpStr, tmpLen, METADATA_PLACEHOLDER,
			      metadata);
		xfree(newDecoder);
		newDecoder = tmpStr;
	}

	encoder = xstrdup(getFormatEncoder(pezConfig->format));
	if (strlen(encoder) == 0) {
		if (vFlag)
			printf("%s: Passing through%s%s data from the decoder\n",
			       __progname,
			       (strcmp(pezConfig->format, THEORA_FORMAT) != 0) ? " (unsupported) " : " ",
			       pezConfig->format);
		commandStringLen = strlen(newDecoder) + 1;
		commandString = xcalloc(1, commandStringLen);
		strlcpy(commandString, newDecoder, commandStringLen);
		xfree(decoder);
		xfree(encoder);
		xfree(newDecoder);
		return (commandString);
	}

	newEncoderLen = strlen(encoder) + strlen(metadata) + 1;
	newEncoder = xcalloc(1, newEncoderLen);
	replaceString(encoder, newEncoder, newEncoderLen, METADATA_PLACEHOLDER,
		      metadata);

	commandStringLen = strlen(newDecoder) + strlen(" | ") +
		strlen(newEncoder) + 1;
	commandString = xcalloc(1, commandStringLen);
	snprintf(commandString, commandStringLen, "%s | %s", newDecoder,
		 newEncoder);

	xfree(decoder);
	xfree(encoder);
	xfree(newDecoder);
	xfree(newEncoder);

	return (commandString);
}

char *
processMetadata(shout_t *shout, const char *extension, const char *fileName)
{
	FILE			*filepstream = NULL;
	char			*songInfo = NULL;
	size_t			 songLen = 0;
	ID3Tag			 id3tag;
	shout_metadata_t	*pmetadata = NULL;

	if ((filepstream = fopen(fileName, "rb")) == NULL) {
		printf("%s: processMetadata(): %s: %s\n",
		       __progname, fileName, strerror(errno));
		return (xstrdup(blankString));
	}

	if (strcmp(extension, ".mp3") == 0) {
		/* Look for the ID3 tag */
		memset(&id3tag, '\000', sizeof(id3tag));
		fseek(filepstream, -128L, SEEK_END);
		fread(&id3tag, 1, 127, filepstream);
		if (strncmp(id3tag.tag, "TAG", strlen("TAG")) == 0) {
			char 	tempTrackName[31];
			char 	tempArtistName[31];

			snprintf(tempTrackName, sizeof(tempTrackName), "%s",
				 id3tag.trackName);
			snprintf(tempArtistName, sizeof(tempArtistName), "%s",
				 id3tag.artistName);

			if (strlen(tempTrackName) > 0 ||
			    strlen(tempArtistName) > 0) {
				songLen = strlen(tempArtistName) +
					strlen(" - ") + strlen(tempTrackName)
					+ 1;
				songInfo = xmalloc(songLen);
				strlcpy(songInfo, tempArtistName, songLen);
				if (strlen(songInfo) > 0 &&
				    strlen(tempTrackName) > 0)
					strlcat(songInfo, " - ", songLen);
				strlcat(songInfo, tempTrackName, songLen);
			}
		}
	} else if (strcmp(extension, ".ogg") == 0) {
		OggVorbis_File	vf;
		int		ret;

		if ((ret = ov_open(filepstream, &vf, NULL, 0)) != 0) {
			switch (ret) {
			case OV_EREAD:
				printf("%s: No metadata support: %s: Media read error.\n",
				       __progname, fileName);
				break;
			case OV_ENOTVORBIS:
				printf("%s: No metadata support: %s: Invalid Vorbis bitstream.\n",
				       __progname, fileName);
				break;
			case OV_EVERSION:
				printf("%s: No metadata support: %s: Vorbis version mismatch.\n",
				       __progname, fileName);
				break;
			case OV_EBADHEADER:
				printf("%s: No metadata support: %s: Invalid Vorbis bitstream header.\n",
				       __progname, fileName);
				break;
			case OV_EFAULT:
				printf("%s: Fatal: Internal libvorbisfile fault.\n",
				       __progname);
				abort();
			default:
				printf("%s: No metadata support: %s: ov_read() returned unknown error.\n",
				       __progname, fileName);
				break;
			}
		} else {
			char	**ptr = ov_comment(&vf, -1)->user_comments;
			char	*artist = NULL;
			char	*title = NULL;

			while(*ptr){
				if (artist == NULL &&
				    STRNCASECMP(*ptr, "ARTIST", strlen("ARTIST")) == 0)
					artist = xstrdup(*ptr + strlen("ARTIST="));
				if (title == NULL &&
				    STRNCASECMP(*ptr, "TITLE", strlen("TITLE")) == 0)
					title = xstrdup(*ptr + strlen("TITLE="));
				++ptr;
			}

			if (artist != NULL || title != NULL) {
				songLen = 0;
				if (artist != NULL)
					songLen += strlen(artist);
				if (title != NULL)
					songLen += strlen(title);
				songLen += strlen(" - ") + 1;
				songInfo = xcalloc(1, songLen);

				if (artist != NULL)
					strlcpy(songInfo, artist, songLen);
				if (title != NULL) {
					if (artist != NULL)
						strlcat(songInfo, " - ", songLen);
					strlcat(songInfo, title, songLen);
					xfree(title);
				}
				if (artist != NULL)
					xfree(artist);
			}

			ov_clear(&vf);
			filepstream = NULL;
		}
	}

	if (filepstream != NULL)
		fclose(filepstream);

	if (songInfo == NULL) {
		/*
		 * If we didn't get any song info via tags or comments, then
		 * let's just use the filename.
		 */
		char	*p1 = NULL;
		char	*p2 = NULL;
		char	*filename_copy = NULL;

		filename_copy = xstrdup(fileName);
		p2 = basename(filename_copy);
		if (p2 == NULL) {
			/* Assert that basename() cannot fail. */
			printf("%s: Internal error: basename() failed with '%s'\n",
			       __progname, filename_copy);
			exit(1);
		}
		songInfo = xstrdup(p2);
		xfree(filename_copy);
		p1 = strrchr(songInfo, '.');
		if (p1 != NULL)
			*p1 = '\000';
	}

	if ((pmetadata = shout_metadata_new()) == NULL) {
		printf("%s: shout_metadata_new(): %s\n", __progname,
		       strerror(ENOMEM));
		exit(1);
	}
	shout_metadata_add(pmetadata, "song", songInfo);
	shout_set_metadata(shout, pmetadata);
	shout_metadata_free(pmetadata);

	return (songInfo);
}

FILE *
openResource(shout_t *shout, const char *fileName, int *popenFlag,
	     char **metaCopy, int *isStdin)
{
	FILE	*filep = NULL;
	char	 extension[25];
	char	*p1 = NULL;
	char	*pMetadata = NULL;
	char	*pCommandString = NULL;

	if (strcmp(fileName, "stdin") == 0) {
		if (vFlag)
			printf("%s: Reading from standard input.\n",
			       __progname);
		if (isStdin != NULL)
			*isStdin = 1;
#ifdef WIN32
		_setmode(_fileno(stdin), _O_BINARY);
#endif
		filep = stdin;
		return (filep);
	}
	if (isStdin != NULL)
		*isStdin = 0;

	extension[0] = '\0';
	p1 = strrchr(fileName, '.');
	if (p1 != NULL)
		strlcpy(extension, p1, sizeof(extension));
	for (p1 = extension; *p1 != '\0'; p1++)
		*p1 = tolower((int)*p1);
	pMetadata = processMetadata(shout, extension, fileName);
	if (metaCopy != NULL)
		*metaCopy = xstrdup(pMetadata);

	*popenFlag = 0;
	if (pezConfig->reencode) {
		if (strlen(extension) > 0) {
			pCommandString = buildCommandString(extension, fileName, pMetadata);
			if (vFlag > 1)
				printf("%s: Running command `%s`\n", __progname,
				       pCommandString);
			fflush(NULL);
			errno = 0;
			if ((filep = popen(pCommandString, "r")) == NULL) {
				printf("%s: popen(): Error while executing '%s'",
				       __progname, pCommandString);
				/* popen() does not set errno reliably ... */
				if (errno)
					printf(": %s\n", strerror(errno));
				else
					printf("\n");
			} else {
				*popenFlag = 1;
#ifdef WIN32
				_setmode(_fileno(filep), _O_BINARY );
#endif
			}
			xfree(pCommandString);
		} else
			printf("%s: Error: Cannot determine file type of '%s'.\n",
			       __progname, fileName);

		xfree(pMetadata);
		return (filep);
	}

	xfree(pMetadata);

	if ((filep = fopen(fileName, "rb")) == NULL)
		printf("%s: %s: %s\n", __progname, fileName,
		       strerror(errno));

	return (filep);
}

int
sendStream(shout_t *shout, FILE *filepstream, const char *fileName,
	   int isStdin, void *tv)
{
	unsigned char	 buff[4096];
	size_t		 read, total, oldTotal;
	int		 retval = 0;
#ifdef HAVE_GETTIMEOFDAY
	double		 kbps = -1.0;
	struct timeval	 timeStamp, *startTime = (struct timeval *)tv;

	if (startTime == NULL) {
		printf("%s: sendStream(): Internal error: startTime is NULL\n",
		       __progname);
		abort();
	}

	timeStamp.tv_sec = startTime->tv_sec;
	timeStamp.tv_usec = startTime->tv_usec;
#endif /* HAVE_GETTIMEOFDAY */

	total = oldTotal = 0;
	while ((read = fread(buff, 1, sizeof(buff), filepstream)) > 0) {
		int	ret;

		if (rereadPlaylist_notify) {
			rereadPlaylist_notify = 0;
			printf("%s: SIGHUP signal received, will reread playlist after this file.\n",
			       __progname);
		}
		if (skipTrack) {
			skipTrack = 0;
			retval = 2;
			break;
		}

		ret = shout_send(shout, buff, read);
		if (ret != SHOUTERR_SUCCESS) {
			printf("%s: shout_send(): %s\n", __progname,
			       shout_get_error(shout));
			while (1) {
				printf("%s: Disconnected from server, reconnecting ...\n",
				       __progname);
				shout_close(shout);
				if (shout_open(shout) == SHOUTERR_SUCCESS) {
					printf("%s: Reconnect to server successful.\n",
					       __progname);
					ret = shout_send(shout, buff, read);
					if (ret != SHOUTERR_SUCCESS)
						printf("%s: shout_send(): %s\n",
						       __progname,
						       shout_get_error(shout));
					else
						break;
				} else {
					printf("%s: Reconnect failed. Waiting 5 seconds ...\n",
					       __progname);
#ifdef WIN32
					Sleep(5000);
#else
					sleep(5);
#endif
				}
			}
		}

		shout_sync(shout);

		total += read;
		if (qFlag && vFlag) {
#ifdef HAVE_GETTIMEOFDAY
			struct timeval	tv;
			double		oldTime, newTime;
			unsigned int	hrs, mins, secs;
#endif /* HAVE_GETTIMEOFDAY */

			if (!isStdin && playlistMode)
				printf("  [%4lu/%-4lu]",
				       playlist_get_position(playlist),
				       playlist_get_num_items(playlist));

#ifdef HAVE_GETTIMEOFDAY
			oldTime = (double)timeStamp.tv_sec
				+ (double)timeStamp.tv_usec / 1000000.0;
			gettimeofday(&tv, NULL);
			newTime = (double)tv.tv_sec
				+ (double)tv.tv_usec / 1000000.0;
			secs = tv.tv_sec - startTime->tv_sec;
			hrs = secs / 3600;
			secs %= 3600;
			mins = secs / 60;
			secs %= 60;
			if (newTime - oldTime >= 1.0) {
				kbps = (((double)(total - oldTotal) / (newTime - oldTime)) * 8.0) / 1000.0;
				timeStamp.tv_sec = tv.tv_sec;
				timeStamp.tv_usec = tv.tv_usec;
				oldTotal = total;
			}
			printf("  [ %uh%02um%02us]", hrs, mins, secs);
			if (kbps < 0)
				printf("                 ");
			else
				printf("  [%8.2f kbps]", kbps);
#endif /* HAVE_GETTIMEOFDAY */

			printf("  \r");
			fflush(stdout);
		}
	}
	if (ferror(filepstream)) {
		if (errno == EINTR) {
			clearerr(filepstream);
			retval = 1;
		} else
			printf("%s: streamFile(): Error while reading '%s': %s\n",
			       __progname, fileName, strerror(errno));
	}

	return (retval);
}

int
streamFile(shout_t *shout, const char *fileName)
{
	FILE		*filepstream = NULL;
	int		 popenFlag = 0;
	char		*metaData = NULL;
	int		 isStdin = 0;
	int              ret, retval = 0;
#ifdef HAVE_GETTIMEOFDAY
	struct timeval	 startTime;
#endif

	if ((filepstream = openResource(shout, fileName, &popenFlag,
					&metaData, &isStdin))
	    == NULL) {
		return (retval);
	}

	if (metaData != NULL) {
		printf("%s: Streaming ``%s''", __progname, metaData);
		if (vFlag)
			printf(" (file: %s)\n", fileName);
		else
			printf("\n");
		xfree(metaData);
	}

#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&startTime, NULL);
	do {
		ret = sendStream(shout, filepstream, fileName, isStdin,
				 (void *)&startTime);
#else
	do {
		ret = sendStream(shout, filepstream, fileName, isStdin, NULL);
#endif
		if (ret != 0) {
			if (skipTrack && rereadPlaylist) {
				skipTrack = 0;
				ret = 1;
			}
			if (ret == 2 || skipTrack) {
				skipTrack = 0;
				if (!isStdin && vFlag)
					printf("%s: SIGUSR1 signal received, skipping current track.\n",
					       __progname);
				retval = 1;
				ret = 0;
			}
		} else
			retval = 1;
	} while (ret);

	if (popenFlag)
		pclose(filepstream);
	else
		fclose(filepstream);

	return (retval);
}

int
streamPlaylist(shout_t *shout, const char *fileName)
{
	const char	*song;
	char		 lastSong[PATH_MAX + 1];

	/*
	 * XXX: This preserves traditional behavior, however, rereading the
	 *      playlist after each walkthrough seems a bit more logical.
	 */
	if (playlist == NULL) {
		if ((playlist = playlist_read(fileName)) == NULL)
			return (0);
	} else
		playlist_rewind(playlist);

	if (pezConfig->shuffle)
		playlist_shuffle(playlist);

	while ((song = playlist_get_next(playlist)) != NULL) {
		strlcpy(lastSong, song, sizeof(lastSong));
		if (!streamFile(shout, song))
			return (0);
		if (rereadPlaylist) {
			rereadPlaylist = rereadPlaylist_notify = 0;
			printf("%s: Rereading playlist\n", __progname);
			if (!playlist_reread(&playlist))
				return (0);
			if (pezConfig->shuffle)
				playlist_shuffle(playlist);
			else {
				playlist_goto_entry(playlist, lastSong);
				playlist_skip_next(playlist);
			}
			continue;
		}
	}

	return (1);
}

/* Borrowed from OpenNTPd-portable's compat-openbsd/bsd-misc.c */
char *
getProgname(const char *argv0)
{
#ifdef HAVE___PROGNAME
	return (xstrdup(__progname));
#else
	char	*p;

	if (argv0 == NULL)
		return ((char *)"ezstream");
	p = strrchr(argv0, '/');
	if (p == NULL)
		p = (char *)argv0;
	else
		p++;

	return (xstrdup(p));
#endif /* HAVE___PROGNAME */
}

void
usage(void)
{
	printf("usage: %s [-hqv] [-c configfile]\n", __progname);
}

void
usageHelp(void)
{
	printf("\n");
	printf("  -c configfile  use XML configuration in configfile\n");
	printf("  -h             display this additional help and exit\n");
	printf("  -q             suppress STDERR output from external en-/decoders\n");
	printf("  -v             verbose output\n");
	printf("\n");
	printf("See the ezstream(1) manual for detailed information.\n");
}

int
main(int argc, char *argv[])
{
	int		 c;
	char		*configFile = NULL;
	char		*host = NULL;
	int		 port = 0;
	char		*mount = NULL;
	shout_t 	*shout;
	extern char	*optarg;
	extern int	 optind;
#ifdef HAVE_SIGNALS
	struct sigaction act;
#endif

	__progname = getProgname(argv[0]);
	pezConfig = getEZConfig();

	qFlag = 0;
	vFlag = 0;

#ifdef HAVE_GETEUID
	if (geteuid() == 0) {
		printf("WARNING: You should not run %s as root. It can run other programs, which\n",
		       __progname);
		printf("         may cause serious security problems.\n");
	}
#endif

	while ((c = getopt(argc, argv, "c:hqv")) != -1) {
		switch (c) {
		case 'c':
			if (configFile != NULL) {
				printf("Error: multiple -c arguments given.\n");
				usage();
				return (2);
			}
			configFile = xstrdup(optarg);
			break;
		case 'h':
			usage();
			usageHelp();
			return (0);
		case 'q':
			qFlag = 1;
			break;
		case 'v':
			vFlag++;
			break;
		case '?':
			usage();
			return (2);
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (configFile == NULL) {
		printf("You must supply a config file with the -c argument.\n");
		usage();
		return (2);
	} else {
		/*
		 * Attempt to open configFile here for a more meaningful error
		 * message.
		 */
		FILE	*tmp;

		if ((tmp = fopen(configFile, "r")) == NULL) {
			printf("%s: %s\n", configFile, strerror(errno));
			usage();
			return (2);
		} else
			fclose(tmp);
	}

	if (!parseConfig(configFile))
		return (2);

	shout_init();
	playlist_init();

	if (pezConfig->URL == NULL) {
		printf("%s: Error: Missing <url>\n", configFile);
		return (2);
	}
	if (!urlParse(pezConfig->URL, &host, &port, &mount)) {
		printf("%s: Error: Invalid <url>:\n", configFile);
		printf("Must be of the form ``http://server:port/mountpoint''.\n");
		return (2);
	}
	if ((host == NULL)) {
		printf("%s: Error: Invalid <url>: Missing server:\n", configFile);
		printf("Must be of the form ``http://server:port/mountpoint''.\n");
		return (2);
	}
	if ((port < 1 || port > 65535)) {
		printf("%s: Error: Invalid <url>: Missing or invalid port:\n", configFile);
		printf("Must be of the form ``http://server:port/mountpoint''.\n");
		return (2);
	}
	if ((mount == NULL)) {
		printf("%s: Error: Invalid <url>: Missing mountpoint:\n", configFile);
		printf("Must be of the form ``http://server:port/mountpoint''.\n");
		return (2);
	}
	if ((pezConfig->password == NULL)) {
		printf("%s: Error: Missing <sourcepassword>\n", configFile);
		return (2);
	}
	if ((pezConfig->fileName == NULL)) {
		printf("%s: Error: Missing <filename>\n", configFile);
		return (2);
	}
	if (pezConfig->format == NULL) {
		printf("%s: Warning: Missing <format>:\n", configFile);
		printf("Specify a stream format of either MP3, VORBIS or THEORA.\n");
	}

	xfree(configFile);

	if ((shout = shout_new()) == NULL) {
		printf("%s: shout_new(): %s", __progname, strerror(ENOMEM));
		return (1);
	}

	if (shout_set_host(shout, host) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_host(): %s\n", __progname,
			shout_get_error(shout));
		return (1);
	}
	if (shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_protocol(): %s\n", __progname,
			shout_get_error(shout));
		return (1);
	}
	if (shout_set_port(shout, port) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_port: %s\n", __progname,
			shout_get_error(shout));
		return (1);
	}
	if (shout_set_password(shout, pezConfig->password) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_password(): %s\n", __progname,
			shout_get_error(shout));
		return (1);
	}
	if (shout_set_mount(shout, mount) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_mount(): %s\n", __progname,
			shout_get_error(shout));
		return (1);
	}
	if (shout_set_user(shout, "source") != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_user(): %s\n", __progname,
			shout_get_error(shout));
		return (1);
	}

	if (!strcmp(pezConfig->format, MP3_FORMAT)) {
		if (shout_set_format(shout, SHOUT_FORMAT_MP3) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_format(MP3): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}
	if (!strcmp(pezConfig->format, VORBIS_FORMAT) ||
	    !strcmp(pezConfig->format, THEORA_FORMAT)) {
		if (shout_set_format(shout, SHOUT_FORMAT_OGG) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_format(OGG): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}

	if (pezConfig->serverName) {
		if (shout_set_name(shout, pezConfig->serverName) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_name(): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}
	if (pezConfig->serverURL) {
		if (shout_set_url(shout, pezConfig->serverURL) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_url(): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}
	if (pezConfig->serverGenre) {
		if (shout_set_genre(shout, pezConfig->serverGenre) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_genre(): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}
	if (pezConfig->serverDescription) {
		if (shout_set_description(shout, pezConfig->serverDescription) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_description(): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}
	if (pezConfig->serverBitrate) {
		if (shout_set_audio_info(shout, SHOUT_AI_BITRATE, pezConfig->serverBitrate) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_audio_info(AI_BITRATE): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}
	if (pezConfig->serverChannels) {
		if (shout_set_audio_info(shout, SHOUT_AI_CHANNELS, pezConfig->serverChannels) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_audio_info(AI_CHANNELS): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}
	if (pezConfig->serverSamplerate) {
		if (shout_set_audio_info(shout, SHOUT_AI_SAMPLERATE, pezConfig->serverSamplerate) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_audio_info(AI_SAMPLERATE): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}
	if (pezConfig->serverQuality) {
		if (shout_set_audio_info(shout, SHOUT_AI_QUALITY, pezConfig->serverQuality) != SHOUTERR_SUCCESS) {
			printf("%s: shout_set_audio_info(AI_QUALITY): %s\n",
			       __progname, shout_get_error(shout));
			return (1);
		}
	}

	if (shout_set_public(shout, pezConfig->serverPublic) != SHOUTERR_SUCCESS) {
		printf("%s: shout_set_public(): %s\n",
		       __progname, shout_get_error(shout));
		return (1);
	}

#ifdef HAVE_SIGNALS
	memset(&act, 0, sizeof(act));
	act.sa_handler = sig_handler;
# ifdef SA_RESTART
	act.sa_flags = SA_RESTART;
# endif
	SIGS_INSTALL(ezstream_signals, &act);
#endif /* HAVE_SIGNALS */

	if (qFlag) {
		int fd;

		if ((fd = open(_PATH_DEVNULL, O_RDWR, 0)) == -1) {
			printf("%s: Cannot open %s for redirecting STDERR output: %s\n",
			       __progname, _PATH_DEVNULL, strerror(errno));
			return (1);
		}

		dup2(fd, STDERR_FILENO);
		if (fd > 2)
			close(fd);
	}

	if (shout_open(shout) == SHOUTERR_SUCCESS) {
		int	ret;
		char	*tmpFileName, *p;

		printf("%s: Connected to http://%s:%d%s\n", __progname,
		       host, port, mount);

		tmpFileName = xstrdup(pezConfig->fileName);
		for (p = tmpFileName; *p != '\0'; p++)
			*p = tolower((int)*p);
		if (strrcmp(tmpFileName, ".m3u") == 0 ||
		    strrcmp(tmpFileName, ".txt") == 0)
			playlistMode = 1;
		else
			playlistMode = 0;
		xfree(tmpFileName);

		ret = 1;
		do {
			if (playlistMode)
				ret = streamPlaylist(shout,
						     pezConfig->fileName);
			else
				ret = streamFile(shout, pezConfig->fileName);
		} while (ret);
	} else
		printf("%s: Connection to http://%s:%d%s failed: %s\n", __progname,
		       host, port, mount, shout_get_error(shout));

	shout_close(shout);

	playlist_free(playlist);
	playlist_shutdown();

	shout_shutdown();

	xfree(host);
	xfree(mount);

	return 0;
}
