#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Every TC the readable C writes with its place, held to the listing.

The readable C carries a transfer of control by a C call. tc_at(PLACE) of
src/agc/agc.h says only where the TC stands, so that Q gets its return;
tc_to(PLACE, ROUTINE) and Pinball's tc(PLACE, ROUTINE) name the routine
too. A case test sees a wrong place only where Q is still read before the
next TC writes it, and that is seldom: a place moved by one word passes
most suites (tests/mutation_ledger.txt). So this check reads every place
against the assembled listing instead:

  - The word at the place is one the AGC runs as a TC: the listing's TC or
    TCR; or a constant whose three high bits are clear (a made-up state
    can come to run it, and it runs as a TC to its own address: `OCT
    01206, run as TC 1206`); or any word after an INDEX (the instruction
    run is then the word and the index together).
  - For tc_to() and tc(), the TC goes to the routine: the word's twelve
    low bits are the routine's address by the mission's names (not asked
    after an INDEX, nor where the routine is no constant).

A place is a name of src/agc/names.h for a label of the listing, plus or
minus constants. It is read where it is written: at the call, in a #define
the file can see, or in the arguments of the callers of a function that
passes its own argument on (`static void flash(word verb_noun, word at)`),
through as many as four callers. A word a function sets once, where it
declares it (`word at = R36 + 3u;`), reads as a #define does. Each
mission's text is checked apart (#if LUMINARY099 and the like) against
its own listing.

A place that is none of that is not checked: one computed from an index
or a table, or a register. COMPUTED below names each of them by its file,
with how many times the file writes it, and the check fails where the
list and the source differ, so no place leaves the check unnoticed. A
place written outside any function fails: nothing would read it.

The other half reads the listing: every TC to the fixed memory is named by
a place, or stands in UNNAMED below (a jump table's rows, the arms of a
CCS, a call the C writes with no place).

What the two halves leave open (the check prints the counts):

  - Of a tc_at() place the first half asks only that its word is a TC, not
    which one. A place moved to another TC shows by the second half, the
    TC it left being named no more; not so a place whose word is no TC of
    the listing to the fixed memory (a constant that is run, a word after
    an INDEX, a TC to an erasable word), nor one of two places on one TC.
    Such a place can move onto another TC, or go, unseen.
  - A COMPUTED place is held by its text and its count, and its sum is
    not worked out. The TCs those places stand on (the tables at WTLST5,
    STORJUMP, CHARIN2 and the like) are in UNNAMED with the rest, so no
    part of the check reads them.
  - A routine that is no constant (`tc(at, key_routine[key])`) is not
    compared, and tc_through(), a TC through an erasable word, is outside
    the check.

  tc_sites.py               check
  tc_sites.py --list        print every place with its word of the listing
  tc_sites.py --computed    print COMPUTED as the source stands
  tc_sites.py --self-check  run the check on made-up sources
"""
import re
import sys
from collections import ChainMap, Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import listing_names as LN  # noqa: E402
import style_check  # noqa: E402  (blanked, body_of, for_mission)

IL = LN.IL
ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / 'src'
NAMES = 'agc/names.h'
SITE = re.compile(r'\b(tc_at|tc_to|tc)\(')
DEFINITION = re.compile(r'^(?!(?:if|for|while|switch|else|return)\b)([A-Za-z_][\w \t\*]*?)\b(\w+)\(([^;{}]*)\)\s*\{',
                        re.M)
DEFINE = re.compile(r'^[ \t]*#[ \t]*define[ \t]+(\w+)[ \t]+(\S.*?)[ \t]*$', re.M)
LOCAL = re.compile(r'\b(?:const\s+)?(?:word|unsigned)\s+(\w+)\s*=\s*([^;{}]+);')
INCLUDE = re.compile(r'^[ \t]*#[ \t]*include[ \t]+"([^"]+)"', re.M)
K_VALUE = re.compile(r'^#define (K_\w+) (0[0-7]*)u$', re.M)
TOKEN = re.compile(r'\s*(\((?:word|unsigned)\)|(?:ADDRESS_OF|IN_BANK)\(\w+\)|[A-Za-z_]\w*|0[xX][0-9a-fA-F]+[uU]?'
                   r'|\d+[uU]?|[()+\-])')
CALLERS = 4     # how many callers deep a place is followed

# The places the check does not read, by file: `call(place)` as written, and how many times the file writes
# it (` x4`) where that is more than once. The count is of the places written, whichever mission
# compiles them (two on one line are two).
COMPUTED = {
    'burn_baby/burn_baby.c': [
        'tc_at(Q_ADDRESS)',
    ],
    'executive/executive.c': [
        'tc_at((word)(at + 1u)) x2',
        'tc_at((word)(at + 2u)) x2',
        'tc_at(1u)',
    ],
    'extended_verbs/front.c': [
        'tc_at(Z) x2',
    ],
    'imu_calibration/imu_calibration.h': [
        'tc_at(Q_ADDRESS + 1u)',
        'tc_at(Q_ADDRESS)',
    ],
    'imu_mode_switching/gyro_torquing.c': [
        'tc_at(at) x4',
    ],
    'integration_init/intwakeu.c': [
        'tc_at(ADDRESS_OF(INTWAKUQ))',
    ],
    'interpreter/load_store.c': [
        'tc_at(indexed(STORJUMP, entry))',
    ],
    'interpreter/shifts.c': [
        'tc_at(tc_site)',
    ],
    'p11/aterjob.c': [
        'tc_at(Q_ADDRESS)',
    ],
    'p51_p53_cm/p52.c': [
        'tc_at(ccs_positive(A_ADDRESS) ? P52T + 032u : P52T + 025u)',
    ],
    'p70_p71/p70_p71.h': [
        'tc_at(2u)',
    ],
    'pinball/keys.c': [
        'tc(CHARIN2 + 4u + key)',
        'tc_at(registers[i].at)',
    ],
    'pinball/nvsub.c': [
        'tc_at(at + 1u)',
        'tc_at(at + 3u)',
    ],
    'pinball/output_scale.c': [
        'tc_at(at) x2',
    ],
    'q_r_axis/q_r_axis.c': [
        'tc_at(0u)',
        'tc_at(at + 1u)',
        'tc_at(at + 2u)',
        'tc_at(at)',
    ],
    'self_check/self_check.c': [
        'tc_at((word)(SOPTION1 + option))',
    ],
    'servicer/servicer.c': [
        'tc_at(A_ADDRESS)',
        'tc_at(Q_ADDRESS)',
    ],
    'servicer_cm/servicer_cm.h': [
        'tc_at((word)address)',
    ],
    'stable_orbit/stable_orbit.c': [
        'tc_at(ADDRESS_OF(RTRN))',
    ],
    'sxtmark/sxtmark.c': [
        'tc_at(EBANK_ADDRESS)',
        'tc_at(Q_ADDRESS)',
        'tc_at(at + 1u)',
        'tc_at(at + 3u)',
    ],
    't4rupt/t4rupt.c': [
        'tc_at(to)',
    ],
    'tvc/tvc.h': [
        'tc_at(Q_ADDRESS + 1u)',
        'tc_at(Q_ADDRESS)',
    ],
    'tvc/tvcdaps.c': [
        'tc_at(Q_ADDRESS)',
    ],
    'tvc/tvcstroketest.c': [
        'tc_at(Q_ADDRESS)',
    ],
    'waitlist/waitlist.c': [
        'tc_at((word)(WTLST5 + 6u * n - 2u))',
    ],
}

# The listing's TCs to the fixed memory that no place names, by mission, as runs: (the label before, the
# words past it, how many).
UNNAMED = {
    'lm': [
        ('SLOPEHI', 0o4, 1), ('TEST', 0o6, 1), ('EJSCAN', 0o1, 2), ('EJSCAN', 0o5, 2), ('EJSCAN', 0o11, 1),
        ('-CCSPR', 0o3, 2), ('-CCSPR', 0o7, 2), ('-CCSPR', 0o13, 2), ('-CCSPR', 0o17, 2), ('-CCSPR', 0o23, 2),
        ('WTLST5', 0o4, 1), ('WTLST5', 0o12, 1), ('WTLST5', 0o20, 1), ('WTLST5', 0o26, 1), ('WTLST5', 0o34, 1),
        ('WTLST5', 0o42, 1), ('WTLST5', 0o50, 1), ('ENDINST', 0o0, 1), ('TCNOVAC', 0o0, 1), ('TCWAIT', 0o0, 1),
        ('TCTSKOVR', 0o0, 1), ('TCFINDVC', 0o0, 1), ('STORJUMP', 0o0, 1), ('STORJUMP', 0o2, 1), ('STORJUMP', 0o4, 1),
        ('STORJUMP', 0o6, 1), ('STORJUMP', 0o10, 1), ('STORJUMP', 0o12, 1), ('STORJUMP', 0o14, 1),
        ('STORJUMP', 0o16, 1), ('STARTSIM', 0o1, 1), ('DOFSTRT1', 0o35, 1), ('T4JUMP', 0o2, 1), ('T4JUMP', 0o6, 1),
        ('STRTGYR2', 0o5, 1), ('STRTGYR2', 0o7, 1), ('STRTGYR2', 0o11, 1), ('R61C+L1', 0o4, 1), ('PIPJOBB', 0o13, 1),
        ('ALFLT', 0o1, 1), ('PERFERAS', 0o10, 1), ('CHARIN2', 0o4, 32), ('INRELTAB', 0o17, 1), ('SIGNTEST', 0o6, 1),
        ('SIGNTEST', 0o10, 1), ('SIGNTEST', 0o12, 1), ('NVSBENDL', 0o0, 1), ('BLNKSUB1', 0o3, 1),
        ('BLNKSUB1', 0o5, 1), ('BLNKSUB1', 0o7, 1), ('BLNKSUB1', 0o11, 1), ('BLNKSUB1', 0o13, 1),
        ('BLNKSUB1', 0o15, 1), ('DISPLACE', 0o0, 3), ('M/SOUT', 0o5, 1), ('M/SOUT', 0o10, 3), ('M/SOUT', 0o15, 1),
        ('M/SOUT', 0o20, 3), ('LST2FAN', 0o0, 9), ('LST2FAN', 0o12, 19), ('VERB69', 0o1, 30),
    ],
    'cm': [
        ('SLOPEHI', 0o4, 1), ('TEST', 0o6, 1), ('EJSCAN', 0o1, 2), ('EJSCAN', 0o5, 2), ('EJSCAN', 0o11, 1),
        ('-CCSPR', 0o3, 2), ('-CCSPR', 0o7, 2), ('-CCSPR', 0o13, 2), ('-CCSPR', 0o17, 2), ('WTLST5', 0o4, 1),
        ('WTLST5', 0o12, 1), ('WTLST5', 0o20, 1), ('WTLST5', 0o26, 1), ('WTLST5', 0o34, 1), ('WTLST5', 0o42, 1),
        ('WTLST5', 0o50, 1), ('ENDINST', 0o0, 1), ('TCNOVAC', 0o0, 1), ('TCWAIT', 0o0, 1), ('TCTSKOVR', 0o0, 1),
        ('TCFINDVC', 0o0, 1), ('STORJUMP', 0o0, 1), ('STORJUMP', 0o2, 1), ('STORJUMP', 0o4, 1), ('STORJUMP', 0o6, 1),
        ('STORJUMP', 0o10, 1), ('STORJUMP', 0o12, 1), ('STORJUMP', 0o14, 1), ('STORJUMP', 0o16, 1),
        ('STARTSIM', 0o1, 1), ('DOFSTART', 0o33, 1), ('MARKOK', 0o1, 1), ('MARKOK', 0o3, 1), ('MARKOK', 0o5, 1),
        ('MARKOK', 0o7, 1), ('MARKOK', 0o11, 1), ('STRTGYR2', 0o5, 1), ('STRTGYR2', 0o7, 1), ('STRTGYR2', 0o11, 1),
        ('P52T', 0o25, 1), ('P52T', 0o32, 1), ('PHSCHK2', 0o4, 1), ('PCDUDOTS', 0o4, 1), ('PCDUDOTS', 0o12, 1),
        ('PERORLIM', 0o0, 1), ('PFORWARD', 0o11, 1), ('PACLIM', 0o0, 1), ('PPRECOMP', 0o11, 1), ('PCOPYCYC', 0o0, 1),
        ('YERORLIM', 0o0, 1), ('YFORWARD', 0o11, 1), ('YACLIM', 0o0, 1), ('YPRECOMP', 0o11, 1), ('YCOPYCYC', 0o0, 1),
        ('STRKTSTI', 0o0, 1), ('RSTGTS1', 0o27, 1), ('ALFLT1', 0o2, 1), ('CHARIN2', 0o4, 32), ('INRELTAB', 0o17, 1),
        ('SIGNTEST', 0o6, 1), ('SIGNTEST', 0o10, 1), ('SIGNTEST', 0o12, 1), ('NVSBENDL', 0o0, 1),
        ('BLNKSUB1', 0o3, 1), ('BLNKSUB1', 0o5, 1), ('BLNKSUB1', 0o7, 1), ('BLNKSUB1', 0o11, 1),
        ('BLNKSUB1', 0o13, 1), ('BLNKSUB1', 0o15, 1), ('DISPLACE', 0o0, 3), ('M/SOUT', 0o5, 1), ('M/SOUT', 0o10, 3),
        ('M/SOUT', 0o15, 1), ('M/SOUT', 0o20, 3), ('LST2FAN', 0o0, 9), ('LST2FAN', 0o12, 19), ('VERB69', 0o1, 26),
        ('VERB69', 0o34, 3),
    ],
}


class Unread(Exception):
    """A place or a routine that is no constant: .name is the name that stopped the reading, or None."""

    def __init__(self, why, name=None):
        super().__init__(why)
        self.name = name


class Mission:
    """One mission's listing and names: where each label's word is, and each name's value."""

    def __init__(self, key):
        self.key, self.lm = key, key == 'lm'
        path = str(LN.PORT / IL.LISTINGS[key])
        listing = IL.Listing(path)
        equals = LN.symbols(path)
        self.line_at, self.labels, self.label_of = {}, listing.labels, {}
        for line in listing.lines:
            for i in range(len(line.words)):
                self.line_at[line.phys + i] = (line, i)
        for name, where in listing.labels.items():
            self.label_of.setdefault(where, name)
        self.at = {}
        for kind, names in (('K_', LN.CODE), ('I_', LN.INTERPRETIVE)):
            for name in names:
                base, _, offset = name.partition('+')
                if name in listing.labels:
                    where = listing.labels[name]
                elif offset.isdigit() and base in listing.labels:
                    where = listing.labels[base] + int(offset)
                elif offset.isdigit() and base in equals:
                    where = equals[base] + int(offset)
                else:
                    where = equals.get(name)
                if where is not None:
                    self.at.setdefault(kind + LN.macro(name), where)
        self.k = {k: int(value, 8) for k, value in K_VALUE.findall((SRC / 'agc' / (key + '_names.inc')).read_text())}

    def label(self, name):
        """A name of the mission's names as (label, 0) for a label of the fixed memory, (None, its value) for
        another; None if it is no name of them."""
        if name in self.at:
            return name, 0
        return (None, self.k[name]) if name in self.k else None

    def spot(self, where):
        """A word of the listing as (the label before it in its file, the words past the label)."""
        at, file = where, self.line_at[where][0].src
        while at not in self.label_of and at - 1 in self.line_at and self.line_at[at - 1][0].src == file:
            at -= 1
        return self.label_of.get(at, '%05o' % at), where - at

    def calls(self):
        """Where the listing's TCs to the fixed memory are (a TC to an erasable word or a register is a
        return's, or tc_through()'s)."""
        return {where for where, (line, part) in self.line_at.items()
                if line.op in ('TC', 'TCR') and not part and line.words[0] & 0o7777 >= 0o2000}

    def address(self, label):
        """A label's twelve bits, as a TC to it holds them."""
        return LN.labels.logical(self.at[label]) & 0o7777


class Sources:
    """The source as one mission compiles it: each file's code, #defines and functions."""

    def __init__(self, texts, lm):
        self.code = {rel: style_check.for_mission(style_check.blanked(text), lm) for rel, text in texts.items()}
        self.own = {rel: dict(DEFINE.findall(code)) for rel, code in self.code.items()}
        self.headers = {rel: INCLUDE.findall(style_check.for_mission(style_check.blanked(text, strings=False), lm))
                        for rel, text in texts.items()}
        self.seen, self.defined, self.local = {}, {}, {}

    def defines(self, rel):
        """The #defines a file can see: its own and those of the headers it includes."""
        if rel not in self.seen:
            self.seen[rel] = found = {}
            todo, done = [rel], set()
            while todo:
                at = todo.pop()
                if at in done or at not in self.own:
                    continue
                done.add(at)
                for name, body in self.own[at].items():
                    found.setdefault(name, body)
                for header in self.headers[at]:
                    todo.append((Path(at).parent / header).as_posix())
                    todo[-1] = re.sub(r'[^/]+/\.\./', '', todo[-1])
        return self.seen[rel]

    def names(self, rel, function):
        """The names an expression in a function can use: the file's #defines, and the function's own
        words that are set once, where they are declared (`word at = R36 + 3u;`)."""
        if function is None:
            return self.defines(rel)
        key = rel, function[2]
        if key not in self.local:
            body = self.code[rel][function[2]:function[3]]
            found = {}
            for name, value in LOCAL.findall(body):
                if len(re.findall(r'(?:\+\+|--)\s*%s\b|\b%s\s*(?:=(?!=)|[-+*/|&^]=|<<=|>>=|\+\+|--)' % (name, name),
                                  body)) == 1:
                    found[name] = value
            self.local[key] = ChainMap(found, self.defines(rel)) if found else self.defines(rel)
        return self.local[key]

    def functions(self, rel):
        """A file's function definitions: [(name, [its arguments' names], first, last, own to the file)]."""
        if rel not in self.defined:
            self.defined[rel] = out = []
            code = self.code[rel]
            for found in DEFINITION.finditer(code):
                span = style_check.body_of(code, found.end() - 1)
                if span:
                    names = [re.findall(r'\w+', part)[-1] for part in split(found.group(3)) if re.search(r'\w', part)]
                    own = 'static' in found.group(1).split() and rel.endswith('.c')
                    out.append((found.group(2), names, span[0] - 1, span[1], own))
        return self.defined[rel]

    def within(self, rel, at):
        for function in self.functions(rel):
            if function[2] < at <= function[3]:
                return function
        return None

    def calls(self, rel, function):
        """Every call of a function: [(file, where, [arguments])]. One own to its file is called there only."""
        out = []
        for other in sorted(self.code) if not function[4] else [rel]:
            code = self.code[other]
            for found in re.finditer(r'\b' + function[0] + r'\(', code):
                if self.within(other, found.start()):
                    arguments = split(inside(code, found.end()))
                    if len(arguments) == len(function[1]):
                        out.append((other, found.start(), arguments))
        return out


def inside(code, at):
    """The text of the parentheses that open just before code[at]."""
    depth = 0
    for end in range(at, len(code)):
        if code[end] == '(':
            depth += 1
        elif code[end] == ')':
            if not depth:
                return code[at:end]
            depth -= 1
    return code[at:]


def split(arguments):
    """A call's arguments, apart."""
    out, depth, start = [], 0, 0
    for at, c in enumerate(arguments):
        depth += (c == '(') - (c == ')')
        if c == ',' and not depth:
            out.append(arguments[start:at].strip())
            start = at + 1
    return out + [arguments[start:].strip()]


def read(text, mission, defines, depth=0):
    """An expression as (a label's name or None, the number to add). Unread if it is no such thing."""
    tokens, at = [], 0
    text = text.strip()
    while at < len(text):
        found = TOKEN.match(text, at)
        if not found:
            raise Unread('%r is no sum of a name and numbers' % ' '.join(text.split()))
        tokens.append(found.group(1))
        at = found.end()
    where = [0]

    def term():
        if where[0] >= len(tokens):
            raise Unread('%r ends early' % text)
        token = tokens[where[0]]
        where[0] += 1
        if token in ('(word)', '(unsigned)'):
            return term()
        if token == '(':
            got = total()
            if where[0] >= len(tokens) or tokens[where[0]] != ')':
                raise Unread('%r: a parenthesis is not closed' % text)
            where[0] += 1
            return got
        if token.startswith(('ADDRESS_OF(', 'IN_BANK(', 'K_')):
            name = token if token.startswith('K_') else \
                ('K_' if token.startswith('A') else 'I_') + token[:-1].partition('(')[2]
            if mission.label(name) is None:
                raise Unread('%s is no name of the listing' % token)
            return mission.label(name)
        if token[0].isdigit():
            digits = token.rstrip('uU')
            return None, int(digits, 16 if digits[:2] in ('0x', '0X') else 8 if digits[0] == '0' else 10)
        if token in ('+', '-', ')'):
            raise Unread('%r: %s where a name or a number should be' % (text, token))
        if token in defines and depth < 8:
            return read(defines[token], mission, defines, depth + 1)
        raise Unread('%s is no label of the listing' % token, token)

    def total():
        name, number = term()
        while where[0] < len(tokens) and tokens[where[0]] in ('+', '-'):
            minus = tokens[where[0]] == '-'
            where[0] += 1
            other, more = term()
            if other and (name or minus):
                raise Unread('%r: two labels' % text)
            name, number = name or other, number - more if minus else number + more
        return name, number

    got = total()
    if where[0] != len(tokens):
        raise Unread('%r: more than a sum' % text)
    return got


def through(sources, mission, rel, function, texts, depth=0, chain=()):
    """Each way the expressions of one call come to be read: [(chain of callers, [(name, number) or Unread])].
    Where one rests on an argument of the function it is in, once for every caller of the function."""
    got, needs = [], False
    for text in texts:
        try:
            got.append(read(text, mission, sources.names(rel, function)))
        except Unread as why:
            got.append(why)
            needs = needs or (function is not None and why.name in function[1])
    if not needs:
        return [(chain, got)]
    calls = sources.calls(rel, function) if depth < CALLERS else []
    if not calls:
        return [(chain, got)]
    out = []
    for other, at, arguments in calls:
        caller = sources.within(other, at)
        passed = []
        for text in texts:
            for name, argument in zip(function[1], arguments):
                text = re.sub(r'\b%s\b' % re.escape(name),
                              lambda _: '(' + expanded(argument, sources, other, caller) + ')', text)
            passed.append(expanded(text, sources, rel, function, keep=function[1]))
        line = sources.code[other].count('\n', 0, at) + 1
        out += through(sources, mission, other, caller, passed, depth + 1,
                       chain + ('%s:%d' % (other, line),))
    return out


def expanded(text, sources, rel, function, keep=()):
    """An expression with the names of its place written out, so that it reads the same in another."""
    defines = sources.names(rel, function)
    for _ in range(8):
        new = re.sub(r'\b[A-Za-z_]\w*\b', lambda found: '(' + defines[found.group()] + ')'
                     if found.group() in defines and found.group() not in keep else found.group(), text)
        if new == text:
            break
        text = new
    return text


def sites(sources, mission):
    """Every tc_at(), tc_to() and tc() of the source: [(file, line, the call's name, [arguments], ways)];
    and those written outside any function, which nothing reads: [(file, line)]."""
    out, outside = [], []
    for rel in sorted(sources.code):
        code = sources.code[rel]
        for found in SITE.finditer(code):
            function = sources.within(rel, found.start())
            line = code.count('\n', 0, found.start()) + 1
            if function is None:
                # A helper's own declaration (`static inline void tc_at(word at)`) is no place; any other
                # (a static initialiser, a #define's body) is not read, and fails.
                before = code[code.rfind('\n', 0, found.start()) + 1:found.start()]
                if not re.fullmatch(r'(?:static\s+)?(?:inline\s+)?void\s+', before):
                    outside.append((rel, line))
                continue
            if function[0] in ('tc_to', 'tc'):
                continue                # the helper's tc_at(at), read where the helper is called
            arguments = split(inside(code, found.end()))
            out.append((rel, line, found.group(1), arguments,
                        through(sources, mission, rel, function, arguments[:2])))
    return out, outside


def judge(mission, name, got):
    """One way of one call against the listing: (what the word is, or None; the failure, or None)."""
    label, number = got[0]
    at = mission.at[label] + number
    here, before = mission.line_at.get(at), mission.line_at.get(at - 1)
    if not here:
        return None, 'the listing has no word there'
    line, part = here
    word = line.words[part]
    indexed = bool(before) and before[0].op in ('INDEX', 'NDX')
    listed = line.op in ('TC', 'TCR') and not part
    shown = '%s %05o %s %s' % (line.where(), word, line.op, line.operand)
    if not (listed or indexed or not word & 0o70000):
        return shown, 'the word there is no TC: ' + shown
    if len(got) > 1 and not isinstance(got[1], Unread) and not indexed:
        routine, more = got[1]
        want = ((mission.address(routine) if routine else 0) + more) & 0o7777
        if word & 0o7777 != want:
            return shown, 'the TC there is to %04o, the routine is %04o: %s' % (word & 0o7777, want, shown)
    return shown + (' (after an INDEX)' if indexed else '' if listed else ' (a constant, run)'), None


def written(name, arguments):
    return '%s(%s)' % (name, ' '.join(arguments[0].split()))


def check(sources, mission, show=None):
    """(failures, {file: {place not read: the lines it is written on}}, counts, the words named) of one
    mission's sources."""
    failures, computed, named = [], {}, Counter()
    calls = mission.calls()
    counts = dict(places=0, callers=0, constants=0, indexed=0, routines=0, apart=0, unread=0)
    found, outside = sites(sources, mission)
    for rel, line in outside:
        failures.append('%s %s:%d: a place written outside any function, which the check does not read'
                        % (mission.key.upper(), rel, line))
    for rel, line, name, arguments, ways in found:
        where = '%s %s:%d %s' % (mission.key.upper(), rel, line, written(name, arguments))
        unread = False
        for chain, got in ways:
            via = ''.join(', from ' + caller for caller in chain)
            if isinstance(got[0], Unread) or got[0][0] is None:
                unread = True
                continue
            word, failure = judge(mission, name, got)
            if failure:
                failures.append('%s%s: %s' % (where, via, failure))
                continue
            counts['places'] += 1
            named[mission.at[got[0][0]] + got[0][1]] += 1
            counts['apart'] += mission.at[got[0][0]] + got[0][1] not in calls
            counts['unread'] += len(got) > 1 and isinstance(got[1], Unread)
            counts['callers'] += bool(chain)
            counts['constants'] += word.endswith('run)')
            counts['indexed'] += word.endswith('INDEX)')
            counts['routines'] += len(got) > 1 and not isinstance(got[1], Unread) and not word.endswith('INDEX)')
            if show:
                show('%s%s: %s' % (where, via, word))
        if unread:
            # (a place written twice on one line counts twice: the line and which of them)
            place = written(name, arguments)
            ordinal = sum(1 for at in computed.get(rel, {}).get(place, ()) if at[0] == line)
            computed.setdefault(rel, {}).setdefault(place, set()).add((line, ordinal))
    counts['twice'] = sum(1 for where, times in named.items() if times > 1 and where in calls)
    return failures, computed, counts, set(named)


def texts_of(root=SRC):
    return {path.relative_to(root).as_posix(): path.read_text() for path in sorted(root.rglob('*.[ch]'))}


def runs(mission, words):
    """Words of the listing as runs: [(label, words past it, how many)]."""
    out = []
    for where in sorted(words):
        label, past = mission.spot(where)
        if out and out[-1][0] == label and out[-1][1] + out[-1][2] == past:
            out[-1] = (label, out[-1][1], out[-1][2] + 1)
        else:
            out.append((label, past, 1))
    return out


def run(texts, missions, listed=COMPUTED, unnamed=None, show=None, say=print):
    """Both missions: the failures, with those of COMPUTED and UNNAMED against the source. (unnamed: None
    for UNNAMED, or {} to leave that half out.)"""
    failures, computed, left = [], {}, {}
    unnamed = UNNAMED if unnamed is None else unnamed
    for mission in missions:
        found, unread, counts, named = check(Sources(texts, mission.lm), mission, show)
        failures += found
        left[mission.key] = runs(mission, mission.calls() - named)
        if unnamed:
            have = {(label, past + i) for label, past, count in left[mission.key] for i in range(count)}
            want = {(label, past + i) for label, past, count in unnamed.get(mission.key, ()) for i in range(count)}
            for label, past in sorted(have - want):
                failures.append('%s: no place names the TC at %s +%o, and UNNAMED does not have it'
                                % (mission.key.upper(), label, past))
            for label, past in sorted(want - have):
                failures.append('%s: UNNAMED has the TC at %s +%o, and %s' % (
                    mission.key.upper(), label, past, 'a place names it' if label in mission.labels and
                    mission.labels[label] + past in named else 'the listing has no such TC'))
            say('%s: %d of the listing\'s %d TCs to the fixed memory named, %d not (UNNAMED)'
                % (mission.key.upper(), len(mission.calls() & named), len(mission.calls()), len(have)))
        for rel, places in unread.items():
            for place, lines in places.items():
                computed.setdefault(rel, {}).setdefault(place, set()).update(lines)
        say('%s: %d places, each a TC of the listing (%d read through a caller, %d a constant that is run, '
            '%d after an INDEX); %d with the routine the TC goes to, %d with a routine that is no constant'
            % (mission.key.upper(), counts['places'], counts['callers'], counts['constants'], counts['indexed'],
               counts['routines'], counts['unread']))
        say('%s: %d places stand on a word that is no TC to the fixed memory, and %d of the TCs are named by '
            'more than one place: the half that reads the listing\'s TCs does not see such a place go'
            % (mission.key.upper(), counts['apart'], counts['twice']))
    for rel in sorted(set(computed) | set(listed)):
        have, want = computed.get(rel, {}), times_listed(listed.get(rel, ()))
        for place in sorted(set(have) - set(want)):
            failures.append('%s: %s is not read against the listing, and COMPUTED does not name it' % (rel, place))
        for place in sorted(set(want) - set(have)):
            failures.append('%s: COMPUTED names %s, and the source has no such place unread' % (rel, place))
        for place in sorted(set(want) & set(have)):
            if len(have[place]) != want[place]:
                failures.append('%s: %s is written %d times unread, and COMPUTED has it %d times'
                                % (rel, place, len(have[place]), want[place]))
    say('%d places not read, %d as written (COMPUTED)' % (
        sum(len(lines) for places in computed.values() for lines in places.values()),
        sum(len(places) for places in computed.values())))
    return failures, computed, left


def times_listed(places):
    """COMPUTED's entries of one file: {the place as written: how many times}."""
    out = {}
    for entry in places:
        often = re.fullmatch(r'(.*\S) x(\d+)', entry)
        out[often.group(1) if often else entry] = int(often.group(2)) if often else 1
    return out


def self_check(missions):
    """The check on made-up sources, with names taken from the LM's listing as it stands."""
    lm = missions[0]
    value_of = {}
    names = sorted(name for name in lm.at if name.startswith('K_'))
    for name in names:
        value_of.setdefault(lm.address(name), name)
    good = None
    for name in names:
        here, after = lm.line_at.get(lm.at[name]), lm.line_at.get(lm.at[name] + 1)
        if here and after and here[0].op == 'TC' and not here[1] and here[0].words[0] >= 0o2000 \
                and here[0].words[0] in value_of and after[0].words[after[1]] & 0o70000 \
                and after[0].op not in ('TC', 'TCR') and value_of[here[0].words[0]] != name:
            good = name, value_of[here[0].words[0]]
            break
    if not good:
        return ['self-check: the LM listing has no label at a TC to a named routine']
    place, routine = good
    other = next(name for name in names if lm.address(name) != lm.address(routine))
    head = '#include "x.h"\n'

    def outcome(body, header='', listed=None):
        texts = {'x/x.c': head + body, 'x/x.h': header}
        failures, computed, _ = run(texts, [lm], listed or {}, {}, say=lambda _: None)
        return len(failures), sorted(place for places in computed.values() for place in places)

    parts = [
        ('a TC at its place, to its routine', 'void f(void) { tc_to(%s, %s); tc_at(%s); }' % (place, routine, place),
         '', None, (0, [])),
        ('a place one word on', 'void f(void) { tc_at(%s + 1u); }' % place, '', None, (1, [])),
        ('a place with a cast, one on and one back', 'void f(void) { tc_at((word)(%s + 1u - 1u)); }' % place, '',
         None, (0, [])),
        ('another routine', 'void f(void) { tc_to(%s, %s); }' % (place, other), '', None, (1, [])),
        ('a place in a comment', 'void f(void) { /* tc_at(%s + 1u); */ }' % place, '', None, (0, [])),
        ("a place by the header's #define", 'void f(void) { tc_at(HERE); tc_at(HERE + 1u); }',
         '#define HERE (word)(%s)\n' % place, None, (1, [])),
        ('a place passed by two callers, one wrong',
         'static void g(word at) { tc_to(at, %s); }\nvoid f(void) { g(%s); g(%s + 1u); }' % (routine, place, place),
         '', None, (1, [])),
        ('a place passed through two functions',
         'static void g(word at) { tc_at(at + 1u); }\nstatic void h(word to) { g(to - 1u); }\n'
         'void f(void) { h(%s); h(%s + 1u); }' % (place, place), '', None, (1, [])),
        ("the other mission's place", '#if COMANCHE055\nvoid f(void) { tc_at(%s + 1u); }\n#endif\n' % place, '',
         None, (0, [])),
        ('a computed place, not named', 'void f(unsigned n) { tc_at(%s + n); }' % place, '', None,
         (1, ['tc_at(%s + n)' % place])),
        ('a computed place, named', 'void f(unsigned n) { tc_at(%s + n); }' % place, '',
         {'x/x.c': ['tc_at(%s + n)' % place]}, (0, ['tc_at(%s + n)' % place])),
        ('a name COMPUTED has and the source has not', 'void f(void) { tc_at(%s); }' % place, '',
         {'x/x.c': ['tc_at(n)']}, (1, [])),
        ('a register', 'void f(void) { tc_at(2u); }', '', {'x/x.c': ['tc_at(2u)']}, (0, ['tc_at(2u)'])),
        ('a computed place written twice, named once', 'void f(unsigned n) { tc_at(%s + n);\n tc_at(%s + n); }'
         % (place, place), '', {'x/x.c': ['tc_at(%s + n)' % place]}, (1, ['tc_at(%s + n)' % place])),
        ('a computed place written twice, named twice', 'void f(unsigned n) { tc_at(%s + n);\n tc_at(%s + n); }'
         % (place, place), '', {'x/x.c': ['tc_at(%s + n) x2' % place]}, (0, ['tc_at(%s + n)' % place])),
        ('a computed place written twice on one line, named once',
         'void f(unsigned n) { tc_at(%s + n); tc_at(%s + n); }' % (place, place), '',
         {'x/x.c': ['tc_at(%s + n)' % place]}, (1, ['tc_at(%s + n)' % place])),
        ('a computed place written twice on one line, named twice',
         'void f(unsigned n) { tc_at(%s + n); tc_at(%s + n); }' % (place, place), '',
         {'x/x.c': ['tc_at(%s + n) x2' % place]}, (0, ['tc_at(%s + n)' % place])),
        ('a place outside any function', 'static const int t = tc_at(%s + 1u);\nvoid f(void) { }' % place, '',
         None, (1, [])),
        ("a helper's own declaration", 'static inline void tc_at(word at)\n{\n}\nvoid f(void) { tc_at(%s); }'
         % place, '', None, (0, [])),
    ]
    bad = []
    for name, body, header, listed, want in parts:
        have = outcome(body, header, listed)
        if have != want:
            bad.append('self-check: %s: %r, not %r' % (name, have, want))
    # The other half: every TC of the listing named, or in the list.
    texts = {'x/x.c': 'void f(void) { tc_at(%s); }' % place}
    rest = runs(lm, lm.calls() - {lm.at[place]})
    label, past, count = rest[0]
    halves = [
        ('every TC named or listed', rest, 0),
        ('a TC neither named nor listed', [(label, past + 1, count - 1)] + rest[1:], 1),
        ('a TC listed that a place names', rest + [lm.spot(lm.at[place]) + (1,)], 1),
        ('a TC listed that the listing has not', rest + [(label, 0o7777, 1)], 1),
    ]
    for name, listed, want in halves:
        have = len(run(texts, [lm], {}, {'lm': listed}, say=lambda _: None)[0])
        if have != want:
            bad.append('self-check: %s: %d failures, not %d' % (name, have, want))
    print('tc-sites self-check: %d parts, %d wrong' % (len(parts) + len(halves), len(bad)))
    return bad


def main(argv):
    missions = [Mission('lm'), Mission('cm')]
    if '--self-check' in argv:
        failures = self_check(missions)
    else:
        show = print if '--list' in argv else None
        quiet = '--computed' in argv or '--unnamed' in argv
        failures, computed, left = run(texts_of(), missions, show=show, say=(lambda _: None) if quiet else print)
        if '--computed' in argv:
            for rel in sorted(computed):
                print('    %r: [' % rel)
                for place in sorted(computed[rel]):
                    times = len(computed[rel][place])
                    print('        %r,' % (place + (' x%d' % times if times > 1 else '')))
                print('    ],')
        if '--unnamed' in argv:
            for mission in missions:
                print('    %r: [' % mission.key)
                line = '       '
                for run_of in left[mission.key]:
                    entry = ' (%r, 0o%o, %d),' % run_of
                    if len(line) + len(entry) > 118:
                        print(line)
                        line = '       '
                    line += entry
                print(line + '\n    ],')
        if quiet:
            return 0
    for failure in failures:
        print('FAIL ' + failure)
    print('tc-sites: %d failures' % len(failures))
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
