#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Hold each routine's header to its page of the listing (STYLE.md, "Comments
and listing references").

A function of src/ that stands at a place of the listing gives that place's
page in its header, for each mission it is built for. The place is known in
two ways, and this reads both:

  the keys      a table entry {KEY(bank, LABEL + n), function} or
                {LABEL + n, function}, in the binding or in the directory's
                own table of places;
  the header    its first words, LABEL or LABEL +n, when LABEL is a label of
                the mission's listing.

The page is read from the assembled listing (c64-port's assembly.lst, the
one tools/listing_names.py reads): the word's line there, and the last page
mark before that line in the listing file.

What is counted as a miss:

  no-page   the header does not give the page of the function's place, for
            a mission the function is built for;
  place     the header's first words name a place, the keys another, and
            the two are not one word of the listing;
  decimal   the header's first words give an offset in decimal with no D
            (the listing's offsets are octal: STYLE.md).

A function no key names and whose header does not begin with a label is a
helper (an idiom of the listing written once), and has no page.

  header_pages.py               check: any miss fails
  header_pages.py --list        each miss, with its place and page
  header_pages.py --write       put the pages in (and the octal offsets)
  header_pages.py --self-check  judge made-up headers
"""
import bisect
import collections
import re
import sys
import textwrap
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
PORT = READABLE.parent / 'c64-port'
sys.path.insert(0, str(PORT / 'tools'))
sys.path.insert(0, str(READABLE / 'tools'))
import interp_listing as IL  # noqa: E402
import listing_names as LN  # noqa: E402

MISSIONS = (('lm', 'LM'), ('cm', 'CM'))
NO_ROUTINES = ('agc', 'assembly', 'binding')
WIDTH = 77
MARK = re.compile(r'##? Page +(\d+)\b')  # (a file's history may say "#<tab>Page 924 corrected")
# An instruction's name at the head of a comment is not a label (CM has INDEX).
OPS = frozenset('INDEX NDX TC TCF TCR CA CAF CS DCA DCS XCH LXCH DXCH TS AD ADS SU MASK MSK CCS BZF '
                'BZMF EXTEND INHINT RELINT'.split())
FUNCTION = re.compile(r'(/\*(?:(?!\*/).)*\*/)\n((?:static )?(?:inline )?(?:const )?'
                      r'(?:void|bool|word|int|unsigned|double|enum \w+|struct \w+ \*?|[a-z_0-9]+_t)'
                      r' ?\**(\w+)\([^;{]*\)\n\{)', re.S)
OFFSET = r'(?: ([+-]) ?(\d+)(D?)(?!\w|\.\w))?'  # ("TIG -30.1" is no offset; "BLANKSUB +8." is)
LEAD = re.compile(r'/\* ([^\s,:;()]+)' + OFFSET)
WHOLE = re.compile(r'/\* ((\S+?)[,.:;]?)(?=\s)')
STOPPED = re.compile(r'/\* ([^\s,:;()]+?)(?=\.(?:\s|$))()()()')
LISTED = re.compile(r'([+-])(\d+)(D?)')
MORE = re.compile(r'(?:,? (?:and )?[+-]\d+D?(?!\w|\.\w))*')  # "+5, +6 and +7"
CLAIM = re.compile(r'\bpp?\. ?(\d+(?:(?:-|, | and | to )\d+)*)')  # p. 7; pp. 7-9; pp. 7, 9 and 12
# "VB64 +6 (LM) or +7 (CM)": an offset for each mission.
EACH = re.compile(r' \(LM\) or ([+-]) ?(\d+)(D?) \(CM\)')
IF = re.compile(r'^#\s*(if|ifdef|ifndef|else|elif|endif)\b(.*)$', re.M)
# A table's entry: {LABEL + n, function}, {KEY(bank, LABEL + n), function},
# {K36(K_LABEL + n), function}; in a binding also K(LABEL + n, function) and
# OLDER(function, LABEL + n).
PLACE = r'(?:\(word\)\s*)?\(*((?:[KI]_)?[A-Z_][A-Z0-9_]*)(?:\s*([+-])\s*(0?[0-9]+)u?)?\)*'
PAIR = re.compile(r'\{\s*(?:[A-Z][A-Z0-9_]*\(\s*(?:\w+\s*,\s*)?)?' + PLACE + r'\s*,\s*([a-z_][a-z0-9_]*)\s*[,}]')
CALLED = re.compile(r'^\s*[A-Z][A-Z0-9_]*\(' + PLACE + r',\s*([a-z_][a-z0-9_]*)\)', re.M)
TURNED = re.compile(r'^\s*[A-Z][A-Z0-9_]*\(([a-z_][a-z0-9_]*),\s*' + PLACE + r'\)', re.M)
INCLUDE = re.compile(r'^#include "\.\./(\w+)/', re.M)
DEFINE = re.compile(r'^#define (\w+)\s+\(\((?:word|unsigned)\)[KI]_(\w+)\)', re.M)
ONLY = {'LUMINARY099': 'lm', 'COMANCHE055': 'cm', '!LUMINARY099': 'cm', '!COMANCHE055': 'lm'}


def first_mark(path):
    """A listing file's first page. The two transcriptions name some files
    differently (R60,R62.agc and R60_62.agc), so a file is known by this."""
    for line in open(path, errors='replace'):
        mark = MARK.match(line)
        if mark:
            return int(mark.group(1))
    return None


def listing_files():
    """{directory of src/: {mission: the listing files it is written from, each
    by its first page}}, from COVERAGE."""
    out = collections.defaultdict(lambda: collections.defaultdict(set))
    tree = {'Luminary099': 'lm', 'Comanche055': 'cm'}
    for line in open(READABLE / 'COVERAGE'):
        line = line.split('#')[0].split()
        if line:
            out[line[1].rstrip('/').split('/')[-1]][tree[line[0].split('/')[0]]].add(
                first_mark(READABLE.parent / line[0]))
    return out


class Pages:
    """A mission's listing: each label's word, each word's page."""

    def __init__(self, key):
        path = PORT / IL.LISTINGS[key]
        self.labels = IL.Listing(str(path)).labels
        self.page, self.file = {}, {}
        marks, source = {}, 'MAIN.agc'
        for raw in open(path, errors='replace'):
            if raw.startswith('(End of include-file'):
                source = 'MAIN.agc'
                continue
            if not IL.PREFIX.match(raw):
                continue
            if raw[15:].startswith('$'):
                source = raw[16:].split()[0]
                continue
            address = raw[15:22].strip()
            banked, fixed = IL.BANKED.match(address), IL.FIXED_FIXED.match(address)
            if banked:
                bank, at = int(banked.group(1), 8), int(banked.group(2), 8)
            elif fixed and int(fixed.group(1), 8) >= 0o4000:
                at = int(fixed.group(1), 8)
                bank = 2 if at < 0o6000 else 3
            else:
                continue
            words = sum(1 for a, b in ((33, 38), (39, 44)) if IL.OCTAL5.match(raw[a:b]))
            if not words:
                continue
            if source not in marks:
                marks[source] = [(n, int(m.group(1))) for n, line in
                                 enumerate(open(path.parent / source, errors='replace'), 1)
                                 for m in [MARK.match(line)] if m]
            mark = bisect.bisect_right(marks[source], (int(raw[7:13]), 1 << 30)) - 1
            for i in range(words):
                self.page.setdefault(IL.physical(bank, at) + i, marks[source][mark][1] if mark >= 0 else None)
                self.file.setdefault(IL.physical(bank, at) + i, marks[source][0][1] if marks[source] else None)
        self.spelt = collections.defaultdict(list)  # the C spelling -> the listing's labels
        for label in self.labels:
            self.spelt[LN.macro(label)].append(label)

    def word(self, place):
        return self.labels[place[0]] + place[1]

    def within(self, place, files):
        """Whether the place is a word of one of the listing files."""
        return self.file.get(self.word(place)) in files

    def page_of(self, place):
        return self.page.get(self.word(place))


def missions_at(text, position):
    """The missions the line at position is compiled for."""
    stack = []
    for found in IF.finditer(text, 0, position):
        word, rest = found.group(1), found.group(2).split('/*')[0].strip()
        if word in ('if', 'ifdef', 'ifndef'):
            stack.append(ONLY.get(rest))
        elif word == 'else' and stack:
            stack[-1] = {'lm': 'cm', 'cm': 'lm', None: None}[stack[-1]]
        elif word == 'endif' and stack:
            stack.pop()
    return [key for key, _ in MISSIONS if all(only in (None, key) for only in stack)]


def claims(comment):
    """The page numbers a comment gives (each page of a range)."""
    out = set()
    for found in CLAIM.finditer(' '.join(comment.replace('\n * ', '\n').split())):
        for part in re.split(r', | and ', found.group(1)):
            first, _, last = re.sub(' to ', '-', part).partition('-')
            first, last = int(first), int(last or first)
            out.update(range(first, last + 1) if 0 <= last - first < 80 else (first, last))
    return out


def led_by(comment, known=lambda label: True):
    """The place a header's first words name: (label, offset, decimal, the
    match). The offset is octal unless it has a D. decimal is the offset
    read as decimal where it has no D and that reading differs; with an 8 or
    a 9 it is the only reading, and offset is None. None if there is no first
    word.
    A sentence's full stop after a label is not the label's ("CHEKBITS."),
    unless the listing has it so ("J6.")."""
    whole = WHOLE.match(comment)  # a label may hold what ends a word elsewhere: ZON1,2,3  F(MASS)
    if whole and whole.group(1) != whole.group(2) and known(whole.group(1)):
        lead = re.compile(r'/\* (' + re.escape(whole.group(1)) + ')' + OFFSET).match(comment)
    elif whole and known(whole.group(2)):
        lead = re.compile(r'/\* (' + re.escape(whole.group(2)) + ')' + OFFSET).match(comment)
    else:
        lead = LEAD.match(comment)
        if lead and not lead.group(3) and lead.group(1).endswith('.') and not known(lead.group(1)):
            lead = STOPPED.match(comment)
    if not lead or lead.group(1) in OPS:
        return None
    offset = decimal = 0
    if lead.group(3):
        digits, sign = lead.group(3), -1 if lead.group(2) == '-' else 1
        if lead.group(4):
            offset, decimal = sign * int(digits), None
        elif re.search('[89]', digits):
            offset, decimal = None, sign * int(digits)
        else:
            offset, decimal = sign * int(digits, 8), sign * int(digits)
            decimal = None if decimal == offset else decimal
    else:
        decimal = None
    return lead.group(1), offset, decimal, lead


def keyed_places(sources, pages, names):
    """{(directory or file, function): {mission: set of (label, offset)}} from
    the tables of places in sources, a {path: text}. A table serves the
    directories whose headers its file reads (a binding), or its own; a
    static function is served by its own file's tables only."""
    out = collections.defaultdict(lambda: collections.defaultdict(set))
    for file, text in sources.items():
        directory = file.split('/')[0]
        serves = set(INCLUDE.findall(text)) - set(NO_ROUTINES) if directory in NO_ROUTINES else {directory}
        entries = [found.groups() + (found.start(),) for found in PAIR.finditer(text)]
        if directory in NO_ROUTINES:
            entries += [found.groups() + (found.start(),) for found in CALLED.finditer(text)]
            entries += [found.groups()[1:] + (found.group(1), found.start()) for found in TURNED.finditer(text)]
        for name, sign, digits, function, at in entries:
            offset = int(digits, 8 if digits.startswith('0') else 10) if digits else 0
            offset = -offset if sign == '-' else offset
            name = name[2:] if name[:2] in ('K_', 'I_') else names.get(name, name)
            for key in missions_at(text, at):
                spelt = pages[key].spelt.get(name, [])
                if len(spelt) == 1:
                    for where in serves | {file}:
                        out[where, function][key].add((spelt[0], offset))
    return out


class Header:
    """One function's header, its place, and what it misses."""

    def __init__(self, file, text, found, keyed, pages, listings):
        self.file, self.comment, self.name = file, found.group(1), found.group(3)
        self.start = found.start()
        self.line = text.count('\n', 0, found.start()) + 1
        directory = file.split('/')[0]
        self.built = [key for key in missions_at(text, found.start()) if listings[directory][key]]
        self.led = led_by(self.comment, lambda label: any(label in pages[key].labels for key, _ in MISSIONS))
        self.places = {}   # mission -> the places, in order
        self.misses = []
        self.elsewhere = False
        self.octal = {}    # (start, end) of an offset in decimal -> its octal text
        mine = keyed.get((file if found.group(2).startswith('static ') else directory, self.name), {})
        led = {}
        for key in self.built:
            keys = sorted(mine.get(key, ()), key=lambda p: pages[key].word(p))
            at = {pages[key].word(p) for p in keys}
            if self.led and self.led[0] in pages[key].labels:
                label, offset, decimal, lead = self.led
                each = EACH.match(self.comment, lead.end()) if lead.group(3) else None
                if each and key == 'cm':
                    offset, decimal = (-1 if each.group(1) == '-' else 1) * int(each.group(2), 10 if each.group(3) else 8), None
                if not pages[key].within((label, 0), listings[directory][key]):
                    self.elsewhere = True
                elif offset is None or (keys and pages[key].labels[label] + offset not in at
                                        and decimal is not None and pages[key].labels[label] + decimal in at):
                    self.misses.append(('decimal', key, f'{label} {decimal:+d} is octal {octal_text(decimal)}'))
                    self.octal[lead.start(2), lead.end(3)] = octal_text(decimal)
                    led[key] = (label, decimal)
                else:
                    led[key] = (label, offset)
                if key in led and lead.group(3) and not each:
                    # The offsets after the first: "DSPRRLOS +011, +012 and +013".
                    for more in LISTED.finditer(self.comment, lead.end(), MORE.match(self.comment, lead.end()).end()):
                        value = (-1 if more.group(1) == '-' else 1) * int(more.group(2))
                        eight = None if re.search('[89]', more.group(2)) else int(more.group(1) + more.group(2), 8)
                        # (decimal after a decimal first offset: "+12 and +14")
                        if not more.group(3) and (eight is None or (lead.start(2), lead.end(3)) in self.octal or (
                                eight != value and keys and pages[key].labels[label] + eight not in at
                                and pages[key].labels[label] + value in at)):
                            self.misses.append(('decimal', key, f'{label} {value:+d} is octal {octal_text(value)}'))
                            self.octal[more.start(1), more.end(2)] = octal_text(value)
            if key in led and keys and self.comment.startswith(' +n', self.led[3].end()):
                pass  # "LST2FAN +n": any word after the label, as the keys have them
            elif key in led and keys:
                if pages[key].word(led[key]) in at:
                    keys = [led[key]] + [p for p in keys if pages[key].word(p) != pages[key].word(led[key])]
                else:
                    self.misses.append(('place', key, 'the keys have ' + ', '.join(place_text(p) for p in keys)))
            if keys or key in led:
                self.places[key] = keys or [led[key]]
        self.want = {key: sorted({pages[key].page_of(p) for p in places} - {None})
                     for key, places in self.places.items()}
        has = claims(self.comment)
        for key, want in self.want.items():
            if not set(want) <= has:
                self.misses.append(('no-page', key, 'p. ' + ', '.join(map(str, want))))


def octal_text(offset):
    return '%s%s%o' % ('-' if offset < 0 else '+', '0' if abs(offset) > 7 else '', abs(offset))


def place_text(place):
    return place[0] + (' ' + octal_text(place[1]) if place[1] else '')


def headers(sources, binding, pages, names, listings):
    keyed = keyed_places({**sources, **binding}, pages, names)
    for file, text in sorted(sources.items()):
        for found in FUNCTION.finditer(text):
            yield Header(file, text, found, keyed, pages, listings)


def read_sources():
    sources, binding = {}, {}
    for path in sorted((READABLE / 'src').glob('*/*.c')):
        file = str(path.relative_to(READABLE / 'src'))
        (binding if path.parts[-2] in NO_ROUTINES else sources)[file] = path.read_text()
    return sources, binding


# ---- Writing the pages in ----

def pages_text(header, bare):
    """ "LM p. 207, CM p. 181"; in a file of one mission that writes its pages
    bare, "p. 207". """
    parts = []
    for key, name in MISSIONS:
        want = header.want.get(key)
        if not want:
            continue
        if len(want) == 1:
            pages = 'p. %d' % want[0]
        elif want[-1] - want[0] == len(want) - 1:
            pages = 'pp. %d-%d' % (want[0], want[-1])
        else:
            pages = 'pp. ' + ', '.join(map(str, want[:-1])) + ' and %d' % want[-1]
        parts.append(pages if bare else name + ' ' + pages)
    return ', '.join(parts)


def places_text(header):
    by_mission = [[place_text(p) for p in header.places[key]] for key, _ in MISSIONS if key in header.places]
    if all(places == by_mission[0] for places in by_mission):
        return ' and '.join(by_mission[0])
    return ', '.join(name + ' ' + ' and '.join(place_text(p) for p in header.places[key])
                     for key, name in MISSIONS if key in header.places)


def refill(comment):
    """A comment whose first line is too long, its first paragraph filled
    again when the paragraph is running text, else only the first line
    broken."""
    lines = comment.split('\n')
    if len(lines[0]) <= WIDTH:
        return comment
    end = 0
    while end + 1 < len(lines) and re.match(r' \* \S', lines[end + 1]):
        end += 1
    closed = lines[end].endswith('*/')
    body = [lines[0][3:]] + [line[3:] for line in lines[1:end + 1]]
    running = all(len(line) >= 56 for line in lines[1:end]) and not any('  ' in line.strip() for line in body)
    if not running:
        cut = lines[0].rfind(' ', 0, WIDTH + 1)
        return '\n'.join([lines[0][:cut], ' * ' + lines[0][cut + 1:]] + lines[1:])
    if closed:
        body[-1] = body[-1][:-2].rstrip()
    filled = textwrap.wrap(' '.join(body), WIDTH - 3, break_long_words=False, break_on_hyphens=False)
    if closed:
        if len(filled[-1]) + 3 <= WIDTH - 3:
            filled[-1] += ' */'
        else:
            last = filled[-1].rsplit(' ', 1)
            filled[-1:] = [last[0], last[1] + ' */']
    return '\n'.join(['/* ' + filled[0]] + [' * ' + line for line in filled[1:]] + lines[end + 1:])


def rewritten(header, bare):
    """The header's comment with its pages put in, or None where a person has
    to (the header gives another page already, or names another place)."""
    kinds = {kind for kind, _, _ in header.misses}
    comment = header.comment
    lead_end = header.led[3].end(4 if header.led[3].group(3) else 1) if header.led else 0
    lead_end += 3 if comment.startswith(' +n', lead_end) else 0  # "LST2FAN +n"
    if 'place' in kinds:
        # The label alone, where the keys have one place of that label: its
        # offset goes in ("ALMXIT, after its EXIT" at ALMXIT +4).
        lead, places = header.led[3], {tuple(places) for places in header.places.values()}
        if lead.group(3) or len(places) != 1 or len(next(iter(places))) != 1:
            return None
        place = next(iter(places))[0]
        if place[0] != header.led[0]:
            return None
        comment = comment[:lead.end(1)] + place_text(place)[len(place[0]):] + comment[lead.end(1):]
        lead_end += len(place_text(place)) - len(place[0])
    for (start, end), octal in sorted(header.octal.items(), reverse=True):
        comment = comment[:start] + octal + comment[end:]
        lead_end += len(octal) - (end - start) if end <= lead_end else 0
    if 'no-page' not in kinds:
        return comment
    if claims(comment):
        return None
    pages = pages_text(header, bare)
    if header.led and any(header.led[0] in (p[0] for p in places) for places in header.places.values()):
        end = MORE.match(comment, lead_end).end()
        rest = comment[end:]
        if rest.startswith(' -- '):
            rest = ': ' + rest[4:]
        elif rest.startswith(' --\n'):
            rest = ':' + rest[3:]
        elif rest.startswith(' */'):
            rest = '.' + rest
        comment = comment[:end] + ' -- ' + pages + rest
    else:
        rest = comment[3:]
        comment = '/* ' + places_text(header) + ' -- ' + pages + (': ' + rest if rest.strip(' */') else '. */')
    return refill(comment)


def write(sources, found):
    by_file = collections.defaultdict(list)
    for header in found:
        if header.misses:
            by_file[header.file].append(header)
    done = left = 0
    for file, mine in sorted(by_file.items()):
        text = sources[file]
        one = len({key for header in headers_of(file, found) for key in header.built}) == 1
        bare = one and not re.search(r'\b(?:LM|CM) pp?\. ?\d', text)
        for header in sorted(mine, key=lambda h: -h.start):
            new = rewritten(header, bare)
            if new is None:
                left += 1
                continue
            assert text.startswith(header.comment, header.start)
            text = text[:header.start] + new + text[header.start + len(header.comment):]
            done += 1
        (READABLE / 'src' / file).write_text(text)
    print(f'header_pages: {done} headers written, {left} left for a person')


def headers_of(file, found):
    return [header for header in found if header.file == file]


# ---- The check ----

# This repository's Comanche055/STABLE_ORBIT.agc has no mark for page 531:
# its last two routines read as page 530 there.
UNMARKED = {'cm': {'PREC/TT': (531, 530), 'INTRPVP': (531, 530)}}


def transcriptions(pages):
    """The pages are read from the assembled transcription. Each label's page
    there against its page by this repository's `# Page` marks: (labels
    compared, [those that differ, but for UNMARKED])."""
    compared, differ = 0, []
    for key, tree in (('lm', 'Luminary099'), ('cm', 'Comanche055')):
        mine = {}
        for path in sorted((READABLE.parent / tree).glob('*.agc')):
            page = None
            for line in open(path, errors='replace'):
                mark = MARK.match(line)
                if mark:
                    page = int(mark.group(1))
                elif re.match(r'[^\s#+-]\S*\s', line):
                    mine.setdefault(line.split()[0], page)
        for label, word in pages[key].labels.items():
            theirs = pages[key].page.get(word)
            if theirs is None or label not in mine or label[0] in '+-':
                continue
            compared += 1
            if mine[label] != theirs and UNMARKED.get(key, {}).get(label) != (theirs, mine[label]):
                differ.append(f'{tree} {label}: p. {theirs} assembled, p. {mine[label]} here')
    return compared, differ

def self_check():
    """Made-up headers, each judged as it should be."""
    sources, binding = read_sources()
    pages = {key: Pages(key) for key, _ in MISSIONS}
    names = dict(DEFINE.findall((READABLE / 'src/agc/names.h').read_text()))
    novac = pages['lm'].page_of(('NOVAC', 0))
    novac_cm = pages['cm'].page_of(('NOVAC', 0))
    listings = listing_files()
    table = {'binding/x.c': '#include "../executive/executive.h"\nstatic const entry t[] = {\n    {NOVAC + 2u, novac_plus2},\n'
                            '#if LUMINARY099\n    {NOVAC + 2u, novac_each},\n#else\n    {NOVAC + 012u, novac_each},\n#endif\n'
                            '    {KEY(01u, NOVAC + 012u), novac_later},\n'
                            '    {NOVAC + 2u, novac_two},\n    {NOVAC + 012u, novac_two},\n};\n'}
    cases = [
        ('a page for each mission', f'/* NOVAC -- LM p. {novac}, CM p. {novac_cm}.\n * A job. */\nvoid novac(void)\n{{', []),
        ('one mission of two', f'/* NOVAC -- LM p. {novac}. */\nvoid novac(void)\n{{', ['no-page']),
        ('no page', '/* NOVAC: a job. */\nvoid novac(void)\n{', ['no-page', 'no-page']),
        ('a range', f'/* NOVAC -- LM pp. {novac - 1}-{novac + 1}, CM pp. {novac_cm}-{novac_cm + 1}. */\n'
                    'void novac(void)\n{', []),
        ('built for one mission', f'#if LUMINARY099\n/* NOVAC -- p. {novac}. */\nvoid novac(void)\n{{', []),
        ('the other mission', f'#if LUMINARY099\n#else\n/* NOVAC -- p. {novac_cm}. */\nvoid novac(void)\n{{', []),
        ('a helper', '/* INDEX A; CA base: the word at an index. */\nstatic word at_index(void)\n{', []),
        ('a helper in words', '/* The word at an index. */\nstatic word at_index(void)\n{', []),
        ('keyed, no first word', '/* and the job ends. */\nvoid novac_plus2(void)\n{', ['no-page', 'no-page']),
        ('keyed, another place named', f'/* NOVAC +3 -- LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_plus2(void)\n{{',
         ['place', 'place']),
        ('keyed, the place named', f'/* NOVAC +2 -- LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_plus2(void)\n{{', []),
        ('a decimal offset', f'/* NOVAC +10 -- LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_later(void)\n{{',
         ['decimal', 'decimal']),
        ('a decimal offset with an 8', f'/* NOVAC +8 -- LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_8(void)\n{{',
         ['decimal', 'decimal']),
        ('a decimal offset in a list', f'/* NOVAC +2 and +10 -- LM p. {novac}, CM p. {novac_cm}. */\n'
                                       'void novac_two(void)\n{', ['decimal', 'decimal']),
        ('an octal list', f'/* NOVAC +2 and +012 -- LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_two(void)\n{{', []),
        ('a decimal offset with its D', f'/* NOVAC +8D -- LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_8(void)\n{{', []),
        ('an offset for each mission', f'/* NOVAC +2 (LM) or +012 (CM) -- LM p. {novac}, CM p. {novac_cm}. */\n'
                                       'void novac_each(void)\n{', []),
        ('an offset for each mission, one wrong', f'/* NOVAC +2 (LM) or +011 (CM) -- LM p. {novac}, CM p. {novac_cm}. */\n'
                                                  'void novac_each(void)\n{', ['place']),
        ('any offset', f'/* NOVAC +n -- LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_plus2(void)\n{{', []),
        ('pages in a list', f'/* NOVAC -- LM pp. 3, {novac} and {novac + 4}, CM p. {novac_cm}. */\nvoid novac(void)\n{{', []),
        ('a label with a comma in it', '/* ZON1,2,3: the zones. */\nvoid zon(void)\n{', ['no-page'], 'tjet_law'),
        ('a full stop after the offset', f'/* NOVAC +2. LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_plus2(void)\n{{', []),
        ('a full stop after the label', '/* NOVAC. A job. */\nvoid novac(void)\n{', ['no-page', 'no-page']),
        ('the octal offset', f'/* NOVAC +012 -- LM p. {novac}, CM p. {novac_cm}. */\nvoid novac_later(void)\n{{', []),
        ('a wrong page', f'/* NOVAC -- LM p. {novac + 7}, CM p. {novac_cm}. */\nvoid novac(void)\n{{', ['no-page']),
    ]
    bad = 0
    for name, text, want, *where in cases:
        got = [kind for header in headers({(where or ['executive'])[0] + '/x.c': text}, table, pages, names, listings)
               for kind, _, _ in header.misses]
        if sorted(got) != sorted(want):
            bad += 1
            print(f'FAIL self-check "{name}": judged {got}, should be {want}')
    filled = refill('/* NOVAC -- LM p. 1103, CM p. 1208: a job that needs no VAC area is started, and more words\n'
                    ' * that run on to the end of the second line of this paragraph, here. */')
    if any(len(line) > WIDTH for line in filled.split('\n')) or not filled.endswith('here. */'):
        bad += 1
        print('FAIL self-check "a long first line is filled again"')
    kept = refill('/* NOVAC -- LM p. 1103, CM p. 1208: a job that needs no VAC area is started, and the words\n'
                  ' * 00  one\n * 01  two */')
    if not kept.endswith(' * 00  one\n * 01  two */') or len(kept.split('\n')[0]) > WIDTH:
        bad += 1
        print('FAIL self-check "a table under a long first line is kept"')
    print(f'{"FAIL" if bad else "ok  "} header pages self-check: {len(cases) + 2} made-up headers, '
          f'{bad} judged wrongly')
    return bad


def main(argv):
    unknown = [a for a in argv if a not in ('--list', '--write', '--self-check')]
    if unknown:
        raise SystemExit('header_pages: unknown option ' + ' '.join(unknown))
    if '--self-check' in argv:
        return 1 if self_check() else 0
    sources, binding = read_sources()
    pages = {key: Pages(key) for key, _ in MISSIONS}
    names = dict(DEFINE.findall((READABLE / 'src/agc/names.h').read_text()))
    found = list(headers(sources, binding, pages, names, listing_files()))
    if '--write' in argv:
        write(sources, found)
        return 0
    count = collections.Counter()
    placed = helpers = 0
    for header in found:
        placed += bool(header.places)
        helpers += not header.places
        for kind in {kind for kind, _, _ in header.misses}:
            count[kind] += 1
        if '--list' in argv:
            for kind, key, what in header.misses:
                print(f'src/{header.file}:{header.line}: {kind} ({key}): {header.name}: {what}')
    compared, differ = transcriptions(pages)
    for line in differ:
        print('header_pages: ' + line)
    misses = sum(count.values()) + len(differ)
    print(f'{"FAIL" if misses else "ok  "} header pages: {placed} functions at a place of the listing '
          f'({helpers} have none the tables or their first words give): {count["no-page"]} without the page '
          f'of the place, {count["place"]} naming another place than the keys, {count["decimal"]} with a '
          f'decimal offset; {compared} labels on one page in both transcriptions, {len(differ)} not')
    return 1 if misses else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
