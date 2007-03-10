/*
 *  ezstream - source client for Icecast with external en-/decoder support
 *  Copyright (C) 2003, 2004, 2005, 2006  Ed Zaleski <oddsock@oddsock.org>
 *  Copyright (C) 2007                    Moritz Grimm <gtgbr@gmx.net>
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

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "compat.h"
#include "configfile.h"
#include "strfctns.h"
#include "util.h"

extern char		*__progname;

static EZCONFIG		 ezConfig;
static const char	*blankString = "";

void	        freeConfig(EZCONFIG *);
unsigned int	checkDecoderLine(const char *, const char *, long);
unsigned int	checkEncoderLine(const char *, const char *, long);

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
	char		*ls_xmlContentPtr;
	int		 program_set, reconnect_set, shuffle_set,
			 streamOnce_set, svrinfopublic_set;
	unsigned int	 config_error;

	xmlLineNumbersDefault(1);
	if ((doc = xmlParseFile(fileName)) == NULL) {
		printf("%s: Parse error (not well-formed XML.)\n", fileName);
		return (0);
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		printf("%s: Parse error (empty XML document.)\n", fileName);
		xmlFreeDoc(doc);
		return (0);
	}

	memset(&ezConfig, '\000', sizeof(ezConfig));

	config_error = 0;
	program_set = 0;
	reconnect_set = 0;
	shuffle_set = 0;
	streamOnce_set = 0;
	svrinfopublic_set = 0;
	for (cur = cur->xmlChildrenNode; cur != NULL; cur = cur->next) {
		if (!xmlStrcmp(cur->name, BAD_CAST "url")) {
			if (ezConfig.URL != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <url> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.URL = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "sourcepassword")) {
			if (ezConfig.password != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <sourcepassword> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.password = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "format")) {
			if (ezConfig.format != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <format> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
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
		if (!xmlStrcmp(cur->name, BAD_CAST "filename")) {
			if (ezConfig.fileName != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <filename> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (strlen(ls_xmlContentPtr) > PATH_MAX - 1) {
					printf("%s[%ld]: Error: Path or filename in <filename> is too long\n",
					       fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				ezConfig.fileName = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "metadata_progname")) {
			if (ezConfig.metadataProgram != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <metadata_progname> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (strlen(ls_xmlContentPtr) > PATH_MAX - 1) {
					printf("%s[%ld]: Error: Path or filename in <metadata_progname> is too long\n",
					       fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				ezConfig.metadataProgram = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "playlist_program")) {
			if (program_set) {
				printf("%s[%ld]: Error: Cannot have multiple <playlist_program> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.fileNameIsProgram = (int)strtonum(ls_xmlContentPtr, 0, 1, &errstr);
				if (errstr) {
					printf("%s[%ld]: Error: <playlist_program> may only contain 1 or 0\n",
					       fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				program_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "shuffle")) {
			if (shuffle_set) {
				printf("%s[%ld]: Error: Cannot have multiple <shuffle> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.shuffle = (int)strtonum(ls_xmlContentPtr, 0, 1, &errstr);
				if (errstr) {
					printf("%s[%ld]: Error: <shuffle> may only contain 1 or 0\n",
					       fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				shuffle_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "stream_once")) {
			if (streamOnce_set) {
				printf("%s[%ld]: Error: Cannot have multiple <stream_once> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.streamOnce = (int)strtonum(ls_xmlContentPtr, 0, 1, &errstr);
				if (errstr) {
					printf("%s[%ld]: Error: <stream_once> may only contain 1 or 0\n",
					       fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				streamOnce_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "reconnect_tries")) {
			if (reconnect_set) {
				printf("%s[%ld]: Error: Cannot have multiple <reconnect_tries> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.reconnectAttempts = (unsigned int)strtonum(ls_xmlContentPtr, 0, UINT_MAX, &errstr);
				if (errstr) {
					printf("%s[%ld]: Error: In <reconnect_tries>: '%s' is %s\n",
					       fileName, xmlGetLineNo(cur), ls_xmlContentPtr, errstr);
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				reconnect_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfoname")) {
			if (ezConfig.serverName != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfoname> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverName = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfourl")) {
			if (ezConfig.serverURL != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfourl> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverURL = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfogenre")) {
			if (ezConfig.serverGenre != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfogenre> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverGenre = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfodescription")) {
			if (ezConfig.serverDescription != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfodescription> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverDescription = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfobitrate")) {
			if (ezConfig.serverBitrate != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfobitrate> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverBitrate = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}

		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfochannels")) {
			if (ezConfig.serverChannels != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfochannels> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverChannels = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfosamplerate")) {
			if (ezConfig.serverSamplerate != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfosamplerate> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverSamplerate = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfoquality")) {
			if (ezConfig.serverQuality != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfoquality> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverQuality = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfopublic")) {
			if (svrinfopublic_set) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfopublic> elements\n",
				       fileName, xmlGetLineNo(cur));
				config_error++;
				continue;
			}
			if (cur->xmlChildrenNode != NULL) {
				const char *errstr;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverPublic = (int)strtonum(ls_xmlContentPtr, 0, 1, &errstr);
				if (errstr) {
					printf("%s[%ld]: Error: <svrinfopublic> may only contain 1 or 0\n",
					       fileName, xmlGetLineNo(cur));
					config_error++;
					continue;
				}
				xmlFree(ls_xmlContentPtr);
				svrinfopublic_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "reencode")) {
			xmlNodePtr	cur2;
			int		enable_set;

			enable_set = 0;
			for (cur2 = cur->xmlChildrenNode; cur2 != NULL;
			     cur2 = cur2->next) {
				if (!xmlStrcmp(cur2->name, BAD_CAST "enable")) {
					if (enable_set) {
						printf("%s[%ld]: Error: Cannot have multiple <enable> elements\n",
						       fileName, xmlGetLineNo(cur));
						config_error++;
						continue;
					}
					if (cur2->xmlChildrenNode != NULL) {
						const char *errstr;

						ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
						ezConfig.reencode = (int)strtonum(ls_xmlContentPtr, 0, 1, &errstr);
						if (errstr) {
							printf("%s[%ld]: Error: <enable> may only contain 1 or 0\n",
							       fileName, xmlGetLineNo(cur));
							config_error++;
							continue;
						}
						xmlFree(ls_xmlContentPtr);
						enable_set = 1;
					}
				}
				if (!xmlStrcmp(cur2->name, BAD_CAST "encdec")) {
					xmlNodePtr	 cur3;
					FORMAT_ENCDEC	*pformatEncDec;

					pformatEncDec = xcalloc(1, sizeof(FORMAT_ENCDEC));

					for (cur3 = cur2->xmlChildrenNode;
					     cur3 != NULL; cur3 = cur3->next) {
						if (!xmlStrcmp(cur3->name, BAD_CAST "format")) {
							if (pformatEncDec->format != NULL) {
								printf("%s[%ld]: Error: Cannot have multiple <format> elements\n",
								       fileName, xmlGetLineNo(cur3));
								config_error++;
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
						if (!xmlStrcmp(cur3->name, BAD_CAST "match")) {
							if (pformatEncDec->match != NULL) {
								printf("%s[%ld]: Error: Cannot have multiple <match> elements\n",
								       fileName, xmlGetLineNo(cur3));
								config_error++;
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
						if (!xmlStrcmp(cur3->name, BAD_CAST "decode")) {
							if (pformatEncDec->decoder != NULL) {
								printf("%s[%ld]: Error: Cannot have multiple <decode> elements\n",
								       fileName, xmlGetLineNo(cur3));
								config_error++;
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								unsigned int	ret;

								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->decoder = xstrdup(ls_xmlContentPtr);
								xmlFree(ls_xmlContentPtr);
								if ((ret = checkDecoderLine(pformatEncDec->decoder,
											    fileName, xmlGetLineNo(cur3)))
								    > 0) {
									config_error += ret;
									continue;
								}
							}
						}
						if (!xmlStrcmp(cur3->name, BAD_CAST "encode")) {
							if (pformatEncDec->encoder != NULL) {
								printf("%s[%ld]: Error: Cannot have multiple <encode> elements\n",
								       fileName, xmlGetLineNo(cur3));
								config_error++;
								continue;
							}
							if (cur3->xmlChildrenNode != NULL) {
								unsigned int	ret;

								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->encoder = xstrdup(ls_xmlContentPtr);
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
	printf("%s: %u configuration error(s) in %s\n", __progname,
	       config_error, fileName);

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

	errors = 0;
	if ((p = strstr(str, TRACK_PLACEHOLDER)) != NULL) {
		p += strlen(TRACK_PLACEHOLDER);
		if ((p = strstr(p, TRACK_PLACEHOLDER)) != NULL) {
			printf("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
			       file, line, TRACK_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, METADATA_PLACEHOLDER)) != NULL) {
		p += strlen(METADATA_PLACEHOLDER);
		if ((p = strstr(p, METADATA_PLACEHOLDER)) != NULL) {
			printf("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
			       file, line, METADATA_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, ARTIST_PLACEHOLDER)) != NULL) {
		p += strlen(ARTIST_PLACEHOLDER);
		if ((p = strstr(p, ARTIST_PLACEHOLDER)) != NULL) {
			printf("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
			       file, line, ARTIST_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, TITLE_PLACEHOLDER)) != NULL) {
		p += strlen(TITLE_PLACEHOLDER);
		if ((p = strstr(p, TITLE_PLACEHOLDER)) != NULL) {
			printf("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
			       file, line, TITLE_PLACEHOLDER);
			errors++;
		}
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
		printf("%s[%ld]: Error: `%s' placeholder not allowed in encoder command\n",
		       file, line, TRACK_PLACEHOLDER);
		errors++;
	}
	if ((p = strstr(str, METADATA_PLACEHOLDER)) != NULL) {
		p += strlen(METADATA_PLACEHOLDER);
		if ((p = strstr(p, METADATA_PLACEHOLDER)) != NULL) {
			printf("%s[%ld]: Error: Multiple `%s' placeholders in encoder command\n",
			       file, line, METADATA_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, ARTIST_PLACEHOLDER)) != NULL) {
		p += strlen(ARTIST_PLACEHOLDER);
		if ((p = strstr(p, ARTIST_PLACEHOLDER)) != NULL) {
			printf("%s[%ld]: Error: Multiple `%s' placeholders in encoder command\n",
			       file, line, ARTIST_PLACEHOLDER);
			errors++;
		}
	}
	if ((p = strstr(str, TITLE_PLACEHOLDER)) != NULL) {
		p += strlen(TITLE_PLACEHOLDER);
		if ((p = strstr(p, TITLE_PLACEHOLDER)) != NULL) {
			printf("%s[%ld]: Error: Multiple `%s' placeholders in encoder command\n",
			       file, line, TITLE_PLACEHOLDER);
			errors++;
		}
	}

	return (errors);
}
