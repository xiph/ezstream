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
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "compat.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ezconfig0.h"
#include "log.h"
#include "util.h"

#include <libxml/parser.h>

#define XML_CHAR(s)	(const xmlChar *)(s)
#define STD_CHAR(s)	(const char *)(s)

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

int
parseConfig(const char *fileName)
{
	xmlDocPtr	 doc;
	xmlNodePtr	 cur;
	xmlChar 	*ls_xmlContentPtr;
	int		 program_set, reconnect_set, shuffle_set,
			 streamOnce_set, svrinfopublic_set,
			 refresh_set;
	unsigned int	 config_error;

	xmlLineNumbersDefault(1);
	if ((doc = xmlParseFile(fileName)) == NULL) {
		log_error("%s: Parse error (not well-formed XML.)\n", fileName);
		return (0);
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		log_error("%s: Parse error (empty XML document.)\n", fileName);
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
				log_error("%s[%ld]: Error: Cannot have multiple <url> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.URL = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"sourceuser")) {
			if (ezConfig.username != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <sourceuser> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.username = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"sourcepassword")) {
			if (ezConfig.password != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <sourcepassword> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.password = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"format")) {
			if (ezConfig.format != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <format> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				char *p;

				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.format = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
				for (p = ezConfig.format; *p != '\0'; p++)
					*p = (char)toupper((int)*p);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"filename")) {
			if (ezConfig.fileName != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <filename> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (strlen(STD_CHAR(ls_xmlContentPtr)) > PATH_MAX - 1) {
					log_error("%s[%ld]: Error: Path or filename in <filename> is too long\n",
					    fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				ezConfig.fileName = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"metadata_progname")) {
			if (ezConfig.metadataProgram != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <metadata_progname> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (strlen(STD_CHAR(ls_xmlContentPtr)) > PATH_MAX - 1) {
					log_error("%s[%ld]: Error: Path or filename in <metadata_progname> is too long\n",
					    fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				ezConfig.metadataProgram = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"metadata_format")) {
			if (ezConfig.metadataFormat != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <metadata_format> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				unsigned int	ret;

				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.metadataFormat = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
				if ((ret = checkFormatLine(ezConfig.metadataFormat,
							   fileName, xmlGetLineNo(cur)))
				    > 0) {
					config_error += ret;
					continue;
				}
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"metadata_refreshinterval")) {
			if (refresh_set) {
				log_error("%s[%ld]: Error: Cannot have multiple <metadata_refreshinterval> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.metadataRefreshInterval = (int)strtonum(STD_CHAR(ls_xmlContentPtr), -1LL, (long long)INT_MAX, &errstr);
				if (errstr) {
					log_error("%s[%ld]: Error: In <metadata_refreshinterval>: '%s' is %s\n",
					    fileName, xmlGetLineNo(cur), STD_CHAR(ls_xmlContentPtr), errstr);
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				refresh_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"playlist_program")) {
			if (program_set) {
				log_error("%s[%ld]: Error: Cannot have multiple <playlist_program> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.fileNameIsProgram = (int)strtonum(STD_CHAR(ls_xmlContentPtr), 0LL, 1LL, &errstr);
				if (errstr) {
					log_error("%s[%ld]: Error: <playlist_program> may only contain 1 or 0\n",
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
				log_error("%s[%ld]: Error: Cannot have multiple <shuffle> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.shuffle = (int)strtonum(STD_CHAR(ls_xmlContentPtr), 0LL, 1LL, &errstr);
				if (errstr) {
					log_error("%s[%ld]: Error: <shuffle> may only contain 1 or 0\n",
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
				log_error("%s[%ld]: Error: Cannot have multiple <stream_once> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.streamOnce = (int)strtonum(STD_CHAR(ls_xmlContentPtr), 0LL, 1LL, &errstr);
				if (errstr) {
					log_error("%s[%ld]: Error: <stream_once> may only contain 1 or 0\n",
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
				log_error("%s[%ld]: Error: Cannot have multiple <reconnect_tries> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.reconnectAttempts = (unsigned int)strtonum(STD_CHAR(ls_xmlContentPtr), 0LL, (long long)UINT_MAX, &errstr);
				if (errstr) {
					log_error("%s[%ld]: Error: In <reconnect_tries>: '%s' is %s\n",
					    fileName, xmlGetLineNo(cur), STD_CHAR(ls_xmlContentPtr), errstr);
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				reconnect_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfoname")) {
			if (ezConfig.serverName != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfoname> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverName = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfourl")) {
			if (ezConfig.serverURL != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfourl> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverURL = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfogenre")) {
			if (ezConfig.serverGenre != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfogenre> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverGenre = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfodescription")) {
			if (ezConfig.serverDescription != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfodescription> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverDescription = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfobitrate")) {
			if (ezConfig.serverBitrate != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfobitrate> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverBitrate = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}

		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfochannels")) {
			if (ezConfig.serverChannels != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfochannels> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverChannels = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfosamplerate")) {
			if (ezConfig.serverSamplerate != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfosamplerate> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverSamplerate = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfoquality")) {
			if (ezConfig.serverQuality != NULL) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfoquality> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverQuality = strdup(STD_CHAR(ls_xmlContentPtr));
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *)"svrinfopublic")) {
			if (svrinfopublic_set) {
				log_error("%s[%ld]: Error: Cannot have multiple <svrinfopublic> elements\n",
				    fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverPublic = (int)strtonum(STD_CHAR(ls_xmlContentPtr), 0LL, 1LL, &errstr);
				if (errstr) {
					log_error("%s[%ld]: Error: <svrinfopublic> may only contain 1 or 0\n",
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
						log_error("%s[%ld]: Error: Cannot have multiple <enable> elements\n",
						    fileName, xmlGetLineNo(cur));
						config_error++;
						continue;
					}
					if (cur2->xmlChildrenNode != NULL) {
						const char *errstr;

						ls_xmlContentPtr = xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
						ezConfig.reencode = (int)strtonum(STD_CHAR(ls_xmlContentPtr), 0LL, 1LL, &errstr);
						if (errstr) {
							log_error("%s[%ld]: Error: <enable> may only contain 1 or 0\n",
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

					pformatEncDec = calloc(1UL, sizeof(FORMAT_ENCDEC));

					for (cur3 = cur2->xmlChildrenNode;
					     cur3 != NULL; cur3 = cur3->next) {
						if (!xmlStrcmp(cur3->name, (const xmlChar *)"format")) {
							if (pformatEncDec->format != NULL) {
								log_error("%s[%ld]: Error: Cannot have multiple <format> elements\n",
								    fileName, xmlGetLineNo(cur3));
								config_error++;
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								char *p;

								ls_xmlContentPtr = xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->format = strdup(STD_CHAR(ls_xmlContentPtr));
								xmlFree(ls_xmlContentPtr);
								for (p = pformatEncDec->format; *p != '\0'; p++)
									*p = (char)toupper((int)*p);
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *)"match")) {
							if (pformatEncDec->match != NULL) {
								log_error("%s[%ld]: Error: Cannot have multiple <match> elements\n",
								    fileName, xmlGetLineNo(cur3));
								config_error++;
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								char *p;

								ls_xmlContentPtr = xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->match = strdup(STD_CHAR(ls_xmlContentPtr));
								xmlFree(ls_xmlContentPtr);
								for (p = pformatEncDec->match; *p != '\0'; p++)
									*p = (char)tolower((int)*p);
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *)"decode")) {
							if (pformatEncDec->decoder != NULL) {
								log_error("%s[%ld]: Error: Cannot have multiple <decode> elements\n",
								    fileName, xmlGetLineNo(cur3));
								config_error++;
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								unsigned int	ret;

								ls_xmlContentPtr = xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->decoder = strdup(STD_CHAR(ls_xmlContentPtr));
								xmlFree(ls_xmlContentPtr);
								if ((ret = checkDecoderLine(pformatEncDec->decoder,
											    fileName, xmlGetLineNo(cur3)))
								    > 0) {
									config_error += ret;
									continue;
								}
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *)"encode")) {
							if (pformatEncDec->encoder != NULL) {
								log_error("%s[%ld]: Error: Cannot have multiple <encode> elements\n",
								    fileName, xmlGetLineNo(cur3));
								config_error++;
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								unsigned int	ret;

								ls_xmlContentPtr = xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->encoder = strdup(STD_CHAR(ls_xmlContentPtr));
								xmlFree(ls_xmlContentPtr);
								if ((ret = checkEncoderLine(pformatEncDec->encoder,
											    fileName, xmlGetLineNo(cur3)))
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
	log_error("%u configuration error(s) in %s\n",
	    config_error, fileName);

	return (0);
}

void
freeConfig(EZCONFIG *cfg)
{
	unsigned int	i;

	if (cfg == NULL)
		return;

	free(cfg->URL);
	free(cfg->password);
	free(cfg->format);
	free(cfg->fileName);
	free(cfg->metadataProgram);
	free(cfg->metadataFormat);
	free(cfg->serverName);
	free(cfg->serverURL);
	free(cfg->serverGenre);
	free(cfg->serverDescription);
	free(cfg->serverBitrate);
	free(cfg->serverChannels);
	free(cfg->serverSamplerate);
	free(cfg->serverQuality);
	for (i = 0; i < MAX_FORMAT_ENCDEC; i++) {
		if (NULL == cfg->encoderDecoders[i])
			continue;
		free(cfg->encoderDecoders[i]->format);
		free(cfg->encoderDecoders[i]->match);
		free(cfg->encoderDecoders[i]->encoder);
		free(cfg->encoderDecoders[i]->decoder);
		free(cfg->encoderDecoders[i]);
	}

	memset(cfg, 0, sizeof(*cfg));
}

unsigned int
checkDecoderLine(const char *str, const char *file, long line)
{
	unsigned int	  errors;
	char		 *p;
	int		  have_track = 0;

	errors = 0;
	if ((p = strstr(str, STRING_PLACEHOLDER)) != NULL) {
		log_error("%s[%ld]: Error: `%s' placeholder not allowed in decoder command\n",
		    file, line, STRING_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, TRACK_PLACEHOLDER)) != NULL) {
		p += strlen(TRACK_PLACEHOLDER);
		if ((p = strstr(p, TRACK_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
			    file, line, TRACK_PLACEHOLDER);
			errors++;
		} else
			have_track = 1;
	}
	if ((p = strstr(str, START_TIMESTAMP_PLACEHOLDER)) != NULL) {                                                          
        p += strlen(START_TIMESTAMP_PLACEHOLDER);                                                                          
        if ((p = strstr(p, START_TIMESTAMP_PLACEHOLDER)) != NULL) {                                                        
            log_error("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",                         
                file, line, START_TIMESTAMP_PLACEHOLDER);                                                                  
            errors++;                                                                                            
		}
    }   	
	if ((p = strstr(str, METADATA_PLACEHOLDER)) != NULL) {
		p += strlen(METADATA_PLACEHOLDER);
		if ((p = strstr(p, METADATA_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
			    file, line, METADATA_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, ARTIST_PLACEHOLDER)) != NULL) {
		p += strlen(ARTIST_PLACEHOLDER);
		if ((p = strstr(p, ARTIST_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
			    file, line, ARTIST_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, TITLE_PLACEHOLDER)) != NULL) {
		p += strlen(TITLE_PLACEHOLDER);
		if ((p = strstr(p, TITLE_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
			    file, line, TITLE_PLACEHOLDER);
			errors++;
		}
	}

	if (!have_track) {
		log_error("%s[%ld]: Error: The decoder command requires the '%s' track placeholder\n",
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
		log_error("%s[%ld]: Error: `%s' placeholder not allowed in encoder command\n",
		    file, line, TRACK_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, STRING_PLACEHOLDER)) != NULL) {
		log_error("%s[%ld]: Error: `%s' placeholder not allowed in encoder command\n",
		    file, line, STRING_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, METADATA_PLACEHOLDER)) != NULL) {
		p += strlen(METADATA_PLACEHOLDER);
		if ((p = strstr(p, METADATA_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in encoder command\n",
			    file, line, METADATA_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, ARTIST_PLACEHOLDER)) != NULL) {
		p += strlen(ARTIST_PLACEHOLDER);
		if ((p = strstr(p, ARTIST_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in encoder command\n",
			    file, line, ARTIST_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, TITLE_PLACEHOLDER)) != NULL) {
		p += strlen(TITLE_PLACEHOLDER);
		if ((p = strstr(p, TITLE_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in encoder command\n",
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
		log_error("%s[%ld]: Error: `%s' placeholder not allowed in <metadata_format>\n",
		    file, line, METADATA_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, TRACK_PLACEHOLDER)) != NULL) {
		p += strlen(TRACK_PLACEHOLDER);
		if ((p = strstr(p, TRACK_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in <metadata_format>\n",
			    file, line, TRACK_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, STRING_PLACEHOLDER)) != NULL) {
		p += strlen(STRING_PLACEHOLDER);
		if ((p = strstr(p, STRING_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in <metadata_format>\n",
			    file, line, STRING_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, ARTIST_PLACEHOLDER)) != NULL) {
		p += strlen(ARTIST_PLACEHOLDER);
		if ((p = strstr(p, ARTIST_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in <metadata_format>\n",
			    file, line, ARTIST_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, TITLE_PLACEHOLDER)) != NULL) {
		p += strlen(TITLE_PLACEHOLDER);
		if ((p = strstr(p, TITLE_PLACEHOLDER)) != NULL) {
			log_error("%s[%ld]: Error: Multiple `%s' placeholders in <metadata_format>\n",
			    file, line, TITLE_PLACEHOLDER);
			errors++;
		}
	}

	return (errors);
}
