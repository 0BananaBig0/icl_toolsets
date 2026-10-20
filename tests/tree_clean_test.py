#!/usr/bin/env python3
"""Assert that nothing generated is tracked by git.

Generated artefacts have been committed twice in this project's history, and
both times it was invisible until something else made it visible: `build/`
showed up as 9.8 MB of history behind 330 KB of source, and
`docs/codegraph/` showed up because every `make codegraph` left the tree
dirty. Neither was noticed by reading the file list — only by measuring.

So this is a test rather than a note. It checks two things:

1. No tracked path matches a generated pattern. The list is deliberately a
   literal restatement of the intent behind .gitignore, not an import of it —
   if the two ever disagree, that disagreement is the finding.

2. **No .gitignore line has a trailing comment.** gitignore has no
   end-of-line comments: a `#` after a pattern is part of the pattern, so
   `build/    # object files` ignores a file whose name contains spaces and
   a hash, and ignores nothing else. The previous .gitignore was written
   exactly that way and silently ignored nothing at all — which is the most
   dangerous failure available here, because `git status` looks clean either
   way.

Skipped, loudly, when there is no git repository: a `make dist` tarball has
no .git, and that is not a failure.
"""

import argparse
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

# Suffixes and directory prefixes that are always produced, never written.
GENERATED_SUFFIXES = (
    '.o', '.a', '.gcda', '.gcno', '.gcov',
    '.tar.gz', '.tgz', '.zip',
)
GENERATED_PREFIXES = (
    'build/', 'gen/', 'docs/codegraph/',
)


def git(*args):
    proc = subprocess.run(['git'] + list(args), cwd=ROOT,
                          capture_output=True, text=True)
    return proc.returncode, proc.stdout


def in_repo():
    code, out = git('rev-parse', '--is-inside-work-tree')
    return code == 0 and out.strip() == 'true'


def tracked_files():
    code, out = git('ls-files')
    if code != 0:
        return []
    return [line for line in out.splitlines() if line]


def check_gitignore_comments():
    """Every ignored pattern must stand alone on its line.

    gitignore parses `#` as a comment only at the start of a line. A pattern
    with a trailing comment becomes a pattern containing spaces and a hash,
    which matches nothing — and `git status` still looks clean.
    """
    path = os.path.join(ROOT, '.gitignore')
    if not os.path.exists(path):
        return []
    problems = []
    with open(path, encoding='utf-8') as handle:
        for number, line in enumerate(handle, 1):
            body = line.strip()
            if not body or body.startswith('#'):
                continue
            if '#' in body:
                problems.append(
                    '.gitignore:%d has a trailing comment; gitignore has no '
                    'end-of-line comments, so this pattern matches nothing: '
                    '%r' % (number, body))
    return problems


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', default='build/icl-format')
    args = parser.parse_args()

    if not in_repo():
        print('tree: no git repository here (a dist tarball has none) — skipped')
        return 0

    files = tracked_files()
    dirty = []
    for name in files:
        if name.startswith(GENERATED_PREFIXES) or \
           name.endswith(GENERATED_SUFFIXES):
            dirty.append(name)

    problems = list(dirty) + check_gitignore_comments()

    if problems:
        print('tree: %d problem(s) — generated files are tracked, or '
              '.gitignore is written so that it ignores nothing'
              % len(problems))
        for item in problems:
            print('   %s' % item)
        return 1

    print('tree: %d tracked files, none generated; .gitignore patterns are '
          'all standalone' % len(files))
    return 0


if __name__ == '__main__':
    sys.exit(main())
