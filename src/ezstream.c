/* example.c: Demonstration of the libshout API. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <shout/shout.h>
#include <getopt.h>
#include "configfile.h"
#ifndef WIN32
#include <libgen.h>
#endif
#include <vorbis/vorbisfile.h>

EZCONFIG	*pezConfig = NULL;
int rereadPlaylist = 0;
static char	*blankString = "";

#ifndef WIN32
#include <signal.h>

void hup_handler(int sig) 
{
	rereadPlaylist = 1;	
	printf("Will reread the playlist on next song\n");
}
#endif
#ifdef WIN32
#define STRNCASECMP strnicmp
#define popen _popen
#else
#define STRNCASECMP strncasecmp
#endif

typedef struct tag_ID3Tag {
	char tag[3];
	char trackName[30];
	char artistName[30];
	char albumName[30];
	char year[3];
	char comment[30];
	char genre;
} ID3Tag;

void usage() {
	fprintf(stdout, "usage: ezstream -h -c ezstream.xml\n");
	fprintf(stdout, "where :\n");
	fprintf(stdout, "	-h = display this help\n");
	fprintf(stdout, "	-c = ezstream config file\n");
	
	exit(1);
}

int strrcmp(char *s, char *sub)
{
	int slen = strlen(s);
	int sublen = strlen(sub);

	if (sublen > slen) {
		return 1;
	}
	return memcmp(s + slen - sublen, sub, sublen);
}


int urlParse(char *url, char *hostname, int *port, char *mountname)
{
	char	*p1;
	char	*p2;
	char	*p3;
	char	tmpPort[25] = "";

	if (strncmp(url, "http://", strlen("http://"))) {
		printf("Invalid URL, must be of the form http://server:port/mountpoint\n");
		return 0;
	}
	p1 = url + strlen("http://");
	p2 = strchr(p1, ':');
	if (!p2) {
		printf("Invalid URL, must be of the form http://server:port/mountpoint\n");
		return 0;
	}
	strncpy(hostname, p1, p2-p1);
	p2++;
	p3 = strchr(p2, '/');
	if (!p3) {
		printf("Invalid URL, must be of the form http://server:port/mountpoint\n");
		return 0;
	}
	memset(tmpPort, '\000', sizeof(tmpPort));
	strncpy(tmpPort, p2, p3-p2);
	*port = atoi(tmpPort);
	strcpy(mountname, p3);
	return 1;
	
}

void ReplaceString(char *source, char *dest, char *from, char *to)
{
	char *p2 = (char *)1;
	char	*p1 = source;
	while (p2) {
		p2 = strstr(p1, from);
		if (p2) {
			strncat(dest, p1, p2-p1);
			strcat(dest, to);
			p1 = p2 + strlen(from);
		}
		else {
			strcat(dest, p1);
		}
	}
}

void setMetadata(shout_t *shout, char *metadata)
{
	shout_metadata_t *shoutMetadata = shout_metadata_new();
	shout_metadata_add(shoutMetadata, "song", metadata); 
	shout_set_metadata(shout, shoutMetadata);
	shout_metadata_free(shoutMetadata);
}

char* buildCommandString(char *extension, char *fileName, char *metadata)
{
	char	*commandString = NULL;
	char *encoder = NULL;
	char *decoder = NULL;
	int	newDecoderLen = 0;
	char *newDecoder = NULL;
	char *newEncoder = NULL;
	int	newEncoderLen = 0;
	int	commandStringLen = 0;

	decoder = strdup(getFormatDecoder(extension));
	if (strlen(decoder) == 0) {
		printf("Unknown extension %s, cannot decode\n", extension);
		return commandString;
	}
	newDecoderLen = strlen(decoder) + strlen(fileName) + 1;
	newDecoder = (char *)malloc(newDecoderLen);
	memset(newDecoder, '\000', newDecoderLen);
	ReplaceString(decoder, newDecoder, "@T@", fileName);

	encoder = strdup(getFormatEncoder(pezConfig->format));
	if (strlen(encoder) == 0) {
		printf("Unknown format %s, passing right on through!\n", pezConfig->format);
		commandStringLen = strlen(newDecoder) + 1;
		commandString = (char *)malloc(commandStringLen);
		memset(commandString, '\000', commandStringLen);
		sprintf(commandString, "%s", newDecoder);
		if (decoder) {
			free(decoder);
		}
		if (encoder) {
			free(encoder);
		}
		return commandString;
	}
	else {

		newEncoderLen = strlen(encoder) + strlen(metadata) + 1;
		newEncoder = (char *)malloc(newEncoderLen);
		memset(newEncoder, '\000', newEncoderLen);
		ReplaceString(encoder, newEncoder, "@M@", metadata);

		commandStringLen = strlen(newDecoder) + strlen(" | ") + strlen(newEncoder) + 1;
		commandString = (char *)malloc(commandStringLen);
		memset(commandString, '\000', commandStringLen);
		sprintf(commandString, "%s | %s", newDecoder, newEncoder);
	}
	if (decoder) {
		free(decoder);
	}
	if (encoder) {
		free(encoder);
	}
	printf("Going to execute (%s)\n", commandString);
	return(commandString);
}

#ifdef WIN32
char *basename(char *fileName) {
	char *pLast = strrchr(fileName, '\\');
	if (pLast) {
		return pLast+1;
	}
	return NULL;
}
#endif
char * processMetadata(shout_t *shout, char *extension, char *fileName) {
	FILE	*filepstream = NULL;
	char	*artist = NULL;
	char	*title = NULL;
	char	*songInfo = NULL;
	int songLen = 0;
	ID3Tag	id3tag;

	filepstream = fopen(fileName, "rb");
	if (filepstream == NULL) {
		printf("Cannot open (%s) - No metadata support.\n", fileName);
		return strdup(blankString);
	}

	if (!strcmp(extension, ".mp3")) {
		/* Look for the ID3 tag */
		if (filepstream) {
			memset(&id3tag, '\000', sizeof(id3tag));
			fseek(filepstream, -128L, SEEK_END);
			fread(&id3tag, 1, 127, filepstream);
			if (!strncmp(id3tag.tag, "TAG", strlen("TAG"))) {
				/* We have an Id3 tag */
				songLen = strlen(id3tag.artistName) + strlen(" - ") + strlen(id3tag.trackName);
				songInfo = (char *)malloc(songLen);
				memset(songInfo, '\000', songLen);

				sprintf(songInfo, "%s - %s", id3tag.artistName, id3tag.trackName);
			}
		}
	}
	if (!strcmp(extension, ".ogg")) {
		OggVorbis_File vf;
		if(ov_open(filepstream, &vf, NULL, 0) < 0) {
			printf("Input does not appear to be an Ogg Vorbis bitstream. No metadata support.\n");
		}
		else {
			char **ptr=ov_comment(&vf,-1)->user_comments;
			while(*ptr){
				if (!STRNCASECMP(*ptr, "ARTIST", strlen("ARTIST"))) {
					artist = (char *)strdup(*ptr + strlen("ARTIST="));
				}
				if (!STRNCASECMP(*ptr, "TITLE", strlen("TITLE"))) {
					title = (char *)strdup(*ptr + strlen("TITLE="));
				}
				++ptr;
			}
			if (artist) {
				songLen = songLen + strlen(artist);
			}
			if (title) {
				songLen = songLen + strlen(title);
			}
			songLen = songLen + strlen(" - ") + 1;
			songInfo = (char *)malloc(songLen);
			memset(songInfo, '\000', songLen);
			if (artist) {
				strcat(songInfo, artist);
				strcat(songInfo, " - ");
				free(artist);
			}
			if (title) {
				strcat(songInfo, title);
				free(title);
			}
			ov_clear(&vf);
			filepstream = NULL;
		}

	}
	if (!songInfo) {
		/* If we didn't get any song info via tags or comments,
		   then lets just use the filename */
		char *p1 = NULL;
		char *p2 = basename(fileName);
		if (p2) {
			songInfo = strdup(p2);
			p1 = strrchr(songInfo, '.');
			if (p1) {
				*p1 = '\000';
			}
		}
	}

	if (songInfo) {
		shout_metadata_t *pmetadata = shout_metadata_new();
		shout_metadata_add(pmetadata, "song", songInfo);
		shout_set_metadata(shout, pmetadata);
		shout_metadata_free(pmetadata);
	}
	else {
		songInfo = strdup(blankString);
	}
	if (filepstream) {
		fclose(filepstream);
	}
	printf("Songinfo is (%s)\n", songInfo);
	return songInfo;
}

FILE *openResource(shout_t *shout, char *fileName)
{
	FILE	*filep = NULL;

	if (!strcmp(fileName, "stdin")) {
#ifdef WIN32
		_setmode(_fileno(stdin), _O_BINARY);
#endif
		filep = stdin;
		return filep;
	}
	else {
		char extension[25];
		char *p1 = NULL;
		char *pMetadata = NULL;
		char *pCommandString = NULL;
		memset(extension, '\000', sizeof(extension));
		p1 = strrchr(fileName, '.');
		if (p1) {
			strncpy(extension, p1, sizeof(extension)-1);
		}

		pMetadata = processMetadata(shout, extension, fileName);
		if (pezConfig->reencode) {
			/* Lets set the metadata first */
			if (strlen(extension) > 0) {
				pCommandString = buildCommandString(extension, fileName, pMetadata);	
				/* Open up the decode/encode loop using popen() */
				filep = popen(pCommandString, "r");
				free(pMetadata);
				free(pCommandString);
				return filep;
			}
			else {
				printf("Cannot determine extension, don't know how to deal with (%s)\n", fileName);
				free(pMetadata);
				return NULL;
			}
			free(pMetadata);

		}
		else {
			filep = fopen(fileName, "rb");
			return filep;
		}
	}
	return NULL;
	
}


int streamFile(shout_t *shout, char *fileName) {
	FILE	*filepstream = NULL;
	char buff[4096];
	long read, ret, total;
	
	
	printf("Streaming %s\n", fileName);
	
	filepstream = openResource(shout, fileName);
	if (!filepstream) {
		printf("Cannot open %s\n", fileName);
		return 0;
	}
	total = 0;
	while (!feof(filepstream)) {
		read = fread(buff, 1, sizeof(buff), filepstream);
		total = total + read;

		if (read > 0) {
			ret = shout_send(shout, buff, read);
			if (ret != SHOUTERR_SUCCESS) {
				printf("DEBUG: Send error: %s\n", shout_get_error(shout));
				break;
			}
			shout_delay(shout);
		} else {
			break;
		}

		shout_sync(shout);
	}
	fclose(filepstream);
	filepstream = NULL;
	return 1;
}
int streamPlaylist(shout_t *shout, char *fileName) {
	FILE	*filep = NULL;
	char	streamFileName[8096] = "";
	char	lastStreamFileName[8096] = "";
	int		loop = 1;

	filep = fopen(fileName, "r");
	if (filep == 0) {
		printf("Cannot open %s\n", fileName);
		return(0);
	}
	while (loop) {
		while (!feof(filep)) {
			memset(streamFileName, '\000', sizeof(streamFileName));
			fgets(streamFileName, sizeof(streamFileName), filep);
			streamFileName[strlen(streamFileName)-1] = '\000';
			if (strlen(streamFileName) > 0) {
				memset(lastStreamFileName, '\000', sizeof(lastStreamFileName));
				strcpy(lastStreamFileName, streamFileName);
				/* Skip entries that begin with a # */
				if (strncmp(streamFileName, "#", 1)) {
					streamFile(shout, streamFileName);
				}
			}
			if (rereadPlaylist) {
				rereadPlaylist = 0;
				fclose(filep);
				printf("Reopening playlist\n");
				filep = fopen(fileName, "r");
				if (filep == 0) {
					printf("Cannot open %s\n", fileName);
					return(0);
				}
				else {
					int loop2 = 1;
					printf("Repositioning to (%s)\n", lastStreamFileName);
					while (loop2) {
						/* If we reach the end before finding
						   our last spot, we will start over at the
						   beginning */
						if (feof(filep)) {
							loop2 = 0;
						}
						else {
							memset(streamFileName, '\000', sizeof(streamFileName));
							fgets(streamFileName, sizeof(streamFileName), filep);
							streamFileName[strlen(streamFileName)-1] = '\000';
							if (!strcmp(streamFileName, lastStreamFileName)) {
							/* If we found our last position, then bump out of the loop */
								loop2 = 0;
							}
						}
					}

				}
			}
		}
		rewind(filep);
	}
	return(1);
}

int main(int argc, char **argv)
{
	char	c;
	char	*configFile = NULL;
	char	*host = NULL;
	int		port = 0;
	char	*mount = NULL;
	shout_t *shout;


	pezConfig = getEZConfig();
#ifndef WIN32
	signal(SIGHUP, hup_handler);
#endif

	shout_init();

	while ((c = getopt(argc, argv, "hc:")) != -1) {
		switch (c) {
			case 'c':
				configFile = optarg;
				break;
			case 'h':
				usage();
				break;
			default:
				break;
		}
	}

	if (!configFile) {
		printf("You must supply a config file\n");
		usage();
	}
	else {
		parseConfig(configFile);
	}

	if (pezConfig->URL) {
		host = (char *)malloc(strlen(pezConfig->URL) +1);
		memset(host, '\000', strlen(pezConfig->URL) +1);
		mount = (char *)malloc(strlen(pezConfig->URL) +1);
		memset(mount, '\000', strlen(pezConfig->URL) +1);
		if (!urlParse(pezConfig->URL, host, &port, mount)) {
			exit(0);
		}
	}
	if ((host == NULL)) {
		printf("server is required\n");
		usage();
	}
	if ((port == 0)) {
		printf("port is required\n");
		usage();
	}
	if ((pezConfig->password == NULL)) {
		printf("-p password is required\n");
		usage();
	}
	if ((mount == NULL)) {
		printf("mountpoint is required\n");
		usage();
	}
	if ((pezConfig->fileName == NULL)) {
		printf("-f fileName is required\n");
		usage();
	}
	if (pezConfig->format == 0) {
		printf("You must specify a format type of MP3, VORBIS, or THEORA\n");
	}
	if (!(shout = shout_new())) {
		printf("Could not allocate shout_t\n");
		return 1;
	}
	

	if (shout_set_host(shout, host) != SHOUTERR_SUCCESS) {
		printf("Error setting hostname: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_protocol(shout, SHOUT_PROTOCOL_HTTP) != SHOUTERR_SUCCESS) {
		printf("Error setting protocol: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_port(shout, port) != SHOUTERR_SUCCESS) {
		printf("Error setting port: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_password(shout, pezConfig->password) != SHOUTERR_SUCCESS) {
		printf("Error setting password: %s\n", shout_get_error(shout));
		return 1;
	}
	if (shout_set_mount(shout, mount) != SHOUTERR_SUCCESS) {
		printf("Error setting mount: %s\n", shout_get_error(shout));
		return 1;
	}

	if (shout_set_user(shout, "source") != SHOUTERR_SUCCESS) {
		printf("Error setting user: %s\n", shout_get_error(shout));
		return 1;
	}

	if (!strcmp(pezConfig->format, MP3_FORMAT)) {
		if (shout_set_format(shout, SHOUT_FORMAT_MP3) != SHOUTERR_SUCCESS) {
			printf("Error setting user: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (!strcmp(pezConfig->format, VORBIS_FORMAT)) {
		if (shout_set_format(shout, SHOUT_FORMAT_OGG) != SHOUTERR_SUCCESS) {
			printf("Error setting user: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (!strcmp(pezConfig->format, THEORA_FORMAT)) {
		if (shout_set_format(shout, SHOUT_FORMAT_OGG) != SHOUTERR_SUCCESS) {
			printf("Error setting user: %s\n", shout_get_error(shout));
			return 1;
		}
	}

	if (pezConfig->serverName) {
		if (shout_set_name(shout, pezConfig->serverName) != SHOUTERR_SUCCESS) {
			printf("Error setting server name: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (pezConfig->serverURL) {
		if (shout_set_url(shout, pezConfig->serverURL) != SHOUTERR_SUCCESS) {
			printf("Error setting server url: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (pezConfig->serverGenre) {
		if (shout_set_genre(shout, pezConfig->serverGenre) != SHOUTERR_SUCCESS) {
			printf("Error setting server genre: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (pezConfig->serverDescription) {
		if (shout_set_description(shout, pezConfig->serverDescription) != SHOUTERR_SUCCESS) {
			printf("Error setting server description: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (pezConfig->serverBitrate) {
		if (shout_set_audio_info(shout, SHOUT_AI_BITRATE, pezConfig->serverBitrate) != SHOUTERR_SUCCESS) {
			printf("Error setting server bitrate: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (pezConfig->serverChannels) {
		if (shout_set_audio_info(shout, SHOUT_AI_CHANNELS, pezConfig->serverChannels) != SHOUTERR_SUCCESS) {
			printf("Error setting server channels: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (pezConfig->serverSamplerate) {
		if (shout_set_audio_info(shout, SHOUT_AI_SAMPLERATE, pezConfig->serverSamplerate) != SHOUTERR_SUCCESS) {
			printf("Error setting server samplerate: %s\n", shout_get_error(shout));
			return 1;
		}
	}
	if (pezConfig->serverQuality) {
		if (shout_set_audio_info(shout, SHOUT_AI_QUALITY, pezConfig->serverQuality) != SHOUTERR_SUCCESS) {
			printf("Error setting server quality: %s\n", shout_get_error(shout));
			return 1;
		}
	}

	if (shout_set_public(shout, pezConfig->serverPublic) != SHOUTERR_SUCCESS) {
		printf("Error setting server public flag: %s\n", shout_get_error(shout));
		return 1;
	}

	printf("Connecting to %s...", pezConfig->URL);
	if (shout_open(shout) == SHOUTERR_SUCCESS) {
		printf("SUCCESS.\n");
		while (1) {
			if (!strrcmp(pezConfig->fileName, ".m3u")) {
				streamPlaylist(shout, pezConfig->fileName);
			}
			else {
				streamFile(shout, pezConfig->fileName);
			}
		}
	} else {
		printf("FAILED: %s\n", shout_get_error(shout));
	}

	shout_close(shout);

	shout_shutdown();

	if (host) {
		free(host);
	}
	if (mount) {
		free(mount);
	}

	return 0;
}
