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
* Copied from html.c and then modified (hacked!).
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

#include "manpage.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "escape.h"

#define USE_XHTML(opt) 0

#define TODO_CODEME_EXIT { fprintf(stderr,"%s:%s:%u This hasn't been coded yet.\n", __FILE__, __func__, __LINE__); fflush(stderr); exit(1); }

static void escape_html(hoedown_buffer *ob, const uint8_t *source, size_t length)
{
	/*hoedown_escape_html(ob, source, length, 0);*/
	hoedown_buffer_put(ob, source, length);
}

/*
static void escape_href(hoedown_buffer *ob, const uint8_t *source, size_t length)
{
	/ *hoedown_escape_href(ob, source, length);* /
	hoedown_buffer_put(ob, source, length);
}
*/

static int
rndr_autolink(hoedown_buffer *ob, const hoedown_buffer *link, hoedown_autolink_type type, const hoedown_renderer_data *data)
{
	if (!link || !link->size)
		return 0;

	hoedown_buffer_put(ob, link->data, link->size);

	return 1;
}

static int rndr_linebreak(hoedown_buffer *ob, const hoedown_renderer_data *data);

static void
rndr_blockcode(hoedown_buffer *ob, const hoedown_buffer *text, const hoedown_buffer *lang, const hoedown_renderer_data *data)
{
	HOEDOWN_BUFPUTSL(ob, ".RS\n");
	if (text)
	{	size_t i = 0;
		size_t org;

		while (i < text->size) {
			org = i;
			while (i < text->size && text->data[i] != '\n')
				i++;

			if (i > org)
				hoedown_buffer_put(ob, text->data + org, i - org);

			/*
			 * do not insert a line break if this newline
			 * is the last character on the paragraph
			 */
			if (i >= text->size - 1)
				break;

			rndr_linebreak(ob, data);
			i++;
		}
	}
	HOEDOWN_BUFPUTSL(ob, "\n.RE\n");
}

static void
rndr_blockquote(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	HOEDOWN_BUFPUTSL(ob, "<blockquote>\n");
	if (content) hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</blockquote>\n");
	*/
}

static int
rndr_codespan(hoedown_buffer *ob, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	HOEDOWN_BUFPUTSL(ob, ".RS\n");
	if (text)
		escape_html(ob, text->data, text->size);
	HOEDOWN_BUFPUTSL(ob, ".RE\n");
	return 1;
}

static int
rndr_strikethrough(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (!content || !content->size)
		return 0;

	HOEDOWN_BUFPUTSL(ob, "<del>");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</del>");
	*/
	return 1;
}

static int
rndr_double_emphasis(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	HOEDOWN_BUFPUTSL(ob, "\n.B ");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "\n");

	return 1;
}

static int
rndr_emphasis(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	if (!content || !content->size)
		return 0;
	HOEDOWN_BUFPUTSL(ob, "\n.B ");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "\n");

	return 1;
}

static int
rndr_underline(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	if (!content || !content->size)
		return 0;

	HOEDOWN_BUFPUTSL(ob, "\n.U ");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "\n");

	return 1;
}

static int
rndr_highlight(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (!content || !content->size)
		return 0;

	HOEDOWN_BUFPUTSL(ob, "<mark>");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</mark>");
	*/

	return 1;
}

static int
rndr_quote(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (!content || !content->size)
		return 0;

	HOEDOWN_BUFPUTSL(ob, "<q>");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</q>");
	*/

	return 1;
}

static int
rndr_linebreak(hoedown_buffer *ob, const hoedown_renderer_data *data)
{
	HOEDOWN_BUFPUTSL(ob, "\n.br\n");
	return 1;
}

static void
rndr_header(hoedown_buffer *ob, const hoedown_buffer *content, int level, const hoedown_renderer_data *data)
{
	switch(level)
	{
		case 2:
			hoedown_buffer_printf(ob, ".SH \"%*.*s\"\n", content->size, content->size, content->data);
			break;
		case 3:
			hoedown_buffer_printf(ob, ".IP \"%*.*s\" %u\n", content->size, content->size, content->data, level);
			break;
	}
}

static int
rndr_link(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_buffer *link, const hoedown_buffer *title, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	hoedown_manpage_renderer_state *state = data->opaque;

	HOEDOWN_BUFPUTSL(ob, "<a href=\"");

	if (link && link->size)
		escape_href(ob, link->data, link->size);

	if (title && title->size) {
		HOEDOWN_BUFPUTSL(ob, "\" title=\"");
		escape_html(ob, title->data, title->size);
	}

	if (state->link_attributes) {
		hoedown_buffer_putc(ob, '\"');
		state->link_attributes(ob, link, data);
		hoedown_buffer_putc(ob, '>');
	} else {
		HOEDOWN_BUFPUTSL(ob, "\">");
	}

	if (content && content->size) hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</a>");
	*/

	return 1;
}

static void
rndr_list(hoedown_buffer *ob, const hoedown_buffer *content, hoedown_list_flags flags, const hoedown_renderer_data *data)
{
	if (!content || !content->size)
		return;

	hoedown_buffer_put(ob, content->data, content->size);
}

static void
rndr_listitem(hoedown_buffer *ob, const hoedown_buffer *content, hoedown_list_flags flags, const hoedown_renderer_data *data)
{
	size_t size = (content ? content->size : 0);
	size_t f=0,l=0;

	while (size && content->data[size - 1] == '\n')
		size--;

	while(l<size)
	{
		while(l<size && content->data[l] != '\n')
			l++;
		hoedown_buffer_printf(ob, "%*.*s", l-f, l-f, &content->data[f]);
		rndr_linebreak(ob, data);
		l++;
		f=l;
	}
}

static void
rndr_paragraph(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	size_t i = 0;

	if (!content || !content->size)
		return;

	while (i < content->size && isspace(content->data[i])) i++;

	if (i == content->size)
		return;

	hoedown_buffer_printf(ob, "%*.*s\n", content->size-i, content->size-i, content->data+i);
}

static void
rndr_raw_block(hoedown_buffer *ob, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	size_t org, sz;

	if (!text)
		return;

	/* FIXME: Do we *really* need to trim the HTML? How does that make a difference? */
	sz = text->size;
	while (sz > 0 && text->data[sz - 1] == '\n')
		sz--;

	org = 0;
	while (org < sz && text->data[org] == '\n')
		org++;

	if (org >= sz)
		return;

	if (ob->size)
		hoedown_buffer_putc(ob, '\n');

	hoedown_buffer_put(ob, text->data + org, sz - org);
	hoedown_buffer_putc(ob, '\n');
}

static void
rndr_hrule(hoedown_buffer *ob, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	hoedown_manpage_renderer_state *state = data->opaque;
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	hoedown_buffer_puts(ob, USE_XHTML(state) ? "<hr/>\n" : "<hr>\n");
	*/
}

static int
rndr_raw_html(hoedown_buffer *ob, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	hoedown_buffer_put(ob, text->data, text->size);
	*/

	return 1;
}

static void
rndr_table(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	HOEDOWN_BUFPUTSL(ob, "<table>\n");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</table>\n");
	*/
}

static void
rndr_table_header(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	HOEDOWN_BUFPUTSL(ob, "<thead>\n");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</thead>\n");
	*/
}

static void
rndr_table_body(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (ob->size) hoedown_buffer_putc(ob, '\n');
	HOEDOWN_BUFPUTSL(ob, "<tbody>\n");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</tbody>\n");
	*/
}

static void
rndr_tablerow(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	HOEDOWN_BUFPUTSL(ob, "<tr>\n");
	if (content) hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</tr>\n");
	*/
}

static void
rndr_tablecell(hoedown_buffer *ob, const hoedown_buffer *content, hoedown_table_flags flags, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (flags & HOEDOWN_TABLE_HEADER) {
		HOEDOWN_BUFPUTSL(ob, "<th");
	} else {
		HOEDOWN_BUFPUTSL(ob, "<td");
	}

	switch (flags & HOEDOWN_TABLE_ALIGNMASK) {
	case HOEDOWN_TABLE_ALIGN_CENTER:
		HOEDOWN_BUFPUTSL(ob, " style=\"text-align: center\">");
		break;

	case HOEDOWN_TABLE_ALIGN_LEFT:
		HOEDOWN_BUFPUTSL(ob, " style=\"text-align: left\">");
		break;

	case HOEDOWN_TABLE_ALIGN_RIGHT:
		HOEDOWN_BUFPUTSL(ob, " style=\"text-align: right\">");
		break;

	default:
		HOEDOWN_BUFPUTSL(ob, ">");
	}

	if (content)
		hoedown_buffer_put(ob, content->data, content->size);

	if (flags & HOEDOWN_TABLE_HEADER) {
		HOEDOWN_BUFPUTSL(ob, "</th>\n");
	} else {
		HOEDOWN_BUFPUTSL(ob, "</td>\n");
	}
	*/
}

static int
rndr_superscript(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	if (!content || !content->size) return 0;
	HOEDOWN_BUFPUTSL(ob, "<sup>");
	hoedown_buffer_put(ob, content->data, content->size);
	HOEDOWN_BUFPUTSL(ob, "</sup>");
	*/

	return 1;
}

static void
rndr_normal_text(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	if (content)
		escape_html(ob, content->data, content->size);
}

static void
rndr_footnotes(hoedown_buffer *ob, const hoedown_buffer *content, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	hoedown_manpage_renderer_state *state = data->opaque;

	if (ob->size) hoedown_buffer_putc(ob, '\n');
	HOEDOWN_BUFPUTSL(ob, "<div class=\"footnotes\">\n");
	hoedown_buffer_puts(ob, USE_XHTML(state) ? "<hr/>\n" : "<hr>\n");
	HOEDOWN_BUFPUTSL(ob, "<ol>\n");

	if (content) hoedown_buffer_put(ob, content->data, content->size);

	HOEDOWN_BUFPUTSL(ob, "\n</ol>\n</div>\n");
	*/
}

static void
rndr_footnote_def(hoedown_buffer *ob, const hoedown_buffer *content, unsigned int num, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	size_t i = 0;
	int pfound = 0;

	/ * insert anchor at the end of first paragraph block * /
	if (content) {
		while ((i+3) < content->size) {
			if (content->data[i++] != '<') continue;
			if (content->data[i++] != '/') continue;
			if (content->data[i++] != 'p' && content->data[i] != 'P') continue;
			if (content->data[i] != '>') continue;
			i -= 3;
			pfound = 1;
			break;
		}
	}

	hoedown_buffer_printf(ob, "\n<li id=\"fn%d\">\n", num);
	if (pfound) {
		hoedown_buffer_put(ob, content->data, i);
		hoedown_buffer_printf(ob, "&nbsp;<a href=\"#fnref%d\" rev=\"footnote\">&#8617;</a>", num);
		hoedown_buffer_put(ob, content->data + i, content->size - i);
	} else if (content) {
		hoedown_buffer_put(ob, content->data, content->size);
	}
	HOEDOWN_BUFPUTSL(ob, "</li>\n");
	*/
}

static int
rndr_footnote_ref(hoedown_buffer *ob, unsigned int num, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	hoedown_buffer_printf(ob, "<sup id=\"fnref%d\"><a href=\"#fn%d\" rel=\"footnote\">%d</a></sup>", num, num, num);
	*/

	return 1;
}

static void rndr_doc_header(hoedown_buffer *ob, int inline_render, const hoedown_renderer_data *data)
{
	hoedown_manpage_renderer_state *state = data->opaque;
	manpage_TH_t *pMpth = state->pMpth;

	hoedown_buffer_printf(ob, ".TH %s %s \"%s\" \"%s\" \"%s\"\n"
		, pMpth->title ? pMpth->title : ""
		, pMpth->section ? pMpth->section : ""
		, pMpth->extra1 ? pMpth->extra1 : ""
		, pMpth->extra2 ? pMpth->extra2 : ""
		, pMpth->extra3 ? pMpth->extra3 : ""
		);
}

hoedown_renderer *
hoedown_manpage_renderer_new(manpage_TH_t *pMpth, int nesting_level)
{
	static const hoedown_renderer cb_default = {
		NULL,

		rndr_blockcode,
		rndr_blockquote,
		rndr_header,
		rndr_hrule,
		rndr_list,
		rndr_listitem,
		rndr_paragraph,
		rndr_table,
		rndr_table_header,
		rndr_table_body,
		rndr_tablerow,
		rndr_tablecell,
		rndr_footnotes,
		rndr_footnote_def,
		rndr_raw_block,

		rndr_autolink,
		rndr_codespan,
		rndr_double_emphasis,
		rndr_emphasis,
		rndr_underline,
		rndr_highlight,
		rndr_quote,
		NULL,
		rndr_linebreak,
		rndr_link,
		NULL,
		rndr_strikethrough,
		rndr_superscript,
		rndr_footnote_ref,
		NULL,
		rndr_raw_html,

		NULL,
		rndr_normal_text,

		rndr_doc_header,
		NULL
	};

	hoedown_manpage_renderer_state *state;
	hoedown_renderer *renderer;

	/* Prepare the state pointer */
	state = hoedown_malloc(sizeof(hoedown_manpage_renderer_state));
	memset(state, 0x0, sizeof(hoedown_manpage_renderer_state));

	state->toc_data.nesting_level = nesting_level;
	state->pMpth = pMpth;

	/* Prepare the renderer */
	renderer = hoedown_malloc(sizeof(hoedown_renderer));
	memcpy(renderer, &cb_default, sizeof(hoedown_renderer));

	renderer->opaque = state;
	return renderer;
}

void
hoedown_manpage_renderer_free(hoedown_renderer *renderer)
{
	free(renderer->opaque);
	free(renderer);
}
