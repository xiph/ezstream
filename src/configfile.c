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

static EZCONFIG	 ezConfig;
static char	*blankString = "";

EZCONFIG *
getEZConfig(void)
{
	return (&ezConfig);
}

char*	getFormatEncoder(char *format)
{
	int i = 0;
	for (i=0;i<ezConfig.numEncoderDecoders;i++) {
		if (ezConfig.encoderDecoders[i]) {
			if (ezConfig.encoderDecoders[i]->format) {
				if (!strcmp(ezConfig.encoderDecoders[i]->format, format)) {
					if (ezConfig.encoderDecoders[i]->encoder) {
						return ezConfig.encoderDecoders[i]->encoder;
					}
					else {
						return blankString;
					}
				}
			}
		}
	}
	return blankString;
}

char*	getFormatDecoder(char *match)
{
	int i = 0;
	for (i=0;i<ezConfig.numEncoderDecoders;i++) {
		if (ezConfig.encoderDecoders[i]) {
			if (ezConfig.encoderDecoders[i]->match) {
				if (!strcmp(ezConfig.encoderDecoders[i]->match, match)) {
					if (ezConfig.encoderDecoders[i]->decoder) {
						return ezConfig.encoderDecoders[i]->decoder;
					}
					else {
						return blankString;
					}
				}
			}
		}
	}
	return blankString;
}

int parseConfig(char *fileName)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	char    *ls_xmlContentPtr;

	doc = xmlParseFile(fileName);
	if (doc == NULL) {
		printf("Unable to parse config file (%s)", fileName);
		return 0;
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		printf("Unable to parse config file (empty document)(%s)", fileName);
		xmlFreeDoc(doc);
		return 0;
	}

	memset(&ezConfig, '\000', sizeof(ezConfig));

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (!xmlStrcmp(cur->name, (const xmlChar *) "url")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.URL = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.URL, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.URL, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "sourcepassword")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.password = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.password, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.password, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "format")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.format = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.format, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.format, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "filename")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.fileName = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.fileName, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.fileName, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfoname")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverName = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.serverName, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.serverName, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfourl")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverURL = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.serverURL, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.serverURL, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfogenre")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverGenre = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.serverGenre, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.serverGenre, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfodescription")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverDescription = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.serverDescription, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.serverDescription, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfobitrate")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverBitrate = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.serverBitrate, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.serverBitrate, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}

		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfochannels")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverChannels = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.serverChannels, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.serverChannels, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfosamplerate")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverSamplerate = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.serverSamplerate, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.serverSamplerate, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfoquality")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverQuality = (char *)malloc(strlen(ls_xmlContentPtr) +1);
					memset(ezConfig.serverQuality, '\000', strlen(ls_xmlContentPtr) +1);
					strcpy(ezConfig.serverQuality, ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "svrinfopublic")) {
			if (cur->xmlChildrenNode != NULL) {
				ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur->xmlChildrenNode,1);
				if ( strlen(ls_xmlContentPtr) > 0 ) {
					ezConfig.serverPublic = atoi(ls_xmlContentPtr);
				}
				xmlFree(ls_xmlContentPtr);
			}
		}
		if (!xmlStrcmp(cur->name, (const xmlChar *) "reencode")) {
			xmlNodePtr cur2;
			cur2 = cur->xmlChildrenNode;
			while (cur2 != NULL) {
				if (!xmlStrcmp(cur2->name, (const xmlChar *) "enable")) {
					if (cur2->xmlChildrenNode != NULL) {
						ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur2->xmlChildrenNode,1);
						if ( strlen(ls_xmlContentPtr) > 0 ) {
							ezConfig.reencode = atoi(ls_xmlContentPtr);
						}
						xmlFree(ls_xmlContentPtr);
					}
				}
				if (!xmlStrcmp(cur2->name, (const xmlChar *) "encdec")) {
					xmlNodePtr cur3;
					FORMAT_ENCDEC	*pformatEncDec = malloc(sizeof(FORMAT_ENCDEC));
					memset(pformatEncDec, '\000', sizeof(FORMAT_ENCDEC));
					cur3 = cur2->xmlChildrenNode;
					while (cur3 != NULL) {
						if (!xmlStrcmp(cur3->name, (const xmlChar *) "format")) {
							if (cur3->xmlChildrenNode != NULL) {
								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode,1);
								if ( strlen(ls_xmlContentPtr) > 0 ) {
									pformatEncDec->format = (char *)malloc(strlen(ls_xmlContentPtr) +1);
									memset(pformatEncDec->format, '\000', strlen(ls_xmlContentPtr) +1);
									strcpy(pformatEncDec->format, ls_xmlContentPtr);
								}
								xmlFree(ls_xmlContentPtr);
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *) "match")) {
							if (cur3->xmlChildrenNode != NULL) {
								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode,1);
								if ( strlen(ls_xmlContentPtr) > 0 ) {
									pformatEncDec->match = (char *)malloc(strlen(ls_xmlContentPtr) +1);
									memset(pformatEncDec->match, '\000', strlen(ls_xmlContentPtr) +1);
									strcpy(pformatEncDec->match, ls_xmlContentPtr);
								}
								xmlFree(ls_xmlContentPtr);
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *) "decode")) {
							if (cur3->xmlChildrenNode != NULL) {
								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode,1);
								if ( strlen(ls_xmlContentPtr) > 0 ) {
									pformatEncDec->decoder = (char *)malloc(strlen(ls_xmlContentPtr) +1);
									memset(pformatEncDec->decoder, '\000', strlen(ls_xmlContentPtr) +1);
									strcpy(pformatEncDec->decoder, ls_xmlContentPtr);
								}
								xmlFree(ls_xmlContentPtr);
							}
						}
						if (!xmlStrcmp(cur3->name, (const xmlChar *) "encode")) {
							if (cur3->xmlChildrenNode != NULL) {
								ls_xmlContentPtr = (char *)xmlNodeListGetString(doc, cur3->xmlChildrenNode,1);
								if ( strlen(ls_xmlContentPtr) > 0 ) {
									pformatEncDec->encoder = (char *)malloc(strlen(ls_xmlContentPtr) +1);
									memset(pformatEncDec->encoder, '\000', strlen(ls_xmlContentPtr) +1);
									strcpy(pformatEncDec->encoder, ls_xmlContentPtr);
								}
								xmlFree(ls_xmlContentPtr);
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
	return(1);
}
