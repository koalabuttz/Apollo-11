#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The rules of STYLE.md that a script can hold the sources to.

A whole-tree read of readable/src against STYLE.md (issue #1334) found the
same few breaks many times over. This check knows those breaks by their
form, counts them for each directory of src/ and holds the counts to
tests/style_owed.txt: the directories that still owe a fix, with how many
of each. A break in a directory that owes none fails. So does a count that
is not the ledger's, either way: one more is a new break, one fewer is a
fix whose line has to come off the ledger (--write does it). The ledger
only gets shorter.

The rules, each named as the ledger names it:

  ccs-order         A switch on a CCS has its cases in the listing's
                    order: POSITIVE, IS_PLUS_ZERO, NEGATIVE, IS_MINUS_ZERO
                    (STYLE.md, Structure). Each mission's text is looked
                    at apart (#if LUMINARY099 and the like).
  ccs-default       A switch on a CCS names its four outcomes: no default:
                    that stands for two or three of them, since the
                    listing has a word for each (STYLE.md, Structure).
  ccs-not-modelled  An outcome of a CCS is followed, not refused: a case
                    of a CCS switch does not begin with not_modelled()
                    (STYLE.md, Fidelity). One whose comment says the
                    listing has no word there (the CCS's last words fall
                    past the end of a bank's program) is let be: there is
                    nothing to follow. So is one whose comment says the
                    next word is an interpretive word or an address word
                    (no instruction's line), and one in a helper that
                    stands for one CCS at several places, whose comment
                    says that what would run differs with the place. In
                    each the comment (on the case's lines, or right above
                    them) also says that the outcome never comes, and
                    what rules it out; the check holds it to the word
                    never.
  raw-channel       A channel is written by its name, not a bare number
                    (STYLE.md, Naming).
  continue-offset   A place inside its own directory that a routine ends
                    its step at, continue_at(LABEL + n), can be followed by
                    name: the directory has the function label_plusN
                    (STYLE.md, Structure). LABEL is a routine of that
                    directory. A jump whose line, or the line before,
                    says in a comment that it never happens is let be:
                    it says where the listing would go, and stops there.
  own-helper        COUNT() and tc_to() are defined once, in
                    src/agc/agc.h (COUNT() also in src/assembly/assembly.h,
                    for the tables that do not use the machine), and
                    IN_BANK() and BANK_OF() in src/interpreter/notation.h.
  decimal-offset    An offset from a label of the fixed memory is in
                    octal, with its leading 0, as the listing counts:
                    POODOO + 014u, not POODOO + 12u (STYLE.md, Naming).
  long-line         No line of C is over 100 characters (src/agc/names.h
                    apart: its tables are not wrapped).
  file-header       A .c file's header names the faithful C it corresponds
                    to, a file under c64-port/ (STYLE.md, Comments). Not
                    asked of src/agc (the machine) and src/assembly (the
                    listing's tables), which have none.
  goto              No goto (STYLE.md, Language).
  wording           No "as the original" and no "as yaAGC": a comment says
                    what a thing is compared with (STYLE.md, Fidelity).

  style_check.py               count, and hold the tree to the ledger
  style_check.py --write       write the ledger from the count
  style_check.py --list RULE   print each break of one rule, with its line
  style_check.py --self-check  the check itself on made-up sources

`make style-check`, part of `make all`. What it cannot see is everything
else in STYLE.md: names, comments, whether the C says what the listing
does. A person reads those. A function header without its page is
tools/header_pages.py's to find (the same target runs it). One more
finding of issue #1334 is not counted yet: a name the listing does not
have.
"""
import collections
import re
import sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
SRC = READABLE / 'src'
LEDGER = READABLE / 'tests/style_owed.txt'
ORDER = ('POSITIVE', 'IS_PLUS_ZERO', 'NEGATIVE', 'IS_MINUS_ZERO')
RULES = ('ccs-order', 'ccs-default', 'ccs-not-modelled', 'raw-channel', 'continue-offset', 'own-helper',
         'decimal-offset', 'long-line', 'file-header', 'goto', 'wording')
CCS = re.compile(r'\bswitch \(ccs(?:_at|_register)?\(')
CASE = re.compile(r'\bcase (\w+):')
CHANNEL = re.compile(r'\b(?:read|write|rand|wand|wor|rxor|ror)_channel\('
                     r'\s*\(?(?:word|unsigned)?\)?\s*0?[0-9]+u?\s*[,)]')
CONTINUE = re.compile(r'\bcontinue_at\(\s*(?:\(word\)\s*)?\(?\s*([A-Z][A-Z0-9_]*)\s*([+-])\s*(0?[0-9]+)u?')
HELPER = re.compile(r'^[ \t]*#[ \t]*define[ \t]+(?:COUNT|IN_BANK|BANK_OF)\('
                    r'|^(?:static )?(?:inline )?void tc_to\(', re.M)
DECIMAL = re.compile(r'\b(?:K_)?([A-Z][A-Z0-9_]*) \+ ([89]|[1-9][0-9]+)u\b')
LABEL = re.compile(r'^#define (\w+) +\(\(word\)K_\w+\)', re.M)
NO_WORD = re.compile(r'\b(?:no word|interpretive words?|address words?|differs with the place)\b')
NEVER = re.compile(r'\bnever\b', re.I)
ABOVE = re.compile(r'\s*(?:/\*|\*|case \w+:\s*(?:/\*.*\*/)?\s*$)')  # a comment's line, or a case's label alone
NEVER = re.compile(r'/\*[^\n]*\bnever\b|^[^\n]*\bnever\b[^\n]*\*/', re.M)
NAMES = 'agc/names.h'
WIDTH = 100
HEADER_END = re.compile(r'^#[ \t]*include|^\w[^\n]*\(', re.M)
NO_FAITHFUL = ('agc', 'assembly')
GOTO = re.compile(r'\bgoto\b(?!_)')
WORDING = re.compile(r'\bas the original\b|\bas yaAGC\b', re.I)
FUNCTION = re.compile(r'^(?:static )?(?:inline )?(?:void|bool|word|int|unsigned|enum \w+) (\w+)\(', re.M)
HELPER_HOMES = ('agc/agc.h', 'assembly/assembly.h', 'interpreter/notation.h')
ENTRY = re.compile(r'(\S+) (\S+) (\d+)$')


def blanked(text, strings=True):
    """The text with each comment (and each string) turned to spaces, line
    for line: what is left is code."""
    pattern = r'/\*.*?\*/' + (r'|"(?:\\.|[^"\\\n])*"' if strings else '')
    return re.sub(pattern, lambda found: re.sub(r'[^\n]', ' ', found.group()), text, flags=re.S)


def for_mission(text, lm):
    """The text as one mission compiles it: the lines of the other
    mission's branch of an #if on LUMINARY099 or COMANCHE055 turned to
    blank ones. (Of an #if on anything else, both branches stay.)"""
    out, kept = [], []      # kept: for each open #if, whether its lines stay (None: not a mission's)
    for line in text.split('\n'):
        told = re.match(r'[ \t]*#[ \t]*(if|ifdef|ifndef|else|elif|endif)\b(.*)', line)
        if told:
            word, rest = told.group(1), told.group(2).strip()
            if word in ('if', 'ifdef', 'ifndef'):
                found = re.fullmatch(r'(!?)\s*(?:defined\s*\(?\s*)?(LUMINARY099|COMANCHE055)\s*\)?', rest)
                if found and word == 'if':
                    kept.append((found.group(2) == 'LUMINARY099') == lm != bool(found.group(1)))
                else:
                    kept.append(None)
            elif word == 'else' and kept and kept[-1] is not None:
                kept[-1] = not kept[-1]
            elif word == 'endif' and kept:
                kept.pop()
            out.append('')
            continue
        out.append(line if all(k is not False for k in kept) else '')
    return '\n'.join(out)


def body_of(text, start):
    """The braces' contents that begin at or after start, and where they
    end: (first, last), or None."""
    first = text.find('{', start)
    if first < 0:
        return None
    depth = 0
    for at in range(first, len(text)):
        if text[at] == '{':
            depth += 1
        elif text[at] == '}':
            depth -= 1
            if not depth:
                return first + 1, at
    return None


def ccs_switches(code):
    """Each switch on a CCS in code (comments blanked): (where it is, its
    cases in order, each (name, where, the code up to the next case))."""
    out = []
    for found in CCS.finditer(code):
        span = body_of(code, found.end())
        if not span:
            continue
        first, last = span
        depth, labels = 0, []
        for at in range(first, last):
            if code[at] == '{':
                depth += 1
            elif code[at] == '}':
                depth -= 1
            elif not depth:
                label = re.compile(r'\b(?:case (\w+)|default):').match(code, at)
                if label and (at == first or not (code[at - 1].isalnum() or code[at - 1] == '_')):
                    labels.append((label.group(1) or 'default', at, label.end()))
        cases = [(name, at, code[end:(labels[i + 1][1] if i + 1 < len(labels) else last)])
                 for i, (name, at, end) in enumerate(labels)]
        out.append((found.start(), cases))
    return out


def breaks(file, text, routines=(), labels=()):
    """The breaks in one file's text: [(rule, line, what)]. (routines: the
    names of the functions its directory defines; labels: the names of
    the fixed memory's addresses.)"""
    out = []
    code = blanked(text)
    lines = text.split('\n')

    def line_of(source, at):
        return source.count('\n', 0, at) + 1
    seen = set()
    for lm in (True, False):
        mission = for_mission(code, lm)
        for at, cases in ccs_switches(mission):
            names = [name for name, _, _ in cases if name in ORDER]
            if names != sorted(names, key=ORDER.index) and ('ccs-order', at) not in seen:
                seen.add(('ccs-order', at))
                out.append(('ccs-order', line_of(mission, at), ', '.join(names)))
            if 'default' in [name for name, _, _ in cases] and ('ccs-default', at) not in seen:
                seen.add(('ccs-default', at))
                out.append(('ccs-default', line_of(mission, at), 'default:'))
            for name, where, statements in cases:
                if name in ORDER and re.match(r'\s*not_modelled\(\)', statements) and \
                        ('ccs-not-modelled', where) not in seen:
                    seen.add(('ccs-not-modelled', where))
                    first, last = line_of(mission, where), line_of(mission, mission.index('not_modelled', where))
                    top = first
                    while top > 1 and ABOVE.match(lines[top - 2]):
                        top -= 1
                    said = '\n'.join(lines[top - 1:last])
                    if not (NO_WORD.search(said) and NEVER.search(said)):
                        out.append(('ccs-not-modelled', first, name))
    for found in CHANNEL.finditer(code):
        out.append(('raw-channel', line_of(code, found.start()), found.group().strip()))
    for found in CONTINUE.finditer(code):
        label, sign, digits = found.group(1).lower(), found.group(2), found.group(3)
        value = int(digits, 8 if digits.startswith('0') else 10)
        # (the name has the digits as its header writes them: +12, +012, or 10 for +10D)
        named = {f"{label}_{'plus' if sign == '+' else 'minus'}{number}"
                 for number in (f'{value:o}', f'0{value:o}', str(value))}
        line = line_of(code, found.start())
        never = NEVER.search('\n'.join(lines[max(line - 2, 0):line]))
        if label in routines and not named & set(routines) and not never:
            out.append(('continue-offset', line_of(code, found.start()), found.group().strip() + ' ...'))
    if file not in HELPER_HOMES:
        for found in HELPER.finditer(code):
            out.append(('own-helper', line_of(code, found.start()), found.group().strip()))
    for found in DECIMAL.finditer(code):
        if found.group(1) in labels:
            out.append(('decimal-offset', line_of(code, found.start()), found.group()))
    if file != NAMES:
        for number, line in enumerate(lines, 1):
            if len(line) > WIDTH:
                out.append(('long-line', number, f'{len(line)} characters'))
    if file.endswith('.c') and file.split('/')[0] not in NO_FAITHFUL:
        end = HEADER_END.search(code)
        if 'c64-port/' not in text[:end.start() if end else len(text)]:
            out.append(('file-header', 1, 'no file of c64-port/ named'))
    for found in GOTO.finditer(code):
        out.append(('goto', line_of(code, found.start()), 'goto'))
    for found in WORDING.finditer(text):
        out.append(('wording', line_of(text, found.start()), found.group()))
    return sorted(out, key=lambda b: (RULES.index(b[0]), b[1]))


def measure():
    """Every break under src/: {(rule, directory): [(file, line, what)]}."""
    texts = {str(path.relative_to(SRC)): path.read_text() for path in sorted(SRC.rglob('*'))
             if path.suffix in ('.c', '.h')}
    routines = collections.defaultdict(set)
    for file, text in texts.items():
        routines[file.split('/')[0]] |= set(FUNCTION.findall(text))
    labels = set(LABEL.findall(texts[NAMES]))
    found = collections.defaultdict(list)
    for file, text in texts.items():
        directory = file.split('/')[0]
        for rule, line, what in breaks(file, text, routines[directory], labels):
            found[(rule, directory)].append((file, line, what))
    return found


def read_ledger(path=LEDGER):
    """{(rule, directory): count}."""
    owed = {}
    if path.is_file():
        for line in path.read_text().split('\n'):
            told = ENTRY.match(line)
            if told and not line.startswith('#'):
                owed[(told.group(1), told.group(2))] = int(told.group(3))
    return owed


def write_ledger(path, found):
    totals = collections.Counter()
    for (rule, _), where in found.items():
        totals[rule] += len(where)
    out = ['# The breaks of STYLE.md that tools/style_check.py counts, by rule and',
           '# directory of src/: what is still owed of issue #1334. The check holds',
           '# the tree to these counts; `tools/style_check.py --write` writes them.',
           '# A directory that is not here owes nothing and may not break a rule.',
           '#',
           '# Owed in all: ' + (', '.join(f'{rule} {totals[rule]}' for rule in RULES if totals[rule]) or 'nothing')
           + '.',
           '']
    for rule in RULES:
        out += [f'{rule} {directory} {len(where)}' for (r, directory), where in sorted(found.items()) if r == rule]
    path.write_text('\n'.join(out) + '\n')


def judge(found, owed):
    """What is wrong between the count and the ledger, a line each."""
    out = []
    for key in sorted(set(found) | set(owed), key=lambda k: (RULES.index(k[0]) if k[0] in RULES else 99, k[1])):
        rule, directory = key
        have, listed = len(found.get(key, ())), owed.get(key)
        if rule not in RULES:
            out.append(f'the ledger names a rule this check does not have ({rule})')
        elif listed is None:
            file, line, what = found[key][0]
            out.append(f'{rule}: src/{directory} owes none and has {have} (src/{file}:{line}: {what})')
        elif have > listed:
            out.append(f'{rule}: src/{directory} has {have}, {have - listed} more than the ledger\'s {listed}')
        elif have < listed:
            out.append(f'{rule}: src/{directory} has {have}, fewer than the ledger\'s {listed}: write the ledger '
                       'again (--write)')
    return out


def self_check():
    """Each rule on a made-up source that breaks it and on one that does
    not, and the ledger's three ways of being wrong."""
    failures, tried = [], []
    HEADER = '/* X.agc, LM pp. 1-2, CM pp. 3-4. The faithful C is c64-port/src/x.c. */\n'

    def expect(name, text, want, routines=(), file='x/x.c', bare=False):
        tried.append(name)
        header = '' if 'file-header' in [rule for rule, _ in want] or bare else HEADER
        got = [(rule, line - header.count('\n')) for rule, line, _ in
               breaks(file, header + text, routines, {'POODOO', 'SGNCOM'})]
        if got != want:
            failures.append(f'{name}: {got}, expected {want}')
    in_order = ('void f(void)\n{\n    switch (ccs_at(X)) {\n    case POSITIVE: a(); break;\n'
                '    case IS_PLUS_ZERO: /* case NEGATIVE: in a comment */\n    case NEGATIVE: b(); return;\n'
                '    case IS_MINUS_ZERO: break;\n    }\n}\n')
    expect('a CCS in order', in_order, [])
    expect('a CCS out of order', in_order.replace('case POSITIVE', 'case NEGATIVE', 1).replace(
        'case NEGATIVE: b', 'case POSITIVE: b'), [('ccs-order', 3)])
    expect('a default for the outcomes not named',
           'switch (ccs(X)) {\ncase IS_PLUS_ZERO: return;\ndefault: break;\n}\n', [('ccs-default', 1)])
    expect('a default in a switch inside a case of a CCS', in_order.replace(
        'a(); break;', 'switch (a_word()) {\n    case 1: break;\n    default: break;\n    }\n    break;'), [])
    nested = ('switch (ccs_register(A_ADDRESS)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO:\n'
              '    switch (ccs_at(Y)) {\n    case POSITIVE:\n    case IS_PLUS_ZERO: {\n        x();\n        break;\n'
              '    }\n    case NEGATIVE:\n    case IS_MINUS_ZERO: break;\n    }\n    break;\ncase NEGATIVE:\n'
              'case IS_MINUS_ZERO: break;\n}\n')
    expect('a CCS inside a case of another, both in order', nested, [])
    expect('the inner one out of order', nested.replace('    case POSITIVE:\n    case IS_PLUS_ZERO: {',
                                                         '    case IS_PLUS_ZERO:\n    case POSITIVE: {'),
           [('ccs-order', 4)])
    missions = ('switch (ccs_at(X)) {\ncase POSITIVE: break;\n#if LUMINARY099\ncase IS_PLUS_ZERO: a();\n'
                'case NEGATIVE: b(); return;\ncase IS_MINUS_ZERO: return;\n#else\ncase IS_PLUS_ZERO: return;\n'
                'case NEGATIVE: return;\ncase IS_MINUS_ZERO: c(); return;\n#endif\n}\n')
    expect('a CCS whose cases differ by mission, each in order', missions, [])
    expect('one mission\'s cases out of order', missions.replace('case NEGATIVE: return;\ncase IS_MINUS_ZERO: c',
                                                                 'case IS_MINUS_ZERO: return;\ncase NEGATIVE: c'),
           [('ccs-order', 1)])
    expect('a switch that is not on a CCS', 'switch (step(X)) {\ncase NEGATIVE: break;\ncase POSITIVE: break;\n}\n', [])
    refused = 'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO:\n    not_modelled();\n    return;\n' \
              'default: break;\n}\n'
    expect('an outcome of a CCS refused', refused, [('ccs-default', 1), ('ccs-not-modelled', 3)])
    followed = refused.replace('    not_modelled();', '    a();\n    not_modelled();')
    expect('an outcome that is followed to a refusal', followed, [('ccs-default', 1)])
    expect('an outcome the listing has no word for',
           'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE: break;\n'
           'case IS_MINUS_ZERO:\n    /* (never) TCF X is the last word: the listing has no word\n     * after it. */\n'
           '    not_modelled();\n}\n', [])
    for words in ('never: a bit. Next are interpretive words', 'never: a count. Next is an address word',
                  'never; what would run differs with the place'):
        expect(f'an outcome whose next word is no instruction ({words.split(". ")[-1]})',
               'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE:\n'
               f'case IS_MINUS_ZERO: not_modelled(); /* {words} */\n}}\n', [])
    expect('one that says why above its cases\' labels',
           'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\n/* (never: a masked bit) */\n'
           'case NEGATIVE:\ncase IS_MINUS_ZERO: not_modelled(); /* the next word is an address word */\n}\n', [])
    expect('one that does not say the outcome never comes',
           'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE: break;\n'
           'case IS_MINUS_ZERO: not_modelled(); /* the next word is an address word, CADR X */\n}\n',
           [('ccs-not-modelled', 5)])
    expect('a value outside a table refused', 'switch (a_word()) {\ncase 1: break;\ndefault: not_modelled();\n}\n', [])
    expect('a channel by number', 'x = rand_channel(031);\nwor_channel(013u);\nwrite_channel(6, A);\n',
           [('raw-channel', 1), ('raw-channel', 2), ('raw-channel', 3)])
    expect('a channel by name', 'x = rand_channel(CHAN31);\nwor_channel(DSALMOUT);\n/* rand_channel(031) */\n', [])
    expect('a step that ends at a place no function is named for', 'continue_at(GETLCX + 012u);\n/* GETLCX +12: x */\n',
           [('continue-offset', 1)], {'getlcx', 'getlcx_plus13'})
    expect('one that never happens', 'if (x)\n    continue_at(GETLCX + 012u); /* (never: x is +0) */\n'
           'case 035: /* TCF +0 (never) */\n    continue_at(GETLCX + 035u);\n'
           '/* never */\n\ncontinue_at(GETLCX + 1u);\n', [('continue-offset', 7)], {'getlcx'})
    expect('at a place a function is named for', 'continue_at(GETLCX + 012u);\ncontinue_at((word)(GETLCX - 2u));\n',
           [], {'getlcx', 'getlcx_plus12', 'getlcx_minus2'})
    expect('at a place a function is named for with its leading 0', 'continue_at(GETLCX + 012u);\n',
           [], {'getlcx', 'getlcx_plus012'})
    expect('a jump to an offset of a routine of another', 'continue_at((word)(POODOO + 014u));\ncontinue_at(GETLCX);\n',
           [], {'getlcx'})
    expect('a COUNT of its own', '#define COUNT(a) (sizeof(a) / sizeof((a)[0]))\n', [('own-helper', 1)])
    expect('an IN_BANK of its own', '#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))\n',
           [('own-helper', 1)])
    expect('a tc_to of its own', 'static void tc_to(word at, word routine)\n{\n}\n', [('own-helper', 1)])
    expect('a decimal offset from a label', 'tc_at((word)(POODOO + 12u));\n#define X ((word)(K_SGNCOM + 9u))\n',
           [('decimal-offset', 1), ('decimal-offset', 2)])
    expect('octal offsets, small ones and an erasable\'s', 'tc_at(POODOO + 014u);\ntc_at(POODOO + 7u);\n'
           'ts(FIXLOC + 26u);\nts(ADDRESS_OF(DSPTAB) + 11u); /* POODOO + 12u */\n', [])
    expect('a line of 101 characters', '/* ' + 'x' * 95 + ' */\n' + 'y' * 100 + '\n', [('long-line', 1)])
    expect('the same in the names', 'z' * 101 + '\n', [], file=NAMES)
    expect('a file whose header names no faithful C', '/* X.agc, pp. 1-2. */\n#include "x.h"\n/* c64-port/src/x.c */\n',
           [('file-header', 1)])
    expect('the same of the tables', '/* MAIN.agc */\nint x;\n', [], file='assembly/main.c', bare=True)
    expect('a goto', 'void f(void)\n{\n    goto out;\n    goto_();\n}\n', [('goto', 3)])
    expect('the wording', '/* As the original does. */\nx(); /* as yaAGC writes it */\n/* the original listing */\n',
           [('wording', 1), ('wording', 2)])
    found = {('ccs-order', 'a'): [('a/a.c', 3, 'x')] * 2, ('goto', 'b'): [('b/b.c', 1, 'goto')]}

    def held(name, owed, want):
        tried.append(name)
        got = len(judge(found, owed))
        if got != want:
            failures.append(f'{name}: {got} lines, expected {want}')
    held('a ledger that is the count', {('ccs-order', 'a'): 2, ('goto', 'b'): 1}, 0)
    held('a break where none is owed', {('ccs-order', 'a'): 2}, 1)
    held('one break more than owed', {('ccs-order', 'a'): 1, ('goto', 'b'): 1}, 1)
    held('one break fewer than owed', {('ccs-order', 'a'): 3, ('goto', 'b'): 1}, 1)
    held('a directory owed that has none now', {('ccs-order', 'a'): 2, ('goto', 'b'): 1, ('goto', 'c'): 1}, 1)
    held('a rule the check does not have', {('ccs-order', 'a'): 2, ('goto', 'b'): 1, ('nonesuch', 'a'): 1}, 1)
    for failure in failures:
        print(f'FAIL style check self-check: {failure}')
    print(f"{'FAIL' if failures else 'ok  '} style check self-check: {len(tried)} made-up sources and ledgers, "
          f'{len(failures)} judged wrongly')
    return 1 if failures else 0


def main():
    args = sys.argv[1:]
    unknown = [a for a in args if a.startswith('-') and a not in ('--write', '--list', '--self-check')]
    if unknown:
        print(__doc__ if unknown[0] in ('-h', '--help') else f'style_check: no option {unknown[0]} (see --help)')
        return 0 if unknown[0] in ('-h', '--help') else 2
    if '--self-check' in args:
        return self_check()
    found = measure()
    if '--list' in args:
        rule = args[args.index('--list') + 1]
        for (r, _), where in sorted(found.items()):
            for file, line, what in where if r == rule else ():
                print(f'src/{file}:{line}: {what}')
        return 0
    if '--write' in args:
        write_ledger(LEDGER, found)
        print(f'wrote {LEDGER.relative_to(READABLE)}')
        return 0
    totals = collections.Counter()
    for (rule, _), where in found.items():
        totals[rule] += len(where)
    wrong = judge(found, read_ledger())
    for line in wrong[:40]:
        print(f'FAIL {line}')
    print(f"{'FAIL' if wrong else 'ok  '} style check against {LEDGER.relative_to(READABLE)}: owed "
          + (', '.join(f'{rule} {totals[rule]}' for rule in RULES if totals[rule]) or 'nothing')
          + f'; {len(wrong)} not as the ledger has it')
    return 1 if wrong else 0


if __name__ == '__main__':
    sys.exit(main())
