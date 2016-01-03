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
#include <assert.h>

#include "escape.h"

#define USE_XHTML(opt) 0

#define TODO_CODEME_EXIT { fprintf(stderr,"%s:%s:%u This hasn't been coded yet.\n", __FILE__, __func__, __LINE__); fflush(stderr); exit(1); }

static void *new_object(void *opaque)
{
	hoedown_manpage_renderer_object *target = hoedown_malloc(sizeof(hoedown_manpage_renderer_object));
	target->ob = hoedown_buffer_new(64);

	return target;
}

static void free_object(void *target_, void *opaque)
{
	hoedown_manpage_renderer_object *target = target_;
	hoedown_buffer_free(target->ob);
	free(target);
}

static void *object_get(int is_inline, hoedown_features ft, hoedown_preview_flags flags, void *parent_, const hoedown_renderer_data *data)
{
	hoedown_manpage_renderer_state *state = data->opaque;
	hoedown_manpage_renderer_object *target = hoedown_pool_get(&state->objects);
	hoedown_manpage_renderer_object *parent = parent_;
	target->ob->size = 0;
	target->is_tight = (ft == HOEDOWN_FT_LIST) && (flags & HOEDOWN_PF_LIST_TIGHT);
	target->render_tags = (parent == NULL) || parent->render_tags;

	if ((ft == HOEDOWN_FT_LINK) && (flags & HOEDOWN_PF_LINK_IMAGE))
		target->render_tags = 0;

	return target;
}

static void object_merge(void *target_, void *content_, int is_inline, const hoedown_renderer_data *data)
{
	hoedown_manpage_renderer_object *target = target_, *content = content_;
	hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
	target->is_tight = content->is_tight;
}

static void object_pop(void *target_, int is_inline, const hoedown_renderer_data *data)
{
	hoedown_manpage_renderer_state *state = data->opaque;
	hoedown_pool_pop(&state->objects, target_);
}

static void render_start(int is_inline, const hoedown_renderer_data *data)
{
}

static void *render_end(void *target_, int is_inline, const hoedown_renderer_data *data)
{
	hoedown_manpage_renderer_object *target = target_;
	hoedown_manpage_renderer_state *state = data->opaque;
	hoedown_buffer *ob = target->ob;
	target->ob = hoedown_buffer_new(64);
	hoedown_pool_pop(&state->objects, target);

	/*
	// Render the footnotes
	if (state->footnote_count)
	{
		HOEDOWN_BUFPUTSL(ob, "\n<ul class=\"footnotes\">\n");
		hoedown_buffer_put(ob, state->footnotes->data, state->footnotes->size);
		HOEDOWN_BUFPUTSL(ob, "</ul>\n");
	}
	*/

	assert(state->objects.size == state->objects.isize);
	state->footnotes->size = 0;
	state->footnote_count = 0;

	return ob;
}

// ------------
static void escape_html(hoedown_buffer *ob, const uint8_t *source, size_t length)
{
	/*hoedown_escape_html(ob, source, length, 0);*/
	hoedown_buffer_put(ob, source, length);
}

static void escape_href(hoedown_buffer *ob, const uint8_t *source, size_t length)
{
	/*hoedown_escape_href(ob, source, length);*/
	hoedown_buffer_put(ob, source, length);
}

static void rndr_autolink(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	hoedown_buffer_put(ob, text->data, text->size);
}

static void rndr_linebreak(void *target_, int is_hard, int is_soft, const hoedown_renderer_data *data);

static void rndr_fenced_blockcode(void *target_, const hoedown_buffer *text, const hoedown_buffer *info, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	HOEDOWN_BUFPUTSL(ob, ".RS\n");
	if (text)
	{	size_t i = 0;
		size_t org;

		while (i < text->size)
		{
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

			HOEDOWN_BUFPUTSL(ob, "\n.br\n");
			//rndr_linebreak(target_, 0, 0, data);
			i++;
		}
	}
	HOEDOWN_BUFPUTSL(ob, "\n.RE\n");
}

static void rndr_blockcode(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	rndr_fenced_blockcode(target_, text, NULL, data);
}

static void rndr_blockquote(void *target_, void *text_, const hoedown_renderer_data *data)
{
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;
	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;

	HOEDOWN_BUFPUTSL(ob, "\n.PP\n.RS\n");
	size_t size = text->size;
	while (size && text->data[size - 1] == '\n')
		size--;

	hoedown_buffer_put(ob, text->data, size);
	HOEDOWN_BUFPUTSL(ob, "\n.RE\n");
}

static void rndr_codespan(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	HOEDOWN_BUFPUTSL(ob, "\\fB\\fC");
	escape_html(ob, text->data, text->size);
	HOEDOWN_BUFPUTSL(ob, "\\fR");
}

static void rndr_strikethrough(void *target_, void *text_, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;
	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;

	HOEDOWN_BUFPUTSL(ob, "<del>");
	hoedown_buffer_put(ob, text->data, text->size);
	HOEDOWN_BUFPUTSL(ob, "</del>");
	*/
}

static void rndr_emphasis(void *target_, void *text_, size_t width, const hoedown_renderer_data *data)
{
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;

	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	char const *pfmt = NULL;

	switch(width)
	{
		case 1:
		default:
			pfmt = "\\fI%*.*s\\fP";
			break;
		case 2:
			pfmt = "\\fB%*.*s\\fP";
			break;
		case 3:
			pfmt = "\\s+2%*.*s\\s-2";
			break;
	}

	if(pfmt != NULL)
		hoedown_buffer_printf(ob, pfmt, text->size, text->size, text->data);
}

static void rndr_highlight(void *target_, void *text_, const hoedown_renderer_data *data)
{
	rndr_emphasis(target_, text_, 2, data);
	/*
	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;
	hoedown_buffer_printf(ob, "\\fB%*.*s\\fP", text->size, text->size, text->data);
	*/
}

static void rndr_linebreak(void *target_, int is_hard, int is_soft, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;

	char *pfmt = (is_hard ? "\n.P\n" : is_soft ? "\n" : "\n.br\n");
	hoedown_buffer_put(ob, pfmt, strlen(pfmt));
}

static void rndr_header(void *target_, void *text_, int level, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;

	switch(level)
	{
		case 2:
			hoedown_buffer_printf(ob, ".SH \"%*.*s\"\n", (int)text->size, (int)text->size, text->data);
			break;
		case 3:
			hoedown_buffer_printf(ob, ".IP \"%*.*s\" %u\n", (int)text->size, (int)text->size, text->data, level);
			break;
	}
}

static void rndr_atx_header(void *target_, void *content_, size_t width, const hoedown_renderer_data *data)
{
	rndr_header(target_, content_, width, data);
}

static void rndr_setext_header(void *target_, void *content_, int is_double, const hoedown_renderer_data *data)
{
	rndr_header(target_, content_, 2 + (is_double != 0), data);
}

static void rndr_link(void *target_, void *text_, const hoedown_buffer *link, const hoedown_buffer *title, int is_image, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;
	//hoedown_manpage_renderer_state *state = data->opaque;

	if (title && title->size)
		escape_html(ob, title->data, title->size);
	else if (text && text->size)
		hoedown_buffer_put(ob, text->data, text->size);

	//if (state->link_attributes) state->link_attributes(ob, link, data);

	if (link && link->size)
	{
		if((title && title->size) || (text && text->size))
			HOEDOWN_BUFPUTSL(ob, " ");
		escape_href(ob, link->data, link->size);
	}
}

static void rndr_list(void *target_, void *text_, int is_ordered, int is_tight, int start, const hoedown_renderer_data *data)
{
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;

	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	HOEDOWN_BUFPUTSL(ob, ".RS\n");
	hoedown_buffer_put(ob, text->data, text->size);
	HOEDOWN_BUFPUTSL(ob, ".RE");
	rndr_linebreak(target_, 0, 0, data);
}

static void rndr_listitem(void *target_, void *text_, int is_ordered, int is_tight, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;
	size_t size = (text ? text->size : 0);
	size_t f=0,l=0;

	while (size && text->data[size - 1] == '\n')
		size--;

	while(l<size)
	{
		while(l<size && text->data[l] != '\n')
			l++;
		hoedown_buffer_printf(ob, ".IP \\(bu 2\n%*.*s\n", (int)(l-f), (int)(l-f), &text->data[f]);
		l++;
		f=l;
	}
}

static void rndr_paragraph(void *target_, void *text_, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;

	size_t i = 0;

	if (!text || !text->size)
		return;

	while (i < text->size && isspace(text->data[i]))
		i++;

	if (i == text->size)
		return;

	hoedown_buffer_printf(ob, "%*.*s\n", (int)(text->size-i), (int)(text->size-i), text->data+i);
}

static void rndr_raw_block(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	if(text && text->size)
	{
		size_t org = 0, sz = text->size;

		while (sz > 0 && text->data[sz - 1] == '\n')
			sz--;

		while (org < sz && text->data[org] == '\n')
			org++;

		if (org < sz)
		{
			hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;

			if (ob->size)
				hoedown_buffer_putc(ob, '\n');

			hoedown_buffer_put(ob, text->data + org, sz - org);
			hoedown_buffer_putc(ob, '\n');
		}
	}
}

static void rndr_hrule(void *target_, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	//hoedown_manpage_renderer_state *state = data->opaque;
	if (ob->size)
		HOEDOWN_BUFPUTSL(ob, "\n.ti 0\n\\l'\\n(.lu'\n");
	//hoedown_buffer_puts(ob, USE_XHTML(state) ? "<hr/>\n" : "<hr>\n");
}

static void rndr_raw_html(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;
	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	hoedown_buffer_put(ob, text->data, text->size);
	*/
}

static void rndr_superscript(void *target_, void *text_, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
	/*
	hoedown_buffer *text = ((hoedown_manpage_renderer_object *)text_)->ob;
	if (!text || !text->size)
		return;

	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
	HOEDOWN_BUFPUTSL(ob, "<sup>");
	hoedown_buffer_put(ob, text->data, text->size);
	HOEDOWN_BUFPUTSL(ob, "</sup>");
	*/
}

static void rndr_normal_text(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;

	if (text && text->size)
		hoedown_buffer_put(ob, text->data, text->size);
}

static void rndr_escape(void *target_, uint8_t character, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
}

static void rndr_entity(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
}

static void rndr_math(void *target_, const hoedown_buffer *text, int is_inline, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
}

static void rndr_sidenote(void *target_, void *text_, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
}

static void rndr_emoji(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
}

static void rndr_typography(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data)
{
	TODO_CODEME_EXIT
}

static void rndr_doc_header(void *target_, int is_block, const hoedown_renderer_data *data)
{
	hoedown_buffer *ob = ((hoedown_manpage_renderer_object *)target_)->ob;
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
hoedown_manpage_renderer_new(manpage_TH_t *pMpth)
{
	static const hoedown_renderer cb_default = {
		NULL,

		rndr_paragraph,
		rndr_blockcode,
		rndr_fenced_blockcode,
		rndr_hrule,
		rndr_atx_header,
		rndr_setext_header,
		rndr_list,
		rndr_listitem,
		rndr_blockquote,
		rndr_raw_block,

		rndr_normal_text, // string
		rndr_escape,
		rndr_linebreak,
		rndr_autolink, // uri link
		rndr_autolink, // email link
		rndr_raw_html,
		rndr_entity,
		rndr_codespan, // code
		rndr_emphasis,
		rndr_link,
		rndr_math,
		rndr_superscript,
		rndr_strikethrough,
		rndr_highlight,
		rndr_sidenote,
		rndr_emoji,
		rndr_typography,

		object_get,
		object_merge,
		object_pop,

		rndr_doc_header,

		render_start,
		render_end,
	};

	hoedown_manpage_renderer_state *state;
	hoedown_renderer *renderer;

	/* Prepare the state pointer */
	state = hoedown_malloc(sizeof(hoedown_manpage_renderer_state));
	memset(state, 0x0, sizeof(hoedown_manpage_renderer_state));

	state->pMpth = pMpth;

	/* Prepare the renderer */
	renderer = hoedown_malloc(sizeof(hoedown_renderer));
	memcpy(renderer, &cb_default, sizeof(hoedown_renderer));

	renderer->opaque = state;
	hoedown_pool_init(&state->objects, 16, new_object, free_object, NULL);

	state->footnote_count = 0;
	state->footnotes = hoedown_buffer_new(64);

	return renderer;
}

void
hoedown_manpage_renderer_free(hoedown_renderer *renderer)
{
	if(!renderer)
		return;

	hoedown_manpage_renderer_state *state = renderer->opaque;
	hoedown_pool_uninit(&state->objects);
	hoedown_buffer_free(state->footnotes);
	free(state);

	free(renderer);
}
