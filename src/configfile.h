#ifndef __EZSTREAM_CONFIG_H__
#define __EZSTREAM_CONFIG_H__

#include <libxml/parser.h>


#define MP3_FORMAT 1
#define OGG_FORMAT 2

typedef struct tag_EZCONFIG {
	char	*URL;
	char	*password;
	int		format;
	char	*fileName;
	char	*serverName;
	char	*serverURL;
	char	*serverGenre;
	char	*serverDescription;
	char	*serverBitrate;
	char	*serverChannels;
	char	*serverSamplerate;
	char	*serverQuality;
	int		serverPublic;
} EZCONFIG;

void printConfig();
int parseConfig(char *fileName);
EZCONFIG *getEZConfig();
#endif
