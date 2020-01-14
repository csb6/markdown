# Markdown

A parser for a subset of Markdown, written in C++17.

# Supports

- `<h1>`
- `<p>`
- `<em>`
- `<strong>`
- `<ul>`
- `<ol>`
- `<li>`

## Installation

Run `build.sh` to compile using any standards-conforming C++ compiler;
the only dependency is the standard library.

## Usage

Run `./markdown` followed by the name of the file to parse. The program
will print the resulting HTML to the standard output, which you can
easily redirect to a file. For example, `./markdown index.md > index.html`
works on most Unix-like systems.