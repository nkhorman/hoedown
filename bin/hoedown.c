#include "document.h"
#include "html.h"
#include "manpage.h"

#include "common.h"
#include "noop.h"

#include <time.h>


/* FEATURES INFO / DEFAULTS */

enum renderer_type {
  RENDERER_HTML,
  RENDERER_NOOP,
  RENDERER_MANPAGE
};

struct preset_info {
  hoedown_features flags;
  const char *option_name;
  const char *label;
  const char *description;
};

struct feature_category_info {
  hoedown_features flags;
  const char *option_name;
  const char *label;
};

struct feature_info {
  hoedown_features flag;
  const char *option_name;
  const char *description;
};

static struct preset_info presets_info[] = {
  {HOEDOWN_FT_COMMONMARK, "stmd", "CommonMark", "Parse standard CommonMark. The default."},
  {HOEDOWN_FT_MARKDOWN, "markdown", "Markdown", "Parse classic Markdown."},
  {0, "none", "None", "Start with all flags disabled."},
};

static struct feature_category_info categories_info[] = {
  {HOEDOWN_FT_BLOCK, "block", "Block construct features"},
  {HOEDOWN_FT_INLINE, "inline", "Inline construct features"},
  {HOEDOWN_FT_OTHER, "other", "Other features"},
  {HOEDOWN_FT_FLAGS, "flags", "Feature flags"},
};

static struct feature_info features_info[] = {
  {HOEDOWN_FT_DIRECTIVE, "directive", "Parse generic directive syntax."},
  {HOEDOWN_FT_INDENTED_CODE_BLOCK, "indented-code-block", "Parse indented code blocks."},
  {HOEDOWN_FT_FENCED_CODE_BLOCK, "fenced-code-block", "Parse fenced code blocks."},
  {HOEDOWN_FT_HORIZONTAL_RULE, "horizontal-rule", "Parse horizontal rules."},
  {HOEDOWN_FT_ATX_HEADER, "atx-header", "Parse ATX headers."},
  {HOEDOWN_FT_SETEXT_HEADER, "setext-header", "Parse Setext headers"},
  {HOEDOWN_FT_LIST, "list", "Parse bullet and numbered lists."},
  {HOEDOWN_FT_QUOTE_BLOCK, "quote-block", "Parse email-style quote blocks."},
  {HOEDOWN_FT_HTML_BLOCK, "html-block", "Parse raw HTML blocks."},
  {HOEDOWN_FT_TABLE, "table", "Parse Kramdown style tables."},
  {HOEDOWN_FT_FOOTNOTE, "footnote", "Parse Pandoc-style footnotes and footnote references."},

  {HOEDOWN_FT_ROLE, "role", "Parse generic role syntax."},
  {HOEDOWN_FT_ESCAPE, "escape", "Parse backslash escapes."},
  {HOEDOWN_FT_LINEBREAK, "linebreak", "Cut spaces around lines and parse two-space linebreaks."},
  {HOEDOWN_FT_URI_AUTOLINK, "uri-autolink", "Parse URI autolinks between angle brackets."},
  {HOEDOWN_FT_EMAIL_AUTOLINK, "email-autolink", "Parse email autolinks between angle brackets."},
  {HOEDOWN_FT_HTML, "inline-html", "Parse inline HTML."},
  {HOEDOWN_FT_ENTITY, "entity", "Parse HTML entities."},
  {HOEDOWN_FT_CODE, "code", "Parse code spans between backticks."},
  {HOEDOWN_FT_EMPHASIS, "emphasis", "Parse emphasis and strong emphasis."},
  {HOEDOWN_FT_LINK, "link", "Parse links and link references."},
  {HOEDOWN_FT_MATH, "math", "Parse Kramdown-style math spans."},
  {HOEDOWN_FT_SUPERSCRIPT, "superscript", "Parse superscript spans."},
  {HOEDOWN_FT_QUOTE, "quote", "Parse quote-delimited spans."},
  {HOEDOWN_FT_STRIKETHROUGH, "strikethrough", "Parse strikethrough spans."},
  {HOEDOWN_FT_HIGHLIGHT, "highlight", "Parse highlight spans."},
  {HOEDOWN_FT_SIDENOTE, "sidenote", "Parse Pandoc-style inline footnotes."},
  {HOEDOWN_FT_EMOJI, "emoji", "Parse emoji codes between colons."},
  {HOEDOWN_FT_TYPOGRAPHY, "typography", "Parse ASCII punctuation and symbols such as `--` or `(c)`."},

  {HOEDOWN_FT_PREPROCESS, "preprocess", "Preprocess the input. Hoedown can give bad results if input is not preprocessed."},

  {HOEDOWN_FT_LINEBREAK_HARD, "linebreak-hard", "Requires --linebreak. Parse backslash-style linebreaks."},
  {HOEDOWN_FT_LINEBREAK_SOFT, "linebreak-soft", "Requires --linebreak. Parse all newlines as linebreaks."},
  {HOEDOWN_FT_LINK_IMAGE, "link-image", "Requires --link. Allow links preceded by a bang to be rendered as images."},
  {HOEDOWN_FT_INTRA_EMPHASIS, "intra-emphasis", "Requires --emphasis. Allow underscores between words to be parsed as emphasis."},
  {HOEDOWN_FT_MATH_EXPLICIT, "math-explicit", "Requires --math. Enables parsing $...$ as inline math and restricts $$...$$ to block math."},
};

static const char *category_prefix = "all-";
static const char *negative_prefix = "no-";

#define DEF_IUNIT 1024
#define DEF_OUNIT 64
#define DEF_MAX_NESTING 16


/* PRINT HELP */

static void print_feature_option(struct feature_info *feature) {
  size_t i;

  /* prepare the description */
  char *desc = malloc(strlen(feature->description)+40);
  memcpy(desc, feature->description, strlen(feature->description)+1);

  /* add tags to the description */
  for (i = 0; i < count_of(presets_info); i++)
    if (feature->flag & presets_info[i].flags) {
      strcat(desc, " [");
      strcat(desc, presets_info[i].label);
      strcat(desc, "]");
    }

  /* print feature option */
  print_option(  0, feature->option_name, desc);
}

static void print_help(const char *basename) {
  size_t i, e;

  /* usage */
  printf("Usage: %s [OPTION]... [FILE]\n\n", basename);

  /* description */
  printf("Process the Markdown in FILE (or standard input) and render it to standard output, using the Hoedown library. "
         "Parsing and rendering can be customized through the options below. The default is to parse pure CommonMark and output HTML.\n\n");

  /* main options */
  printf("Main options:\n");
  print_option('n', "max-nesting=N", "Maximum level of block nesting parsed. Default is " str(DEF_MAX_NESTING) ".");
  print_option(  0, "html", "Render (X)HTML. The default.");
  print_option(  0, "noop", "Use a special renderer whose callbacks do nothing.");
  print_option('T', "time", "Show time spent in rendering.");
  print_option('i', "input-unit=N", "Reading block size. Default is " str(DEF_IUNIT) ".");
  print_option('o', "output-unit=N", "Writing block size. Default is " str(DEF_OUNIT) ".");
  print_option(  0, "block", "Parse block-level Markdown. The default.");
  print_option(  0, "inline", "Parse inline-level Markdown.");
  print_option('h', "help", "Print this help text.");
  print_option('v', "version", "Print Hoedown version.");
  print_option(  0, "man", "Render Manpage.");
  print_option(  0, "man-th-t", "Manpage TitleHeader Title");
  print_option(  0, "man-th-s", "Manpage TitleHeader Section");
  print_option(  0, "man-th-e1", "Manpage TitleHeader Extra 1");
  print_option(  0, "man-th-e2", "Manpage TitleHeader Extra 2");
  print_option(  0, "man-th-e3", "Manpage TitleHeader Extra 3");
  printf("\n");

  /* base presets */
  printf("Base presets:\n");
  for (i = 0; i < count_of(presets_info); i++) {
    struct preset_info *preset = &presets_info[i];
    print_option(  0, preset->option_name, preset->description);
  }
  printf("\n");

  /* features */
  for (i = 0; i < count_of(categories_info); i++) {
    struct feature_category_info *category = &categories_info[i];
    printf("%s (--%s%s):\n", category->label, category_prefix, category->option_name);
    for (e = 0; e < count_of(features_info); e++) {
      struct feature_info *feature = &features_info[e];
      if (feature->flag & category->flags)
        print_feature_option(feature);
    }
    printf("\n");
  }

  /* ending */
  printf("Feature flags can be negated by prepending 'no' to them, as in '--no-table', '--no-flags' or '--no-escape'. "
         "Base presets override any flag given previously. Options are processed in order, so in case of contradictory options the last specified stands.\n\n");

  printf("When FILE is '-', read standard input. If no FILE was given, read standard input. Use '--' to signal end of option parsing. "
         "Exit status is 0 if no errors occured, 1 with option parsing errors, 4 with memory allocation errors or 5 with I/O errors.\n\n");
}


/* OPTION PARSING */

struct option_data {
  char *basename;
  int done;

  /* time reporting */
  int show_time;

  /* I/O */
  size_t iunit;
  const char *filename;

  /* renderer */
  enum renderer_type renderer;
  manpage_TH_t mpth;

  /* parsing */
  int is_block;
  hoedown_features features;
  size_t max_nesting;
};

static int parse_short_option(char opt, char *next, void *opaque) {
  struct option_data *data = opaque;
  long int num;
  int isNum = next ? parseint(next, &num) : 0;

  if (opt == 'h') {
    print_help(data->basename);
    data->done = 1;
    return 0;
  }

  if (opt == 'v') {
    print_version();
    data->done = 1;
    return 0;
  }

  if (opt == 'T') {
    data->show_time = 1;
    return 1;
  }

  /* options requiring value */
  /* FIXME: add validation */

  if (opt == 'n' && isNum) {
    data->max_nesting = num;
    return 2;
  }

  if (opt == 'i' && isNum) {
    data->iunit = num;
    return 2;
  }

  fprintf(stderr, "Wrong option '-%c' found.\n", opt);
  return 0;
}

static int parse_preset_option(char *opt, struct option_data *data) {
  size_t i;

  for (i = 0; i < count_of(presets_info); i++) {
    struct preset_info *preset = &presets_info[i];
    if (strcmp(opt, preset->option_name)==0) {
      data->features = preset->flags;
      return 1;
    }
  }

  return 0;
}

static int parse_category_option(char *opt, struct option_data *data) {
  size_t i;
  const char *name = strprefix(opt, category_prefix);
  if (!name) return 0;

  for (i = 0; i < count_of(categories_info); i++) {
    struct feature_category_info *category = &categories_info[i];
    if (strcmp(name, category->option_name)==0) {
      data->features |= category->flags;
      return 1;
    }
  }

  return 0;
}

static int parse_flag_option(char *opt, struct option_data *data) {
  size_t i;

  for (i = 0; i < count_of(features_info); i++) {
    struct feature_info *feature = &features_info[i];
    if (strcmp(opt, feature->option_name)==0) {
      data->features |= feature->flag;
      return 1;
    }
  }

  return 0;
}

static int parse_negative_option(char *opt, struct option_data *data) {
  size_t i;
  const char *name = strprefix(opt, negative_prefix);
  if (!name) return 0;

  for (i = 0; i < count_of(categories_info); i++) {
    struct feature_category_info *category = &categories_info[i];
    if (strcmp(name, category->option_name)==0) {
      data->features &= ~(category->flags);
      return 1;
    }
  }

  for (i = 0; i < count_of(features_info); i++) {
    struct feature_info *feature = &features_info[i];
    if (strcmp(name, feature->option_name)==0) {
      data->features &= ~(feature->flag);
      return 1;
    }
  }

  return 0;
}

static int parse_long_option(char *opt, char *next, void *opaque) {
  struct option_data *data = opaque;
  long int num;
  int isNum = next ? parseint(next, &num) : 0;

  if (strcmp(opt, "help")==0) {
    print_help(data->basename);
    data->done = 1;
    return 0;
  }

  if (strcmp(opt, "version")==0) {
    print_version();
    data->done = 1;
    return 0;
  }

  if (strcmp(opt, "time")==0) {
    data->show_time = 1;
    return 1;
  }

  if (strcmp(opt, "block")==0) {
    data->is_block = 1;
    return 1;
  }

  if (strcmp(opt, "inline")==0) {
    data->is_block = 0;
    return 1;
  }

  /* FIXME: validation */

  if (strcmp(opt, "max-nesting")==0 && isNum) {
    data->max_nesting = num;
    return 2;
  }
  if (strcmp(opt, "input-unit")==0 && isNum) {
    data->iunit = num;
    return 2;
  }

  if (strcmp(opt, "html")==0) {
    data->renderer = RENDERER_HTML;
    return 1;
  }
  if (strcmp(opt, "noop")==0) {
    data->renderer = RENDERER_NOOP;
    return 1;
  }
  if (strcmp(opt, "man")==0) { data->renderer = RENDERER_MANPAGE; return 1; }
  if (strcmp(opt, "man-th-t")==0 && next) { data->mpth.title = next; return 2; }
  if (strcmp(opt, "man-th-s")==0 && next) { data->mpth.section = next; return 2; }
  if (strcmp(opt, "man-th-e1")==0 && next) { data->mpth.extra1 = next; return 2; }
  if (strcmp(opt, "man-th-e2")==0 && next) { data->mpth.extra2 = next; return 2; }
  if (strcmp(opt, "man-th-e3")==0 && next) { data->mpth.extra3 = next; return 2; }

  if (parse_preset_option(opt, data) || parse_category_option(opt, data) || parse_flag_option(opt, data) || parse_negative_option(opt, data))
    return 1;

  fprintf(stderr, "Wrong option '--%s' found.\n", opt);
  return 0;
}

static int parse_argument(int argn, char *arg, int is_forced, void *opaque) {
  struct option_data *data = opaque;

  if (argn == 0) {
    /* Input file */
    if (strcmp(arg, "-")!=0 || is_forced) data->filename = arg;
    return 1;
  }

  fprintf(stderr, "Too many arguments.\n");
  return 0;
}


/* MAIN LOGIC */

int main(int argc, char **argv) {
  struct option_data data;
  clock_t t1, t2;
  FILE *file = stdin;
  hoedown_buffer *ib, *ob;
  hoedown_renderer *renderer = NULL;
  void (*renderer_free)(hoedown_renderer *) = NULL;
  hoedown_document *document;

  /* zero all option data */
  memset((void *)&data, 0 ,sizeof(data));
  /* Parse options */
  data.basename = argv[0];
  data.iunit = DEF_IUNIT;
  data.filename = NULL;
  data.renderer = RENDERER_HTML;
  data.is_block = 1;
  data.max_nesting = DEF_MAX_NESTING;
  data.features = presets_info[0].flags;

  // get basename of executable
  const char *pBasename = argv[0] + strlen(argv[0]) -1;

  while(*pBasename && pBasename > argv[0] + 1 && *pBasename != '/' && *(pBasename-1) != '/')
    pBasename --;

  // are we mandown or hoedown
  if(*pBasename && strcmp(pBasename, "mandown") == 0)
    parse_long_option("man", NULL, &data);

  argc = parse_options(argc, argv, parse_short_option, parse_long_option, parse_argument, &data);
  if (data.done) return 0;
  if (!argc) return 1;

  /* Open input file, if needed */
  if (data.filename) {
    file = fopen(data.filename, "r");
    if (!file) {
      fprintf(stderr, "Unable to open input file \"%s\": %s\n", data.filename, strerror(errno));
      return 5;
    }
  }

  /* Read everything */
  ib = hoedown_buffer_new(data.iunit);

  if (hoedown_buffer_putf(ib, file)) {
    fprintf(stderr, "I/O errors found while reading input.\n");
    return 5;
  }

  if (file != stdin) fclose(file);

  /* Create the renderer */
  switch (data.renderer) {
    case RENDERER_HTML:
      renderer = hoedown_html_renderer_new();
      renderer_free = hoedown_html_renderer_free;
      break;
    case RENDERER_NOOP:
      renderer = hoedown_noop_renderer_new();
      renderer_free = hoedown_noop_renderer_free;
      break;
    case RENDERER_MANPAGE:
      renderer = hoedown_manpage_renderer_new(&data.mpth);
      renderer_free = hoedown_manpage_renderer_free;
      break;
  };

  /* Perform Markdown rendering */
  document = hoedown_document_new(renderer, data.features, data.max_nesting);

  t1 = clock();
  ob = hoedown_document_render(document, ib->data, ib->size, data.is_block, NULL);
  t2 = clock();

  /* Cleanup */
  hoedown_buffer_free(ib);

  hoedown_document_free(document);
  renderer_free(renderer);

  /* Write the result to stdout */
  if (ob) {
    (void)fwrite(ob->data, 1, ob->size, stdout);
    hoedown_buffer_free(ob);
  }

  if (ferror(stdout)) {
    fprintf(stderr, "I/O errors found while writing output.\n");
    return 5;
  }

  /* Show rendering time */
  if (data.show_time) {
    if (t1 == -1 || t2 == -1) {
      fprintf(stderr, "Failed to get the time.\n");
      return 1;
    }

    double elapsed = (double)(t2 - t1) / CLOCKS_PER_SEC;
    if (elapsed < 1)
      fprintf(stderr, "Time spent on rendering: %7.2f ms.\n", elapsed*1e3);
    else
      fprintf(stderr, "Time spent on rendering: %6.3f s.\n", elapsed);
  }

  return 0;
}
