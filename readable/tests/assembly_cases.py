#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""src/assembly's tables against the listing files they carry, word for
word (make assembly-cases).

For each file below, build/assembly-dump-lm or -cm prints the C tables back
as text, and that text must be the listing file's own, every word in
order: nothing of the listing left out of the C, and nothing in the C that
the listing does not say. Spacing and line breaks are free.

The listing's words, for a file of comments: everything after the
transcriber's header (which ends at the first "# Page" line), without the
"# Page N" markers, the comment signs, and the rows of asterisks that
frame a box or a heading.

TAGS_FOR_RELATIVE_SETLOC.agc is code as well as comment; its words are
compared the same way, the number on a BLOCK, BANK or BNKSUM card as a
number (the CM's listing writes BANK 4 and BANK 04 alike). Then what the
tables say is compared with the assembled program: every tag must be, in
the assembler's symbol table (c64-port/build/fixed-data/<listing>/
assembly.lst), the first word of the bank the table gives it; and each of
the CM's ECADRs must be the rope's word (MAIN.agc.bin) at the bank and
address the table gives, which must be the symbol table's for its name.

FLAGWORD_ASSIGNMENTS.agc has code and comment side by side, so its words
are compared block by block, a block's code before its comment
(flagword_words). Then every flag must have, in the symbol table, the
number the table gives it, every bit's name the address of its BITn, and
every flagword its place after STATE; the SET and RESET text must sit in
the listing's columns (flagword_columns); and the file's alphabetical list
must agree with its assignments except at LIST_SLIPS.

CONTROLLED_CONSTANTS.agc's words are compared in order, and then
(constants_where) its addresses and words with the assembler's listing of
the file and with the rope, and each decimal constant's words with the
words its number makes.

ASSEMBLY_AND_OPERATION_INFORMATION.agc is lists set in columns. Its words
are compared in order, and then (information_where) its lines one by one:
each line of the listing must be a line of the table INFORMATION_LISTS
gives its list, with each run of text in the cell of the column it stands
in.

ERASABLE_ASSIGNMENTS.agc is cards and comments. Its words are compared in
order, and then (erasable_where) its lines one by one, each with its name,
card, operand and comment in their fields; every name's value with the
assembler's symbol table; the ERASE cards by following them from each
SETLOC to the word each must name; and the CM's sorted list of flags with
the flags' numbers, which it must agree with except at CM_LIST_SLIPS.

MAIN.agc is compared line by line instead: a file's line by its name and
pages (the table writes "p. 5" and "pp. 5" alike as first and last page 5),
a comment line by its words. Each file's directory in the table must be
the one COVERAGE gives the file, or src/assembly for a file COVERAGE does
not list yet, and must exist.
"""
import re
import subprocess
import sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
ROOT = READABLE.parent

# (mission, listing directory, file).
FILES = (
    ('lm', 'Luminary099', 'ASSEMBLY_AND_OPERATION_INFORMATION'),
    ('lm', 'Luminary099', 'INPUT_OUTPUT_CHANNEL_BIT_DESCRIPTIONS'),
    ('lm', 'Luminary099', 'CONTROLLED_CONSTANTS'),
    ('lm', 'Luminary099', 'ERASABLE_ASSIGNMENTS'),
    ('lm', 'Luminary099', 'FLAGWORD_ASSIGNMENTS'),
    ('lm', 'Luminary099', 'MAIN'),
    ('lm', 'Luminary099', 'TAGS_FOR_RELATIVE_SETLOC'),
    ('cm', 'Comanche055', 'ASSEMBLY_AND_OPERATION_INFORMATION'),
    ('cm', 'Comanche055', 'CONTRACT_AND_APPROVALS'),
    ('cm', 'Comanche055', 'ERASABLE_ASSIGNMENTS'),
    ('cm', 'Comanche055', 'MAIN'),
    ('cm', 'Comanche055', 'TAGS_FOR_RELATIVE_SETLOC'),
)

# Lines of MAIN.agc that name a file the transcription keeps under another
# name.
KEPT_AS = {
    ('Luminary099', 'LAMBERT_AIMPOINT_GUIDANCE'): 'GENERAL_LAMBERT_AIMPOINT_GUIDANCE',
    ('Luminary099', 'TRIM_GIMBAL_CNTROL_SYSTEM'): 'TRIM_GIMBAL_CONTROL_SYSTEM',
}

# A page marker of the transcription ("# Page 61"; also "# Page63",
# "# Page 89 (nothing on this page)" and "# Page 130 ... is empty."), and
# a transcriber's note beside a line ("# OH 2009-05-15 Not in scan"):
# neither is the listing's.
PAGE = re.compile(r'#\s*Page\s*\d+(\s*\(.*\)|\s*\.\.\. is empty\.)?\s*$')
NOTE = re.compile(r'#\s*[A-Z]{2,3} \d{4}-\d\d-\d\d .*$')


def listing_words(path):
    lines = path.read_text().splitlines()
    start = next(n for n, line in enumerate(lines) if PAGE.match(line))
    words = []
    for line in lines[start:]:
        if PAGE.match(line):
            continue
        words += [w for w in line.lstrip('#').split() if w.strip('*')]
    return words


def table_text(mission, name):
    dump = READABLE / 'build' / f'assembly-dump-{mission}'
    return subprocess.run([str(dump), name], check=True, capture_output=True, text=True).stdout


def bank_numbers(words):
    """The words, with the number on a BLOCK, BANK or BNKSUM card as a
    number, and without the comment signs and the rows of asterisks."""
    out = []
    for word in words:
        if out and out[-1] in ('BLOCK', 'BANK', 'BNKSUM') and re.fullmatch('[0-7]+', word):
            word = '%o' % int(word, 8)
        if word.strip('*') and word != '#':
            out.append(word)
    return out


def symbol_table(tree, kind):
    """The assembler's symbol table for the names of one kind (F in fixed
    memory, E in erasable, C a constant), {name: value as written}."""
    text = (ROOT / 'c64-port' / 'build' / 'fixed-data' / tree / 'assembly.lst').read_text(
        errors='replace')
    return dict(re.findall(r'\d+,%s:\s+(\S+)\s+(\S+)\s' % kind,
                           text[text.index('Symbol Table'):] + '\n'))


def assembled(tree):
    """The assembler's symbol table for the fixed-memory names, {name:
    (bank, address)}, and the rope as a function of bank and address."""
    symbols = {}
    for name, value in symbol_table(tree, 'F').items():
        bank, _, address = value.rpartition(',')
        address = int(address, 8)
        # Fixed-fixed memory is banks 2 and 3.
        symbols[name] = (int(bank, 8), address) if bank else (address // 0o2000, address)
    rope = (ROOT / 'c64-port' / 'build' / 'fixed-data' / tree / 'MAIN.agc.bin').read_bytes()

    def word(bank, address):
        # The rope's file has banks 2 and 3 first, then 0, 1, 4 and on;
        # each word is two bytes, its parity bit the low bit.
        at = ({2: 0, 3: 1, 0: 2, 1: 3}.get(bank, bank) * 0o2000 + address % 0o2000) * 2
        return (rope[at] << 8 | rope[at + 1]) >> 1

    return symbols, word


def place(where):
    """A bank and address as the listing writes them."""
    return 'nowhere' if where is None else '%02o,%04o' % where


def setloc_where(mission, tree):
    """What is wrong with where TAGS_FOR_RELATIVE_SETLOC's tables put the
    tags and the ECADRs, against the assembled program; and how many
    there are."""
    symbols, rope = assembled(tree)
    wrong, lines = [], table_text(mission, 'TAGS_FOR_RELATIVE_SETLOC:where').splitlines()
    for line in lines:
        kind, name, *numbers = line.split()
        bank, *rest = (int(n, 8) for n in numbers)
        # A bank's first word: 4000 and 6000 for fixed-fixed 02 and 03,
        # 2000 in a switched bank.
        first = bank * 0o2000 if bank in (2, 3) else 0o2000
        if kind == 'TAG' and symbols.get(name) != (bank, first):
            wrong.append(f'tag {name} is not the first word of bank {bank:02o}: the assembler '
                         f'has it at {place(symbols.get(name))}')
        elif kind == 'WORD':
            address, value = rest
            if symbols.get(name) != (bank, address):
                wrong.append(f'{name} is not at {place((bank, address))}: the assembler has it '
                             f'at {place(symbols.get(name))}')
            elif rope(bank, address) != value:
                wrong.append(f'{name} is {value:05o}, the rope has {rope(bank, address):05o}')
    return wrong, len(lines)


def flagword_words(path):
    """FLAGWORD_ASSIGNMENTS.agc's words with each block's code before its
    comment, as the dump writes them. A block starts at a "# BIT n FLAG m"
    line, at a flagword's line (NAME = STATE +n) and at a flagword's
    other name (NAME EQUALS ...); within it the listing has code and
    comment side by side."""
    lines = path.read_text().splitlines()
    start = next(n for n, line in enumerate(lines) if PAGE.match(line))
    out, code, comment = [], [], []

    def flush():
        out.extend(code + comment)
        code.clear()
        comment.clear()

    for line in lines[start:]:
        if PAGE.match(line):
            continue
        line = NOTE.sub('', line)
        text, _, remark = line.partition('#')
        words = [w for w in remark.replace('#', ' ').split() if w.strip('*')]
        if re.match(r'# BIT \d+ FLAG \d+', line):
            flush()
            out.extend(words)
            continue
        fields = text.split()
        if 'EQUALS' in fields or 'STATE' in fields:
            flush()
        code.extend(fields)
        comment.extend(words)
    flush()
    return out


# The columns of a flag's text in FLAGWORD_ASSIGNMENTS.agc, counted from 0
# with tab stops every 8: the comment sign stands in column 48, SET's text
# starts in column 56 and RESET's in column 80, as the column heads
# "(SET)" and "(RESET)" do.
RESET_COLUMN = 80


def flagword_columns(path):
    """Each flag's text by column, as the dump's :columns writes it: a
    comment whose text follows a tab is in columns, each run of text
    between tabs in SET's column if it starts left of RESET_COLUMN and
    in RESET's if not; any other comment is a note across both."""
    lines = path.read_text().splitlines()
    start = next(n for n, line in enumerate(lines)
                 if 'ASSIGNMENT AND DESCRIPTION OF FLAGWORDS' in line)
    out, flag, row = [], -1, 0
    for line in lines[start + 1:]:
        if PAGE.match(line):
            continue
        line = NOTE.sub('', line)
        if re.match(r'# BIT \d+ FLAG \d+', line):
            flag, row = flag + 1, 0
            continue
        fields = line.partition('#')[0].split()
        if 'EQUALS' in fields or 'STATE' in fields:
            row = None  # a flagword's line, and the column heads after it
        if '#' not in line or row is None or not line.partition('#')[2].strip():
            continue
        comment = line.partition('#')[2]
        if not comment.lstrip(' ').startswith('\t'):
            out.append(f'{flag} N{row} ' + ' '.join(comment.split()))
            row += 1
            continue
        column = len(line.partition('#')[0].expandtabs(8)) + 1
        texts, text, at = {'S': [], 'R': []}, '', None
        for character in comment + '\t':
            if character == '\t':
                if text.strip():
                    texts['S' if at < RESET_COLUMN else 'R'].append(' '.join(text.split()))
                text, at = '', None
                column = (column // 8 + 1) * 8
            else:
                if at is None and character != ' ':
                    at = column
                text += character
                column += 1
        for name in 'SR':
            if texts[name]:
                out.append(f'{flag} {name}{row} ' + ' '.join(texts[name]))
        row += 1
    return out


# Where FLAGWORD_ASSIGNMENTS.agc's alphabetical list, as transcribed, differs
# from its own assignments; the tables keep both as they stand. Each is
# name: (what the list says, what the assignments say), as (number, bit,
# flagword, bit's name), with -1 for a number the list leaves out and None
# for a name one of the two does not have.
LIST_SLIPS = {
    # The list gives these two no flagword.
    'DIDFLAG': ((16, 14, -1, 'DIDFLBIT'), (16, 14, 1, 'DIDFLBIT')),
    'NJETSFLG': ((15, 15, -1, 'NJETSBIT'), (15, 15, 1, 'NJETSBIT')),
    # The list's number is AVEMIDSW's; bit 10 of flagword 9 is 140.
    'FLRCS': ((149, 10, 9, 'FLRCSBIT'), (140, 10, 9, 'FLRCSBIT')),
    'MID1FLAG': ((147, 3, 9, 'MID1BIT'), (147, 3, 9, 'MID1FBIT')),
    # Number 61 is in flagword 4.
    'PRIODFLG': ((61, 14, 1, 'PRIODBIT'), (61, 14, 4, 'PRIODBIT')),
    # The letter O in the list, the digit 0 in the assignments.
    'DIMOFLAG': ((59, 1, 3, 'DIMOBIT'), None),
    'DIM0FLAG': (None, (59, 1, 3, 'DIM0BIT')),
}


def flagword_where(mission, tree):
    """What is wrong with what FLAGWORD_ASSIGNMENTS's tables assign,
    against the assembled program, and with its list against its
    assignments; and how many lines were compared."""
    erasable, constant = symbol_table(tree, 'E'), symbol_table(tree, 'C')
    fixed = symbol_table(tree, 'F')
    wrong, assigned, slips = [], {}, {}
    lines = table_text(mission, 'FLAGWORD_ASSIGNMENTS:where').splitlines()
    for line in lines:
        kind, name, *rest = line.split()
        if kind == 'WORD':
            want = '%04o' % (int(erasable['STATE'], 8) + int(rest[0]))
            for word in [name] + rest[1:]:
                if erasable.get(word) != want:
                    wrong.append(f'{word} is not STATE +{rest[0]}: the assembler has it at '
                                 f'{erasable.get(word)}, not {want}')
        elif kind == 'FLAG':
            number, bit_name, bit = int(rest[0]), rest[1], int(rest[2])
            if int(constant.get(name, '-1'), 8) != number:
                wrong.append(f'flag {name} is not {number}: the assembler has '
                             f'{constant.get(name)} (octal)')
            if fixed.get(bit_name) != fixed[f'BIT{bit}']:
                wrong.append(f'{bit_name} is not BIT{bit}')
            assigned[name] = (number, bit, number // 15, bit_name)
        else:
            listed = (int(rest[0]), int(rest[1]), int(rest[2]), rest[3])
            if listed != assigned.get(name):
                slips[name] = (listed, assigned.get(name))
    for name in assigned:
        if not any(line.split()[:2] == ['LISTED', name] for line in lines):
            slips[name] = (None, assigned[name])
    if slips != LIST_SLIPS:
        wrong.append('the list differs from the assignments at ' + repr(slips) +
                     ', and LIST_SLIPS says at ' + repr(LIST_SLIPS))
    want = flagword_columns(ROOT / tree / 'FLAGWORD_ASSIGNMENTS.agc')
    got = [' '.join(line.split()) for line in
           table_text(mission, 'FLAGWORD_ASSIGNMENTS:columns').splitlines()]
    for line in sorted(set(want) ^ set(got)):
        wrong.append(('the listing has' if line in want else 'the tables have') +
                     f' in flag, column and line: {line}')
    return wrong, len(lines) + len(want)


def constants_where(mission, tree):
    """What is wrong with CONTROLLED_CONSTANTS's words and addresses,
    against the assembled program, and with its numbers against its
    words; and how many lines were compared. The assembler's listing has
    the file's lines between its $CONTROLLED_CONSTANTS.agc line and the
    end of the include, each word-making line with its address and
    words; the table must have those, in order, and each word must be the
    rope's. A group's bank must be its tag's, and a name an = card gives
    must be the other name's word."""
    symbols, rope = assembled(tree)
    text = (ROOT / 'c64-port' / 'build' / 'fixed-data' / tree / 'assembly.lst').read_text(
        errors='replace')
    text = text[text.index('$CONTROLLED_CONSTANTS.agc'):
                text.index('(End of include-file CONTROLLED_CONSTANTS.agc')]
    want = []
    for line in text.splitlines():
        made = re.match(r'\d+,\d+:\s+(?:(\d\d),)?(\d{4})((?:\s+\d{5}){1,2})\s', line)
        if made:
            bank, address, words = made.groups()
            address = int(address, 8)
            bank = int(bank, 8) if bank else address // 0o2000
            want.append(' '.join(['%o %o' % (bank, address)] + words.split()))
    wrong, got = [], []
    lines = table_text(mission, 'CONTROLLED_CONSTANTS:where').splitlines()
    for line in lines:
        if line.startswith('NAME '):
            _, name, other = line.split()
            if name not in symbols or symbols[name] != symbols.get(other):
                wrong.append(f'{name} is not {other} in the assembler\'s symbol table')
            continue
        if line.startswith('TAG '):
            _, tag, bank = line.split()
            if symbols.get(tag, (None,))[0] != int(bank, 8):
                wrong.append(f'tag {tag} is not in bank {bank}: the assembler has it at '
                             f'{place(symbols.get(tag))}')
            continue
        placed, _, worked = line.partition(' = ')
        got.append(placed)
        bank, address, *words = placed.split()
        for n, word in enumerate(words):
            if rope(int(bank, 8), int(address, 8) + n) != int(word, 8):
                wrong.append(f'the rope does not have {word} at {placed}')
        if worked and worked.split() != words:
            wrong.append(f'the number at {placed} makes the words {worked}')
    if want != got:
        at = next((n for n, (a, b) in enumerate(zip(want, got)) if a != b),
                  min(len(want), len(got)))
        wrong.append(f'word-making line {at} of {len(want)} (the table has {len(got)}): the '
                     f'assembler has {want[at:at + 1]}, the table {got[at:at + 1]}')
    return wrong, len(lines)


# ASSEMBLY_AND_OPERATION_INFORMATION.agc's lists, in the listing's order:
# the words a list's first line begins with, the list's name in the C, and
# the columns its cells start at (the comment sign is column 0, and a tab
# goes to the next multiple of 8).
INFORMATION_LISTS = {
    'Luminary099': (
        ('THIS LGC PROGRAM', 'INTRODUCTION', (0,)),
        ('TABLE OF LOG CARDS', 'LOG_SECTIONS', (0, 8, 16)),
        ('VERB LIST FOR LUMINARY', 'VERBS', (0, 8)),
        ('IN THE FOLLOWING NOUN LIST', 'NOUN_LIST_INTRODUCTION', (0,)),
        ('NORMAL NOUNS', 'NORMAL_NOUNS', (0, 8, 44, 56, 96)),
        ('MIXED NOUNS', 'MIXED_NOUNS', (0, 8, 44, 56, 96)),
        ('REGISTERS AND SCALING FOR NORMAL NOUNS', 'NORMAL_NOUN_REGISTERS', (0, 16, 40)),
        ('REGISTERS AND SCALING FOR MIXED NOUNS', 'MIXED_NOUN_REGISTERS', (0, 16, 32, 56)),
        ('NOUN SCALES AND FORMATS', 'NOUN_SCALES', (0, 24, 48, 56)),
        ('ALARM CODES FOR LUMINARY', 'ALARM_CODES', (0, 8, 16, 64)),
        ('CHECKLIST CODES FOR LUMINARY', 'CHECKLIST_CODES', (0, 16, 31, 72)),
        ('OPTION CODES FOR LUMINARY', 'OPTION_CODES', (0, 16, 48, 80, 96)),
    ),
    'Comanche055': (
        ('ASSEMBLY AND OPERATIONS INFORMATION', 'LOG_SECTIONS', (0, 8, 16)),
        ('VERB LIST FOR CSM', 'VERBS', (0,)),
        ('IN THE FOLLOWING NOUN LIST', 'NOUN_LIST_INTRODUCTION', (0,)),
        ('NORMAL NOUNS', 'NORMAL_NOUNS', (0, 7, 43, 56, 80)),
        ('MIXED NOUNS', 'MIXED_NOUNS', (0, 7, 43, 56, 80)),
        ('REGISTERS AND SCALING FOR NORMAL NOUNS', 'NORMAL_NOUN_REGISTERS', (0, 8, 32)),
        ('REGISTERS AND SCALING FOR MIXED NOUNS', 'MIXED_NOUN_REGISTERS', (0, 8, 16, 32)),
        ('NOUN SCALES AND FORMATS', 'NOUN_SCALES', (0, 24, 48, 56)),
        ('ALARM CODES FOR 504', 'ALARM_CODES', (0, 8, 16, 64, 104)),
        ('CHECKLIST CODES FOR 504', 'CHECKLIST_CODES', (0, 16, 32)),
        ('OPTION CODES FOR 504', 'OPTION_CODES', (0, 16, 56, 88, 104)),
    ),
}


def runs_of_text(line):
    """A comment line's runs of text, each with the column it starts at: a
    tab, or three spaces or more, parts one run from the next."""
    runs, text, start, column, spaces = [], '', None, 0, 0
    for at, char in enumerate(line + '\t'):
        if char == '\t' or (spaces >= 3 and char != ' '):
            if text.strip():
                runs.append((start, ' '.join(text.split())))
            text, start = '', None
        if char == '\t':
            column, spaces = (column // 8 + 1) * 8, 0
            continue
        spaces = spaces + 1 if char == ' ' else 0
        if start is None and char != ' ' and not (char == '#' and at == 0):
            start = column
        if start is not None:
            text += char
        column += 1
    return runs


def information_lines(tree):
    """The listing's lines as the dump writes them (:lines): the list's
    name, then a tab before each cell; a run of text goes in the cell of
    the last column that starts at or before it."""
    lines = (ROOT / tree / 'ASSEMBLY_AND_OPERATION_INFORMATION.agc').read_text().splitlines()
    start = next(n for n, line in enumerate(lines) if PAGE.match(line))
    lists = list(INFORMATION_LISTS[tree])
    out, name, columns = [], None, None
    for line in lines[start:]:
        runs = runs_of_text(line)
        if PAGE.match(line) or not runs:
            continue
        if lists and ' '.join(line.lstrip('#').split()).startswith(lists[0][0]):
            _, name, columns = lists.pop(0)
        cells = [[] for _ in columns]
        for at, text in runs:
            cells[max(n for n, column in enumerate(columns) if column <= at)].append(text)
        out.append('\t'.join([name] + [' '.join(cell) for cell in cells]))
    if lists:
        raise SystemExit(f'{tree}: no line begins {lists[0][0]}')
    return out


def information_where(mission, tree):
    want = information_lines(tree)
    got = table_text(mission, 'ASSEMBLY_AND_OPERATION_INFORMATION:lines').splitlines()
    wrong = []
    if want != got:
        at = next((n for n, (a, b) in enumerate(zip(want, got)) if a != b),
                  min(len(want), len(got)))
        wrong.append(f'line {at} of {len(want)} (the tables have {len(got)}): the listing has '
                     f'{want[at:at + 1]}, the tables {got[at:at + 1]}')
    return wrong, len(want)


# In the CM's ERASABLE_ASSIGNMENTS.agc the lines that assign the flags, from
# FLAGWRD0's card to the heading CM_FLAGS_END, have their comments in two
# columns, SET's and RESET's, the second at CM_RESET_COLUMN.
CM_FLAGS_END = 'GENERAL ERASABLE ASSIGNMENTS'
CM_RESET_COLUMN = 64

# The words taken by a card that is not SETLOC, ERASE, EQUALS or =: the
# CM's line with nothing but an exponent on it, which the assembler gives a
# word (see erasable_assignments.c).
OTHER_CARDS = {'-16': 1}

# The flags of the CM's sorted list that its assignments do not have as
# the list says (erasable_assignments.c says what each slip is).
CM_LIST_SLIPS = ['DMENFLAG', 'ENG1FLAG', 'IDLEFLAG', 'PRFTRKAT', 'RPQFLAG', 'V67FLAG']


def erasable_lines(tree):
    """ERASABLE_ASSIGNMENTS.agc's lines as the dump's :lines writes them:
    name, card, operand, comment, SET text and RESET text, a tab between.
    A line that begins with a comment sign is a comment. On any other,
    what stands at the margin is the name, the next field the card and
    the rest before the comment sign the operand. In the CM's flag lines
    the comment after a card, or on a line under one, goes by its columns
    to SET or RESET (each run of text between tabs by the column it
    starts at), unless it is one run that reaches across both columns or
    the card is a flagword's own (its operand begins STATE)."""
    lines = (ROOT / tree / 'ERASABLE_ASSIGNMENTS.agc').read_text().splitlines()
    start = next(n for n, line in enumerate(lines) if PAGE.match(line))
    out, flags = [], False
    for line in lines[start:]:
        if PAGE.match(line) or not line.strip():
            continue
        code, sign, comment = line.partition('#')
        if line.startswith('FLAGWRD0'):
            flags = tree == 'Comanche055'
        elif CM_FLAGS_END in line:
            flags = False
        fields = code.split()
        name = fields.pop(0) if fields and not line[0].isspace() else ''
        card = fields.pop(0) if fields else ''
        remark, columns = ' '.join(word for word in comment.split() if word != '#'), ['', '']
        if flags and sign and not line.startswith('#') and not fields[:1] == ['STATE']:
            runs, text, at, column = [], '', None, len(code.expandtabs(8)) + 1
            for char in comment + '\t':
                if char == '\t':
                    if text.strip():
                        runs.append((at, ' '.join(text.split())))
                    text, at, column = '', None, (column // 8 + 1) * 8
                else:
                    if at is None and char != ' ':
                        at = column
                    text += char
                    column += 1
            across = [at < CM_RESET_COLUMN < at + len(text) for at, text in runs]
            if not any(across):
                remark = ''
                columns = [' '.join(text for at, text in runs if (at >= CM_RESET_COLUMN) == right)
                           for right in (False, True)]
            elif len(runs) > 1:
                raise SystemExit(f'{tree}: a comment across the columns and more: {line}')
        if name or card or remark or any(columns):
            out.append('\t'.join([name, card, ' '.join(fields), remark] + columns))
    return out


def erasable_where(mission, tree):
    want = erasable_lines(tree)
    got = table_text(mission, 'ERASABLE_ASSIGNMENTS:lines').splitlines()
    wrong = []
    if want != got:
        at = next((n for n, (a, b) in enumerate(zip(want, got)) if a != b),
                  min(len(want), len(got)))
        wrong.append(f'line {at} of {len(want)} (the table has {len(got)}): the listing has '
                     f'{want[at:at + 1]}, the table {got[at:at + 1]}')

    # Every name's value against the assembler's symbol table: an erasable
    # address there is aaaa or, in a switched bank, En,aaaa.
    symbols = {}
    for kind in 'ECF':
        for name, value in symbol_table(tree, kind).items():
            bank, _, address = value.rpartition(',')
            if kind == 'E' and bank:
                symbols[name] = (kind, int(bank[1:], 8) * 0o400 + int(address, 8) - 0o1400)
            elif not bank:
                symbols[name] = (kind, int(address, 8))
    values = {}
    for line in table_text(mission, 'ERASABLE_ASSIGNMENTS:where').splitlines():
        name, kind, value = line.split()
        if name in values:
            wrong.append(f'{name} is named twice')
        values[name] = (kind, int(value, 8))
        if symbols.get(name) != values[name]:
            wrong.append(f'{name}: the table has {kind} {value}, the assembler '
                         + ('%s %o' % symbols[name] if name in symbols else 'no such name'))

    # The ERASE cards, followed from each SETLOC: each names the next word
    # (the one card with a range names the range's first word; no ERASE
    # follows it before the next SETLOC), and a name with EQUALS or = and
    # no operand is the next word too.
    at, erased = None, 0
    for line in got:
        name, card, operand = line.split('\t')[:3]
        more = re.fullmatch(r'\+(\d+)(D?)', operand)
        span = re.fullmatch(r'([0-7]+) - [0-7]+', operand)
        here = int(span.group(1), 8) if card == 'ERASE' and span else at
        if card == 'SETLOC':
            at = int(operand, 8)
        elif card == 'ERASE' and (more or span or not operand):
            if more:
                at += int(more.group(1), 10 if more.group(2) else 8)
            at += 0 if span else 1
        elif card in OTHER_CARDS:
            at += OTHER_CARDS[card]
        elif card not in ('', 'EQUALS', '='):
            wrong.append(f'a card of no known kind: {line.split()}')
        if name and (card == 'ERASE' or card in ('EQUALS', '=') and not operand):
            erased += 1
            if values.get(name) != ('E', here):
                wrong.append(f'{name}: the cards before it leave it at '
                             f'{"nowhere" if here is None else "%04o" % here}, the table has '
                             + '%s %o' % values.get(name, ('nothing', 0)))

    # The CM's sorted list of flags: each line's name must be a number, the
    # line's, and its bit and flagword that number's.
    listed, slips = 0, []
    for line in got if tree == 'Comanche055' else ():
        entry = re.fullmatch(r'\t\t\t(\S+) (\d+)D BIT (\d+) FLAG (\d+)( .*)?\t\t', line)
        if line.startswith('FLAGWRD0\t'):
            break
        if entry:
            listed += 1
            name, number, bit, flagword = entry.group(1), *map(int, entry.group(2, 3, 4))
            if values.get(name) != ('C', number) or number != flagword * 15 + 15 - bit:
                slips.append(name)
    if slips != (CM_LIST_SLIPS if tree == 'Comanche055' else []):
        wrong.append(f'the sorted list of flags and the flags differ at {slips}, not at '
                     f'{CM_LIST_SLIPS}')
    return wrong, len(want) + len(values) + erased + listed


def main_lines(path):
    """MAIN.agc's lines as the dump writes them, without the directories."""
    lines = []
    for line in path.read_text().splitlines():
        file = re.match(r'\$(\S+)\s+#\s+pp?\.\s*(\d+)(?:-(\d+))?\s*$', line)
        if file:
            name, first, last = file.groups()
            lines.append(f'${name} {first} {last or first}')
        elif line.strip():
            if not line.lstrip().startswith('#'):
                raise SystemExit(f'{path}: a line of neither kind: {line}')
            lines.append('# ' + ' '.join(line.lstrip().lstrip('#').split()))
    return lines


def coverage():
    listed = {}
    for line in (READABLE / 'COVERAGE').read_text().splitlines():
        line = line.split('#')[0].split()
        if line:
            listed[line[0]] = line[1]
    return listed


def main_table(tree, text):
    """The dump's lines without the directories, and what is wrong with
    the directories."""
    listed, lines, wrong = coverage(), [], []
    for line in text.splitlines():
        if line.startswith('$'):
            name, first, last, where = line.split()
            lines.append(f'{name} {first} {last}')
            stem = name[1:-len('.agc')]
            file = f'{tree}/{KEPT_AS.get((tree, stem), stem)}.agc'
            if not (ROOT / file).is_file():
                wrong.append(f'{file} does not exist')
            elif where != listed.get(file, 'src/assembly') or not (READABLE / where).is_dir():
                wrong.append(f'{file} is in {listed.get(file, "src/assembly")}, '
                             f'not {where}')
        else:
            lines.append(line)
    return lines, wrong


def main():
    failures = total = 0
    for mission, tree, name in FILES:
        wrong = []
        if name == 'MAIN':
            want = main_lines(ROOT / tree / f'{name}.agc')
            got, wrong = main_table(tree, table_text(mission, name))
        elif name == 'ASSEMBLY_AND_OPERATION_INFORMATION':
            # The alarm list's marks are asterisks, which listing_words
            # leaves out with the rows of asterisks; the lines keep them.
            want = listing_words(ROOT / tree / f'{name}.agc')
            got = [w for w in table_text(mission, name).split() if w.strip('*')]
            wrong, placed = information_where(mission, tree)
            total += placed
        elif name == 'ERASABLE_ASSIGNMENTS':
            # A comment sign may stand against the comment's first word.
            want = [w.lstrip('#') for w in listing_words(ROOT / tree / f'{name}.agc')
                    if w.strip('#')]
            got = [w for w in table_text(mission, name).split() if w.strip('*')]
            wrong, placed = erasable_where(mission, tree)
            total += placed
        elif name == 'CONTROLLED_CONSTANTS':
            want = [w for w in listing_words(ROOT / tree / f'{name}.agc') if w != '#']
            got = [w for w in table_text(mission, name).split() if w.strip('*')]
            wrong, placed = constants_where(mission, tree)
            total += placed
        elif name == 'FLAGWORD_ASSIGNMENTS':
            want = flagword_words(ROOT / tree / f'{name}.agc')
            got = [w for w in table_text(mission, name).split() if w.strip('*')]
            wrong, placed = flagword_where(mission, tree)
            total += placed
        elif name == 'TAGS_FOR_RELATIVE_SETLOC':
            want = bank_numbers(listing_words(ROOT / tree / f'{name}.agc'))
            got = bank_numbers(table_text(mission, name).split())
            wrong, placed = setloc_where(mission, tree)
            total += placed
        else:
            want = listing_words(ROOT / tree / f'{name}.agc')
            got = table_text(mission, name).split()
        total += len(want)
        for problem in wrong:
            failures += 1
            print(f'FAIL: {tree}/{name}.agc: {problem}')
        if want != got:
            failures += 1
            at = next((n for n, (a, b) in enumerate(zip(want, got)) if a != b),
                      min(len(want), len(got)))
            print(f'FAIL: {tree}/{name}.agc, item {at} of {len(want)} '
                  f'(the tables print {len(got)}):')
            near = 1 if name == 'MAIN' else 6
            print('  listing:', ' '.join(want[max(0, at - near):at + near]))
            print('  tables: ', ' '.join(got[max(0, at - near):at + near]))
    print(f'assembly-cases: {len(FILES)} files, {total} words and lines, {failures} failures')
    return 1 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
