/*
 *  ezstream - source client for Icecast with external en-/decoder support
 *  Copyright (C) 2003, 2004, 2005, 2006  Ed Zaleski <oddsock@oddsock.org>
 *  Copyright (C) 2007, 2009, 2015, 2017  Moritz Grimm <mgrimm@mrsserver.net>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "compat.h"

#include "ezstream.h"

#include <signal.h>

#include "cfg.h"
#include "cmdline.h"
#include "log.h"
#include "mdata.h"
#include "playlist.h"
#include "stream.h"
#include "util.h"
#include "xalloc.h"

#define STREAM_DONE	0
#define STREAM_CONT	1
#define STREAM_SKIP	2
#define STREAM_SERVERR	3
#define STREAM_UPDMDATA 4

playlist_t		 playlist;
int			 playlistMode;
unsigned int		 resource_errors;

const int		 ezstream_signals[] = {
	SIGTERM, SIGINT, SIGHUP, SIGUSR1, SIGUSR2
};

volatile sig_atomic_t	 rereadPlaylist;
volatile sig_atomic_t	 rereadPlaylist_notify;
volatile sig_atomic_t	 skipTrack;
volatile sig_atomic_t	 queryMetadata;
volatile sig_atomic_t	 quit;

void		sig_handler(int);
char *		_build_reencode_cmd(const char *, const char *, mdata_t);
FILE *		openResource(stream_t, const char *, int *, mdata_t *,
			     int *, long *);
int		reconnect(stream_t);
const char *	getTimeString(long);
int		sendStream(stream_t, FILE *, const char *, int, const char *,
			   struct timespec *);
int		streamFile(stream_t, const char *);
int		streamPlaylist(stream_t);
int		ez_shutdown(int);

void
sig_handler(int sig)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		quit = 1;
		break;
	case SIGHUP:
		rereadPlaylist = 1;
		rereadPlaylist_notify = 1;
		break;
	case SIGUSR1:
		skipTrack = 1;
		break;
	case SIGUSR2:
		queryMetadata = 1;
		break;
	default:
		break;
	}
}

char *
_build_reencode_cmd(const char *extension, const char *filename,
    mdata_t md)
{
	cfg_decoder_t		 decoder;
	cfg_encoder_t		 encoder;
	char			*artist, *album, *title, *songinfo, *tmp;
	char			*filename_quoted;
	char			*custom_songinfo;
	struct util_dict	 dicts[6];
	char			*dec_str;
	char			*enc_str;
	char			*cmd_str;
	size_t			 cmd_str_size;

	decoder = cfg_decoder_find(extension);
	if (!decoder) {
		log_error("cannot decode: %s: unsupported file extension %s",
		    filename, extension);
		return (NULL);
	}
	encoder = cfg_encoder_get(cfg_get_stream_encoder());
	if (!encoder) {
		log_error("cannot encode: %s: unknown encoder",
		    cfg_get_stream_encoder());
		return (NULL);
	}

	tmp = util_utf82char(mdata_get_artist(md));
	artist = util_shellquote(tmp);
	xfree(tmp);

	tmp = util_utf82char(mdata_get_album(md));
	album = util_shellquote(tmp);
	xfree(tmp);

	tmp = util_utf82char(mdata_get_title(md));
	title = util_shellquote(tmp);
	xfree(tmp);

	tmp = util_utf82char(mdata_get_songinfo(md));
	songinfo = util_shellquote(tmp);
	xfree(tmp);

	filename_quoted = util_shellquote(filename);

	/*
	 * if (prog && format)
	 *    metatoformat
	 * else
	 *   if (!prog && title)
	 *     emptymeta
	 *   else
	 *     replacemeta
	 */
	if (cfg_get_metadata_program() &&
	    cfg_get_metadata_format_str()) {
		char	 buf[BUFSIZ];
		char	*unquoted;

		mdata_strformat(md, buf, sizeof(buf),
		    cfg_get_metadata_format_str());
		unquoted = util_utf82char(buf);
		custom_songinfo = util_shellquote(unquoted);
		xfree(unquoted);
	} else {
		if (!cfg_get_metadata_program() &&
		    strstr(cfg_decoder_get_program(decoder),
			PLACEHOLDER_TITLE) != NULL) {
			custom_songinfo = xstrdup("");
		} else {
			custom_songinfo = xstrdup(songinfo);
		}
	}
	xfree(songinfo);

	memset(dicts, 0, sizeof(dicts));
	dicts[0].from = PLACEHOLDER_ARTIST;
	dicts[0].to = artist;
	dicts[1].from = PLACEHOLDER_ALBUM;
	dicts[1].to = album;
	dicts[2].from = PLACEHOLDER_TITLE;
	dicts[2].to = title;
	dicts[3].from = PLACEHOLDER_TRACK;
	dicts[3].to = filename_quoted;
	dicts[4].from = PLACEHOLDER_METADATA;
	dicts[4].to = custom_songinfo;

	dec_str = util_expand_words(cfg_decoder_get_program(decoder), dicts);

	if (!cfg_get_metadata_program() &&
	    strstr(cfg_encoder_get_program(encoder),
		PLACEHOLDER_TITLE) != NULL) {
		xfree(custom_songinfo);
		dicts[4].to = custom_songinfo = xstrdup("");
	}

	enc_str = util_expand_words(cfg_encoder_get_program(encoder), dicts);

	cmd_str_size = strlen(dec_str) + strlen(" | ") + strlen(enc_str) + 1;
	cmd_str = xcalloc(cmd_str_size, sizeof(char));
	snprintf(cmd_str, cmd_str_size, "%s | %s", dec_str, enc_str);
	xfree(dec_str);
	xfree(enc_str);

	xfree(artist);
	xfree(album);
	xfree(title);
	xfree(filename_quoted);
	xfree(custom_songinfo);

	return (cmd_str);
}

FILE *
openResource(stream_t stream, const char *filename, int *popenFlag,
	     mdata_t *md_p, int *isStdin, long *songLen)
{
	FILE		*filep = NULL;
	char		 extension[25];
	char		*p = NULL;
	char		*pCommandString = NULL;
	mdata_t 	 md;

	if (md_p != NULL)
		*md_p = NULL;
	if (songLen != NULL)
		*songLen = 0;

	if ((isStdin && *isStdin) ||
	    strcasecmp(filename, "stdin") == 0) {
		if (cfg_get_metadata_program()) {
			md = mdata_create();

			if (0 > mdata_run_program(md, cfg_get_metadata_program()) ||
			    0 > stream_set_metadata(stream, md, NULL)) {
				mdata_destroy(&md);
				return (NULL);
			}

			if (md_p != NULL)
				*md_p = md;
			else
				mdata_destroy(&md);
		}

		if (isStdin != NULL)
			*isStdin = 1;
		filep = stdin;
		return (filep);
	}

	if (isStdin != NULL)
		*isStdin = 0;

	extension[0] = '\0';
	p = strrchr(filename, '.');
	if (p != NULL)
		strlcpy(extension, p, sizeof(extension));
	for (p = extension; *p != '\0'; p++)
		*p = (char)tolower((int)*p);

	if (strlen(extension) == 0) {
		log_error("%s: cannot determine file type", filename);
		return (filep);
	}

	md = mdata_create();
	if (cfg_get_metadata_program()) {
		if (0 > mdata_run_program(md, cfg_get_metadata_program()))
			mdata_destroy(&md);
	} else {
		if (0 > mdata_parse_file(md, filename))
			mdata_destroy(&md);
	}
	if (NULL == md)
		return (NULL);
	if (songLen != NULL)
		*songLen = mdata_get_length(md);

	*popenFlag = 0;
	if (cfg_get_stream_encoder()) {
		int	stderr_fd = -1;

		pCommandString = _build_reencode_cmd(extension, filename,
		    md);
		if (md_p != NULL)
			*md_p = md;
		else
			mdata_destroy(&md);
		log_info("running command: %s", pCommandString);

		if (cfg_get_program_quiet_stderr()) {
			int fd;

			stderr_fd = dup(fileno(stderr));
			if (0 > stderr_fd) {
				log_alert("dup: %s", strerror(errno));
				exit(1);
			}
			if ((fd = open(_PATH_DEVNULL, O_RDWR, 0)) == -1) {
				log_alert("%s: %s", _PATH_DEVNULL,
				    strerror(errno));
				exit(1);
			}

			dup2(fd, fileno(stderr));
			close(fd);
		}

		fflush(NULL);
		errno = 0;
		if ((filep = popen(pCommandString, "r")) == NULL) {
			/* popen() does not set errno reliably ... */
			if (errno)
				log_error("execution error: %s: %s",
				    pCommandString, strerror(errno));
			else
				log_error("execution error: %s",
				    pCommandString);
		} else {
			*popenFlag = 1;
		}
		xfree(pCommandString);

		if (cfg_get_program_quiet_stderr())
			dup2(stderr_fd, fileno(stderr));

		if (stderr_fd != -1)
			close(stderr_fd);

		return (filep);
	}

	if (md_p != NULL)
		*md_p = md;
	else
		mdata_destroy(&md);

	if ((filep = fopen(filename, "rb")) == NULL) {
		log_error("%s: %s", filename, strerror(errno));
		return (NULL);
	}

	return (filep);
}

int
reconnect(stream_t stream)
{
	unsigned int	i;

	i = 0;
	while (++i) {
		if (cfg_get_server_reconnect_attempts() > 0)
			log_notice("reconnect: %s: attempt #%u/%u ...",
			    cfg_get_server_hostname(), i,
			    cfg_get_server_reconnect_attempts());
		else
			log_notice("reconnect: %s: attempt #%u ...",
			    cfg_get_server_hostname(), i);

		stream_disconnect(stream);
		if (0 == stream_connect(stream)) {
			log_notice("reconnect: %s: success",
			    cfg_get_server_hostname());
			return (0);
		}

		if (cfg_get_server_reconnect_attempts() > 0 &&
		    i >= cfg_get_server_reconnect_attempts())
			break;

		if (quit)
			return (-1);
		else
			sleep(5);
	};

	log_warning("reconnect failed: giving up");

	return (-1);
}

const char *
getTimeString(long seconds)
{
	static char	str[20];
	long		secs, mins, hours;

	if (seconds < 0)
		return (NULL);

	secs = seconds;
	hours = secs / 3600;
	secs %= 3600;
	mins = secs / 60;
	secs %= 60;

	snprintf(str, sizeof(str), "%ldh%02ldm%02lds", hours, mins, secs);
	return ((const char *)str);
}

int
sendStream(stream_t stream, FILE *filepstream, const char *fileName,
	   int isStdin, const char *songLenStr, struct timespec *tv)
{
	char		  buff[4096];
	size_t		  bytes_read, total, oldTotal;
	int		  ret;
	double		  kbps = -1.0;
	struct timespec	  timeStamp, *startTime = tv;
	struct timespec	  callTime, currentTime;

	clock_gettime(CLOCK_MONOTONIC, &callTime);

	timeStamp.tv_sec = startTime->tv_sec;
	timeStamp.tv_nsec = startTime->tv_nsec;

	total = oldTotal = 0;
	ret = STREAM_DONE;
	while ((bytes_read = fread(buff, 1, sizeof(buff), filepstream)) > 0) {
		if (!stream_get_connected(stream)) {
			log_warning("%s: connection lost",
			    cfg_get_server_hostname());
			if (0 > reconnect(stream)) {
				ret = STREAM_SERVERR;
				break;
			}
		}

		stream_sync(stream);

		if (0 > stream_send(stream, buff, bytes_read)) {
			if (0 > reconnect(stream))
				ret = STREAM_SERVERR;
			break;
		}

		if (quit)
			break;
		if (rereadPlaylist_notify) {
			rereadPlaylist_notify = 0;
			if (CFG_MEDIA_PLAYLIST == cfg_get_media_type())
				log_notice("HUP signal received: playlist re-read scheduled");
		}
		if (skipTrack) {
			skipTrack = 0;
			ret = STREAM_SKIP;
			break;
		}

		clock_gettime(CLOCK_MONOTONIC, &currentTime);

		if (queryMetadata ||
		    (0 <= cfg_get_metadata_refresh_interval() &&
			(currentTime.tv_sec - callTime.tv_sec >=
			    cfg_get_metadata_refresh_interval()))) {
			queryMetadata = 0;
			if (cfg_get_metadata_program()) {
				ret = STREAM_UPDMDATA;
				break;
			}
		}

		total += bytes_read;
		if (cfg_get_program_rtstatus_output()) {
			double	oldTime, newTime;

			if (!isStdin && playlistMode) {
				if (CFG_MEDIA_PROGRAM == cfg_get_media_type()) {
					char *tmp = xstrdup(cfg_get_media_filename());
					printf("  [%s]", basename(tmp));
					xfree(tmp);
				} else
					printf("  [%4lu/%-4lu]",
					    playlist_get_position(playlist),
					    playlist_get_num_items(playlist));
			}

			oldTime = (double)timeStamp.tv_sec
			    + (double)timeStamp.tv_nsec / 1000000000.0;
			newTime = (double)currentTime.tv_sec
			    + (double)currentTime.tv_nsec / 1000000000.0;
			if (songLenStr == NULL)
				printf("  [ %s]",
				    getTimeString(currentTime.tv_sec -
					startTime->tv_sec));
			else
				printf("  [ %s/%s]",
				    getTimeString(currentTime.tv_sec -
					startTime->tv_sec),
				    songLenStr);
			if (newTime - oldTime >= 1.0) {
				kbps = (((double)(total - oldTotal)
				    / (newTime - oldTime)) * 8.0) / 1000.0;
				timeStamp.tv_sec = currentTime.tv_sec;
				timeStamp.tv_nsec = currentTime.tv_nsec;
				oldTotal = total;
			}
			if (kbps < 0)
				printf("                 ");
			else
				printf("  [%8.2f kbps]", kbps);

			printf("  \r");
			fflush(stdout);
		}
	}
	if (ferror(filepstream)) {
		if (errno == EINTR) {
			clearerr(filepstream);
			ret = STREAM_CONT;
		} else if (errno == EBADF && isStdin)
			log_notice("no (more) data available on standard input");
		else
			log_error("sendStream: %s: %s", fileName,
			    strerror(errno));
	}

	return (ret);
}

int
streamFile(stream_t stream, const char *fileName)
{
	FILE		*filepstream = NULL;
	int		 popenFlag = 0;
	char		*songLenStr = NULL;
	int		 isStdin = cfg_get_media_type() == CFG_MEDIA_STDIN;
	int		 ret, retval = 0;
	long		 songLen;
	mdata_t 	 md = NULL;
	struct timespec	 startTime;

	if ((filepstream = openResource(stream, fileName, &popenFlag, &md, &isStdin, &songLen))
	    == NULL) {
		mdata_destroy(&md);
		if (++resource_errors > 100) {
			log_error("too many errors; giving up");
			return (0);
		}
		/* Continue with next resource on failure: */
		return (1);
	}
	resource_errors = 0;

	if (md != NULL) {
		const char	*tmp;
		char		*metaData;

		tmp = mdata_get_songinfo(md) ?
		    mdata_get_songinfo(md) : mdata_get_name(md);
		metaData = util_utf82char(tmp);
		log_notice("streaming: %s (%s)", metaData,
		    isStdin ? "stdin" : fileName);
		xfree(metaData);

		/* MP3 streams are special, so set the metadata explicitly: */
		if (CFG_STREAM_MP3 == cfg_get_stream_format())
			stream_set_metadata(stream, md, NULL);

		mdata_destroy(&md);
	} else if (isStdin)
		log_notice("streaming: standard input");

	if (songLen > 0)
		songLenStr = xstrdup(getTimeString(songLen));
	clock_gettime(CLOCK_MONOTONIC, &startTime);
	do {
		ret = sendStream(stream, filepstream, fileName, isStdin,
		    songLenStr, &startTime);
		if (quit)
			break;
		if (ret != STREAM_DONE) {
			if ((skipTrack && rereadPlaylist) ||
			    (skipTrack && queryMetadata)) {
				skipTrack = 0;
				ret = STREAM_CONT;
			}
			if (queryMetadata && rereadPlaylist) {
				queryMetadata = 0;
				ret = STREAM_CONT;
			}
			if (ret == STREAM_SKIP || skipTrack) {
				skipTrack = 0;
				if (!isStdin)
					log_notice("USR1 signal received: skipping current track");
				retval = 1;
				ret = STREAM_DONE;
			}
			if (ret == STREAM_UPDMDATA || queryMetadata) {
				queryMetadata = 0;
				if (cfg_get_metadata_no_updates())
					continue;
				if (cfg_get_metadata_program()) {
					char		*mdataStr = NULL;
					mdata_t 	 prog_md;

					log_info("running metadata program: %s",
					    cfg_get_metadata_program());
					prog_md = mdata_create();
					if (0 > mdata_run_program(md, cfg_get_metadata_program()) ||
					    0 > stream_set_metadata(stream, prog_md, &mdataStr)) {
						mdata_destroy(&prog_md);
						retval = 0;
						ret = STREAM_DONE;
						continue;
					}
					mdata_destroy(&prog_md);
					log_info("new metadata: %s", mdataStr);
					xfree(mdataStr);
				}
			}
			if (ret == STREAM_SERVERR) {
				retval = 0;
				ret = STREAM_DONE;
			}
		} else
			retval = 1;
	} while (ret != STREAM_DONE);

	if (popenFlag)
		pclose(filepstream);
	else if (!isStdin)
		fclose(filepstream);

	if (songLenStr != NULL)
		xfree(songLenStr);

	return (retval);
}

int
streamPlaylist(stream_t stream)
{
	const char	*song;
	char		 lastSong[PATH_MAX];

	if (playlist == NULL) {
		switch (cfg_get_media_type()) {
		case CFG_MEDIA_PROGRAM:
			if ((playlist = playlist_program(cfg_get_media_filename())) == NULL)
				return (0);
			break;
		case CFG_MEDIA_STDIN:
			if ((playlist = playlist_read(NULL)) == NULL)
				return (0);
			break;
		default:
			if ((playlist = playlist_read(cfg_get_media_filename())) == NULL)
				return (0);
			if (playlist_get_num_items(playlist) == 0)
				log_warning("%s: playlist empty",
				    cfg_get_media_filename());
			break;
		}
	} else {
		/*
		 * XXX: This preserves traditional behavior, however,
		 *      rereading the playlist after each walkthrough seems a
		 *      bit more logical.
		 */
		playlist_rewind(playlist);
	}

	if (CFG_MEDIA_PROGRAM != cfg_get_media_type() &&
	    cfg_get_media_shuffle())
		playlist_shuffle(playlist);

	while ((song = playlist_get_next(playlist)) != NULL) {
		strlcpy(lastSong, song, sizeof(lastSong));
		if (!streamFile(stream, song))
			return (0);
		if (quit)
			break;
		if (rereadPlaylist) {
			rereadPlaylist = rereadPlaylist_notify = 0;
			if (CFG_MEDIA_PROGRAM == cfg_get_media_type())
				continue;
			log_notice("rereading playlist");
			if (!playlist_reread(&playlist))
				return (0);
			if (cfg_get_media_shuffle())
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

int
ez_shutdown(int exitval)
{
	stream_exit();
	playlist_exit();
	cfg_encoder_exit();
	cfg_decoder_exit();
	log_exit();
	cfg_exit();

	return (exitval);
}

int
main(int argc, char *argv[])
{
	int		 ret, cont;
	const char	*errstr;
	stream_t	 stream;
	extern char	*optarg;
	extern int	 optind;
	struct sigaction act;
	unsigned int	 i;

	ret = 1;
	if (0 > cfg_init() ||
	    0 > cmdline_parse(argc, argv, &ret) ||
	    0 > log_init() ||
	    0 > cfg_decoder_init() ||
	    0 > cfg_encoder_init() ||
	    0 > playlist_init() ||
	    0 > cfg_file_reload() ||
	    0 > stream_init())
		return (ez_shutdown(ret));

	if (0 > cfg_check(&errstr)) {
		log_error("%s: %s", cfg_get_program_config_file(), errstr);
		return (ez_shutdown(2));
	}

	stream = stream_get(STREAM_DEFAULT);
	if (0 > stream_setup(stream))
		return (ez_shutdown(1));

	memset(&act, 0, sizeof(act));
	act.sa_handler = sig_handler;
#ifdef SA_RESTART
	act.sa_flags = SA_RESTART;
#endif
	for (i = 0; i < sizeof(ezstream_signals) / sizeof(int); i++) {
		if (sigaction(ezstream_signals[i], &act, NULL) == -1) {
			log_syserr(ERROR, errno, "sigaction");
			return (ez_shutdown(1));
		}
	}
	/*
	 * Ignore SIGPIPE, which has been seen to give a long-running ezstream
	 * process trouble. EOF and/or EPIPE are also easier to handle.
	 */
#ifndef SIG_IGN
# define SIG_IGN	 (void (*)(int))1
#endif /* !SIG_IGN */
	act.sa_handler = SIG_IGN;
	if (sigaction(SIGPIPE, &act, NULL) == -1) {
		log_syserr(ERROR, errno, "sigaction");
		return (ez_shutdown(1));
	}

	if (0 > util_write_pid_file(cfg_get_program_pid_file()))
		log_syserr(WARNING, errno, cfg_get_program_pid_file());

	if (0 > stream_connect(stream)) {
		log_error("initial server connection failed");
		return (ez_shutdown(1));
	}
	log_notice("connected: %s://%s:%u%s",
	    cfg_get_server_protocol_str(), cfg_get_server_hostname(),
	    cfg_get_server_port(), cfg_get_stream_mountpoint());

	if (CFG_MEDIA_PROGRAM == cfg_get_media_type() ||
	    CFG_MEDIA_PLAYLIST == cfg_get_media_type() ||
	    (CFG_MEDIA_AUTODETECT == cfg_get_media_type() &&
		(util_strrcasecmp(cfg_get_media_filename(), ".m3u") == 0 ||
		    util_strrcasecmp(cfg_get_media_filename(), ".txt") == 0)))
		playlistMode = 1;
	else
		playlistMode = 0;

	do {
		if (playlistMode) {
			cont = streamPlaylist(stream);
		} else {
			cont = streamFile(stream,
			    cfg_get_media_filename());
		}
		if (quit)
			break;
		if (cfg_get_media_stream_once())
			break;
	} while (cont);

	stream_disconnect(stream);

	if (quit) {
		if (cfg_get_program_quiet_stderr() &&
		    cfg_get_program_verbosity())
			printf("\r");
		log_notice("INT or TERM signal received");
	}

	log_info("exiting");

	playlist_free(&playlist);

	return (ez_shutdown(0));
}
