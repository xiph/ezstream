#include <string.h>
#include "configfile.h"

static EZCONFIG	ezConfig;
static char	*blankString = "";

EZCONFIG *getEZConfig() {
	return &ezConfig;
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
void printConfig()
{
	int i = 0;
	if (ezConfig.URL) {
		printf("URL to connect to (%s)\n", ezConfig.URL);
	}
	else {
		printf("URL not SET\n");
	}
	if (ezConfig.password) {
		printf("source password (%s)\n", ezConfig.password);
	}
	else {
		printf("source password not SET\n");
	}
	if (ezConfig.format == MP3_FORMAT) {
		printf("Broadcasting in MP3 format\n");
	}
	if (ezConfig.format == VORBIS_FORMAT) {
		printf("Broadcasting in Ogg Vorbis format\n");
	}
	if (ezConfig.format == THEORA_FORMAT) {
		printf("Broadcasting in Ogg Theora format\n");
	}
	if (ezConfig.format == 0) {
		printf("Broadcast format not set\n");
	}
	if (ezConfig.fileName) {
		printf("File to broadcast (%s)\n", ezConfig.fileName);
	}
	else {
		printf("broadcast file not SET\n");
	}
	if (ezConfig.serverName) {
		printf("Server Info Name (%s)\n", ezConfig.serverName);
	}
	else {
		printf("Server Info Name not SET\n");
	}
	if (ezConfig.serverURL) {
		printf("Server Info URL (%s)\n", ezConfig.serverURL);
	}
	else {
		printf("Server Info URL not SET\n");
	}
	if (ezConfig.serverGenre) {
		printf("Server Info Genre (%s)\n", ezConfig.serverGenre);
	}
	else {
		printf("Server Info Genre not SET\n");
	}
	if (ezConfig.serverDescription) {
		printf("Server Info Description (%s)\n", ezConfig.serverDescription);
	}
	else {
		printf("Server Info Description not SET\n");
	}
	if (ezConfig.serverBitrate) {
		printf("Server Info Bitrate (%s)\n", ezConfig.serverBitrate);
	}
	else {
		printf("Server Info Bitrate not SET\n");
	}
	if (ezConfig.serverChannels) {
		printf("Server Info Channels (%s)\n", ezConfig.serverChannels);
	}
	else {
		printf("Server Info Channels not SET\n");
	}
	if (ezConfig.serverSamplerate) {
		printf("Server Info Samplerate (%s)\n", ezConfig.serverSamplerate);
	}
	else {
		printf("Server Info Samplerate not SET\n");
	}
	if (ezConfig.serverQuality) {
		printf("Server Info Quality (%s)\n", ezConfig.serverQuality);
	}
	else {
		printf("Server Info Quality not SET\n");
	}
	if (ezConfig.serverPublic) {
		printf("Server is a public server\n");
	}
	else {
		printf("Server is a private server\n");
	}
	if (ezConfig.reencode) {
		printf("We will reencode using the following information:\n");
		printf("\tEncoders/Decoders:\n");
		for (i=0;i<ezConfig.numEncoderDecoders;i++) {
			if (ezConfig.encoderDecoders[i]) {
				if (ezConfig.encoderDecoders[i]->match) {
					if (ezConfig.encoderDecoders[i]->decoder) {
							printf("\t\tFor files of extension (%s)\n", ezConfig.encoderDecoders[i]->match);
							printf("\t\t\tDecoder: (%s)\n", ezConfig.encoderDecoders[i]->decoder);
					}
					else {
						printf("\t\tNull decoder\n");
					}
				}
				else {
					printf("\t\tNull match\n");
				}
				if (ezConfig.encoderDecoders[i]->format) {
					if (ezConfig.encoderDecoders[i]->encoder) {
						printf("\t\tFor output formats of type (%s)\n", ezConfig.encoderDecoders[i]->format);
						printf("\t\t\tEncoder: (%s)\n", ezConfig.encoderDecoders[i]->encoder);
					}
					else {
						printf("\t\tNull encoder\n");
					}
				}
				else {
					printf("\t\tNull match\n");
				}
			}
			else {
				printf("Error, NULL GRABBER\n");
			}
		}
	}
	else {
		printf("We will NOT reencode.\n");
	}

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
