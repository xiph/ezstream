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

static int	_cfg_xmlfile_parse_server(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_servers(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_stream(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_streams(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_media(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_metadata(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_decoder(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_decoders(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_encoder(xmlDocPtr, xmlNodePtr);
static int	_cfg_xmlfile_parse_encoders(xmlDocPtr, xmlNodePtr);

#define XML_CHAR(s)	(const xmlChar *)(s)
#define STD_CHAR(s)	(const char *)(s)

#define XML_SERVER_SET(c, l, f, e)	do {				\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
		int		 error = 0;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {		\
			log_error("%s[%ld]: server (%s): %s: %s",	\
			    doc->name, xmlGetLineNo(cur),		\
			    cfg_server_get_name((c)), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
		if (error)						\
			return (-1);					\
	}								\
} while (0)

static int
_cfg_xmlfile_parse_server(xmlDocPtr doc, xmlNodePtr cur)
{
	cfg_server_list_t	 sl;
	cfg_server_t		 s;
	const char		*errstr;
	long int		 line_no = xmlGetLineNo(cur);

	sl = cfg_get_servers();
	s = cfg_server_list_get(sl, CFG_DEFAULT);

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_SERVER_SET(s, sl, cfg_server_set_name,               "name");
		XML_SERVER_SET(s, sl, cfg_server_set_protocol,           "protocol");
		XML_SERVER_SET(s, sl, cfg_server_set_hostname,           "hostname");
		XML_SERVER_SET(s, sl, cfg_server_set_port,               "port");
		XML_SERVER_SET(s, sl, cfg_server_set_user,               "user");
		XML_SERVER_SET(s, sl, cfg_server_set_password,           "password");
		XML_SERVER_SET(s, sl, cfg_server_set_reconnect_attempts, "reconnect_attempts");
		XML_SERVER_SET(s, sl, cfg_server_set_tls,                "tls");
		XML_SERVER_SET(s, sl, cfg_server_set_tls_cipher_suite,   "tls_cipher_suite");
		XML_SERVER_SET(s, sl, cfg_server_set_ca_dir,             "ca_dir");
		XML_SERVER_SET(s, sl, cfg_server_set_ca_file,            "ca_file");
		XML_SERVER_SET(s, sl, cfg_server_set_client_cert,        "client_cert");
	}

	if (0 > cfg_server_validate(s, &errstr)) {
		log_error("%s[%ld]: server (%s): %s", doc->name, line_no,
		    cfg_server_get_name(s), errstr);
		return (-1);
	}

	return (0);
}

static int
_cfg_xmlfile_parse_servers(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("server")) &&
		    0 > _cfg_xmlfile_parse_server(doc, cur))
			return (-1);
	}

	return (0);
}

#define XML_STREAM_SET(c, l, f, e)	do {				\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
		int		 error = 0;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {		\
			log_error("%s[%ld]: stream (%s): %s: %s",	\
			    doc->name, xmlGetLineNo(cur),		\
			    cfg_stream_get_name((c)), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
		if (error)						\
			return (-1);					\
	}								\
} while (0)

static int
_cfg_xmlfile_parse_stream(xmlDocPtr doc, xmlNodePtr cur)
{
	cfg_stream_list_t	 sl;
	cfg_stream_t		 s;
	const char		*errstr;
	long int		 line_no = xmlGetLineNo(cur);

	sl = cfg_get_streams();
	s = cfg_stream_list_get(sl, CFG_DEFAULT);

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_STREAM_SET(s, sl, cfg_stream_set_name,               "name");
		XML_STREAM_SET(s, sl, cfg_stream_set_mountpoint,         "mountpoint");
		XML_STREAM_SET(s, sl, cfg_stream_set_server,             "server");
		XML_STREAM_SET(s, sl, cfg_stream_set_public,             "public");
		XML_STREAM_SET(s, sl, cfg_stream_set_format,             "format");
		XML_STREAM_SET(s, sl, cfg_stream_set_encoder,            "encoder");
		XML_STREAM_SET(s, sl, cfg_stream_set_stream_url,         "stream_url");
		XML_STREAM_SET(s, sl, cfg_stream_set_stream_genre,       "stream_genre");
		XML_STREAM_SET(s, sl, cfg_stream_set_stream_description, "stream_description");
		XML_STREAM_SET(s, sl, cfg_stream_set_stream_quality,     "stream_quality");
		XML_STREAM_SET(s, sl, cfg_stream_set_stream_bitrate,     "stream_bitrate");
		XML_STREAM_SET(s, sl, cfg_stream_set_stream_samplerate,  "stream_samplerate");
		XML_STREAM_SET(s, sl, cfg_stream_set_stream_channels,    "stream_channels");
	}

	if (0 > cfg_stream_validate(s, &errstr)) {
		log_error("%s[%ld]: stream (%s): %s", doc->name, line_no,
		    cfg_stream_get_name(s), errstr);
		return (-1);
	}

	return (0);
}

static int
_cfg_xmlfile_parse_streams(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("stream")) &&
		    0 > _cfg_xmlfile_parse_stream(doc, cur))
			return (-1);
	}

	return (0);
}

#define XML_STRCONFIG(s, f, e)	do {					\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)(STD_CHAR(val), &err_str)) {			\
			log_error("%s[%ld]: %s: %s: %s", doc->name,	\
			    xmlGetLineNo(cur), (s), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
	}								\
} while (0)

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

#define XML_DECODER_SET(c, l, f, e)	do {				\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
		int		 error = 0;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {		\
			log_error("%s[%ld]: decoder (%s): %s: %s",	\
			    doc->name, xmlGetLineNo(cur),		\
			    cfg_decoder_get_name((c)), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
		if (error)						\
			return (-1);					\
	}								\
} while (0)

static int
_cfg_xmlfile_parse_decoder(xmlDocPtr doc, xmlNodePtr cur)
{
	cfg_decoder_list_t	 dl;
	cfg_decoder_t		 d;
	const char		*errstr;
	long int		 line_no = xmlGetLineNo(cur);

	dl = cfg_get_decoders();
	d = cfg_decoder_list_get(dl, CFG_DEFAULT);

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_DECODER_SET(d, dl, cfg_decoder_set_name,    "name");
		XML_DECODER_SET(d, dl, cfg_decoder_set_program, "program");
		XML_DECODER_SET(d, dl, cfg_decoder_add_match,   "file_ext");
	}

	if (0 > cfg_decoder_validate(d, &errstr)) {
		log_error("%s[%ld]: decoder (%s): %s", doc->name, line_no,
		    cfg_decoder_get_name(d), errstr);
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

#define XML_ENCODER_SET(c, l, f, e)	do {				\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
		int		 error = 0;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {		\
			log_error("%s[%ld]: encoder (%s): %s: %s",	\
			    doc->name, xmlGetLineNo(cur),		\
			    cfg_encoder_get_name((c)), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
		if (error)						\
			return (-1);					\
	}								\
} while (0)

static int
_cfg_xmlfile_parse_encoder(xmlDocPtr doc, xmlNodePtr cur)
{
	cfg_encoder_list_t	 el;
	cfg_encoder_t		 e;
	const char		*errstr;
	long int		 line_no = xmlGetLineNo(cur);

	el = cfg_get_encoders();
	e = cfg_encoder_list_get(el, CFG_DEFAULT);

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_ENCODER_SET(e, el, cfg_encoder_set_name,       "name");
		XML_ENCODER_SET(e, el, cfg_encoder_set_format_str, "format");
		XML_ENCODER_SET(e, el, cfg_encoder_set_program,    "program");
	}

	if (0 > cfg_encoder_validate(e, &errstr)) {
		log_error("%s[%ld]: encoder (%s): %s", doc->name, line_no,
		    cfg_encoder_get_name(e), errstr);
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
 *     servers
 *         server
 *             name
 *             protocol
 *             hostname
 *             port
 *             user
 *             password
 *             reconnect_attempts
 *             tls
 *             tls_cipher_suite
 *             ca_dir
 *             ca_file
 *             client_cert
 *         ...
 *     streams
 *         stream
 *             name
 *             mountpoint
 *             public
 *             server
 *             format
 *             encoder
 *             stream_name
 *             stream_url
 *             stream_genre
 *             stream_description
 *             stream_quality
 *             stream_bitrate
 *             stream_samplerate
 *             stream_channels
 *         ...
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

	if (0 > cfg_file_check(config_file))
		goto error;

	doc = xmlParseFile(config_file);
	if (!doc) {
		log_error("%s: not well-formed XML", config_file);
		goto error;
	}
	if (!doc->name)
		doc->name = (char *)xmlStrdup(XML_CHAR(config_file));
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
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("servers"))) {
			if (0 > _cfg_xmlfile_parse_servers(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("streams"))) {
			if (0 > _cfg_xmlfile_parse_streams(doc, cur))
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
