/*
 * Copyright (c) 2015 Moritz Grimm <mgrimm@mrsserver.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "cfg.h"
#include "cfg_xmlfile.h"
#include "log.h"
#include "xalloc.h"

#include <libxml/parser.h>

static unsigned int	decoder_id, encoder_id;

static int	_cfg_xmlfile_parse_server(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_stream(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_media(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_metadata(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_decoder(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_decoders(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_encoder(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_encoders(xmlDocPtr, xmlNodePtr);

#define XML_CHAR(s)		(const xmlChar *)(s)
#define XML_STRCONFIG(s, f, e)	do {					\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)(val, &err_str)) {				\
			log_error("%s[%ld]: %s: %s: %s", doc->name,	\
			    xmlGetLineNo(cur), (s), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
		continue;						\
	}								\
} while (0)

static int
_cfg_xmlfile_parse_server(xmlDocPtr doc, xmlNodePtr cur)
{
	int	error = 0;

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_STRCONFIG("server", cfg_set_server_protocol,    "protocol");
		XML_STRCONFIG("server", cfg_set_server_hostname,    "hostname");
		XML_STRCONFIG("server", cfg_set_server_port,        "port");
		XML_STRCONFIG("server", cfg_set_server_user,        "user");
		XML_STRCONFIG("server", cfg_set_server_password,    "password");
		XML_STRCONFIG("server", cfg_set_server_ca_dir,      "ca_dir");
		XML_STRCONFIG("server", cfg_set_server_ca_file,     "ca_file");
		XML_STRCONFIG("server", cfg_set_server_client_key,  "client_key");
		XML_STRCONFIG("server", cfg_set_server_client_cert, "client_cert");
		XML_STRCONFIG("server", cfg_set_server_reconnect_attempts,
								    "reconnect_attempts");
	}

	if (error)
		return (-1);

	return (0);
}

static int
_cfg_xmlfile_parse_stream(xmlDocPtr doc, xmlNodePtr cur)
{
	int	error = 0;

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_STRCONFIG("stream", cfg_set_stream_mountpoint,    "mountpoint");
		XML_STRCONFIG("stream", cfg_set_stream_name,          "name");
		XML_STRCONFIG("stream", cfg_set_stream_url,           "url");
		XML_STRCONFIG("stream", cfg_set_stream_genre,         "genre");
		XML_STRCONFIG("stream", cfg_set_stream_description,   "description");
		XML_STRCONFIG("stream", cfg_set_stream_quality,       "quality");
		XML_STRCONFIG("stream", cfg_set_stream_bitrate,       "bitrate");
		XML_STRCONFIG("stream", cfg_set_stream_samplerate,    "samplerate");
		XML_STRCONFIG("stream", cfg_set_stream_channels,      "channels");
		XML_STRCONFIG("stream", cfg_set_stream_server_public, "server_public");
		XML_STRCONFIG("stream", cfg_set_stream_format,        "format");
		XML_STRCONFIG("stream", cfg_set_stream_encoder,       "encoder");
	}

	if (error)
		return (-1);

	return (0);
}

static int
_cfg_xmlfile_parse_media(xmlDocPtr doc, xmlNodePtr cur)
{
	int	error = 0;

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_STRCONFIG("media", cfg_set_media_type,        "type");
		XML_STRCONFIG("media", cfg_set_media_filename,    "filename");
		XML_STRCONFIG("media", cfg_set_media_shuffle,     "shuffle");
		XML_STRCONFIG("media", cfg_set_media_stream_once, "stream_once");
	}

	if (error)
		return (-1);

	return (0);
}

static int
_cfg_xmlfile_parse_metadata(xmlDocPtr doc, xmlNodePtr cur)
{
	int	error = 0;

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_STRCONFIG("metadata", cfg_set_metadata_program,    "program");
		XML_STRCONFIG("metadata", cfg_set_metadata_format_str, "format_str");
		XML_STRCONFIG("metadata", cfg_set_metadata_refresh_interval,
								       "refresh_interval");
		XML_STRCONFIG("metadata", cfg_set_metadata_normalize_strings,
								       "normalize_strings");
		XML_STRCONFIG("metadata", cfg_set_metadata_no_updates, "no_updates");
	}

	if (error)
		return (-1);

	return (0);
}

#define XML_DECODER_SET(c, f, e)	do {				\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
		int		 error = 0;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)((c), val, &err_str)) {			\
			log_error("%s[%ld]: decoder: %s: %s: %s",	\
			    doc->name, xmlGetLineNo(cur),		\
			    cfg_decoder_get_name((c)), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
		if (error)						\
			return (-1);					\
		continue;						\
	}								\
} while (0)

static int
_cfg_xmlfile_parse_decoder(xmlDocPtr doc, xmlNodePtr cur)
{
	cfg_decoder_t	 d;
	char		 d_id[11];
	const char	*errstr;
	long int	 line_no = xmlGetLineNo(cur);

	(void)snprintf(d_id, sizeof(d_id), "%u", ++decoder_id);
	d = cfg_decoder_get(d_id);

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_DECODER_SET(d, cfg_decoder_set_name, "name");
		XML_DECODER_SET(d, cfg_decoder_set_program, "program");
		XML_DECODER_SET(d, cfg_decoder_add_match, "file_ext");
	}

	if (0 > cfg_decoder_validate(d, &errstr)) {
		log_error("%s[%ld]: decoder: %s", doc->name, line_no, errstr);
		return (-1);
	}

	return (0);
}

static int
_cfg_xmlfile_parse_decoders(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("decoder")) &&
		    0 > _cfg_xmlfile_parse_decoder(doc, cur))
			return (-1);
	}

	return (0);
}

#define XML_ENCODER_SET(c, f, e)	do {				\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
		int		 error = 0;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)((c), val, &err_str)) {			\
			log_error("%s[%ld]: encoder: %s: %s: %s",	\
			    doc->name, xmlGetLineNo(cur),		\
			    cfg_encoder_get_name((c)), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
		if (error)						\
			return (-1);					\
		continue;						\
	}								\
} while (0)

static int
_cfg_xmlfile_parse_encoder(xmlDocPtr doc, xmlNodePtr cur)
{
	cfg_encoder_t	 e;
	char		 e_id[11];
	const char	*errstr;
	long int	 line_no = xmlGetLineNo(cur);

	(void)snprintf(e_id, sizeof(e_id), "%u", ++encoder_id);
	e = cfg_encoder_get(e_id);

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_ENCODER_SET(e, cfg_encoder_set_name, "name");
		XML_ENCODER_SET(e, cfg_encoder_set_format_str, "format");
		XML_ENCODER_SET(e, cfg_encoder_set_program, "program");
	}

	if (0 > cfg_encoder_validate(e, &errstr)) {
		log_error("%s[%ld]: encoder: %s", doc->name, line_no, errstr);
		return (-1);
	}

	return (0);
}

static int
_cfg_xmlfile_parse_encoders(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("encoder")) &&
		    0 > _cfg_xmlfile_parse_encoder(doc, cur))
			return (-1);
	}

	return (0);
}

/*
 * XML configuration file structure:
 *
 * ezstream
 *     server
 *         protocol
 *         hostname
 *         port
 *         user
 *         password
 *         ca_dir
 *         ca_file
 *         client_cert
 *         client_key
 *         reconnect_attempts
 *     stream
 *         mountpoint
 *         name
 *         url
 *         genre
 *         description
 *         quality
 *         bitrate
 *         samplerate
 *         channels
 *         server_public
 *         format
 *         encoder
 *     media
 *         type
 *         filename
 *         shuffle
 *         stream_once
 *     metadata
 *         program
 *         format_str
 *         refresh_interval
 *         normalize_strings
 *         no_updates
 *     decoders
 *         decoder
 *             name
 *             program
 *             file_ext
 *             ...
 *         ...
 *     encoders
 *         encoder
 *             name
 *             format
 *             program
 *         ...
 */
int
cfg_xmlfile_parse(const char *config_file)
{
	xmlDocPtr	doc = NULL;
	xmlNodePtr	cur = NULL;
	int		error = 0;

	xmlLineNumbersDefault(1);

	doc = xmlParseFile(config_file);
	if (!doc) {
		log_error("%s: not well-formed XML", config_file);
		goto error;
	}
	if (!doc->name)
		doc->name = xmlStrdup(config_file);
	cur = xmlDocGetRootElement(doc);
	if (!cur) {
		log_error("%s: empty document", config_file);
		goto error;
	}
	if (0 != xmlStrcasecmp(cur->name, XML_CHAR("ezstream"))) {
		log_error("%s: not ezstream config", config_file);
		goto error;
	}

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("server"))) {
			if (0 > _cfg_xmlfile_parse_server(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("stream"))) {
			if (0 > _cfg_xmlfile_parse_stream(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("media"))) {
			if (0 > _cfg_xmlfile_parse_media(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("metadata"))) {
			if (0 > _cfg_xmlfile_parse_metadata(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("decoders"))) {
			if (0 > _cfg_xmlfile_parse_decoders(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("encoders"))) {
			if (0 > _cfg_xmlfile_parse_encoders(doc, cur))
				error = 1;
			continue;
		}
	}
	if (error)
		goto error;

	xmlFreeDoc(doc);

	return (0);

error:
	if (doc)
		xmlFreeDoc(doc);

	return (-1);
}
