#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Which words of interpretive code no test and no scenario runs.

The line measure (tests/line_coverage.py) counts lines of C. Interpretive
code is tables of words (each a struct interpretive_code), with no line to
run, so that measure cannot see which of their words the interpreter ran.
This one can. In the line measure's build the interpreter's binding holds
a recorder (tests/word_coverage.inc), and every program that ran left what
its passes did, word by word, under build/linecov/words: so this measure
needs that build and its run (`make line-coverage`), and reads what it
left.

What is counted (the universe) is every word of every table, as the x86
objects of that build hold them (bank, first word, count: read as
tools/code_extents.py reads them), each with its line of the assembled
listing (c64-port's assembly.lst). A word is one of three things:

  an order pair   one or two orders (the listing's line names them). Each
                  order is counted on its own: the left one runs when
                  NEWOPS takes the word, the right one when the left has
                  ended at DANZIG.
  a store code    STORE, STODL, STOVL or STCALL: run when NEWOPS takes it.
                  (The one after STADR, which the assembler complements,
                  is STADR's operand: run when STADR read it.)
  an operand      an address, a switch and its bit or a shift count in
                  line with the code, or a constant that lies among it.
                  One in line is read when a pass read it just after
                  the word it follows was taken (at most four words on
                  from a pair, two from a store code), in a program that
                  ran that order. A constant is read when a
                  program read it that also took an operand naming it: its
                  name (the constant and the unnamed words after it), or,
                  with an index (X,1), any constant of its table. A read
                  is not enough: a made-up state's address can come to any
                  word of the rope, and the case tests make such states by
                  the thousand. For the constants the count is an upper
                  one: the read and the operand are one program's, and
                  need not be one pass's.

A table can also hold a basic instruction (the TC INTPRET a routine's
code begins at): no word of interpretive code, and not counted.

An order that branches (BPL, BMN, BZE, BOV, BOVB, BHIZ, and the switch
orders BON, BOF, BONCLR, BOFSET, BOFCLR, BOFINV) has two outcomes, and
each is counted: the order went on to the next (it ended at DANZIG), and
it went away. An order that ran is not an order whose both outcomes ran.

What a program left counts only where the value it read is the rope's own
word there: a test that writes made-up programs into the rope (the
interpreter's fuzz) leaves words that are no program's, and the programs
named in MADE_UP are not read at all. Most of what runs, runs in the case
tests, from made-up states at each order; the ledger's head also says how
much the scenarios run by themselves, which is less.

The universe is tied to the run: an order pair or a store code that ran,
with the rope's own value, at a word no table holds, fails the check (a
program the tables miss).

Each word that did not run, or not wholly (an order of a pair unrun, a
branch that only ever went one way, an operand never read), stands in
tests/word_coverage.txt with the listing's line and the reason. Words that
lie together in a table make one stretch, known by its file, its table,
its missions and its text, and the check fails on a stretch that is not
in the ledger, on one that is there and is not found now, and on a reason
that does not stand (none, or one of no class the ledger's head defines;
an [impossible] or a [never] that does not say why; an [unnamed] constant
that a word of the listing names; a [basic] one that no basic instruction
does).

  word_coverage.py               the check, on what the last run left
  word_coverage.py --write       rewrite the ledger from it, keeping each
                                 reason
  word_coverage.py --list NAME   a table's words and what was seen of each
  word_coverage.py --self-check  the check itself, on made-up tables and
                                 records (no build)
"""
import hashlib
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
SRC = READABLE / 'src'
BUILD = READABLE / 'build/linecov'
WORDS = BUILD / 'words'
sys.path.insert(0, str(READABLE / 'tools'))
import code_extents as CE  # noqa: E402

(READ, LEFT, LEFT_ON, LEFT_AWAY, RIGHT, RIGHT_ON, RIGHT_AWAY, STORE, MIXED, ORPHAN, ORPHAN_ON, ORPHAN_AWAY,
 IN_LINE) = (1 << n for n in range(13))
# How many different places of right orders with no pair the recorder keeps
# for a program (WORDS_LOST of tests/word_coverage.inc).
LOST_KEPT = 64
# Programs whose interpretive words are made up: not read.
MADE_UP = ('interpreter-cases-lm', 'interpreter-cases-cm')
STORES = ('STORE', 'STODL', 'STOVL', 'STCALL')
BRANCHES = {'BPL', 'BMN', 'BZE', 'BOV', 'BOVB', 'BHIZ', 'BON', 'BOF', 'BOFF', 'BONCLR', 'BOFSET', 'BOFCLR',
            'BOFINV'}
# Orders the listings name by another name than the notation's table.
ALIASES = {'SINE': 'SIN', 'COSINE': 'COS', 'CLR': 'CLEAR'}
# What stands in the operation column of a word that is no order: a
# constant or an address the assembler makes (an operand has none).
DATA = {'', 'DEC', '2DEC', '2DEC*', 'OCT', '2OCT', 'CADR', 'FCADR', 'ECADR', 'ADRES', 'VN', 'TC', 'BBCON',
        'GENADR', '2CADR', '2FCADR', 'REMADR', 'SBANK='}
LEDGER = READABLE / 'tests/word_coverage.txt'
NO_REASON = '(no reason)'
REASON = re.compile(r'\[([a-z-]+)\]\s*(.*)', re.S)
ENTRY = re.compile(r'^  (\w+(?:\[\d+\])?), (lm cm|lm|cm) (.*) ([0-9a-f]{8})$')
CLASSES = {
    'impossible': "No state brings the interpreter to the word, or the branch the other way: the program's own "
                  'words rule it out, and the reason says how. An argument, not a run.',
    'never': 'No program brings the state, though the words of the table do not rule it out: the reason rests '
             'on what the rest of the program stores or calls, and says what was looked at.',
    'unnamed': "A constant that no word of the mission's listing names. The check looks: the constant has a "
               'name, and no instruction, order or address of the listing has that name as its operand. An '
               'address reckoned from another name could still come to it, and the reason says what stands '
               'beside it.',
    'basic': "Not the interpreter's to read: basic instructions read the word. The check finds one in the "
             "mission's listing that names a word of the stretch, or a TC as the word before it (whose routine "
             'reads the words after its call). The line measure counts the C of those instructions.',
}
ORDER = re.compile(r'#define ORDER_(\w+)\s+(0[0-7]*)u(?:\s*/\* (\S+) \*/)?')


def order_codes():
    """{mnemonic: code} of the interpretive orders, from the notation's
    table (src/interpreter/notation.h)."""
    codes = {}
    for name, code, said in ORDER.findall((SRC / 'interpreter/notation.h').read_text()):
        if not said or said.startswith('('):
            said = re.sub(r'_([12])$', r',\1', re.sub(r'_STAR$', '*', name)).replace('V_SC', 'V/SC')
        codes[said] = int(code, 8)
    for other, name in ALIASES.items():
        codes[other] = codes[name]
    return codes


def pair_codes(value):
    """An order pair's two codes, as NEWOPS finds them: the left one, and
    the right one or None."""
    pair = (~value & 0o77777) - 1
    right = (pair >> 7) & 0o177
    return pair & 0o177, right - 1 if right else None


class Word:
    __slots__ = ('phys', 'value', 'table', 'file', 'line', 'index', 'kind', 'left', 'right')


def universe(mission, objects=None):
    """Every word of the mission's tables, by physical address, and the
    tables: (name, file, bank, first, count)."""
    names = {}
    for source in SRC.rglob('*.c'):
        for name in CE.TABLE.findall(source.read_text()):
            names[name] = None
    listing = CE.Listing(mission)
    codes = order_codes()
    tables, words, wrong = [], {}, []
    for obj in sorted(objects if objects is not None else (BUILD / f'x86-{mission}').rglob('*.o')):
        file = str(obj.relative_to(BUILD / f'x86-{mission}').with_suffix('.c'))
        for name, bank, first, count in CE.elf_tables(obj, names):
            tables.append((name, file, bank, first, count))
            begin = CE.physical(bank, first)
            for phys in range(begin, begin + count):
                if phys in words:
                    wrong.append(f'{mission} {name} and {words[phys].table} both hold the word at {phys:o}')
                    continue
                at = listing.line_at.get(phys)
                word = words[phys] = Word()
                word.phys, word.table, word.file = phys, name, file
                word.line, word.index = at if at else (None, 0)
                word.value = listing.listing.rope.get(phys)
                word.left = word.right = None
                word.kind = 'operand'
                if not at or word.index:
                    continue
                op, operand = word.line.op, word.line.operand.split()[0] if word.line.operand else ''
                if op in CE.BASIC and op not in codes:
                    word.kind = 'basic'
                elif op.rstrip('*') in STORES:
                    word.kind = 'store'
                elif word.value & 0o40000 and op in codes:
                    left, right = pair_codes(word.value)
                    if codes[op] == left and (right is None or codes.get(operand) == right):
                        word.kind, word.left, word.right = 'pair', op, operand if right is not None else None
                if word.kind == 'operand' and op not in DATA:
                    wrong.append(f'{mission} {name}: {word.line!r} is no order the notation names and no constant')
    # The tie to the listing: an order or a store code that no table holds.
    for line in listing.listing.lines:
        if line.phys in words or not line.words or line.op in CE.BASIC:
            continue
        operand = line.operand.split()[0] if line.operand else ''
        left, right = pair_codes(line.words[0])
        if line.op.rstrip('*') in STORES or (line.words[0] & 0o40000 and codes.get(line.op) == left
                                             and (right is None or codes.get(operand) == right)):
            wrong.append(f'{mission} {line.src}: {line!r} is interpretive code and no table holds it')
    return tables, words, wrong


def read_records(mission, only=lambda program: True):
    """What the programs left: {phys: {value: flags}} over the programs
    that are not MADE_UP (and that only() takes), the programs, the
    orphans, the mixed words by program, and the same by program
    ({program: {phys: {value: flags}}})."""
    seen, files, orphans, mixed = defaultdict(lambda: defaultdict(int)), set(), 0, defaultdict(int)
    per = defaultdict(lambda: defaultdict(lambda: defaultdict(int)))
    for path in sorted(WORDS.glob(f'*.{mission}.*.words')):
        program = path.name.rsplit('.', 3)[0]
        if program in MADE_UP or not only(program):
            continue
        files.add(program)
        for record in path.read_text().split('\n'):
            parts = record.split()
            if not parts or parts[0] == 'mission':
                continue
            if parts[0] == 'orphans':
                orphans += int(parts[1])
                continue
            if parts[0] == 'lost':
                continue
            phys, value, flags = int(parts[0], 8), int(parts[1], 8), int(parts[2], 16)
            if flags & MIXED:
                mixed[program] += 1
                continue
            seen[phys][value] |= flags
            per[program][phys][value] |= flags
    return seen, files, orphans, mixed, per


def adopted(word, got):
    """What was seen of a word, with a right order the recorder found the
    pair of behind LOC (an orphan) taken as the pair's right order, where
    the listing has a pair with one there; nothing of it anywhere else."""
    orphan = got & (ORPHAN | ORPHAN_ON | ORPHAN_AWAY)
    got &= ~(ORPHAN | ORPHAN_ON | ORPHAN_AWAY)
    return got | (orphan >> 5 if word.kind == 'pair' and word.right is not None else 0)


OFFSET = re.compile(r'^[+-]\d+D?$')


def layout(words):
    """How the operand words lie: ({an operand in line with the code: the
    pair or store code it follows, or None}, {a constant: where its block
    starts}), by physical address. A block is a named constant and the
    unnamed constants after it; constants with no name that follow no order
    are a block too. (A named constant between an order and the next order
    is the first one's operand, in line: SSP's number, written with a
    name.)"""
    inline, block = {}, {}
    owner = start = last = None
    for phys in sorted(words):
        word = words[phys]
        if phys - 1 != last:
            owner = start = None
        last = phys
        if word.kind != 'operand':
            owner, start = (phys if word.kind in ('pair', 'store') else None), None
            continue
        first = phys - word.index
        if word.index and first in block:
            block[phys] = block[first]
            continue
        if word.index and first in inline:
            inline[phys] = inline[first]
            continue
        constant = bool(word.line) and word.line.op != '' and word.line.op in DATA
        after = phys + 1
        while after in words and words[after].index:
            after += 1
        if constant and word.line.label and not (owner is not None and start is None and after in words and
                                                 words[after].kind in ('pair', 'store')):
            start = phys
        elif constant and start is None and owner is None:
            start = phys
        if constant and start is not None:
            block[phys] = start
        else:
            inline[phys], start = owner, None
    return inline, block


def namers(words, inline, block, labels):
    """{a constant: the operands in line that name it}. An operand names
    the block its address lies in (NAME, or NAME +n), and with an index
    (NAME,1) every constant of that block's table."""
    in_block, in_table, out = defaultdict(list), defaultdict(list), defaultdict(set)
    for phys, start in block.items():
        in_block[start].append(phys)
        in_table[words[phys].table].append(phys)
    for phys in inline:
        tokens = words[phys].line.operand.split() if words[phys].line and words[phys].line.operand else []
        if not tokens:
            continue
        name = re.sub(r',[12]$', '', tokens[0])
        at = labels.get(name)
        if at is None:
            continue
        if len(tokens) > 1 and OFFSET.match(tokens[1]):
            digits = tokens[1][1:].rstrip('D')
            decimal = tokens[1].endswith('D') or set(digits) & set('89')
            at += (-1 if tokens[1][0] == '-' else 1) * int(digits, 10 if decimal else 8)
        if at in block:
            for constant in in_table[words[at].table] if name != tokens[0] else in_block[block[at]]:
                out[constant].add(phys)
    return out


def read_as_named(words, per, labels):
    """The operands that count as read, from what each program saw
    ({program: {phys: flags}}): one in line, read in a pass just after the
    word NEWOPS took (IN_LINE: at most four words after a pair, two after
    a store code), by a program that ran the order it follows; a constant,
    read by a program that took an operand naming it."""
    inline, block = layout(words)
    named = namers(words, inline, block, labels)

    def ran(phys, told):
        got = told.get(phys, 0)
        return got & (LEFT | RIGHT | STORE | ORPHAN) or (words[phys].kind == 'store' and got & READ)

    out = set()
    for told in per.values():
        took = {phys for phys, owner in inline.items()
                if told.get(phys, 0) & IN_LINE and (owner is None or ran(owner, told))}
        out |= took
        out |= {phys for phys in block if told.get(phys, 0) & READ and named.get(phys, set()) & took}
    return out


def lost_places(mission):
    """Where the right orders were that the recorder found no pair for:
    ({LOC's physical address: how often}, the programs that had more
    different places than the recorder keeps). The recorder keeps each
    program's first LOST_KEPT different places (LOC and EDOP)."""
    places, full = Counter(), []
    for path in sorted(WORDS.glob(f'*.{mission}.*.words')):
        if path.name.rsplit('.', 3)[0] in MADE_UP:
            continue
        kept = 0
        for record in path.read_text().split('\n'):
            parts = record.split()
            if parts and parts[0] == 'lost':
                places[int(parts[1], 8)] += int(parts[3])
                kept += 1
        if kept >= LOST_KEPT:
            full.append(path.name)
    return places, full


def lost(mission, places, words, line_at):
    """What to say of the right orders with no pair, and what is wrong.
    Three kinds are no pair's order:

      - basic code that has the interpreter run one routine and come back
        (TC USPRCADR and the CADR after it): the EXIT that waits in EDOP
        is no word of any program;
      - LOC at an indexed store code (STORE X,1): a pair is done before a
        store code is taken, so nothing of the program waits there. A
        made-up state's index can make the store write EDOP itself;
      - LOC outside every table (basic code that sets LOC and EDOP by
        hand).

    Any other is an order of a pair that went uncounted, which is wrong."""
    def where(phys):
        return f'{phys // 1024:02o},{0o2000 + phys % 1024:04o}'

    def text(phys):
        return re.sub(' +', ' ', line_at[phys].text).strip() if phys in line_at else 'no line'

    usprcadr = [phys for phys in places if phys in line_at and line_at[phys].op == 'CADR' and phys - 1 in line_at and
                line_at[phys - 1].op == 'TC' and line_at[phys - 1].operand.split()[:1] == ['USPRCADR']]
    stores = [phys for phys in places if phys in words and words[phys].kind == 'store' and
              re.search(r',[12]$', words[phys].line.operand.strip())]
    others = sorted(set(places) - set(usprcadr) - set(stores))
    wrong = [f'{mission} a right order ran at {where(phys)} ({text(phys)}) with no pair found behind LOC, at the '
             'words of a table' for phys in others if any(at in words for at in range(phys - 2, phys + 1))][:8]
    said = [f'  They were noted at {len(places)} places: {len(usprcadr)} are the CADR after a TC USPRCADR, where',
            '  basic code has the interpreter run one routine and EXIT.']
    if stores:
        said += [f"  {len(stores)} are at an indexed store code, where no pair's order waits: a made-up",
                 "  state's index can make the store write EDOP itself."]
    if others:
        said.append('  The others: ' + ', '.join(f'{where(phys)} ({text(phys)})' for phys in others[:6]) +
                    (f' and {len(others) - 6} more' if len(others) > 6 else '') + '.')
    return said if places else [], wrong


def flown(program):
    """Whether a program is the x86 flight program, which the scenarios
    run."""
    return program.startswith('x86-flight')


def measure(mission):
    """The mission's tables and words, what was seen of each word
    ({phys: flags}), what is wrong, (programs, orphans, mixed), and what
    the scenarios alone saw."""
    tables, words, wrong = universe(mission)
    seen, files, orphans, mixed, per = read_records(mission)
    flags = {phys: seen[phys].get(word.value, 0) for phys, word in words.items() if phys in seen}
    in_flight = read_records(mission, flown)[0]
    flight = {phys: in_flight[phys].get(word.value, 0) for phys, word in words.items() if phys in in_flight}
    listing = CE.Listing(mission)
    # What each program saw, at the rope's own values: an operand is read
    # by what one program did, not by what all of them did between them.
    own = {program: {phys: got[words[phys].value] for phys, got in told.items()
                     if phys in words and words[phys].value in got} for program, told in per.items()}
    for told, programs in ((flags, own), (flight, {p: t for p, t in own.items() if flown(p)})):
        counted = read_as_named(words, programs, listing.listing.labels)
        for phys, word in words.items():
            if word.kind == 'operand' and (phys in told or phys in counted):
                told[phys] = (told.get(phys, 0) & ~READ) | (READ if phys in counted else 0)
        for phys, got in told.items():
            told[phys] = adopted(words[phys], got)
    for phys in sorted(seen):
        if phys in words:
            continue
        ran = seen[phys].get(listing.listing.rope.get(phys), 0) & (LEFT | RIGHT | STORE)
        if ran:
            line = listing.line_at.get(phys)
            wrong.append(f'{mission} {phys // 1024:02o},{0o2000 + phys % 1024:04o} ran as code '
                         f'({ran:03x}) and no table holds it: {line[0] if line else "no line"}')
    for phys, got in sorted(flags.items()):
        word = words[phys]
        ran_as = 'pair' if got & (LEFT | RIGHT) else 'store' if got & STORE else None
        if ran_as and ran_as != word.kind and word.kind != 'basic':
            wrong.append(f'{mission} {word.table} {word.line!r}: ran as a {ran_as}, read from the listing as '
                         f'{word.kind}')
    places, full = lost_places(mission)
    said, bad = lost(mission, places, words, {phys: at[0] for phys, at in listing.line_at.items()})
    bad += [f'{mission}: {name} had {LOST_KEPT} or more places of right orders with no pair: the recorder '
            f'keeps no more, so some were not looked at' for name in full]
    return tables, words, flags, wrong + bad, (files, orphans, mixed, said), flight


def halves(word):
    """A pair's orders: (name, ran, went on, went away) for each."""
    return [(order, *bits) for order, bits in ((word.left, (LEFT, LEFT_ON, LEFT_AWAY)),
                                               (word.right, (RIGHT, RIGHT_ON, RIGHT_AWAY))) if order is not None]


def misses(word, got):
    """What was not seen of a word: nothing if it ran wholly."""
    if word.kind == 'basic':
        return []
    if word.kind == 'store':
        # (A complemented one is STADR's operand, which STADR read, in line.)
        return [] if got & (IN_LINE if word.value & 0o40000 else STORE) else ['unrun']
    if word.kind == 'operand':
        return [] if got & READ else ['unread']
    out = []
    for order, ran, on, away in halves(word):
        if not got & ran:
            out.append(f'{order} unrun')
        elif order in BRANCHES:
            out += [f'{order} never taken'] * (not got & away) + [f'{order} always taken'] * (not got & on)
    return ['unrun'] if out and not got & (LEFT | RIGHT) else out


def counts(words, flags):
    out = Counter()
    for phys, word in words.items():
        got = flags.get(phys, 0)
        if word.kind == 'pair':
            for order, ran, on, away in halves(word):
                out['orders'] += 1
                out['orders run'] += bool(got & ran)
                if order in BRANCHES:
                    out['branches'] += 1
                    out['branches run'] += bool(got & ran)
                    out['branches both'] += bool(got & on) and bool(got & away)
        elif word.kind == 'store':
            out['stores'] += 1
            out['stores run'] += not misses(word, got)
        elif word.kind == 'basic':
            out['basic'] += 1
            continue
        else:
            out['operands'] += 1
            out['operands read'] += bool(got & READ)
        out['words whole'] += not misses(word, got)
    return out


def text_of(word):
    line = re.sub(r'\s+', ' ', word.line.text).strip() if word.line else '(no line of the listing)'
    return line + (f' (word {word.index + 1})' if word.index else '')


def stretches(words, flags):
    """The words not wholly run, those that lie together in a table as
    one: {(file, table, digest): [(first word, [text], [(word, what was
    not seen of it)])]}, in the order of the rope."""
    out, run, last = defaultdict(list), None, None
    for phys in sorted(words):
        word = words[phys]
        missed = misses(word, flags.get(phys, 0))
        if not missed:
            run = None
            continue
        if run is None or last != phys - 1 or run[0].table != word.table:
            run = (word, [], [])
            out[(word.file, word.table)].append(run)
        run[1].append(f'{", ".join(missed):<24}{text_of(word)}')
        run[2].append((word, missed))
        last = phys
    keyed = defaultdict(list)
    for (file, table), runs in out.items():
        for first, text, members in runs:
            digest = hashlib.sha1('\n'.join(text).encode()).hexdigest()[:8]
            keyed[(file, table, digest)].append((first, text, members))
    return keyed


def found_of(measures, held=None):
    """Both missions' stretches as the ledger keys them: {(file, table,
    digest, which of them, missions): (where, text)}. A stretch that both
    missions have, word for word, is one. (held, if given, takes each
    stretch's words: {key: {mission: [(word, what was not seen of it)]}}.)"""
    by_key = defaultdict(dict)
    for mission, (words, flags) in measures.items():
        for key, runs in stretches(words, flags).items():
            for ordinal, (first, text, members) in enumerate(runs):
                by_key[(*key, ordinal)][mission] = (first, text, members)
    found = {}
    for key, per in by_key.items():
        missions = ' '.join(sorted(per, reverse=True))
        where = ' '.join(f'{first.phys // 1024:02o},{0o2000 + first.phys % 1024:04o}'
                         if first.phys >= 4096 else f'{first.phys - 2048 + 0o4000:04o}'
                         for _, (first, _, _) in sorted(per.items(), reverse=True))
        found[(*key, missions)] = (where, next(iter(per.values()))[1])
        if held is not None:
            held[(*key, missions)] = {mission: members for mission, (_, _, members) in per.items()}
    return found


def naming(listing):
    """What a mission's listing names, and where: ({name: [line that has
    it as its operand]}, {physical address: the line there})."""
    names = defaultdict(list)
    for line in listing.listing.lines:
        operand = line.operand.split()[0] if line.operand else ''
        if operand:
            names[re.sub(r',[12]$', '', operand)].append(line)
    return names, {phys: at[0] for phys, at in listing.line_at.items()}


def class_of(reason):
    told = REASON.match(reason or '')
    return (told.group(1), (told.group(2) or '').strip()) if told else (None, reason or '')


def fits(name, why, members=None, named=None):
    """What is wrong with a reason of class name, saying why, for a
    stretch of these words ({mission: [(word, what was not seen of it)]})
    in listings that name these things ({mission: naming()}); nothing if
    it fits."""
    if name not in CLASSES:
        return f'has a class this check has no rule for ([{name}])'
    if not why:
        return f'is [{name}] and does not say why'
    if name in ('impossible', 'never'):
        return ''
    for mission, stretch in sorted((members or {}).items()):
        names, line_at = named[mission]
        if any(word.kind != 'operand' for word, _ in stretch):
            return f'is [{name}] and holds an order or a store code'
        labels = [word.line.label if word.line else '' for word, _ in stretch]
        if name == 'unnamed':
            if not all(labels):
                return 'is [unnamed] and holds a word with no name'
            for label in labels:
                if names.get(label):
                    return f'is [unnamed], and {names[label][0].src} names {label} ({mission.upper()})'
        else:
            before = line_at.get(stretch[0][0].phys - 1)
            if not (before is not None and before.op == 'TC') and not any(
                    line.op in CE.BASIC for label in labels if label for line in names.get(label, [])):
                return f'is [basic], and no basic instruction of the {mission.upper()} listing names a word of it'
    return ''


def read_ledger(path=LEDGER):
    """The ledger's classes and its reasons: {key: reason}."""
    classes, reasons, file, key = {}, {}, None, None
    for line in (path.read_text().split('\n') if path.is_file() else []):
        named = re.match(r'^\[([a-z-]+)\] (.*)$', line)
        entry = ENTRY.match(line)
        if line.startswith('#'):
            continue
        if line.startswith('src/'):
            file, key = line[4:].strip(), None
        elif entry and file:
            name = (file, entry.group(1), entry.group(4))
            ordinal = sum(1 for k in reasons if k[:3] == name and k[4] == entry.group(2))
            key = (*name, ordinal, entry.group(2))
            reasons[key] = ''
        elif line.startswith('    why: ') and key:
            reasons[key] = line[9:].strip()
        elif line.startswith('         ') and key and reasons.get(key):
            reasons[key] += ' ' + line.strip()
        elif named and not file:
            classes[named.group(1)] = named.group(2)
        elif line.startswith('    ') and not file and classes:
            classes[list(classes)[-1]] += ' ' + line.strip()
    return classes, reasons


def wrap(text, first, rest, width=78):
    out, line = [], first
    for word in text.split():
        if len(line) + len(word) + 1 > width and line.strip() and line != first:
            out.append(line.rstrip())
            line = rest
        line += word + ' '
    return out + [line.rstrip()]


def by_reason(found, reasons):
    words, count = Counter(), Counter()
    for key, (_, text) in found.items():
        name = class_of(reasons.get(key, ''))[0] or 'no class'
        words[name] += len(text)
        count[name] += 1
    return [f'{name}: {words[name]} words in {count[name]} stretches' for name in sorted(words)]


def summary(mission, tables, words, flags, flight):
    out = []
    for what, seen in ((f'{mission.upper()}: {len(tables)} tables', flags), ('  The scenarios alone', flight)):
        c = counts(words, seen)
        total = len(words) - c['basic']
        out += [f'{what}: {total} words, {c["words whole"]} of them run wholly '
                f'({100 * c["words whole"] / max(total, 1):.1f}%).',
                f'    orders: {c["orders run"]} of {c["orders"]} run; store codes: {c["stores run"]} of '
                f'{c["stores"]}; operands: {c["operands read"]} of {c["operands"]} read;',
                f'    branches: {c["branches run"]} of {c["branches"]} run, {c["branches both"]} both ways.']
        if c['basic'] and seen is flags:
            out.append(f'    ({c["basic"]} more in the tables are basic instructions, not counted.)')
    return out


def write_ledger(path, heads, found, classes, reasons):
    out = ['# The words of interpretive code that no test and no scenario runs, and why.',
           '# tests/word_coverage.py --write writes this file and keeps each reason;',
           '# `make line-coverage` measures again and holds the tree to it.',
           '#',
           '# Counted: every word of every table of interpretive code in src/ (each a',
           '# struct interpretive_code), as the interpreter took it in the case tests',
           '# and the scenarios: an order pair by its orders (the left one, the right',
           '# one), a branch by its two outcomes, a store code, an operand in line',
           '# with the code by whether a pass read it just after taking the word it',
           '# follows (in a program that ran that order), and a constant among the',
           '# code by whether a program read it that took an operand naming it. (A',
           "# read alone is not enough: a made-up state's address comes to any word",
           '# of the rope. For the constants the count is an upper one: the read and',
           "# the operand are one program's, not one pass's.) A word stands here if",
           "# any of that was not seen: its line says what, then the listing's line.",
           '#',
           *[f'# {line}' for head in heads for line in head],
           '#',
           '# By reason:',
           *[f'#   {line}' for line in by_reason(found, reasons)],
           '#',
           '# A reason in brackets is one of these:',
           '']
    for name in sorted(classes):
        out += wrap(classes[name], f'[{name}] ', '    ')
    file = None
    for key in sorted(found):
        where, text = found[key]
        if key[0] != file:
            file = key[0]
            out += ['', f'src/{file}']
        out.append(f'  {key[1]}, {key[4]} {where} ({len(text)} word{"s" * (len(text) != 1)}) {key[2]}')
        out += [f'    | {t}' for t in text]
        out += wrap(reasons.get(key) or NO_REASON, '    why: ', '         ')
    path.write_text('\n'.join(out) + '\n')


def judge(found, classes, reasons, held=None, named=None):
    """(new, gone, unexplained): the stretches the ledger does not have,
    those of it that are not found now, and those whose reason does not
    stand, with what is wrong. (held: found_of()'s; named: {mission:
    naming()}.)"""
    new = sorted(key for key in found if key not in reasons)
    gone = sorted(key for key in reasons if key not in found)
    unexplained = []
    for key in sorted(found):
        if key not in reasons:
            continue
        name, why = class_of(reasons[key])
        if not reasons[key] or reasons[key] == NO_REASON:
            unexplained.append((key, 'has no reason'))
        elif name is None:
            unexplained.append((key, 'has a reason of no class'))
        elif name not in classes:
            unexplained.append((key, f'has a class the head does not define ([{name}])'))
        elif fits(name, why, (held or {}).get(key), named):
            unexplained.append((key, fits(name, why, (held or {}).get(key), named)))
    return new, gone, unexplained


def self_check():
    """The check, on made-up words and records: each way of being wrong
    must be reported as that."""
    bad = [0]

    def part(what, got, want):
        if got != want:
            bad[0] += 1
            print(f'FAIL self-check, {what}: {got!r}, not {want!r}')

    class Line:
        def __init__(self, text, src='A.agc'):
            self.text, self.src = text, src
            self.label = text.split()[0] if text[:1].strip() else ''
            rest = text[len(self.label):]
            self.op, self.operand = ('', rest.strip()) if rest.startswith('  ') else (rest.split(None, 1) +
                                                                                      ['', ''])[:2]

    def word(phys, kind, text, left=None, right=None, table='T_CODE'):
        made = Word()
        made.phys, made.kind, made.left, made.right, made.table, made.file = phys, kind, left, right, table, 'a/a.c'
        made.line, made.index, made.value = Line(text), 0, 0
        return made

    words = {w.phys: w for w in (
        word(8192, 'pair', ' DLOAD   BPL', 'DLOAD', 'BPL'), word(8193, 'operand', '         X'),
        word(8194, 'operand', '         THERE'), word(8195, 'store', ' STORE   Y,1'),
        word(8196, 'pair', ' EXIT', 'EXIT'), word(8197, 'pair', ' RVQ', 'RVQ', table='U_CODE'))}
    whole = {8192: READ | LEFT | LEFT_ON | RIGHT | RIGHT_ON | RIGHT_AWAY, 8193: READ, 8194: READ,
             8195: READ | STORE, 8196: READ | LEFT | LEFT_AWAY, 8197: READ | LEFT | LEFT_AWAY}
    part('all run', dict(stretches(words, whole)), {})
    part('an order of a pair unrun', misses(words[8192], READ | LEFT | LEFT_ON), ['BPL unrun'])
    part('a branch never taken', misses(words[8192], whole[8192] & ~RIGHT_AWAY), ['BPL never taken'])
    part('a branch always taken', misses(words[8192], whole[8192] & ~RIGHT_ON), ['BPL always taken'])
    part('a pair unrun', misses(words[8192], READ), ['unrun'])
    part('a pair read as an operand only', misses(words[8196], READ), ['unrun'])
    part('a store code read and not run', misses(words[8195], READ), ['unrun'])
    part('an operand unread', misses(words[8193], 0), ['unread'])
    part('an order that does not branch, gone one way', misses(words[8196], whole[8196]), [])
    none = stretches(words, {})
    part('nothing run: a stretch for each table', sorted((k[1], len(runs[0][1])) for k, runs in none.items()),
         [('T_CODE', 5), ('U_CODE', 1)])
    split = stretches(words, {8194: READ})
    part('a word run between two: two stretches', [len(text) for runs in split.values() for _, text, _ in runs
                                                    if runs[0][0].table == 'T_CODE'], [2, 2])
    part('the counts', dict(counts(words, whole)),
         {'orders': 4, 'orders run': 4, 'branches': 1, 'branches run': 1, 'branches both': 1, 'operands': 2,
          'operands read': 2, 'stores': 1, 'stores run': 1, 'words whole': 6})
    stadr = word(8200, 'store', ' STORE   Z')
    stadr.value = 0o77000
    part("STADR's store code, read in line", misses(stadr, READ | IN_LINE), [])
    part("STADR's store code, read from elsewhere", misses(stadr, READ), ['unrun'])
    part("STADR's store code, unread", misses(stadr, 0), ['unrun'])
    part('a basic instruction in a table', misses(word(8201, 'basic', ' TC INTPRET'), 0), [])
    part('an orphan at a pair with a right order', adopted(words[8192], READ | ORPHAN | ORPHAN_AWAY),
         READ | RIGHT | RIGHT_AWAY)
    part('an orphan at a pair with none', adopted(words[8196], READ | ORPHAN), READ)
    part('an orphan at an operand', adopted(words[8193], READ | ORPHAN | ORPHAN_ON), READ)
    found = found_of({'lm': (words, {}), 'cm': (words, {8197: whole[8197]})})
    part('one stretch of both missions, one of one', sorted(k[4] for k in found), ['lm', 'lm cm'])
    reasons = {key: '[impossible] The words before rule it out.' for key in found}
    scratch = BUILD.parent / f'word-coverage-self-check-{id(found)}.txt'
    try:
        scratch.parent.mkdir(parents=True, exist_ok=True)
        write_ledger(scratch, [['made up']], found, CLASSES, reasons)
        classes, read = read_ledger(scratch)
        part('a ledger written and read', (classes, read), (CLASSES, reasons))
    finally:
        scratch.unlink(missing_ok=True)
    key = sorted(found)[0]

    def judged(what, change, want):
        changed = dict(reasons)
        change(changed)
        part(what, tuple(len(x) for x in judge(found, CLASSES, changed)), want)

    judged('a ledger that fits', lambda r: None, (0, 0, 0))
    judged('a stretch the ledger does not have', lambda r: r.pop(key), (1, 0, 0))
    judged('a stretch of the ledger not found now',
           lambda r: r.update({(*key[:2], '00000000', 0, 'lm'): '[never] No.'}), (0, 1, 0))
    judged('no reason', lambda r: r.update({key: NO_REASON}), (0, 0, 1))
    judged('a reason of no class', lambda r: r.update({key: 'It never runs.'}), (0, 0, 1))
    judged('a class the head does not define', lambda r: r.update({key: '[nonesuch] No.'}), (0, 0, 1))
    judged('an [impossible] that does not say why', lambda r: r.update({key: '[impossible]'}), (0, 0, 1))
    judged('a [never] that does not say why', lambda r: r.update({key: '[never]'}), (0, 0, 1))
    judged('an [open] stretch, which is no class any more', lambda r: r.update({key: '[open]'}), (0, 0, 1))
    # The two classes the check looks into: constants, by what names them.
    consts = {w.phys: w for w in (
        word(9000, 'operand', 'K1 DEC 1'), word(9001, 'operand', 'K2 2DEC 2'), word(9002, 'operand', 'K2 2DEC 2'),
        word(9003, 'operand', ' DEC 3'), word(9004, 'pair', ' EXIT', 'EXIT'))}
    stretch = [(consts[phys], ['unread']) for phys in (9000, 9001, 9002)]

    def fit(what, name, stretch, lines, want, before=None):
        names = defaultdict(list)
        for text in lines:
            line = Line(text)
            names[line.operand.split()[0]].append(line)
        got = fits(name, 'Why.', {'lm': stretch}, {'lm': (names, {8999: Line(before)} if before else {})})
        part(what, bool(got), want)

    fit('[unnamed], and nothing names the constants', 'unnamed', stretch, [' CAF K3'], False)
    fit('[unnamed], and an instruction names one', 'unnamed', stretch, [' CAF K2'], True)
    fit('[unnamed], and an operand of an order names one', 'unnamed', stretch, ['  K1'], True)
    fit('[unnamed] with a word of no name', 'unnamed', [(consts[9003], ['unread'])], [], True)
    fit('[unnamed] with an order', 'unnamed', [(consts[9004], ['unrun'])], [], True)
    fit('[basic], and a basic instruction names one', 'basic', stretch, [' CAF K2'], False)
    fit('[basic], and only an order names one', 'basic', stretch, ['  K1', ' DLOAD K2'], True)
    fit('[basic], and nothing names one', 'basic', stretch, [], True)
    fit('[basic] after a TC', 'basic', [(consts[9000], ['unread'])], [], False, before='T TC POLY')
    fit('[basic] after a word that is no TC', 'basic', [(consts[9000], ['unread'])], [], True, before='T DEC 4')
    fit('[basic] with an order', 'basic', [(consts[9004], ['unrun'])], [' CAF K1'], True)
    # An operand is read in line by a program that ran its order, a constant
    # by one that took an operand naming it.
    code = {w.phys: w for w in (
        word(9100, 'pair', ' DLOAD   DAD', 'DLOAD', 'DAD'), word(9101, 'operand', '         K1'),
        word(9102, 'operand', '         V,1'), word(9103, 'pair', ' SSP', 'SSP'),
        word(9104, 'operand', '         S1'), word(9105, 'operand', 'K0 DEC 0'), word(9106, 'pair', ' EXIT', 'EXIT'),
        word(9110, 'operand', 'K1 2DEC 1'), word(9111, 'operand', 'K1 2DEC 1'), word(9112, 'operand', ' DEC 2'),
        word(9113, 'operand', 'K2 DEC 3'), word(9114, 'operand', 'V DEC 4', table='V_TABLE'),
        word(9115, 'operand', 'W DEC 5', table='V_TABLE'))}
    code[9111].index = 1
    labels = {'K1': 9110, 'K2': 9113, 'V': 9114, 'W': 9115}
    inline, block = layout(code)
    part('operands in line, and whose', inline, {9101: 9100, 9102: 9100, 9104: 9103, 9105: 9103})
    part('constants, by block', block, {9110: 9110, 9111: 9110, 9112: 9110, 9113: 9113, 9114: 9114, 9115: 9115})
    part('what names each constant', dict(namers(code, inline, block, labels)),
         {9110: {9101}, 9111: {9101}, 9112: {9101}, 9114: {9102}, 9115: {9102}})
    every = {phys: READ | (IN_LINE if phys in inline else 0) for phys in code}
    took = READ | IN_LINE
    part('one program ran the order and read it all', read_as_named(code, {'a': {**every, 9100: READ | LEFT}}, labels),
         {9101, 9102, 9110, 9111, 9112, 9114, 9115})
    part("a named number in line, its order run", read_as_named(code, {'a': {**every, 9103: READ | LEFT}}, labels),
         {9104, 9105})
    part('a program that read everything and ran nothing', read_as_named(code, {'a': every}, labels), set())
    part('the order in one program, the reads in another',
         read_as_named(code, {'a': {9100: READ | LEFT, 9101: took, 9102: took}, 'b': every}, labels), {9101, 9102})
    part('an operand taken, its constant not read',
         read_as_named(code, {'a': {9100: READ | LEFT, 9101: took, 9111: READ}}, labels), {9101, 9111})
    part('an operand read from elsewhere, its order run',
         read_as_named(code, {'a': {9100: READ | LEFT, 9101: READ, 9102: took, 9110: READ}}, labels), {9102})
    # Right orders with no pair: basic code's, a store over EDOP, or wrong.
    lines = {8999: Line(' TC USPRCADR'), 9000: Line(' CADR X'), 9300: Line(' TCF VXV')}
    part("a lost order at USPRCADR's CADR, at a store code and in basic code",
         len(lost('lm', Counter({9000: 3, 8195: 1, 9300: 2}), words, lines)[1]), 0)
    part('a lost order at an operand of a table', len(lost('lm', Counter({8193: 1}), words, lines)[1]), 1)
    part('a lost order just after a table', len(lost('lm', Counter({8199: 1}), words, lines)[1]), 1)
    part('a lost order at a store code with no index', len(lost('lm', Counter({8200: 1}), {8200: stadr}, lines)[1]), 1)
    codes = order_codes()
    part('DLOAD CALL as the rope has it', pair_codes(0o45145), (codes['DLOAD'], codes['CALL']))
    part('EXIT alone', pair_codes(0o77776), (codes['EXIT'], None))
    print(f"{'FAIL' if bad[0] else 'ok  '} word coverage self-check: {bad[0]} parts judged wrongly")
    return 1 if bad[0] else 0


def listed(name):
    for mission in ('lm', 'cm'):
        _, words, flags, _, _, _ = measure(mission)
        for phys in sorted(words):
            word = words[phys]
            if word.table.split('[')[0] == name or word.table == name:
                got = flags.get(phys, 0)
                print(f'{mission} {phys // 1024:02o},{0o2000 + phys % 1024:04o} {word.value:05o} {got:03x} '
                      f'{word.kind:<8}{", ".join(misses(word, got)) or "run":<24}{text_of(word)}')


def main(args):
    if '--self-check' in args:
        return self_check()
    if '--list' in args:
        listed(args[args.index('--list') + 1])
        return 0
    if not any(WORDS.glob('*.words')):
        sys.exit(f'word_coverage: nothing under {WORDS.relative_to(READABLE)} (make line-coverage leaves it)')
    measures, heads, wrong = {}, [], []
    for mission in ('lm', 'cm'):
        tables, words, flags, bad, (files, orphans, mixed, said), flight = measure(mission)
        measures[mission] = (words, flags)
        wrong += bad
        heads.append(summary(mission, tables, words, flags, flight) +
                     [f'  From {len(files)} programs. {orphans} right orders ran that the recorder found no '
                      'pair for,', '  taken or behind LOC: counted nowhere.'] + said)
        strangers = sorted(set(mixed) - set(MADE_UP))
        if strangers:
            wrong.append(f'{mission} words read with two values (made-up programs) in {", ".join(strangers)}, '
                         'which MADE_UP does not name')
    for head in heads:
        print('\n'.join(head))
    held = {}
    found = found_of(measures, held)
    classes, reasons = read_ledger()
    if '--write' in args:
        write_ledger(LEDGER, heads, found, CLASSES, {k: reasons.get(k) or NO_REASON for k in found})
        print(f'wrote {LEDGER.relative_to(READABLE)}')
        return 0
    for line in by_reason(found, reasons):
        print(f'  {line}')
    named = {mission: naming(CE.Listing(mission)) for mission in measures}
    new, gone, unexplained = judge(found, classes, reasons, held, named)
    for key in new[:40]:
        print(f'FAIL src/{key[0]}: {key[1]} ({key[4]}) {found[key][0]}, {len(found[key][1])} words, is not run '
              'wholly and not in the ledger')
    for key in gone[:40]:
        print(f'FAIL src/{key[0]}: {key[1]} ({key[4]}) {key[2]} is in the ledger and is not found now')
    for key, what in unexplained[:40]:
        print(f'FAIL src/{key[0]}: {key[1]} ({key[4]}) {key[2]} {what}')
    for line in wrong[:40]:
        print(f'FAIL {line}')
    bad = len(new) + len(gone) + len(unexplained) + len(wrong)
    print(f"{'FAIL' if bad else 'ok  '} word coverage against {LEDGER.relative_to(READABLE)}: {len(new)} "
          f'stretches not in it, {len(gone)} of its stretches not found now, {len(unexplained)} with a reason '
          f'that does not stand, {len(wrong)} things wrong with the tables or the records')
    if bad:
        # What --write would make of this measure, for a run whose tree is
        # not at hand (CI keeps it).
        write_ledger(BUILD / LEDGER.name, heads, found, CLASSES, {k: reasons.get(k) or NO_REASON for k in found})
        print(f'the ledger as measured is {(BUILD / LEDGER.name).relative_to(READABLE)}')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
