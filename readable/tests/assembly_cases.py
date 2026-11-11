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
    ('lm', 'Luminary099', 'INPUT_OUTPUT_CHANNEL_BIT_DESCRIPTIONS'),
    ('lm', 'Luminary099', 'MAIN'),
    ('cm', 'Comanche055', 'CONTRACT_AND_APPROVALS'),
    ('cm', 'Comanche055', 'MAIN'),
)

# Lines of MAIN.agc that name a file the transcription keeps under another
# name.
KEPT_AS = {
    ('Luminary099', 'LAMBERT_AIMPOINT_GUIDANCE'): 'GENERAL_LAMBERT_AIMPOINT_GUIDANCE',
    ('Luminary099', 'TRIM_GIMBAL_CNTROL_SYSTEM'): 'TRIM_GIMBAL_CONTROL_SYSTEM',
}

PAGE = re.compile(r'#\s*Page \d+\s*$')


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
