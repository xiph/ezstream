#include <string.h>
#include "configfile.h"

static EZCONFIG	ezConfig;

EZCONFIG *getEZConfig() {
	return &ezConfig;
}

void printConfig()
{
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
	if (ezConfig.format == OGG_FORMAT) {
		printf("Broadcasting in Ogg format\n");
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
					if (!strcmp(ls_xmlContentPtr, "MP3")) {
						ezConfig.format = MP3_FORMAT;
					}
					if (!strcmp(ls_xmlContentPtr, "OGG")) {
						ezConfig.format = OGG_FORMAT;
					}
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
		cur = cur->next;
	}
	return(1);
}
