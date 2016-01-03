#include "html.h"

#include "escape.h"

#include <string.h>
#include <assert.h>

static void *new_object(void *opaque) {
  hoedown_html_renderer_object *target = hoedown_malloc(sizeof(hoedown_html_renderer_object));
  target->ob = hoedown_buffer_new(64);
  return target;
}

static void free_object(void *target_, void *opaque) {
  hoedown_html_renderer_object *target = target_;
  hoedown_buffer_free(target->ob);
  free(target);
}

static void *object_get(int is_inline, hoedown_features ft, hoedown_preview_flags flags, void *parent_, const hoedown_renderer_data *data) {
  hoedown_html_renderer_state *state = data->opaque;
  hoedown_html_renderer_object *target = hoedown_pool_get(&state->objects);
  hoedown_html_renderer_object *parent = parent_;
  target->ob->size = 0;
  target->is_tight = (ft == HOEDOWN_FT_LIST) && (flags & HOEDOWN_PF_LIST_TIGHT);
  target->render_tags = (parent == NULL) || parent->render_tags;

  if ((ft == HOEDOWN_FT_LINK) && (flags & HOEDOWN_PF_LINK_IMAGE))
    target->render_tags = 0;
  return target;
}

static void object_merge(void *target_, void *content_, int is_inline, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;
  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  target->is_tight = content->is_tight;
}

static void object_pop(void *target_, int is_inline, const hoedown_renderer_data *data) {
  hoedown_html_renderer_state *state = data->opaque;
  hoedown_pool_pop(&state->objects, target_);
}

static void render_start(int is_inline, const hoedown_renderer_data *data) {
}

static void render_header(void *target_, int is_block, const hoedown_renderer_data *data) {
}

static void *render_end(void *target_, int is_inline, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;
  hoedown_html_renderer_state *state = data->opaque;
  hoedown_buffer *ob = target->ob;
  target->ob = hoedown_buffer_new(64);
  hoedown_pool_pop(&state->objects, target);

  // Render the footnotes
  if (state->footnote_count) {
    HOEDOWN_BUFPUTSL(ob, "\n<ul class=\"footnotes\">\n");
    hoedown_buffer_put(ob, state->footnotes->data, state->footnotes->size);
    HOEDOWN_BUFPUTSL(ob, "</ul>\n");
  }

  assert(state->objects.size == state->objects.isize);
  state->footnotes->size = 0;
  state->footnote_count = 0;
  return ob;
}


// BLOCK CONSTRUCTS

static void rndr_paragraph(void *target_, void *content_, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  if (target->is_tight) {
    hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
    hoedown_buffer_putc(target->ob, '\n');
  } else {
    HOEDOWN_BUFPUTSL(target->ob, "<p>");
    hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
    HOEDOWN_BUFPUTSL(target->ob, "</p>\n");
  }
}

static void rndr_indented_code_block(void *target_, const hoedown_buffer *code, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  HOEDOWN_BUFPUTSL(target->ob, "<pre><code>");
  hoedown_escape_html(target->ob, code->data, code->size);
  HOEDOWN_BUFPUTSL(target->ob, "</code></pre>\n");
}

static void rndr_fenced_code_block(void *target_, const hoedown_buffer *code, const hoedown_buffer *info, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;
  HOEDOWN_BUFPUTSL(target->ob, "<pre><code");

  if (info) {
    size_t first_word = 0;
    while (first_word < info->size && info->data[first_word] != ' ') first_word++;

    HOEDOWN_BUFPUTSL(target->ob, " class=\"language-");
    hoedown_escape_html(target->ob, info->data, first_word);
    hoedown_buffer_putc(target->ob, '"');
  }

  hoedown_buffer_putc(target->ob, '>');
  hoedown_escape_html(target->ob, code->data, code->size);
  HOEDOWN_BUFPUTSL(target->ob, "</code></pre>\n");
}

static void rndr_horizontal_rule(void *target_, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  HOEDOWN_BUFPUTSL(target->ob, "<hr />\n");
}

static void rndr_atx_header(void *target_, void *content_, size_t width, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  hoedown_buffer_printf(target->ob, "<h%lu>", width);
  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  hoedown_buffer_printf(target->ob, "</h%lu>\n", width);
}

static void rndr_setext_header(void *target_, void *content_, int is_double, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  hoedown_buffer_put(target->ob, (uint8_t *)(is_double ? "<h1>" : "<h2>"), 4);
  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  hoedown_buffer_put(target->ob, (uint8_t *)(is_double ? "</h1>\n" : "</h2>\n"), 6);
}

static void rndr_list(void *target_, void *content_, int is_ordered, int is_tight, int start, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  hoedown_buffer_put(target->ob, (uint8_t *)(is_ordered ? "<ol" : "<ul"), 3);
  if (start && start != 1) hoedown_buffer_printf(target->ob, " start=\"%d\"", start);
  HOEDOWN_BUFPUTSL(target->ob, ">\n");

  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  hoedown_buffer_put(target->ob, (uint8_t *)(is_ordered ? "</ol>\n" : "</ul>\n"), 6);
}

static void rndr_list_item(void *target_, void *content_, int is_ordered, int is_tight, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  HOEDOWN_BUFPUTSL(target->ob, "<li>");
  if (!is_tight) hoedown_buffer_putc(target->ob, '\n');

  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  if (is_tight && target->ob->data[target->ob->size-1] == '\n') target->ob->size--;

  HOEDOWN_BUFPUTSL(target->ob, "</li>\n");
}

static void rndr_quote_block(void *target_, void *content_, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  HOEDOWN_BUFPUTSL(target->ob, "<blockquote>\n");
  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  HOEDOWN_BUFPUTSL(target->ob, "</blockquote>\n");
}

static void rndr_html_block(void *target_, const hoedown_buffer *html, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  hoedown_buffer_put(target->ob, html->data, html->size);
}


// SHARED LOGIC

static void render_footnote(hoedown_buffer *ob, hoedown_buffer *content, hoedown_html_renderer_state *state) {
  int num = ++state->footnote_count;

  // Render the footnote reference
  hoedown_buffer_printf(ob, "<sup id=\"fnref-%1$d\"><a href=\"#fn-%1$d\" rel=\"footnote\">%1$d</a></sup>", num);//FIXME

  // Render the footnote itself
  hoedown_buffer_printf(state->footnotes, "<li id=\"fn-%1$d\">", num);
  hoedown_buffer_put(state->footnotes, content->data, content->size);
  hoedown_buffer_printf(state->footnotes, "&nbsp;<a href=\"#fnref-%d\" rev=\"footnote\">&#8617;</a></li>\n", num);
}


// INLINE CONSTRUCTS

static void rndr_string(void *target_, const hoedown_buffer *text, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  hoedown_escape_html(target->ob, text->data, text->size);
}

static void rndr_escape(void *target_, uint8_t character, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  hoedown_escape_html(target->ob, &character, 1);
}

static void rndr_linebreak(void *target_, int is_hard, int is_soft, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "<br />\n");
  else hoedown_buffer_putc(target->ob, '\n');
}

static void rndr_uri_autolink(void *target_, const hoedown_buffer *uri, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  if (target->render_tags) {
    HOEDOWN_BUFPUTSL(target->ob, "<a href=\"");
    hoedown_escape_href(target->ob, uri->data, uri->size);
    HOEDOWN_BUFPUTSL(target->ob, "\">");
  }
  hoedown_escape_html(target->ob, uri->data, uri->size);
  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "</a>");
}

static void rndr_email_autolink(void *target_, const hoedown_buffer *email, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  if (target->render_tags) {
    HOEDOWN_BUFPUTSL(target->ob, "<a href=\"mailto:");
    hoedown_escape_href(target->ob, email->data, email->size);
    HOEDOWN_BUFPUTSL(target->ob, "\">");
  }
  hoedown_escape_html(target->ob, email->data, email->size);
  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "</a>");
}

static void rndr_html(void *target_, const hoedown_buffer *html, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  if (target->render_tags)
    hoedown_buffer_put(target->ob, html->data, html->size);
}

static void rndr_entity(void *target_, const hoedown_buffer *character, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  hoedown_escape_character(target->ob, character->data, character->size);
}

static void rndr_code(void *target_, const hoedown_buffer *code, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "<code>");
  hoedown_escape_html(target->ob, code->data, code->size);
  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "</code>");
}

static void rndr_emphasis(void *target_, void *content_, size_t width, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;
  int strongs = width / 2, em = width % 2;

  if (target->render_tags) {
    for (int i = 0; i < strongs; i++)
      HOEDOWN_BUFPUTSL(target->ob, "<strong>");
    if (em) HOEDOWN_BUFPUTSL(target->ob, "<em>");
  }

  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);

  if (target->render_tags) {
    if (em) HOEDOWN_BUFPUTSL(target->ob, "</em>");
    for (int i = 0; i < strongs; i++)
      HOEDOWN_BUFPUTSL(target->ob, "</strong>");
  }
}

static void rndr_link(void *target_, void *content_, const hoedown_buffer *dest, const hoedown_buffer *title, int is_image, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  if (!target->render_tags) {
    hoedown_escape_html(target->ob, content->ob->data, content->ob->size);
    return;
  }

  if (is_image) {
    HOEDOWN_BUFPUTSL(target->ob, "<img src=\"");
    hoedown_escape_href(target->ob, dest->data, dest->size);
    HOEDOWN_BUFPUTSL(target->ob, "\" alt=\"");
    if (target->render_tags)
      hoedown_escape_html(target->ob, content->ob->data, content->ob->size);
    else
      hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
    hoedown_buffer_putc(target->ob, '"');

    if (title) {
      HOEDOWN_BUFPUTSL(target->ob, " title=\"");
      hoedown_escape_html(target->ob, title->data, title->size);
      hoedown_buffer_putc(target->ob, '"');
    }

    HOEDOWN_BUFPUTSL(target->ob, " />");
  } else {
    HOEDOWN_BUFPUTSL(target->ob, "<a href=\"");
    hoedown_escape_href(target->ob, dest->data, dest->size);
    hoedown_buffer_putc(target->ob, '"');

    if (title) {
      HOEDOWN_BUFPUTSL(target->ob, " title=\"");
      hoedown_escape_html(target->ob, title->data, title->size);
      hoedown_buffer_putc(target->ob, '"');
    }

    hoedown_buffer_putc(target->ob, '>');
    hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
    HOEDOWN_BUFPUTSL(target->ob, "</a>");
  }
}

static void rndr_math(void *target_, const hoedown_buffer *math, int is_inline, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  hoedown_buffer_put(target->ob, (const uint8_t *)(is_inline ? "\\(" : "\\["), 2);
  hoedown_escape_html(target->ob, math->data, math->size);
  hoedown_buffer_put(target->ob, (const uint8_t *)(is_inline ? "\\)" : "\\]"), 2);
}

static void rndr_superscript(void *target_, void *content_, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "<sup>");
  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "</sup>");
}

static void rndr_strikethrough(void *target_, void *content_, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "<del>");
  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "</del>");
}

static void rndr_highlight(void *target_, void *content_, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;

  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "<mark>");
  hoedown_buffer_put(target->ob, content->ob->data, content->ob->size);
  if (target->render_tags) HOEDOWN_BUFPUTSL(target->ob, "</mark>");
}

static void rndr_sidenote(void *target_, void *content_, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_, *content = content_;
  hoedown_html_renderer_state *state = data->opaque;
  render_footnote(target->ob, content->ob, state);
}

static void rndr_emoji(void *target_, const hoedown_buffer *name, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  hoedown_buffer_putc(target->ob, ':');
  hoedown_escape_html(target->ob, name->data, name->size);
  hoedown_buffer_putc(target->ob, ':');
}

static void rndr_typography(void *target_, const hoedown_buffer *character, const hoedown_renderer_data *data) {
  hoedown_html_renderer_object *target = target_;

  hoedown_escape_html(target->ob, character->data, character->size);
}



// RENDERER INITIALIZATION

hoedown_renderer *hoedown_html_renderer_new() {
  static const hoedown_renderer temp = {
    NULL,

    rndr_paragraph,
    rndr_indented_code_block,
    rndr_fenced_code_block,
    rndr_horizontal_rule,
    rndr_atx_header,
    rndr_setext_header,
    rndr_list,
    rndr_list_item,
    rndr_quote_block,
    rndr_html_block,

    rndr_string,
    rndr_escape,
    rndr_linebreak,
    rndr_uri_autolink,
    rndr_email_autolink,
    rndr_html,
    rndr_entity,
    rndr_code,
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

    render_header,

    render_start,
    render_end,
  };

  hoedown_renderer *rndr = hoedown_malloc(sizeof(hoedown_renderer));
  hoedown_html_renderer_state *state = hoedown_malloc(sizeof(hoedown_html_renderer_state));

  memcpy(rndr, &temp, sizeof(hoedown_renderer));
  rndr->opaque = state;

  hoedown_pool_init(&state->objects, 16, new_object, free_object, NULL);

  state->footnote_count = 0;
  state->footnotes = hoedown_buffer_new(64);

  return rndr;
}

void hoedown_html_renderer_free(hoedown_renderer *rndr) {
  if (!rndr) return;

  hoedown_html_renderer_state *state = rndr->opaque;
  hoedown_pool_uninit(&state->objects);
  hoedown_buffer_free(state->footnotes);
  free(state);

  free(rndr);
}
