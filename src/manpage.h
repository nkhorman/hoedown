/*--------------------------------------------------------------------*
*
* Copyright (c) 2008, Natacha Porté
* Copyright (c) 2011, Vicent Martí
* Copyright (c) 2014, Xavier Mendez, Devin Torres and the Hoedown authors

* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.

* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*--------------------------------------------------------------------
*
* Copied from html.h and then modified (hacked!).
*
* Developed by;
*	Neal Horman - http://www.wanlink.com
*	Copyright (c) 2015 Neal Horman. All Rights Reserved
*
*	Redistribution and use in source and binary forms, with or without
*	modification, are permitted provided that the following conditions
*	are met:
*	1. Redistributions of source code must retain the above copyright
*	   notice, this list of conditions and the following disclaimer.
*	
*	THIS SOFTWARE IS PROVIDED BY NEAL HORMAN AND ANY CONTRIBUTORS ``AS IS'' AND
*	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*	ARE DISCLAIMED.  IN NO EVENT SHALL NEAL HORMAN OR ANY CONTRIBUTORS BE LIABLE
*	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
*	OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
*	OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
*	SUCH DAMAGE.
*
*--------------------------------------------------------------------*/

/* manpage.h - Manpage renderer and utilities */

#ifndef HOEDOWN_MANPAGE_H
#define HOEDOWN_MANPAGE_H

#include "document.h"
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif


/*************
 * CONSTANTS *
 *************/

	/*
typedef enum hoedown_manpage_flags {
	HOEDOWN_HTML_SKIP_HTML = (1 << 0),
	HOEDOWN_HTML_ESCAPE = (1 << 1),
	HOEDOWN_HTML_HARD_WRAP = (1 << 2),
	HOEDOWN_HTML_USE_XHTML = (1 << 3)
} hoedown_manpage_flags;

typedef enum hoedown_manpage_tag {
	HOEDOWN_HTML_TAG_NONE = 0,
	HOEDOWN_HTML_TAG_OPEN,
	HOEDOWN_HTML_TAG_CLOSE
} hoedown_manpage_tag;
*/

typedef struct _manpage_TH_t
{
	const char *title;	/* Header - left */
	const char *section;	/* Header - right */
	const char *extra1;	/* Footer - center */
	const char *extra2;	/* Footer - left */
	const char *extra3;	/* Header - center */
} manpage_TH_t;


/*********
 * TYPES *
 *********/

struct hoedown_manpage_renderer_state {
	void *opaque;

	struct {
		int header_count;
		int current_level;
		int level_offset;
		int nesting_level;
	} toc_data;

	manpage_TH_t *pMpth;

/*	hoedown_manpage_flags flags;*/

	/* extra callbacks */
	void (*link_attributes)(hoedown_buffer *ob, const hoedown_buffer *url, const hoedown_renderer_data *data);
};
typedef struct hoedown_manpage_renderer_state hoedown_manpage_renderer_state;


/*************
 * FUNCTIONS *
 *************/

/* hoedown_manpage_renderer_new: allocates a ManPage renderer */
hoedown_renderer *hoedown_manpage_renderer_new(
	manpage_TH_t *pMpth,
	int nesting_level
) __attribute__ ((malloc));

/* hoedown_manpage_renderer_free: deallocate a ManPage renderer */
void hoedown_manpage_renderer_free(hoedown_renderer *renderer);


#ifdef __cplusplus
}
#endif

#endif /** HOEDOWN_MANPAGE_H **/
