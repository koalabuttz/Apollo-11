#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The rules of STYLE.md that a script can hold the sources to.

A whole-tree read of readable/src against STYLE.md (issue #1334) found the
same few breaks many times over. This check knows those breaks by their
form, finds each under src/ and holds them to tests/style_owed.txt: the
breaks that are still owed a fix, one line each, by its site (the rule, the
file, the function, a digest of the line's text, and which such line of
the function it is: the key tests/line_coverage.txt uses). A break that is
not in the ledger fails. So does a line of the ledger that the tree does
not have: a fix, whose line has to come off (--write does it). The ledger
only gets shorter: --write takes lines off and puts none on. A rule that
is new to the check comes with what it finds (--owe RULE writes those
on, once).

The rules, each named as the ledger names it:

  ccs-order         A switch on a CCS has its cases in the listing's
                    order: POSITIVE, IS_PLUS_ZERO, NEGATIVE, IS_MINUS_ZERO
                    (STYLE.md, Structure). Each mission's text is looked
                    at apart (#if LUMINARY099 and the like). A switch is
                    on a CCS when its head calls one, and also when it
                    names one of the four outcomes: the outcome was kept
                    in a variable first, or a function returned it.
  ccs-default       A switch on a CCS names its four outcomes: no default:
                    that stands for two or three of them, and no outcome
                    left without a case, since the listing has a word for
                    each (STYLE.md, Structure).
  ccs-not-modelled  An outcome of a CCS is followed, not refused: a case
                    of a CCS switch does not begin with not_modelled()
                    (STYLE.md, Fidelity). One whose comment says the
                    listing has no word there (the CCS's last words fall
                    past the end of a bank's program) is let be: there is
                    nothing to follow. So is one whose comment says the
                    next word is an interpretive word or an address word
                    (no instruction's line) or no instruction's word (a
                    constant the assembler makes of no instruction's
                    line: a DAS with an address field of 0), and one in
                    a helper that
                    stands for one CCS at several places, whose comment
                    says that what would run differs with the place. In
                    each the comment (on the case's lines, or right above
                    them) also says that the outcome never comes, and
                    what rules it out; the check holds it to the word
                    never and to three words more than these, a reason
                    (whether the reason is right, a person reads). So
                    does the comment of a case that follows its words as
                    far as a not_modelled() of its own (a constant run as
                    a TC into the erasable memory), or refuses in an
                    else. "Differs with the place" is taken only in a
                    helper that tools/ccs_helpers.py holds to its premise
                    (CCS_HELPERS below). A refusal behind a test of a
                    CCS's outcome, if (ccs_at(X) != POSITIVE)
                    not_modelled(), is held to the same comment as a
                    case that begins with one. A guard on anything else,
                    if (ts(X)) not_modelled(), is no CCS's outcome.
  ccs-test          A CCS's outcome goes to a switch: the CCS is the
                    switch's head, or its outcome is kept in a variable
                    (or returned, or handed to a function) and the
                    switch is on that. It is not
                    compared, if (ccs_at(X) == POSITIVE) or
                    sign != NEGATIVE, since a test folds outcomes that
                    the listing tells apart; and it is not dropped,
                    (void)ccs_at(X). A kept outcome (an enum sign
                    variable or parameter) goes on the same way: it is
                    assigned, switched on, returned or handed on, and
                    not tested, if (sign). The one test of a CCS is
                    ccs_positive() of src/agc/agc.h, for a word that is
                    never negative; tools/ccs_helpers.py holds each call
                    of it to that (STYLE.md, Structure). Not asked of
                    src/agc (the machine) and src/binding (a binding
                    judges a state in C and runs no CCS).
  raw-channel       A channel is written by its name, not a bare number,
                    octal, decimal or hexadecimal (STYLE.md, Naming).
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
                    POODOO + 014u, not POODOO + 12u, however the + and
                    the u are written (STYLE.md, Naming).
  long-line         No line of C is over 100 characters (src/agc/names.h
                    and the .inc files apart: their tables, which tools
                    write, are not wrapped).
  file-header       A .c file's header names the faithful C it corresponds
                    to, a file under c64-port/ (STYLE.md, Comments). Not
                    asked of src/agc (the machine) and src/assembly (the
                    listing's tables), which have none.
  goto              No goto (STYLE.md, Language).
  wording           No "as the original", "as in the original", "as
                    yaAGC" or "as Virtual AGC", in any case of letters: a
                    comment says what a thing is compared with (STYLE.md,
                    Fidelity).

  style_check.py               find the breaks, and hold the tree to the ledger
  style_check.py --write       take the ledger's lines that are fixed off it
  style_check.py --owe RULE    put what a new rule finds on the ledger
  style_check.py --list RULE   print each break of one rule, with its line
  style_check.py --self-check  the check itself on made-up sources

`make style-check`, part of `make all`. What it cannot see is everything
else in STYLE.md: names, comments, whether the C says what the listing
does. A person reads those, and reads whether a reason that the check
holds to the word never and three words more is a reason. A function
header without its page is tools/header_pages.py's to find (the same
target runs it). One more finding of issue #1334 is not counted yet: a
name the listing does not have. The text is read as clang-format leaves
it (CI's format job holds the tree to that); where a rule's pattern
could also meet another spacing, it takes that too.
"""
import collections
import hashlib
import re
import sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
SRC = READABLE / 'src'
LEDGER = READABLE / 'tests/style_owed.txt'
ORDER = ('POSITIVE', 'IS_PLUS_ZERO', 'NEGATIVE', 'IS_MINUS_ZERO')
RULES = ('ccs-order', 'ccs-default', 'ccs-not-modelled', 'ccs-test', 'raw-channel', 'continue-offset', 'own-helper',
         'decimal-offset', 'long-line', 'file-header', 'goto', 'wording')
CCS = re.compile(r'\bswitch\s*\(\s*ccs(?:_at|_register)?\(')
SWITCH = re.compile(r'\bswitch\s*\(')
CASE = re.compile(r'\bcase (\w+):')
CHANNEL = re.compile(r'\b(?:read|write|rand|wand|wor|rxor|ror)_channel\s*\('
                     r'\s*\(?(?:word|unsigned)?\)?\s*(?:0[xX][0-9a-fA-F]+|0?[0-9]+)[uU]?\s*[,)]')
CONTINUE = re.compile(r'\bcontinue_at\s*\(\s*(?:\(word\)\s*)?\(?\s*([A-Z][A-Z0-9_]*)\s*([+-])\s*(0?[0-9]+)[uU]?')
HELPER = re.compile(r'^[ \t]*#[ \t]*define[ \t]+(?:COUNT|IN_BANK|BANK_OF)\('
                    r'|^(?:static )?(?:inline )?void tc_to\(', re.M)
DECIMAL = re.compile(r'\b(?:K_)?([A-Z][A-Z0-9_]*)\s*\+\s*([89]|[1-9][0-9]+)[uU]?\b')
LABEL = re.compile(r'^#define (\w+) +\(\(word\)K_\w+\)', re.M)
NO_WORD = re.compile(r"\b(?:no word|interpretive words?|address words?|no instruction's word|"
                     r'differs with the place)\b')
SAID_NEVER = re.compile(r'\bnever\b', re.I)
REFUSAL = re.compile(r'(?:[;{}:]|\belse)\s*not_modelled\(\);')  # a statement of its own, not a guard's
GUARD = re.compile(r'\bif \(([^;{}]*)\)\s*\{?\s*not_modelled\(\);')
SIGN_TEST = re.compile(r'\b(?:POSITIVE|IS_PLUS_ZERO|NEGATIVE|IS_MINUS_ZERO)\b|\bccs(?:_at|_register)?\(')
COMMENT = re.compile(r'/\*(.*?)\*/', re.S)
WORD = re.compile(r"[A-Za-z0-9][\w+']*")
REASON_WORDS = 3
SIGNS = r'(?:POSITIVE|IS_PLUS_ZERO|NEGATIVE|IS_MINUS_ZERO)\b'
CCS_CALL = re.compile(r'\bccs(?:_at|_register)?\(')
COMPARED = re.compile(r'[=!]=\s*' + SIGNS + r'|\b' + SIGNS + r'\s*[=!]=')
# What a CCS's outcome may go to: a switch's head, a variable (not a comparison's ==), a return, a
# function's argument (the function then has the switch).
TAKEN = re.compile(r'(?:\bswitch\s*\(|(?<![=!<>])=|\breturn|\b(?!(?:if|while|for)\b)[a-z_]\w*\(|,)\s*$')
CHOICE = re.compile(r'\?[^;{}]*:\s*$|\?\s*$')                 # ... or one side of a ?: that goes to one
NO_CCS = ('agc', 'binding')
# The helpers that are one CCS at several places (tools/ccs_helpers.py holds every call of one to its
# premise): only in these may a refusal say that what would run differs with the place.
CCS_HELPERS = ('ccs_positive', 'pack_flag')
ABOVE = re.compile(r'\s*(?:/\*|\*|case \w+:\s*(?:/\*.*\*/)?\s*$)')  # a comment's line, or a case's label alone
NEVER = re.compile(r'/\*[^\n]*\bnever\b|^[^\n]*\bnever\b[^\n]*\*/', re.M)
NAMES = 'agc/names.h'
WIDTH = 100
HEADER_END = re.compile(r'^#[ \t]*include|^\w[^\n]*\(', re.M)
NO_FAITHFUL = ('agc', 'assembly')
GOTO = re.compile(r'\bgoto\b(?!_)')
WORDING = re.compile(r'\bas\s+(?:in\s+)?the\s+original\b|\bas\s+(?:yaAGC|Virtual\s+AGC)\b', re.I)
KEPT = re.compile(r'\benum sign (\w+)\s*[=;,)]')            # an outcome kept: a variable, or a parameter
FUNCTION = re.compile(r'^(?:static )?(?:inline )?(?:void|bool|word|int|unsigned|enum \w+) (\w+)\(', re.M)
HELPER_HOMES = ('agc/agc.h', 'assembly/assembly.h', 'interpreter/notation.h')
ENTRY = re.compile(r'(\S+) src/(\S+) (\S+) ([0-9a-f]{8}) (\d+)$')


def blanked(text, strings=True):
    """The text with each comment (and each string) turned to spaces, line
    for line: what is left is code."""
    pattern = r'/\*.*?\*/' + (r'|"(?:\\.|[^"\\\n])*"' if strings else '')
    return re.sub(pattern, lambda found: re.sub(r'[^\n]', ' ', found.group()), text, flags=re.S)


def for_mission(text, lm):
    """The text as one mission compiles it: the lines of the other
    mission's branch of an #if on LUMINARY099 or COMANCHE055 turned to
    blank ones. (Of an #if on anything else, both branches stay.)"""
    out, kept, taken = [], [], []   # kept: for each open #if, whether its lines stay (None: not a
    for line in text.split('\n'):   # mission's); taken: whether a branch of it has stayed already
        told = re.match(r'[ \t]*#[ \t]*(if|ifdef|ifndef|else|elif|endif)\b(.*)', line)
        if told:
            word, rest = told.group(1), told.group(2).strip()
            found = re.fullmatch(r'(!?)\s*(?:defined\s*\(?\s*)?(LUMINARY099|COMANCHE055)\s*\)?', rest)
            stays = found and ((found.group(2) == 'LUMINARY099') == lm) != bool(found.group(1))
            if word in ('if', 'ifdef', 'ifndef'):
                kept.append(stays if found and word == 'if' else None)
                taken.append(bool(kept[-1]))
            elif word == 'elif' and kept and kept[-1] is not None:
                kept[-1] = None if not found else bool(stays) and not taken[-1]
                taken[-1] = taken[-1] or bool(kept[-1])
            elif word == 'else' and kept and kept[-1] is not None:
                kept[-1] = not taken[-1]
            elif word == 'endif' and kept:
                kept.pop()
                taken.pop()
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
    cases in order, each (name, where, the code up to the next case)).
    A switch is on a CCS when its head calls one, and also when one of
    its cases is a CCS's outcome by name: the outcome was kept in a
    variable, or a function returned it."""
    out = []
    for found in SWITCH.finditer(code):
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
        if CCS.match(code, found.start()) or any(name in ORDER for name, _, _ in cases):
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
    defined = [(line_of(code, found.start()), found.group(1)) for found in FUNCTION.finditer(code)]

    def said_at(first, last):
        """The comments on the lines first to last and right above them, and the function they are in."""
        top = first
        while top > 1 and ABOVE.match(lines[top - 2]):
            top -= 1
        function = ([name for line, name in defined if line <= first] or [None])[-1]
        said = ' '.join(COMMENT.findall('\n'.join(lines[top - 1:last])))     # (as one line: a comment's later
        return ' '.join(re.sub(r'\n\s*\*(?=\s)', ' ', said).split()), function   # lines begin with a *)

    def reasoned(said, function, kind_wanted):
        """Whether a refusal's comment says that the outcome never comes, what kind of word the listing
        has there (where that is wanted), and a reason: some words besides those."""
        kind = NO_WORD.search(said)
        if not SAID_NEVER.search(said) or kind_wanted and not kind:
            return False
        if kind and kind.group() == 'differs with the place':
            return function in CCS_HELPERS
        return len(WORD.findall(SAID_NEVER.sub(' ', NO_WORD.sub(' ', said)))) >= REASON_WORDS
    seen, judged = set(), set()
    for lm in (True, False):
        mission = for_mission(code, lm)
        for at, cases in ccs_switches(mission):
            names = [name for name, _, _ in cases if name in ORDER]
            if names != sorted(names, key=ORDER.index) and ('ccs-order', line_of(mission, at)) not in seen:
                seen.add(('ccs-order', line_of(mission, at)))
                out.append(('ccs-order', line_of(mission, at), ', '.join(names)))
            missing = [name for name in ORDER if name not in names]
            if ('default' in [name for name, _, _ in cases] or missing) and \
                    ('ccs-default', line_of(mission, at)) not in seen:
                seen.add(('ccs-default', line_of(mission, at)))
                out.append(('ccs-default', line_of(mission, at), 'default:' if not missing or 'default' in
                            [name for name, _, _ in cases] else 'no case for ' + ', '.join(missing)))
            for name, where, statements in cases:
                begins = re.match(r'\s*not_modelled\(\)', statements)
                later = None if begins else REFUSAL.search(statements)
                if name in ORDER and (begins or later) and \
                        ('ccs-not-modelled', line_of(mission, where)) not in seen:
                    seen.add(('ccs-not-modelled', line_of(mission, where)))
                    call = mission.index('not_modelled', where) if begins else \
                        mission.index(statements, where) + later.end()
                    first, last = line_of(mission, where), line_of(mission, call)
                    judged.add(last)
                    if not reasoned(*said_at(first, last), not later):
                        out.append(('ccs-not-modelled', first, name))
        for found in GUARD.finditer(mission):
            line = line_of(mission, found.end())
            if SIGN_TEST.search(found.group(1)) and line not in judged and \
                    ('ccs-not-modelled', line) not in seen:
                seen.add(('ccs-not-modelled', line))
                if not reasoned(*said_at(line_of(mission, found.start()), line), True):
                    out.append(('ccs-not-modelled', line, 'if (' + ' '.join(found.group(1).split()) + ')'))
    if file.split('/')[0] not in NO_CCS:
        tests = {line_of(code, found.start()) for found in COMPARED.finditer(code)}
        for found in CCS_CALL.finditer(code):
            depth, close = 0, found.end() - 1
            for close in range(found.end() - 1, len(code)):
                depth += (code[close] == '(') - (code[close] == ')')
                if not depth:
                    break
            before = code[max(0, code.rfind(';', 0, found.start())):found.start()]
            if re.match(r'\s*[=!]=', code[close + 1:]) or not (TAKEN.search(before) or CHOICE.search(before)):
                tests.add(line_of(code, found.start()))
        for kept in KEPT.finditer(code):
            # the function the outcome is kept in: from its first line to the next function's
            first = ([line for line, _ in defined if line <= line_of(code, kept.start())] or [1])[-1]
            last = ([line for line, _ in defined if line > first] or [len(lines) + 1])[0]
            begins = len('\n'.join(lines[:first - 1])) + (first > 1)
            ends = len('\n'.join(lines[:last - 1]))
            for use in re.finditer(r'\b' + kept.group(1) + r'\b', code[begins:ends]):
                before, after = code[begins:begins + use.start()].rstrip(), code[begins + use.end():].lstrip()
                handed = before[-1:] in '(,' and after[:1] in ',)' and \
                    not re.search(r'\b(?:if|while|for)\s*\($', before)
                if not (before.endswith('enum sign') or re.match(r'=(?!=)', after) or handed or
                        before.endswith('return') or re.search(r'(?<![=!<>])=$', before) and after[:1] == ';'):
                    tests.add(line_of(code, begins + use.start()))
        out += [('ccs-test', line, ' '.join(lines[line - 1].split())[:72]) for line in tests]
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
    if file != NAMES and not file.endswith('.inc'):
        for number, line in enumerate(lines, 1):
            if len(line) > WIDTH:
                out.append(('long-line', number, f'{len(line)} characters'))
    if file.endswith('.c') and file.split('/')[0] not in NO_FAITHFUL:
        end = HEADER_END.search(code)
        if 'c64-port/' not in text[:end.start() if end else len(text)]:
            out.append(('file-header', 1, 'no file of c64-port/ named'))
    for found in GOTO.finditer(code):
        out.append(('goto', line_of(code, found.start()), 'goto'))
    # (a comment's lines as one text: the * that begins each of its later lines is blanked)
    flowed = re.sub(r'\n[ \t]*\*(?=[ \t])', lambda star: '\n' + ' ' * (len(star.group()) - 1), text)
    for found in WORDING.finditer(flowed):
        out.append(('wording', line_of(flowed, found.start()), ' '.join(found.group().split())))
    return sorted(out, key=lambda b: (RULES.index(b[0]), b[1]))


def measure():
    """Every break under src/, each with its site: [(rule, file, function, digest, ordinal, line, what)].
    function: the one the line is in ('-' above the first). digest: of the line's text, its spaces
    apart. ordinal: which line of that text in that function, counted from 1."""
    texts = {str(path.relative_to(SRC)): path.read_text() for path in sorted(SRC.rglob('*'))
             if path.suffix in ('.c', '.h', '.inc')}
    routines = collections.defaultdict(set)
    for file, text in texts.items():
        routines[file.split('/')[0]] |= set(FUNCTION.findall(text))
    labels = set(LABEL.findall(texts[NAMES]))
    found = []
    for file, text in texts.items():
        found += sited(file, text, breaks(file, text, routines[file.split('/')[0]], labels))
    return found


def sited(file, text, broken):
    """The breaks of one file, [(rule, line, what)], each with its site."""
    lines = text.split('\n')
    code = blanked(text)
    defined = [(code.count('\n', 0, found.start()) + 1, found.group(1)) for found in FUNCTION.finditer(code)]
    out, seen = [], collections.Counter()
    for rule, line, what in sorted(broken, key=lambda b: (RULES.index(b[0]), b[1])):
        function = ([name for at, name in defined if at <= line] or ['-'])[-1]
        digest = hashlib.sha1(' '.join(lines[line - 1].split()).encode()).hexdigest()[:8]
        seen[rule, function, digest] += 1
        out.append((rule, file, function, digest, seen[rule, function, digest], line, what))
    return out


def read_ledger(path=LEDGER):
    """The sites owed: {(rule, file, function, digest, ordinal)}."""
    owed = set()
    if path.is_file():
        for line in path.read_text().split('\n'):
            told = ENTRY.match(line)
            if told:
                owed.add(told.groups()[:4] + (int(told.group(5)),))
    return owed


def write_ledger(path, found, texts=None):
    """found: the breaks to list, as measure() gives them."""
    totals = collections.Counter(rule for rule, *_ in found)
    out = ['# The breaks of STYLE.md that tools/style_check.py finds and that are still',
           '# owed a fix (issue #1334), one line each: the rule, the file, the',
           '# function, a digest of the line\'s text, and which such line of the',
           '# function it is. Under each, the line as it stood when this was written.',
           '# The check holds the tree to these lines: a break that is not here fails,',
           '# and so does a line here that the tree no longer has (a fix: take it off',
           '# with `tools/style_check.py --write`, which only takes off). A file that',
           '# is not here owes nothing that the check sees. (What the check does not',
           '# see yet: README.md, Gates.)',
           '#',
           '# Owed in all: ' + (', '.join(f'{rule} {totals[rule]}' for rule in RULES if totals[rule]) or 'nothing')
           + '.',
           '']
    for rule, file, function, digest, ordinal, _, what in sorted(found, key=lambda b: (RULES.index(b[0]), b[1], b[5])):
        out += [f'{rule} src/{file} {function} {digest} {ordinal}', f'    | {what}']
    path.write_text('\n'.join(out) + '\n')


def judge(found, owed):
    """What is wrong between the breaks found and the ledger, a line each."""
    out = []
    have = {b[:5]: b for b in found}
    for site in sorted(set(have) - owed, key=lambda b: (RULES.index(b[0]), b[1], have[b][5])):
        rule, file, _, _, _, line, what = have[site]
        out.append(f'{rule}: src/{file}:{line} is not in the ledger: a new break ({what})')
    for rule, file, function, digest, ordinal in sorted(owed - set(have)):
        if rule not in RULES:
            out.append(f'the ledger names a rule this check does not have ({rule})')
        else:
            out.append(f'{rule}: src/{file}, {function}(), {digest} {ordinal} is in the ledger and not in the '
                       'tree: fixed, or its line changed; take it off (--write)')
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
    # (a part of one mission alone is still checked: each way of writing the #if)
    for condition in ('LUMINARY099', 'COMANCHE055', '!LUMINARY099', '!COMANCHE055', 'defined(COMANCHE055)',
                      'OTHER'):
        expect(f'a default under #if {condition} alone',
               f'#if {condition}\nswitch (ccs(X)) {{\ncase IS_PLUS_ZERO: return;\ndefault: break;\n}}\n#endif\n',
               [('ccs-default', 2)])
    expect('a switch that names no outcome of a CCS', 'switch (step(X)) {\ncase 1: break;\ndefault: break;\n}\n', [])
    expect('a switch on an outcome that was kept, out of order',
           'enum sign s = ccs_at(X);\nswitch (s) {\ncase NEGATIVE: break;\ncase POSITIVE: break;\n'
           'case IS_PLUS_ZERO:\ncase IS_MINUS_ZERO: break;\n}\n', [('ccs-order', 2)])
    for few, line in (('switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase NEGATIVE: break;\n}\n', 1),
                      ('enum sign s = ccs_at(X);\nswitch (s) {\ncase POSITIVE: break;\n}\n', 2),
                      ('switch(ccs_at(X)) {\ncase POSITIVE: a();\ncase IS_PLUS_ZERO:\ncase NEGATIVE: break;\n}\n', 1),
                      ('#if LUMINARY099\nswitch (ccs(X)) {\ncase POSITIVE:\ncase IS_PLUS_ZERO: return;\n'
                       '#elif COMANCHE055\nswitch (ccs(Y)) {\ncase POSITIVE: return;\n#endif\ncase NEGATIVE:\n'
                       'case IS_MINUS_ZERO: break;\n}\n', 6)):
        expect(f'a CCS switch that leaves an outcome without a case ({few[:24]})', few, [('ccs-default', line)])
    expect('one with a default', 'enum sign s = ccs_at(X);\nwhile (x) {\n}\nswitch (s) {\ncase IS_PLUS_ZERO: break;\n'
           'default: break;\n}\n', [('ccs-default', 4)])
    expect('a switch on an outcome a function returned, refusing',
           'switch (kept()) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE: break;\n'
           'case IS_MINUS_ZERO: not_modelled();\n}\n', [('ccs-not-modelled', 5)])
    refused = 'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO:\n    not_modelled();\n    return;\n' \
              'default: break;\n}\n'
    expect('an outcome of a CCS refused', refused, [('ccs-default', 1), ('ccs-not-modelled', 3)])
    followed = refused.replace('    not_modelled();', '    a();\n    not_modelled();')
    expect('an outcome that is followed to a refusal', followed, [('ccs-default', 1), ('ccs-not-modelled', 3)])
    expect('one that says the outcome never comes',
           followed.replace('    a();', '    a(); /* (never: a masked bit) OCT 7, run as TC 7 */'),
           [('ccs-default', 1)])
    expect('an outcome with a guard', refused.replace('    not_modelled();', '    if (A > 7u) not_modelled();'),
           [('ccs-default', 1)])
    expect('an outcome the listing has no word for',
           'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE: break;\n'
           'case IS_MINUS_ZERO:\n    /* (never) TCF X is the last word: the listing has no word\n     * after it. */\n'
           '    not_modelled();\n}\n', [])
    for words in ('never: a masked bit. Next are interpretive words',
                  'never: it counts down. Next is an address word'):
        expect(f'an outcome whose next word is no instruction ({words.split(". ")[-1]})',
               'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE:\n'
               f'case IS_MINUS_ZERO: not_modelled(); /* {words} */\n}}\n', [])
    for words in ('never: no word', 'never; an address word', '(never) interpretive words'):
        expect(f'one that gives the words and no reason ({words})',
               'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE:\n'
               f'case IS_MINUS_ZERO: not_modelled(); /* {words} */\n}}\n', [('ccs-not-modelled', 5)])
    helper = ('static bool %s(void)\n{\n    switch (ccs_at(A_ADDRESS)) {\n    case POSITIVE: return true;\n'
              '    case IS_PLUS_ZERO: return false;\n    case NEGATIVE:\n'
              '    case IS_MINUS_ZERO: not_modelled(); /* never; what would run differs with the place */\n    }\n'
              '    return false;\n}\n')
    expect('a helper that is one CCS at several places', helper % CCS_HELPERS[0], [])
    expect('the same words in a function that is no such helper', helper % 'some_bit', [('ccs-not-modelled', 7)])
    expect('an outcome followed to a refusal that says never and no reason',
           followed.replace('    a();', '    a(); /* never */'), [('ccs-default', 1), ('ccs-not-modelled', 3)])
    expect('an outcome refused in an else',
           refused.replace('    not_modelled();', '    if (x)\n        a();\n    else\n        not_modelled();'),
           [('ccs-default', 1), ('ccs-not-modelled', 3)])
    for guard in ('if (ccs_at(X) != POSITIVE) not_modelled(); /* CCS X */',
                  'enum sign s = ccs_at(X); if (s == NEGATIVE || s == IS_MINUS_ZERO) not_modelled();',
                  'if (s != IS_PLUS_ZERO) {\n    not_modelled();\n    return;\n}'):
        expect(f'a refusal behind a test of a CCS\'s outcome ({guard[:28]})', guard + '\n',
               [('ccs-not-modelled', 1 + guard.count('\n', 0, guard.index('not_modelled'))), ('ccs-test', 1)])
    expect('one that says never, the kind of word and why (a test still)',
           '/* (never: it counts down from 41. Next come interpretive words.) */\n'
           'if (ccs_at(X) != IS_PLUS_ZERO) not_modelled();\n', [('ccs-test', 2)])
    for test in ('if (ccs_at(X) == POSITIVE) a();', '} while (ccs_register(A_ADDRESS) != POSITIVE);',
                 'bool up = ccs_at(ADDRESS_OF(X)) == POSITIVE;', 'if (a() || ccs(X) != IS_MINUS_ZERO) b();',
                 'if (sign == NEGATIVE) a();', 'if (POSITIVE != kept) a();', '(void)ccs_at(X);',
                 'ccs_register(A_ADDRESS); /* MRCLEAN +1 */', 'at += 1u + (unsigned)ccs(fetch(X));',
                 'if (s) ccs_at(X);', 'case POSITIVE: ccs_at(Y); break;'):
        expect(f'a CCS as a test, or dropped ({test[:32]})', test + '\n', [('ccs-test', 1 + test.count('\n'))])
    for tested in ('enum sign s = ccs_at(X);\nif (s) a();', 'void f(enum sign s)\n{\n    while (s) a();',
                   'enum sign s;\ns = ccs_at(X);\nb = s ? 1 : 2;', 'enum sign s = ccs_at(X);\nif (a && !s) b();'):
        expect(f'an outcome that was kept, tested ({tested[-16:]})', tested + '\n',
               [('ccs-test', 1 + tested.count('\n'))])
    expect('a name that is a kept outcome in one function and a count in the next',
           'void f(void)\n{\n    enum sign x = ccs_at(X);\n    g(x);\n}\nvoid h(word x)\n{\n    if (x) a();\n}\n', [])
    for taken in ('enum sign s = ccs_at(X);\nswitch (s) {\ncase POSITIVE:\ncase IS_PLUS_ZERO:\ncase NEGATIVE:\n'
                  'case IS_MINUS_ZERO: break;\n}', 'enum sign s = POSITIVE, t;\nt = s;\nreturn t;',
                  'void f(enum sign s, word k)\n{\n    g(k, s);\n    h(s);\n}',
                  'sign = ts(X) ? POSITIVE : ccs_at(Y);', 'switch (ts(X) ? POSITIVE : ccs_at(Y)) {\n}',
                  'return ccs_at(X);', 'aotmark_after_ccs(ccs_at(ADDRESS_OF(MARKSTAT)));',
                  'if (ccs_positive(A_ADDRESS)) a();', '} while (ccs_positive(ADDRESS_OF(X)));',
                  'case IS_PLUS_ZERO: tang = ccs_at(ADDRESS_OF(TANG)); break;', 'if (count >= 3u) a();'):
        expect(f'a CCS whose outcome goes to a switch ({taken[:32]})', taken + '\n', [])
    expect('a binding\'s test of a sign', 'return sign_of(fetch(X)) == NEGATIVE || first == POSITIVE;\n', [],
           file='binding/x.c')
    expect('a refusal behind another test',
           'if (ts(X)) not_modelled();\nif (index > 7u) {\n    not_modelled();\n}\n', [])
    expect('one that says why above its cases\' labels',
           'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\n/* (never: a masked bit) */\n'
           'case NEGATIVE:\ncase IS_MINUS_ZERO: not_modelled(); /* the next word is an address word */\n}\n', [])
    expect('one whose kind of word is broken over two lines of its comment',
           'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE: break;\n'
           'case IS_MINUS_ZERO:\n    /* never: the count is not -0. The next word is an address\n'
           '     * word, CADR X. */\n    not_modelled();\n}\n', [])
    expect('one that does not say the outcome never comes',
           'switch (ccs_at(X)) {\ncase POSITIVE: break;\ncase IS_PLUS_ZERO: break;\ncase NEGATIVE: break;\n'
           'case IS_MINUS_ZERO: not_modelled(); /* the next word is an address word, CADR X */\n}\n',
           [('ccs-not-modelled', 5)])
    expect('a value outside a table refused', 'switch (a_word()) {\ncase 1: break;\ndefault: not_modelled();\n}\n', [])
    expect('a channel by number', 'x = rand_channel(031);\nwor_channel(013u);\nwrite_channel(6, A);\n'
           'x = rand_channel(0x1f);\nread_channel (6);\nwor_channel(013U);\n',
           [('raw-channel', 1), ('raw-channel', 2), ('raw-channel', 3), ('raw-channel', 4), ('raw-channel', 5),
            ('raw-channel', 6)])
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
    expect('a decimal offset from a label', 'tc_at((word)(POODOO + 12u));\n#define X ((word)(K_SGNCOM + 9u))\n'
           'tc_at(POODOO+12u);\ntc_at(POODOO + 12);\ntc_at(POODOO + 12U);\n',
           [('decimal-offset', 1), ('decimal-offset', 2), ('decimal-offset', 3), ('decimal-offset', 4),
            ('decimal-offset', 5)])
    expect('octal offsets, small ones and an erasable\'s', 'tc_at(POODOO + 014u);\ntc_at(POODOO + 7u);\n'
           'ts(FIXLOC + 26u);\nts(ADDRESS_OF(DSPTAB) + 11u); /* POODOO + 12u */\n', [])
    expect('a line of 101 characters', '/* ' + 'x' * 95 + ' */\n' + 'y' * 100 + '\n', [('long-line', 1)])
    expect('the same in the names', 'z' * 101 + '\n', [], file=NAMES)
    expect('the same in a table a tool writes, and a channel by number there',
           'BASIC(9, 014, 02424) /* ' + 'z' * 101 + ' */\nx = read_channel(030);\n', [('raw-channel', 2)],
           file='binding/x_keys.inc')
    expect('a file whose header names no faithful C', '/* X.agc, pp. 1-2. */\n#include "x.h"\n/* c64-port/src/x.c */\n',
           [('file-header', 1)])
    expect('the same of the tables', '/* MAIN.agc */\nint x;\n', [], file='assembly/main.c', bare=True)
    expect('a goto', 'void f(void)\n{\n    goto out;\n    goto_();\n}\n', [('goto', 3)])
    expect('the wording', '/* As the original does. */\nx(); /* as yaAGC writes it */\n/* the original listing */\n'
           '/* as in the Original */\n/* as Virtual AGC writes it, and as\n * the original has it */\n',
           [('wording', 1), ('wording', 2), ('wording', 4), ('wording', 5), ('wording', 5)])
    twice = 'void f(void)\n{\n    goto out;\n}\nvoid g(void)\n{\n    goto out;\n    goto  out;\n}\n'
    found = sited('a/a.c', twice, breaks('a/a.c', HEADER + twice)[:0] + [('goto', 3, 'goto'), ('goto', 7, 'goto'),
                                                                        ('goto', 8, 'goto')])
    tried.append('three breaks\' sites')
    digest = hashlib.sha1(b'goto out;').hexdigest()[:8]
    if [b[:5] for b in found] != [('goto', 'a/a.c', 'f', digest, 1), ('goto', 'a/a.c', 'g', digest, 1),
                                  ('goto', 'a/a.c', 'g', digest, 2)]:
        failures.append(f'three breaks\' sites: {found}')
    sites = {b[:5] for b in found}

    def held(name, owed, want):
        tried.append(name)
        got = len(judge(found, owed))
        if got != want:
            failures.append(f'{name}: {got} lines, expected {want}')
    held('a ledger that is the breaks', sites, 0)
    held('a break that is not owed', sites - {('goto', 'a/a.c', 'g', digest, 2)}, 1)
    held('an owed line that is fixed', sites | {('goto', 'a/a.c', 'g', digest, 3)}, 1)
    held('the same count in another function', sites - {('goto', 'a/a.c', 'f', digest, 1)} |
         {('goto', 'a/a.c', 'h', digest, 1)}, 2)
    held('a rule the check does not have', sites | {('nonesuch', 'a/a.c', 'f', digest, 1)}, 1)
    scratch = READABLE / 'build' / 'style_check_self.txt'
    scratch.parent.mkdir(exist_ok=True)
    write_ledger(scratch, found)
    tried.append('a ledger written and read back')
    if read_ledger(scratch) != sites:
        failures.append(f'a ledger written and read back: {read_ledger(scratch)}')
    scratch.unlink()
    for failure in failures:
        print(f'FAIL style check self-check: {failure}')
    print(f"{'FAIL' if failures else 'ok  '} style check self-check: {len(tried)} made-up sources and ledgers, "
          f'{len(failures)} judged wrongly')
    return 1 if failures else 0


def main():
    args = sys.argv[1:]
    unknown = [a for a in args if a.startswith('-') and a not in ('--write', '--owe', '--list', '--self-check')]
    if unknown:
        print(__doc__ if unknown[0] in ('-h', '--help') else f'style_check: no option {unknown[0]} (see --help)')
        return 0 if unknown[0] in ('-h', '--help') else 2
    if '--self-check' in args:
        return self_check()
    found = measure()
    if '--list' in args:
        rule = args[args.index('--list') + 1]
        for r, file, _, _, _, line, what in found:
            if r == rule:
                print(f'src/{file}:{line}: {what}')
        return 0
    owed = read_ledger()
    if '--write' in args or '--owe' in args:
        new = args[args.index('--owe') + 1] if '--owe' in args else None
        kept = [b for b in found if b[:5] in owed or b[0] == new]
        write_ledger(LEDGER, kept)
        print(f'wrote {LEDGER.relative_to(READABLE)}: {len(kept)} lines, {len(owed - {b[:5] for b in found})} '
              f'taken off, {len([b for b in kept if b[:5] not in owed])} put on')
        owed = read_ledger()
    totals = collections.Counter(rule for rule, *_ in found)
    wrong = judge(found, owed)
    for line in wrong[:40]:
        print(f'FAIL {line}')
    print(f"{'FAIL' if wrong else 'ok  '} style check against {LEDGER.relative_to(READABLE)}: owed "
          + (', '.join(f'{rule} {totals[rule]}' for rule in RULES if totals[rule]) or 'nothing')
          + f'; {len(wrong)} not as the ledger has it')
    return 1 if wrong else 0


if __name__ == '__main__':
    sys.exit(main())
