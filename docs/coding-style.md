# Coding style

Google C++ style is a project requirement for handwritten C++ in `src/`,
including C++ embedded in the Flex and Bison sources. It covers naming,
declarations, headers, class interfaces, comments and layout, not just
indentation. The reference is the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

## C++ conventions

- Files use `lower_snake_case`: `.cc` implementations, `.h` headers, `.l` Flex
  sources and `.yy` Bison sources. Existing source filenames meet this rule.
- Types, classes, structs, aliases and ordinary functions use `UpperCamelCase`.
  Acronyms read as words: `IclDriver`, `Json`, `Uri`.
- Variables, parameters, namespaces and struct fields use `lower_snake_case`.
  Class data members are private and have a trailing underscore. Expose state
  through small accessors rather than public class fields. Google permits
  `snake_case` accessors and `set_` mutators.
- Enum values and constants use `kUpperCamelCase`, including scoped enums and
  static constants. Macros use `UPPER_SNAKE_CASE`.
- Use descriptive names; brief loop counters and tightly scoped locals may use
  short names. Do not rename ICL keywords, LSP method strings or user-facing
  command names to match C++ naming rules.
- Headers must be self-contained, with `ICL_TOOLSETS_SRC_<FILE>_H_` guards.
  Include project headers by paths relative to the `src/` include root
  (`"icl_ast.h"`),
  with the related header first, then standard and other headers in order.
  Generated headers resolve through their build include directory.
- Use two-space indentation, Google's brace/spacing rules and an 80-column
  limit where practical. `.clang-format` selects the Google preset. Keep
  strings, regexes and other indivisible syntax intact when wrapping would
  change their meaning. Avoid multiple unrelated statements on one line.

## Flex/Bison and Makefile boundaries

Google does not publish a Flex/Bison or Makefile style guide. Apply Google C++
style to embedded code, with the following language-specific conventions:

- Bison productions/nonterminals use `lower_snake_case`; terminals use
  `UPPER_SNAKE_CASE`. Keep alternatives and action blocks consistently indented.
- Flex named patterns and start conditions use `UPPER_SNAKE_CASE`. Keep regex
  text unchanged when formatting actions; whitespace can be significant.
- Generator-required names such as `yylex`, `IclParser::error`, `yyterminate`,
  `YY_DECL` and `YY_USER_ACTION` retain their required spellings. Handwritten
  bridge functions use Google naming (`IclScanBytes`, `IclScanEnd`, etc.).
  Generated code is not reformatted or committed.
- Keep the filename `Makefile`, uppercase project/tool variables, lowercase
  hyphenated targets, and GNU installation variables such as `prefix` and
  `bindir`. Use one space around assignment operators, four spaces for wrapped
  variable/prerequisite lists, and a real tab at the start of every recipe.
  Shell nesting after that tab uses two spaces. Preserve continuation
  backslashes, shell quoting and Make expansion semantics.

## Checks and review

From the repository root:

```sh
clang-format -i src/*.cc src/*.h
make formatcheck
make namingcheck BISON=/path/to/bison
make check-dynamic BISON=/path/to/bison -j4
```

The style configuration was validated with clang-format and clang-tidy 22.
Override `CLANG_FORMAT` and `CLANG_TIDY` to select those versions when multiple
versions are installed. They are development tools, not build/install runtime
dependencies. Both configuration files are included in source distributions.

`formatcheck` checks layout in `.cc`/`.h`; `namingcheck` checks their declarations
with the naming rules and Google checks in `.clang-tidy` (including explicit
constructors, braces and integer types). Its narrow accessor allowlist reflects
Google's accessor exception. Neither check proves conformance to every rule in
the guide. Review names, API design, header self-containment and embedded C++
as well. Do not run clang-format on an entire `.l`, `.yy` or Makefile: these are
not C++ files. Format embedded C++ blocks separately, then regenerate and run
the grammar/scanner coverage and full regression to detect changed behavior.
