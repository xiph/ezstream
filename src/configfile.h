#ifndef __EZSTREAM_CONFIG_H__
#define __EZSTREAM_CONFIG_H__

#include <libxml/parser.h>


#define MP3_FORMAT "MP3"
#define VORBIS_FORMAT "VORBIS"
#define THEORA_FORMAT "THEORA"

#define MAX_FORMAT_ENCDEC	15

typedef struct tag_FORMAT_ENCDEC {
	char	*format;
	char	*match;
	char	*encoder;
	char	*decoder;
} FORMAT_ENCDEC;

typedef struct tag_EZCONFIG {
	char	*URL;
	char	*password;
	char	*format;
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
	int	reencode;
	FORMAT_ENCDEC	*encoderDecoders[MAX_FORMAT_ENCDEC];
	int	numEncoderDecoders;
} EZCONFIG;



void printConfig();
int parseConfig(char *fileName);
EZCONFIG *getEZConfig();
char*   getFormatEncoder(char *format);
char*   getFormatDecoder(char *match);
char*   getMetadataGrabber(char *match);

#endif
