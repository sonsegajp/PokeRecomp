#!/usr/bin/env python3
"""
Post-CPP fixup for pret data .s files going through x86 GAS.

GAS's macro-arg parser splits args at the closing paren of a top-level
parenthesized sub-expression: `(a + b) - c` becomes TWO args, `(a + b)`
and `- c`, which then triggers "too many positional arguments".

This pass detects macro-call lines and wraps any arg that has a top-level
arithmetic operator into one extra set of parens, so the whole expression
is one balanced paren group from GAS's perspective.

Heuristic for "macro-call line": starts with whitespace, then a bareword
identifier (not a directive starting with `.`, not a label ending with
`:`), followed by whitespace and at least one comma in the args.

Reads stdin, writes stdout.
"""
import re
import sys


def split_top_level(text: str, sep: str = ',') -> list[str]:
    """Split text on `sep` at paren depth 0."""
    out, cur, depth = [], '', 0
    for c in text:
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
        if c == sep and depth == 0:
            out.append(cur)
            cur = ''
        else:
            cur += c
    out.append(cur)
    return out


def has_top_level_operator(arg: str) -> bool:
    """True if arg contains + or - at paren depth 0 (excluding leading sign)."""
    s = arg.lstrip()
    if s.startswith(('+', '-')):
        s = s[1:]
    depth = 0
    for c in s:
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
        elif depth == 0 and c in '+-':
            return True
    return False


# Identifiers that are NOT macros — directives etc. that shouldn't be wrapped.
_RE_LINE = re.compile(r'^(\s+)([A-Za-z_][A-Za-z0-9_]*)(\s+)(.+?)\s*$')


def fix_line(line: str) -> str:
    m = _RE_LINE.match(line)
    if not m:
        return line
    indent, name, sep, rest = m.groups()
    # Ignore obvious non-macro forms
    if name.startswith('_'):  # mostly nothing, but be safe
        return line
    args = split_top_level(rest, ',')
    if len(args) < 2:
        return line
    changed = False
    new_args = []
    for a in args:
        s = a.strip()
        if s.startswith('(') and has_top_level_operator(s):
            new_args.append('(' + s + ')')
            changed = True
        else:
            new_args.append(s)
    if not changed:
        return line
    return indent + name + sep + ', '.join(new_args)


def main():
    for line in sys.stdin:
        sys.stdout.write(fix_line(line.rstrip('\n')) + '\n')


if __name__ == '__main__':
    main()
