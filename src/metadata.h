/*
 * Copyright (c) 2007 Moritz Grimm <gtgbr@gmx.net>
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

#ifndef __METADATA_H__
#define __METADATA_H__

#define METADATA_MAX	1023

enum metadata_request {
	METADATA_ALL = 0,
	METADATA_STRING,
	METADATA_ARTIST,
	METADATA_TITLE
};

typedef struct metadata metadata_t;

/*
 * Read the metadata of a media file and return a new metadata handle on
 * success, or NULL on failure. The returned handle is "branded" for reading
 * metadata from media files.
 */
metadata_t *	metadata_file(const char * /* filename */);

/*
 * Create a metadata handle that is "branded" for acquiring metadata from an
 * external program. The handle is returned on success, or NULL on failure.
 * The program is NOT YET being queried, use metadata_program_update() for
 * that. Also, the program (or script) needs to follow these rules:
 *
 *   - Print one line to standard output and exit.
 *   - Accept no command line parameter and return a complete metadata string
 *     (for metadata_get_string()). The program *should* always return
 *     something in this case (e.g. something based on the filename in case no
 *     metadata is available.)
 *   - Accept the command line parameter "artist" and return only the artist
 *     metadata, or an empty string if no artist information is available.
 *   - Accept the command line parameter "title" and return only the song title
 *     metadata, or an empty string if no artist information is available.
 *   - Return at most METADATA_MAX characters, or the result will be truncated.
 */
metadata_t *	metadata_program(const char * /* program name */);

/*
 * Free all memory used by a metadata handle that has been created with
 * metadata_file() or metadata_program().
 */
void		metadata_free(metadata_t **);

/*
 * Update/read the metadata for the given handle. Returns 1 on success, and 0
 * on failure.
 */
int		metadata_file_update(metadata_t *);

/*
 * Update/read the specified metadata for the given program-handle. Returns 1
 * on success, and 0 on failure.
 */
int		metadata_program_update(metadata_t *, enum metadata_request);

/*
 * Returns a pointer to a metadata string ``artist - title'', or just
 * ``artist'' or ``title'' if one of the two is not available. If neither
 * are present, it usually returns the filename without the extension.
 * This function never returns NULL.
 */
const char *	metadata_get_string(metadata_t *);

/*
 * Returns a pointer to the artist string, or NULL if the file did not
 * contain any artist information.
 */
const char *	metadata_get_artist(metadata_t *);

/*
 * Returns a pointer to the title string, or NULL if the file did not
 * contain any title information.
 */
const char *	metadata_get_title(metadata_t *);

/*
 * Allocates and returns a meaningful string based on a metadata handle's
 * content. The result is what metadata_get_string() defaults to if an external
 * program is not used.
 */
char *		metadata_assemble_string(metadata_t *);

#endif /* __METADATA_H__ */
