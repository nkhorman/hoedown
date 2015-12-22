Hoedown
=======

This is a fork of [hoedown](https://github.com/hoedown/hoedown) that attempts to add some
support for rendering to a man page using the light weight nroff man macros vs. the nroff mdoc
macros. It is very much a hack in that the manpage renderer is a copy of the html renderer, and modified
just enough to render very simple markdown documents. A number of HTML renderer functions have been
left in place, with the addition of a line of exit code denoting which function needs to be implemented
for code completion of the particular markdown syntax functionality.

`Hoedown` is a revived fork of [Sundown](https://github.com/vmg/sundown),
the Markdown parser based on the original code of the
[Upskirt library](http://fossil.instinctive.eu/libupskirt/index)
by Natacha Porté.

This branch has the code for version 4, in which `Hoedown` has been rewritten
and is now targeting [CommonMark](http://commonmark.org) officially.

> **Important:** this code is in pre-alpha! Don't use it for production yet.
> The section below doesn't fully reflect the current features, but rather
> the goals for the final release.


Features
--------

  - **Standards compliant**

    `Hoedown` follows the CommonMark spec and is tested against it.
    It's entirely compliant except for some
    [differences](https://github.com/hoedown/hoedown/wiki/Spec-differences)
    needed mostly to guarantee security.

  - **Fast and efficient**

    `Hoedown` has been written with speed in mind; it reuses memory, operates
    without an intermediate AST and renders at the same time, among other things.

    `Hoedown` renders at 145% the speed of CMark (the reference C99 implementation)
    at the time of writing. We tested many documents: this README, the spec,
    John's syntax page, etc.

  - **Portable and zero-dependency**

    `Hoedown` is a zero-dependency library composed of some `.c` files and their
    headers. No dependencies, no bullshit. Only standard C89 that builds everywhere.

  - **Increased flexibility**

    `Hoedown` supports (unofficial) CommonMark extensions such as tables, math,
    footnotes, and more! The new version gives even more control, allowing you
    to enable or disable every single construct, not just extensions, at runtime.

  - **Customizable renderers**

    `Hoedown` is not stuck with HTML, renderers can output anything: TeX code,
    an AST tree, statistics about the parsed document, binary data, etc.
    Parsing is completely decoupled from rendering.

  - **Precise source mapping**

    `Hoedown` is more than a parser. It has character-level source mapping, both
    direct and indirect, which is also present at inline constructs. This allows
    for a variety of creative uses, see [the recipes](https://github.com/hoedown/hoedown/wiki).

  - **Safe for production**

    `Hoedown` includes protection against possible attacks like out of memory
    situations, malformed syntax and extensive nesting. We've worked very hard
    to make `Hoedown` never leak, crash, loop or block under *any* input.

    **Warning**: `Hoedown` doesn't validate HTML found in Markdown documents.
    Unless you disable HTML parsing entirely, you should always feed the
    rendered HTML to [Lanli](https://github.com/hoedown/lanli) or another
    sanitizer.

  - **Unicode aware**

    `Hoedown` is fully UTF-8 aware, both when parsing the source document and when
    generating the resulting HTML.


Bindings
--------

You can see a community-maintained list of `Hoedown` bindings at
[the wiki](https://github.com/hoedown/hoedown/wiki/Bindings).


Help us
-------

`Hoedown` is all about security. If you find a (potential) security vulnerability in the
library, or a way to make it crash through malicious input, please report it to us by
emailing the private [Hoedown Security](mailto:hoedown-security@googlegroups.com)
mailing list. The `Hoedown` security team will review the vulnerability and work with you
to reproduce and resolve it.


Install
-------

To build the library and executable, use [CMake](http://cmake.org):

    cmake -D CMAKE_BUILD_TYPE=Release .
    cmake --build .

Or just drop the files at `src` into your project.
Hoedown doesn't have any special building requeriments.
