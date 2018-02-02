/*
 * Copyright (c) 2015, 2018 Moritz Grimm <mgrimm@mrsserver.net>
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

#include <stdio.h>
#include <string.h>

#include "cfg.h"
#include "cfgfile_xml.h"
#include "log.h"
#include "xalloc.h"

#include <libxml/parser.h>

static int	_cfgfile_xml_parse_server(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_servers(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_stream(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_streams(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_intake(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_intakes(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_metadata(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_decoder(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_decoders(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_encoder(xmlDocPtr, xmlNodePtr);
static int	_cfgfile_xml_parse_encoders(xmlDocPtr, xmlNodePtr);
static void	_cfgfile_xml_print_server(cfg_server_t, void *);
static void	_cfgfile_xml_print_stream(cfg_stream_t, void *);
static void	_cfgfile_xml_print_intake(cfg_intake_t, void *);
static void	_cfgfile_xml_print_decoder_ext(const char *, void *);
static void	_cfgfile_xml_print_decoder(cfg_decoder_t, void *);
static void	_cfgfile_xml_print_encoder(cfg_encoder_t, void *);

#define XML_CHAR(s)	(const xmlChar *)(s)
#define STD_CHAR(s)	(const char *)(s)

#define XML_SERVER_SET(c, l, f, e)	do {				\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
		int		 error = 0;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {	\
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
_cfgfile_xml_parse_server(xmlDocPtr doc, xmlNodePtr cur)
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
_cfgfile_xml_parse_servers(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("server")) &&
		    0 > _cfgfile_xml_parse_server(doc, cur))
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
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {	\
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
_cfgfile_xml_parse_stream(xmlDocPtr doc, xmlNodePtr cur)
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
		XML_STREAM_SET(s, sl, cfg_stream_set_intake,             "intake");
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
_cfgfile_xml_parse_streams(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("stream")) &&
		    0 > _cfgfile_xml_parse_stream(doc, cur))
			return (-1);
	}

	return (0);
}

#define XML_INPUT_SET(c, l, f, e)	do {				\
	if (0 == xmlStrcasecmp(cur->name, XML_CHAR((e)))) {		\
		xmlChar 	*val;					\
		const char	*err_str;				\
		int		 error = 0;				\
									\
		val = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1); \
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {	\
			log_error("%s[%ld]: intake (%s): %s: %s",	\
			    doc->name, xmlGetLineNo(cur),		\
			    cfg_intake_get_name((c)), (e), err_str);	\
			error = 1;					\
		}							\
		xmlFree(val);						\
		if (error)						\
			return (-1);					\
	}								\
} while (0)

static int
_cfgfile_xml_parse_intake(xmlDocPtr doc, xmlNodePtr cur)
{
	cfg_intake_list_t	 il;
	cfg_intake_t		 i;
	const char		*errstr;
	long int		 line_no = xmlGetLineNo(cur);

	il = cfg_get_intakes();
	i = cfg_intake_list_get(il, CFG_DEFAULT);

	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		XML_INPUT_SET(i, il, cfg_intake_set_name,        "name");
		XML_INPUT_SET(i, il, cfg_intake_set_type,        "type");
		XML_INPUT_SET(i, il, cfg_intake_set_filename,    "filename");
		XML_INPUT_SET(i, il, cfg_intake_set_shuffle,     "shuffle");
		XML_INPUT_SET(i, il, cfg_intake_set_stream_once, "stream_once");
	}

	if (0 > cfg_intake_validate(i, &errstr)) {
		log_error("%s[%ld]: intake (%s): %s", doc->name, line_no,
		    cfg_intake_get_name(i), errstr);
		return (-1);
	}

	return (0);
}

static int
_cfgfile_xml_parse_intakes(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("intake")) &&
		    0 > _cfgfile_xml_parse_intake(doc, cur))
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
_cfgfile_xml_parse_metadata(xmlDocPtr doc, xmlNodePtr cur)
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
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {	\
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
_cfgfile_xml_parse_decoder(xmlDocPtr doc, xmlNodePtr cur)
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
_cfgfile_xml_parse_decoders(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("decoder")) &&
		    0 > _cfgfile_xml_parse_decoder(doc, cur))
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
		if (0 > (f)((c), (l), STD_CHAR(val), &err_str)) {	\
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
_cfgfile_xml_parse_encoder(xmlDocPtr doc, xmlNodePtr cur)
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
_cfgfile_xml_parse_encoders(xmlDocPtr doc, xmlNodePtr cur)
{
	for (cur = cur->xmlChildrenNode; cur; cur = cur->next) {
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("encoder")) &&
		    0 > _cfgfile_xml_parse_encoder(doc, cur))
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
 *             tls
 *             tls_cipher_suite
 *             ca_dir
 *             ca_file
 *             client_cert
 *             reconnect_attempts
 *         ...
 *     streams
 *         stream
 *             name
 *             mountpoint
 *             intake
 *             server
 *             public
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
 *     intakes
 *         intake
 *             type
 *             filename
 *             shuffle
 *             stream_once
 *         ...
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
cfgfile_xml_parse(const char *config_file)
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
			if (0 > _cfgfile_xml_parse_servers(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("streams"))) {
			if (0 > _cfgfile_xml_parse_streams(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("intakes"))) {
			if (0 > _cfgfile_xml_parse_intakes(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("metadata"))) {
			if (0 > _cfgfile_xml_parse_metadata(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("decoders"))) {
			if (0 > _cfgfile_xml_parse_decoders(doc, cur))
				error = 1;
			continue;
		}
		if (0 == xmlStrcasecmp(cur->name, XML_CHAR("encoders"))) {
			if (0 > _cfgfile_xml_parse_encoders(doc, cur))
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

static void
_cfgfile_xml_print_server(cfg_server_t s, void *arg)
{
	FILE	*fp = (FILE *)arg;

	fprintf(fp, "    <server>\n");
	if (0 != strcasecmp(cfg_server_get_name(s), CFG_DEFAULT))
		fprintf(fp, "      <name>%s</name>\n",
		    cfg_server_get_name(s));
	fprintf(fp, "      <protocol>%s</protocol>\n",
	    cfg_server_get_protocol_str(s));
	if (cfg_server_get_hostname(s))
		fprintf(fp, "      <hostname>%s</hostname>\n",
		    cfg_server_get_hostname(s));
	if (cfg_server_get_port(s) != CFG_SERVER_DEFAULT_PORT)
		fprintf(fp, "      <port>%u</port>\n",
		    cfg_server_get_port(s));
	if (0 != strcasecmp(cfg_server_get_user(s), CFG_SERVER_DEFAULT_USER))
		fprintf(fp, "      <user>%s</user>\n",
		    cfg_server_get_user(s));
	if (cfg_server_get_password(s))
		fprintf(fp, "      <password>%s</password>\n",
		    cfg_server_get_password(s));
	if (cfg_server_get_tls(s) != CFG_TLS_MAY)
		fprintf(fp, "      <tls>%s</tls>\n",
		    cfg_server_get_tls_str(s));
	if (cfg_server_get_tls_cipher_suite(s))
		fprintf(fp, "      <tls_cipher_suite>%s</tls_cipher_suite>\n",
		    cfg_server_get_tls_cipher_suite(s));
	if (cfg_server_get_ca_dir(s))
		fprintf(fp, "      <ca_dir>%s</ca_dir>\n",
		    cfg_server_get_ca_dir(s));
	if (cfg_server_get_ca_file(s))
		fprintf(fp, "      <ca_file>%s</ca_file>\n",
		    cfg_server_get_ca_file(s));
	if (cfg_server_get_client_cert(s))
		fprintf(fp, "      <client_cert>%s</client_cert>\n",
		    cfg_server_get_client_cert(s));
	if (cfg_server_get_reconnect_attempts(s))
		fprintf(fp, "      <reconnect_attempts>%u</reconnect_attempts>\n",
		    cfg_server_get_reconnect_attempts(s));
	fprintf(fp, "    </server>\n");
}

static void
_cfgfile_xml_print_stream(cfg_stream_t s, void *arg)
{
	FILE	*fp = (FILE *)arg;

	fprintf(fp, "    <stream>\n");
	if (0 != strcasecmp(cfg_stream_get_name(s), CFG_DEFAULT))
		fprintf(fp, "      <name>%s</name>\n",
		    cfg_stream_get_name(s));
	if (cfg_stream_get_mountpoint(s))
		fprintf(fp, "      <mountpoint>%s</mountpoint>\n",
		    cfg_stream_get_mountpoint(s));
	if (cfg_stream_get_intake(s))
		fprintf(fp, "      <intake>%s</intake>\n",
		    cfg_stream_get_intake(s));
	if (cfg_stream_get_server(s))
		fprintf(fp, "      <server>%s</server>\n",
		    cfg_stream_get_server(s));
	if (cfg_stream_get_public(s))
		fprintf(fp, "      <public>yes</public>\n");
	fprintf(fp, "      <format>%s</format>\n",
	    cfg_stream_get_format_str(s));
	if (cfg_stream_get_encoder(s))
		fprintf(fp, "      <encoder>%s</encoder>\n",
		    cfg_stream_get_encoder(s));
	if (cfg_stream_get_stream_name(s))
		fprintf(fp, "      <stream_name>%s</stream_name>\n",
		    cfg_stream_get_stream_name(s));
	if (cfg_stream_get_stream_url(s))
		fprintf(fp, "      <stream_url>%s</stream_url>\n",
		    cfg_stream_get_stream_url(s));
	if (cfg_stream_get_stream_genre(s))
		fprintf(fp, "      <stream_genre>%s</stream_genre>\n",
		    cfg_stream_get_stream_genre(s));
	if (cfg_stream_get_stream_description(s))
		fprintf(fp, "      <stream_description>%s</stream_description>\n",
		    cfg_stream_get_stream_description(s));
	if (cfg_stream_get_stream_quality(s))
		fprintf(fp, "      <stream_quality>%s</stream_quality>\n",
		    cfg_stream_get_stream_quality(s));
	if (cfg_stream_get_stream_bitrate(s))
		fprintf(fp, "      <stream_bitrate>%s</stream_bitrate>\n",
		    cfg_stream_get_stream_bitrate(s));
	if (cfg_stream_get_stream_samplerate(s))
		fprintf(fp, "      <stream_samplerate>%s</stream_samplerate>\n",
		    cfg_stream_get_stream_samplerate(s));
	if (cfg_stream_get_stream_channels(s))
		fprintf(fp, "      <stream_channels>%s</stream_channels>\n",
		    cfg_stream_get_stream_channels(s));
	fprintf(fp, "    </stream>\n");
}

static void
_cfgfile_xml_print_intake(cfg_intake_t i, void *arg)
{
	FILE	*fp = (FILE *)arg;

	fprintf(fp, "    <intake>\n");
	if (0 != strcasecmp(cfg_intake_get_name(i), CFG_DEFAULT))
		fprintf(fp, "      <name>%s</name>\n",
		    cfg_intake_get_name(i));
	if (cfg_intake_get_type(i))
		fprintf(fp, "      <type>%s</type>\n",
		    cfg_intake_get_type_str(i));
	if (cfg_intake_get_filename(i))
		fprintf(fp, "      <filename>%s</filename>\n",
		    cfg_intake_get_filename(i));
	if (cfg_intake_get_shuffle(i))
		fprintf(fp, "      <shuffle>yes</shuffle>\n");
	if (cfg_intake_get_stream_once(i))
		fprintf(fp, "      <stream_once>yes</stream_once>\n");
	fprintf(fp, "    </intake>\n");
}

static void
_cfgfile_xml_print_decoder_ext(const char *ext, void *arg)
{
	FILE	*fp = (FILE *)arg;

	fprintf(fp, "      <file_ext>%s</file_ext>\n", ext);
}

static void
_cfgfile_xml_print_decoder(cfg_decoder_t d, void *arg)
{
	FILE	*fp = (FILE *)arg;

	fprintf(fp, "    <decoder>\n");
	if (0 != strcasecmp(cfg_decoder_get_name(d), CFG_DEFAULT))
		fprintf(fp, "      <name>%s</name>\n",
		    cfg_decoder_get_name(d));
	if (cfg_decoder_get_program(d))
		fprintf(fp, "      <program>%s</program>\n",
		    cfg_decoder_get_program(d));
	cfg_decoder_ext_foreach(d, _cfgfile_xml_print_decoder_ext, fp);
	fprintf(fp, "    </decoder>\n");
}

static void
_cfgfile_xml_print_encoder(cfg_encoder_t e, void *arg)
{
	FILE	*fp = (FILE *)arg;

	fprintf(fp, "    <encoder>\n");
	if (0 != strcasecmp(cfg_encoder_get_name(e), CFG_DEFAULT))
		fprintf(fp, "      <name>%s</name>\n",
		    cfg_encoder_get_name(e));
	fprintf(fp, "      <format>%s</format>\n",
	    cfg_stream_fmt2str(cfg_encoder_get_format(e)));
	if (cfg_encoder_get_program(e))
		fprintf(fp, "      <program>%s</program>\n",
		    cfg_encoder_get_program(e));
	fprintf(fp, "    </encoder>\n");
}

void
cfgfile_xml_print(FILE *fp)
{
	fprintf(fp, "<ezstream>\n");
	fprintf(fp, "\n");
	fprintf(fp, "  <servers>\n");
	cfg_server_list_foreach(cfg_get_servers(), _cfgfile_xml_print_server,
	    fp);
	fprintf(fp, "  </servers>\n");
	fprintf(fp, "\n");
	fprintf(fp, "  <streams>\n");
	cfg_stream_list_foreach(cfg_get_streams(), _cfgfile_xml_print_stream,
	    fp);
	fprintf(fp, "  </streams>\n");
	fprintf(fp, "\n");
	fprintf(fp, "  <intakes>\n");
	cfg_intake_list_foreach(cfg_get_intakes(), _cfgfile_xml_print_intake,
	    fp);
	fprintf(fp, "  </intakes>\n");
	if (cfg_decoder_list_nentries(cfg_get_decoders())) {
		fprintf(fp, "\n");
		fprintf(fp, "  <decoders>\n");
		cfg_decoder_list_foreach(cfg_get_decoders(),
		    _cfgfile_xml_print_decoder, fp);
		fprintf(fp, "  </decoders>\n");
	}
	if (cfg_encoder_list_nentries(cfg_get_encoders())) {
		fprintf(fp, "\n");
		fprintf(fp, "  <encoders>\n");
		cfg_encoder_list_foreach(cfg_get_encoders(),
		    _cfgfile_xml_print_encoder, fp);
		fprintf(fp, "  </encoders>\n");
	}
	if (cfg_get_metadata_program() ||
	    cfg_get_metadata_format_str() ||
	    0 <= cfg_get_metadata_refresh_interval() ||
	    cfg_get_metadata_normalize_strings() ||
	    cfg_get_metadata_no_updates()) {
		fprintf(fp, "\n");
		fprintf(fp, "  <metadata>\n");
		if (cfg_get_metadata_program())
			fprintf(fp, "    <program>%s</program>\n",
			    cfg_get_metadata_program());
		if (cfg_get_metadata_format_str())
			fprintf(fp, "    <format_str>%s</format_str>\n",
			    cfg_get_metadata_format_str());
		if (0 <= cfg_get_metadata_refresh_interval())
			fprintf(fp, "    <refresh_interval>%d</refresh_interval>\n",
			    cfg_get_metadata_refresh_interval());
		if (cfg_get_metadata_normalize_strings())
			fprintf(fp, "    <normalize_strings>yes</normalize_strings>\n");
		if (cfg_get_metadata_no_updates())
			fprintf(fp, "    <no_updates>yes</no_updates>\n");
		fprintf(fp, "  </metadata>\n");
	}
	fprintf(fp, "</ezstream>\n");
}
