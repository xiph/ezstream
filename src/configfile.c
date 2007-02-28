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

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "configfile.h"
#include "util.h"

#ifndef PATH_MAX
# define PATH_MAX	256
#endif

static EZCONFIG		 ezConfig;
static const char	*blankString = "";

void	freeConfig(EZCONFIG *);

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

int
parseConfig(const char *fileName)
{
	xmlDocPtr	 doc;
	xmlNodePtr	 cur;
	char		*ls_xmlContentPtr;
	int		 shuffle_set, svrinfopublic_set, program_set;

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

	shuffle_set = 0;
	svrinfopublic_set = 0;
	program_set = 0;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, BAD_CAST "url")) {
			if (ezConfig.URL != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <url> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.URL = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "sourcepassword")) {
			if (ezConfig.password != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <sourcepassword> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.password = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "format")) {
			if (ezConfig.format != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <format> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
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
				printf("%s[%ld]: Error: Cannot have multiple <filename> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (strlen(ls_xmlContentPtr) > PATH_MAX - 1) {
					printf("%s[%ld]: Error: Path or filename in <filename> is too long.\n",
					       fileName, xmlGetLineNo(cur));
					goto config_error;
				}
				ezConfig.fileName = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "playlist_program")) {
			if (program_set) {
				printf("%s[%ld]: Error: Cannot have multiple <playlist_program> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				int tmp;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				tmp = atoi(ls_xmlContentPtr);
				ezConfig.fileNameIsProgram = (tmp == 0) ? 0 : 1;
				xmlFree(ls_xmlContentPtr);
				program_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "shuffle")) {
			if (shuffle_set) {
				printf("%s[%ld]: Error: Cannot have multiple <shuffle> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				int tmp;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				tmp = atoi(ls_xmlContentPtr);
				ezConfig.shuffle = (tmp == 0) ? 0 : 1;
				xmlFree(ls_xmlContentPtr);
				shuffle_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfoname")) {
			if (ezConfig.serverName != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfoname> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverName = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfourl")) {
			if (ezConfig.serverURL != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfourl> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverURL = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfogenre")) {
			if (ezConfig.serverGenre != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfogenre> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverGenre = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfodescription")) {
			if (ezConfig.serverDescription != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfodescription> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverDescription = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfobitrate")) {
			if (ezConfig.serverBitrate != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfobitrate> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverBitrate = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}

		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfochannels")) {
			if (ezConfig.serverChannels != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfochannels> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverChannels = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfosamplerate")) {
			if (ezConfig.serverSamplerate != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfosamplerate> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverSamplerate = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfoquality")) {
			if (ezConfig.serverQuality != NULL) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfoquality> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				ezConfig.serverQuality = xstrdup(ls_xmlContentPtr);
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "svrinfopublic")) {
			if (svrinfopublic_set) {
				printf("%s[%ld]: Error: Cannot have multiple <svrinfopublic> elements.\n",
				       fileName, xmlGetLineNo(cur));
				goto config_error;
			}
			if (cur->xmlChildrenNode != NULL) {
				int tmp;

				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				tmp = atoi(ls_xmlContentPtr);
				ezConfig.serverPublic = (tmp == 0) ? 0 : 1;
				xmlFree(ls_xmlContentPtr);
				svrinfopublic_set = 1;
			}
		}
		if (!xmlStrcmp(cur->name, BAD_CAST "reencode")) {
			xmlNodePtr	cur2;
			int		enable_set;

			enable_set = 0;
			cur2 = cur->xmlChildrenNode;
			while (cur2 != NULL) {
				if (!xmlStrcmp(cur2->name, BAD_CAST "enable")) {
					if (enable_set) {
						printf("%s[%ld]: Error: Cannot have multiple <enable> elements.\n",
						       fileName, xmlGetLineNo(cur));
						goto config_error;
					}
					if (cur2->xmlChildrenNode != NULL) {
						int tmp;

						ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
						tmp = atoi(ls_xmlContentPtr);
						ezConfig.reencode = (tmp == 0) ? 0 : 1;
						xmlFree(ls_xmlContentPtr);
						enable_set = 1;
					}
				}
				if (!xmlStrcmp(cur2->name, BAD_CAST "encdec")) {
					xmlNodePtr	 cur3;
					FORMAT_ENCDEC	*pformatEncDec;
					char		*p;

					pformatEncDec = xcalloc(1, sizeof(FORMAT_ENCDEC));

					cur3 = cur2->xmlChildrenNode;
					while (cur3 != NULL) {
						if (!xmlStrcmp(cur3->name, BAD_CAST "format")) {
							if (pformatEncDec->format != NULL) {
								printf("%s[%ld]: Error: Cannot have multiple <format> elements.\n",
								       fileName, xmlGetLineNo(cur3));
								goto config_error;
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
								printf("%s[%ld]: Error: Cannot have multiple <match> elements.\n",
								       fileName, xmlGetLineNo(cur3));
								goto config_error;
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
								printf("%s[%ld]: Error: Cannot have multiple <decode> elements.\n",
								       fileName, xmlGetLineNo(cur3));
								goto config_error;
							}
							if (cur3->xmlChildrenNode != NULL) {
								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->decoder = xstrdup(ls_xmlContentPtr);
								xmlFree(ls_xmlContentPtr);
								if ((p = strstr(pformatEncDec->decoder, TRACK_PLACEHOLDER)) != NULL) {
									p += strlen(TRACK_PLACEHOLDER);
									if ((p = strstr(p, TRACK_PLACEHOLDER)) != NULL) {
										printf("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
										       fileName, xmlGetLineNo(cur3), TRACK_PLACEHOLDER);
										goto config_error;
									}
								}
								if ((p = strstr(pformatEncDec->decoder, METADATA_PLACEHOLDER)) != NULL) {
									p += strlen(METADATA_PLACEHOLDER);
									if ((p = strstr(p, METADATA_PLACEHOLDER)) != NULL) {
										printf("%s[%ld]: Error: Multiple `%s' placeholders in decoder command\n",
										       fileName, xmlGetLineNo(cur3), METADATA_PLACEHOLDER);
										goto config_error;
									}
								}
							}
						}
						if (!xmlStrcmp(cur3->name, BAD_CAST "encode")) {
							if (pformatEncDec->encoder != NULL) {
								printf("%s[%ld]: Error: Cannot have multiple <encode> elements.\n",
								       fileName, xmlGetLineNo(cur3));
								goto config_error;
							}
							if (cur3->xmlChildrenNode != NULL) {
								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode, 1);
								pformatEncDec->encoder = xstrdup(ls_xmlContentPtr);
								xmlFree(ls_xmlContentPtr);
								if ((p = strstr(pformatEncDec->encoder, TRACK_PLACEHOLDER)) != NULL) {
									printf("%s[%ld]: Error: `%s' placeholder not allowed in encoder command\n",
									       fileName, xmlGetLineNo(cur3), TRACK_PLACEHOLDER);
									goto config_error;
								}
								if ((p = strstr(pformatEncDec->encoder, METADATA_PLACEHOLDER)) != NULL) {
									p += strlen(METADATA_PLACEHOLDER);
									if ((p = strstr(p, METADATA_PLACEHOLDER)) != NULL) {
										printf("%s[%ld]: Error: Multiple `%s' placeholders in encoder command\n",
										       fileName, xmlGetLineNo(cur3), METADATA_PLACEHOLDER);
										goto config_error;
									}
								}
							}
						}
						cur3 = cur3->next;
					}
					ezConfig.encoderDecoders[ezConfig.numEncoderDecoders] = pformatEncDec;
					ezConfig.numEncoderDecoders++;
				}
				cur2 = cur2->next;
			}
		}
		cur = cur->next;
	}

	xmlFreeDoc(doc);
	return(1);

config_error:

	xmlFreeDoc(doc);
	freeConfig(&ezConfig);
	return (0);
}
