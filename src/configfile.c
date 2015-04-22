/*
 *  ezstream - source client for Icecast with external en-/decoder support
 *  Copyright (C) 2003, 2004, 2005, 2006  Ed Zaleski <oddsock@oddsock.org>
 *  Copyright (C) 2007, 2009, 2015        Moritz Grimm <mgrimm@mrsserver.net>
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

#include <libxml/parser.h>

#include "configfile.h"
#include "log.h"
#include "util.h"
#include "xalloc.h"

static EZCONFIG		 ezConfig;
static const char	*blankString = "";

unsigned int	checkDecoderLine(const char *, const char *, long);
unsigned int	checkEncoderLine(const char *, const char *, long);
unsigned int	checkFormatLine(const char *, const char *, long);

EZCONFIG *
getEZConfig(void)
{
	return (&ezConfig);
}

const char *
getFormatEncoder(const char *format)
{
	int	i;

	for (i = 0; i < ezConfig.numEncoderDecoders; i++) {
		if (ezConfig.encoderDecoders[i] != NULL &&
		    ezConfig.encoderDecoders[i]->format != NULL &&
		    strcmp(ezConfig.encoderDecoders[i]->format, format) == 0) {
			if (ezConfig.encoderDecoders[i]->encoder != NULL)
				return (ezConfig.encoderDecoders[i]->encoder);
			else
				return (blankString);
		}
	}

	return (blankString);
}

const char *
getFormatDecoder(const char *match)
{
	int	i;

	for (i = 0; i < ezConfig.numEncoderDecoders; i++) {
		if (ezConfig.encoderDecoders[i] != NULL &&
		    ezConfig.encoderDecoders[i]->match != NULL &&
		    strcmp(ezConfig.encoderDecoders[i]->match, match) == 0) {
			if (ezConfig.encoderDecoders[i]->decoder != NULL)
				return (ezConfig.encoderDecoders[i]->decoder);
			else
				return (blankString);
		}
	}

	return (blankString);
}

#define CFGERROR_TOO_MANY(x) \
	do { \
		log_error("%s[%ld]: more than one <%s> element", \
		    fileName, xmlGetLineNo(cur), (x)); \
		config_error++; \
	} while (0)

int
parseConfig(const char *fileName)
{
	xmlDocPtr	 doc;
	xmlNodePtr	 cur;
	char		*ls_xmlContentPtr;
	int		 program_set, reconnect_set, shuffle_set,
			 streamOnce_set, svrinfopublic_set,
			 refresh_set;
	unsigned int	 config_error;

	xmlLineNumbersDefault(1);
	if ((doc = xmlParseFile(fileName)) == NULL) {
		log_error("%s: not well-formed", fileName);
		return (0);
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		log_error("%s: empty document", fileName);
		xmlFreeDoc(doc);
		return (0);
	}

	memset(&ezConfig, 0, sizeof(ezConfig));
	ezConfig.metadataRefreshInterval = -1;

	config_error = 0;
	program_set = 0;
	reconnect_set = 0;
	refresh_set = 0;
	shuffle_set = 0;
	streamOnce_set = 0;
	svrinfopublic_set = 0;

	for (cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next) {
		if (!xmlStrcmp(cur->name, (const xmlChar *)"url")) {
			if (ezConfig.URL != NULL) {
				CFGERROR_TOO_MANY("url");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.URL = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"sourceuser")) {
			if (ezConfig.username != NULL) {
				CFGERROR_TOO_MANY("sourceuser");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.username = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"sourcepassword")) {
			if (ezConfig.password != NULL) {
				CFGERROR_TOO_MANY("sourcepassword");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.password = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"format")) {
			if (ezConfig.format != NULL) {
				CFGERROR_TOO_MANY("format");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				char *p;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.format = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
				for (p = ezConfig.format; *p != '\0'; p++)
					*p = toupper((int)*p);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"filename")) {
			if (ezConfig.fileName != NULL) {
				CFGERROR_TOO_MANY("filename");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (strlen(ls_xmlContentPtr) > PATH_MAX - 1) {
					log_error("%s[%ld]: path or filename in <filename> too long",
					    fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				ezConfig.fileName = UTF8toCHAR(ls_xmlContentPtr, ICONV_REPLACE);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"metadata_progname")) {
			if (ezConfig.metadataProgram != NULL) {
				CFGERROR_TOO_MANY("metadata_progname");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (strlen(ls_xmlContentPtr) > PATH_MAX - 1) {
					log_error("%s[%ld]: path or filename in <metadata_progname> too long",
					    fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				ezConfig.metadataProgram = UTF8toCHAR(ls_xmlContentPtr, ICONV_REPLACE);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"metadata_format")) {
			if (ezConfig.metadataFormat != NULL) {
				CFGERROR_TOO_MANY("metadata_format");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				unsigned int	ret;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.metadataFormat = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
				if ((ret = checkFormatLine(ezConfig.metadataFormat, fileName, xmlGetLineNo(cur)))
				    > 0) {
					config_error += ret;
					continue;
				}
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"metadata_refreshinterval")) {
			if (refresh_set) {
				CFGERROR_TOO_MANY("metadata_refreshinterval");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.metadataRefreshInterval = (int)strtonum(ls_xmlContentPtr, -1LL, (long long)INT_MAX, &errstr);
				if (errstr) {
					log_error("%s[%ld]: <metadata_refreshinterval>: %s: %s",
					    fileName, xmlGetLineNo(cur), ls_xmlContentPtr, errstr);
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				refresh_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"playlist_program")) {
			if (program_set) {
				CFGERROR_TOO_MANY("playlist_program");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.fileNameIsProgram = (int)strtonum(ls_xmlContentPtr, 0LL, 1LL, &errstr);
				if (errstr) {
					log_error("%s[%ld]: <playlist_program> may only contain 1 or 0",
					    fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				program_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"shuffle")) {
			if (shuffle_set) {
				CFGERROR_TOO_MANY("shuffle");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.shuffle = (int)strtonum(ls_xmlContentPtr, 0LL, 1LL, &errstr);
				if (errstr) {
					log_error("%s[%ld]: <shuffle> may only contain 1 or 0",
					    fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				shuffle_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"stream_once")) {
			if (streamOnce_set) {
				CFGERROR_TOO_MANY("stream_once");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.streamOnce = (int)strtonum(ls_xmlContentPtr, 0LL, 1LL, &errstr);
				if (errstr) {
					log_error("%s[%ld]: <stream_once> may only contain 1 or 0",
					    fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				streamOnce_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"reconnect_tries")) {
			if (reconnect_set) {
				CFGERROR_TOO_MANY("reconnect_tries");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.reconnectAttempts = (unsigned int)strtonum(ls_xmlContentPtr, 0LL, (long long)UINT_MAX, &errstr);
				if (errstr) {
					log_error("%s[%ld]: <reconnect_tries>: %s: %s",
					    fileName, xmlGetLineNo(cur), ls_xmlContentPtr, errstr);
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				reconnect_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfoname")) {
			if (ezConfig.serverName != NULL) {
				CFGERROR_TOO_MANY("svrinfoname");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverName = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfourl")) {
			if (ezConfig.serverURL != NULL) {
				CFGERROR_TOO_MANY("svrinfourl");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverURL = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfogenre")) {
			if (ezConfig.serverGenre != NULL) {
				CFGERROR_TOO_MANY("svrinfogenre");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverGenre = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfodescription")) {
			if (ezConfig.serverDescription != NULL) {
				CFGERROR_TOO_MANY("svrinfodescription");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverDescription = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfobitrate")) {
			if (ezConfig.serverBitrate != NULL) {
				CFGERROR_TOO_MANY("svrinfobitrate");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverBitrate = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}

		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfochannels")) {
			if (ezConfig.serverChannels != NULL) {
				CFGERROR_TOO_MANY("svrinfochannels");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverChannels = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfosamplerate")) {
			if (ezConfig.serverSamplerate != NULL) {
				CFGERROR_TOO_MANY("svrinfosamplerate");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverSamplerate = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfoquality")) {
			if (ezConfig.serverQuality != NULL) {
				CFGERROR_TOO_MANY("svrinfoquality");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverQuality = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfopublic")) {
			if (svrinfopublic_set) {
				CFGERROR_TOO_MANY("svrinfopublic");
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverPublic = (int)strtonum(ls_xmlContentPtr, 0LL, 1LL, &errstr);
				if (errstr) {
					log_error("%s[%ld]: <svrinfopublic> may only contain 1 or 0",
					    fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				svrinfopublic_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"reencode")) {
			xmlNodePtr	cur2;
			int		enable_set;

			enable_set = 0;
			for (cur2 = cur->xmlChildrenNode; cur2 != NULL;
			     cur2 = cur2->next) {
				if (!xmlStrcmp(cur2->name, (const xmlChar *)"enable")) {
					if (enable_set) {
						CFGERROR_TOO_MANY("enable");
						continue;
					}
					if (cur2->xmlChildrenNode != NULL) {
						const char *errstr;

						ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
						ezConfig.reencode = (int)strtonum(ls_xmlContentPtr, 0LL, 1LL, &errstr);
						if (errstr) {
							log_error("%s[%ld]: <enable> may only contain 1 or 0",
							    fileName, xmlGetLineNo(cur));
							config_error++;
							continue;
						}
						xmlFree(ls_xmlContentPtr);
						enable_set = 1;
					}
				}
				if (!xmlStrcmp(cur2->name, (const xmlChar *)"encdec")) {
					xmlNodePtr	 cur3;
					FORMAT_ENCDEC	*pformatEncDec;

					pformatEncDec = xcalloc(1UL, sizeof(FORMAT_ENCDEC));

					for (cur3 = cur2->xmlChildrenNode;
					     cur3 != NULL; cur3 = cur3->next) {
						if (!xmlStrcmp(cur3->name, (const xmlChar *)"format")) {
							if (pformatEncDec->format != NULL) {
								CFGERROR_TOO_MANY("format");
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								char *p;

								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->format = xstrdup(ls_xmlContentPtr);
								xmlFree(ls_xmlContentPtr);
								for (p = pformatEncDec->format; *p != '\0'; p++)
									*p = toupper((int)*p);
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *)"match")) {
							if (pformatEncDec->match != NULL) {
								CFGERROR_TOO_MANY("match");
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								char *p;

								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->match = xstrdup(ls_xmlContentPtr);
								xmlFree(ls_xmlContentPtr);
								for (p = pformatEncDec->match; *p != '\0'; p++)
									*p = tolower((int)*p);
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *)"decode")) {
							if (pformatEncDec->decoder != NULL) {
								CFGERROR_TOO_MANY("decode");
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								unsigned int	ret;

								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->decoder = UTF8toCHAR(ls_xmlContentPtr, ICONV_REPLACE);
								xmlFree(ls_xmlContentPtr);
								if ((ret = checkDecoderLine(pformatEncDec->decoder, fileName, xmlGetLineNo(cur3)))
								    > 0) {
									config_error += ret;
									continue;
								}
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *)"encode")) {
							if (pformatEncDec->encoder != NULL) {
								CFGERROR_TOO_MANY("encode");
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								unsigned int	ret;

								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->encoder = UTF8toCHAR(ls_xmlContentPtr, ICONV_REPLACE);
								xmlFree(ls_xmlContentPtr);
								if ((ret = checkEncoderLine(pformatEncDec->encoder, fileName, xmlGetLineNo(cur3)))
								    > 0) {
									config_error += ret;
									continue;
								}
							}
						}
					}
					ezConfig.encoderDecoders[ezConfig.numEncoderDecoders] = pformatEncDec;
					ezConfig.numEncoderDecoders++;
				}
			}
		}
	}

	xmlFreeDoc(doc);

	if (config_error == 0)
		return (1);

	freeConfig(&ezConfig);
	log_error("%s: %u configuration error(s)", fileName, config_error);

	return (0);
}

void
freeConfig(EZCONFIG *cfg)
{
	unsigned int	i;

	if (cfg == NULL)
		return;

	if (cfg->URL != NULL)
		xfree(cfg->URL);
	if (cfg->password != NULL)
		xfree(cfg->password);
	if (cfg->format != NULL)
		xfree(cfg->format);
	if (cfg->fileName != NULL)
		xfree(cfg->fileName);
	if (cfg->metadataProgram != NULL)
		xfree(cfg->metadataProgram);
	if (cfg->metadataFormat != NULL)
		xfree(cfg->metadataFormat);
	if (cfg->serverName != NULL)
		xfree(cfg->serverName);
	if (cfg->serverURL != NULL)
		xfree(cfg->serverURL);
	if (cfg->serverGenre != NULL)
		xfree(cfg->serverGenre);
	if (cfg->serverDescription != NULL)
		xfree(cfg->serverDescription);
	if (cfg->serverBitrate != NULL)
		xfree(cfg->serverBitrate);
	if (cfg->serverChannels != NULL)
		xfree(cfg->serverChannels);
	if (cfg->serverSamplerate != NULL)
		xfree(cfg->serverSamplerate);
	if (cfg->serverQuality != NULL)
		xfree(cfg->serverQuality);
	if (cfg->encoderDecoders != NULL) {
		for (i = 0; i < MAX_FORMAT_ENCDEC; i++) {
			if (cfg->encoderDecoders[i] != NULL) {
				if (cfg->encoderDecoders[i]->format != NULL)
					xfree(cfg->encoderDecoders[i]->format);
				if (cfg->encoderDecoders[i]->match != NULL)
					xfree(cfg->encoderDecoders[i]->match);
				if (cfg->encoderDecoders[i]->encoder != NULL)
					xfree(cfg->encoderDecoders[i]->encoder);
				if (cfg->encoderDecoders[i]->decoder != NULL)
					xfree(cfg->encoderDecoders[i]->decoder);
				xfree(cfg->encoderDecoders[i]);
			}
		}
	}

	memset(cfg, 0, sizeof(EZCONFIG));
}

unsigned int
checkDecoderLine(const char *str, const char *file, long line)
{
	unsigned int	  errors;
	char		 *p;
	int		  have_track = 0;

	errors = 0;
	if ((p = strstr(str, STRING_PLACEHOLDER)) != NULL) {
		log_error("%s[%ld]: '%s' placeholder not allowed in decoder command",
		    file, line, STRING_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, TRACK_PLACEHOLDER)) != NULL) {
		p += strlen(TRACK_PLACEHOLDER);
		if ((p = strstr(p, TRACK_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in decoder command",
			    file, line, TRACK_PLACEHOLDER);
			errors++;
		} else
			have_track = 1;
	}
	if ((p = strstr(str, METADATA_PLACEHOLDER)) != NULL) {
		p += strlen(METADATA_PLACEHOLDER);
		if ((p = strstr(p, METADATA_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in decoder command",
			    file, line, METADATA_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, ARTIST_PLACEHOLDER)) != NULL) {
		p += strlen(ARTIST_PLACEHOLDER);
		if ((p = strstr(p, ARTIST_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in decoder command",
			    file, line, ARTIST_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, TITLE_PLACEHOLDER)) != NULL) {
		p += strlen(TITLE_PLACEHOLDER);
		if ((p = strstr(p, TITLE_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in decoder command",
			    file, line, TITLE_PLACEHOLDER);
			errors++;
		}
	}

	if (!have_track) {
		log_error("%s[%ld]: decoder command requires '%s' track placeholder",
		    file, line, TRACK_PLACEHOLDER);
		errors++;
	}

	return (errors);
}

unsigned int
checkEncoderLine(const char *str, const char *file, long line)
{
	unsigned int	   errors;
	char		  *p;

	errors = 0;
	if ((p = strstr(str, TRACK_PLACEHOLDER)) != NULL) {
		log_error("%s[%ld]: '%s' placeholder not allowed in encoder command",
		    file, line, TRACK_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, STRING_PLACEHOLDER)) != NULL) {
		log_error("%s[%ld]: '%s' placeholder not allowed in encoder command",
		    file, line, STRING_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, METADATA_PLACEHOLDER)) != NULL) {
		p += strlen(METADATA_PLACEHOLDER);
		if ((p = strstr(p, METADATA_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in encoder command",
			    file, line, METADATA_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, ARTIST_PLACEHOLDER)) != NULL) {
		p += strlen(ARTIST_PLACEHOLDER);
		if ((p = strstr(p, ARTIST_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in encoder command",
			    file, line, ARTIST_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, TITLE_PLACEHOLDER)) != NULL) {
		p += strlen(TITLE_PLACEHOLDER);
		if ((p = strstr(p, TITLE_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in encoder command",
			    file, line, TITLE_PLACEHOLDER);
			errors++;
		}
	}

	return (errors);
}

unsigned int
checkFormatLine(const char *str, const char *file, long line)
{
	unsigned int	   errors;
	char		  *p;

	errors = 0;
	if ((p = strstr(str, METADATA_PLACEHOLDER)) != NULL) {
		log_error("%s[%ld]: '%s' placeholder not allowed in <metadata_format>",
		    file, line, METADATA_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, TRACK_PLACEHOLDER)) != NULL) {
		p += strlen(TRACK_PLACEHOLDER);
		if ((p = strstr(p, TRACK_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in <metadata_format>",
			    file, line, TRACK_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, STRING_PLACEHOLDER)) != NULL) {
		p += strlen(STRING_PLACEHOLDER);
		if ((p = strstr(p, STRING_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in <metadata_format>",
			    file, line, STRING_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, ARTIST_PLACEHOLDER)) != NULL) {
		p += strlen(ARTIST_PLACEHOLDER);
		if ((p = strstr(p, ARTIST_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in <metadata_format>",
			    file, line, ARTIST_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, TITLE_PLACEHOLDER)) != NULL) {
		p += strlen(TITLE_PLACEHOLDER);
		if ((p = strstr(p, TITLE_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: more than one '%s' placeholder in <metadata_format>",
			    file, line, TITLE_PLACEHOLDER);
			errors++;
		}
	}

	return (errors);
}
