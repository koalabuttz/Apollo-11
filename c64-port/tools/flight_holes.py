#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Name the source file and label of each unregistered point that
build/flight-store/flight-host-run-<m> reports (docs/FLIGHT-STORE.md), and
the lane that owns the file (docs/overlay-allocation.json).

  tools/flight_holes.py lm|cm < flight-host-run output
"""
import json, re, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MISSION = {'lm': 'Luminary099', 'cm': 'Comanche055'}
HOLE = re.compile(r'^\s+(basic |interp) (\d\d),(\d{4}) (?:Z [0-7]+ FB [0-7]+ Q [0-7]+(?: at (?P<qat>\d\d,\d{4}))?)?.*: '
                  r'(?P<arrivals>\d+) arrivals, (?P<instructions>\d+) instructions(?:, refused by (?P<refused>\S+))?', re.M)
# Points a scenario bridges only because its core snapshot starts mid-way
# through code, not port gaps, as (mission, 'bank,offset'). None now: the
# NASSP LM cores (lm-p63-nassp, lm-p52) start inside the self-check idle
# loop at 43,3303, and its words 3303, 3332 and 3334 are entries of the
# idle table (docs/SELF-CHECK.md), so the loop runs native from there.
CORE_START = set()


class Words(dict):
    """listing()'s {(bank, address): (file, label)}, with .values_at
    {(bank, address): word} and .erasable {physical address: name}."""


# The central registers, 0-7 of erasable: a bridged point there is a
# transfer to a register (TC Q, a return through Q).
REGISTERS = ['A', 'L', 'Q', 'EBANK', 'FBANK', 'Z', 'BBANK', 'ZERO']
ERASABLE = 0o50  # a point's bank at AGC_TRANSFER_ERASABLE_BASE (40960) and up
SYMBOL = re.compile(r'\d+,E:\s+(\S+)\s+(?:E([0-7]),)?([0-7]{4})\b')


def listing(mission):
    """{(bank, address): (file, label)} for every fixed word, with the label
    the nearest one at or before it in the same file (a Words)."""
    words, name, label = Words(), None, None
    words.values_at, words.erasable = {}, dict(enumerate(REGISTERS))
    for text in (ROOT / 'build/fixed-data' / mission / 'assembly.lst').read_text(errors='replace').splitlines():
        for sym, eb, addr in SYMBOL.findall(text):
            a = int(addr, 8)
            words.erasable.setdefault(int(eb, 8) * 0o400 + a - 0o1400 if eb else a, sym)
        m = re.search(r'#\s*Filename:\s*(\S+\.agc)', text)
        if m:
            name, label = m.group(1), None
            continue
        code = text.split('\t#')[0]
        m = re.match(r'^\d+,\d+:\s+(?:(\d\d),)?([0-7]{4})\s+([0-7]{5})', code)
        if not m or name is None:
            continue
        bank, addr, word = m.groups()
        # The label column starts eight characters after the word.
        lab = code[m.end() + 8:].split()[0] if code[m.end() + 8:m.end() + 9].strip() else None
        address = int(addr, 8)
        if bank is None:  # fixed-fixed 04000-07777: banks 02 and 03
            if address < 0o4000:
                continue
            bank, offset = (2 if address < 0o6000 else 3), address % 0o2000 + 0o2000
        else:
            bank, offset = int(bank, 8), address
        if lab and re.match(r'^[A-Z0-9+\-*/.]+$', lab) and not re.match(r'^[0-7]+$', lab):
            label = (lab, offset)
        words.setdefault((bank, offset), (name, label))
        words.values_at.setdefault((bank, offset), int(word, 8))
    return words


def source_commit():
    """HEAD's short hash, with ' (dirty)' when the port's sources differ from
    it: the commit a run's results belong to."""
    git = lambda *a: subprocess.run(['git', *a], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    dirty = git('status', '--porcelain', '--', 'src', 'include', 'platform', 'tools', 'tests')
    return git('rev-parse', '--short=12', 'HEAD') + (' (dirty)' if dirty else '')


def lanes():
    """{'<Mission>/<file>.agc': lane} from docs/overlay-allocation.json."""
    out = {}

    def walk(x):
        if isinstance(x, dict):
            if 'path' in x and 'lane' in x:
                out[x['path']] = x['lane']
            for v in x.values():
                walk(v)
        elif isinstance(x, list):
            for v in x:
                walk(v)
    walk(json.loads((ROOT / 'docs/overlay-allocation.json').read_text()))
    return out


def erasable_name(words, key):
    """'NAME+n' for an erasable point (bank 50 and up), the nearest erasable
    symbol at or before its physical address, or None for a fixed one."""
    if key[0] < ERASABLE:
        return None
    phys = (key[0] - ERASABLE) * 0o2000 + key[1] - 0o2000
    known = getattr(words, 'erasable', {})
    at = max((a for a in known if a <= phys and (a < len(REGISTERS)) == (phys < len(REGISTERS))), default=None)
    return '?' if at is None else known[at] + (f'+{phys - at:o}' if phys != at else '')


def label(words, point):
    """'LABEL+n' for a 'BB,OOOO' point."""
    bank, offset = point.split(',')
    key = (int(bank, 8), int(offset, 8))
    _, lab = words.get(key, ('?', None))
    if not lab and erasable_name(words, key):
        return erasable_name(words, key)
    return f'{lab[0]}+{key[1] - lab[1]:o}' if lab and key[1] != lab[1] else (lab[0] if lab else '?')


def returned_from(words, qat):
    """(bank, address) of the routine a TC Q returns from, the target of the
    caller's TC at Q - 1 (qat: Q's fixed point, 'BB,AAAA'), or None when
    that word is not a TC or calls switchable code from fixed-fixed (its
    bank, FB at the call, is not known here)."""
    bank, at = (int(x, 8) for x in qat.split(','))
    word, before = words.values_at.get((bank, at - 1)), words.values_at.get((bank, at - 2))
    if word is None or word >> 12 or word & 0o7777 < 0o2000 or before == 0o6:
        return None  # not a TC (after an EXTEND, opcode 0 is an I/O order)
    target = word & 0o7777
    if target < 0o4000 and bank in (2, 3):
        return None
    return (bank, target) if target < 0o4000 else (2 if target < 0o6000 else 3, target % 0o2000 + 0o2000)


def describe(h, words, owners, m):
    """A HOLE match as a dict naming the file, label and owning lane. A TC Q
    return bridged (the point is erasable Q) is named after the routine it
    returns from, the one its caller's TC at Q - 1 calls: 'RDRUSECK TC Q'."""
    kind, bank, offset, arrivals, instructions, refused = h.group(1, 2, 3, 'arrivals', 'instructions', 'refused')
    key = (int(bank, 8), int(offset, 8))
    name, label = words.get(key, ('?', None))
    where = f'{label[0]}+{key[1] - label[1]:o}' if label and key[1] != label[1] else (label[0] if label else '?')
    if not label and erasable_name(words, key):
        where, name = erasable_name(words, key), 'erasable'
        if where == 'Q' and h.group('qat') and hasattr(words, 'values_at'):
            routine = returned_from(words, h.group('qat'))
            if routine and words.get(routine, (None, None))[1]:
                name, lab = words[routine]
                where = f'{lab[0]} TC Q' if lab[1] == routine[1] else f'{lab[0]}+{routine[1] - lab[1]:o} TC Q'
    return dict(kind=kind.strip(), point=f'{bank},{offset}', label=where, file=name,
                lane=owners.get(f'{MISSION[m]}/{name}', '?'), arrivals=int(arrivals),
                instructions=int(instructions), refused_by=refused, core_start=(m, f'{bank},{offset}') in CORE_START)


def main():
    m = sys.argv[1]
    words, owners = listing(MISSION[m]), lanes()
    for text in sys.stdin:
        h = HOLE.match(text)
        if h:
            d = describe(h, words, owners, m)
            print(f"{d['kind']:6} {d['point']} {d['label']:14} {d['file']:40} lane {d['lane']:3} {d['arrivals']:>5} arrivals "
                  f"{d['instructions']:>7} instructions" + (f"  refused by {d['refused_by']}" if d['refused_by'] else '')
                  + ('  core-start artifact, not a port gap' if d['core_start'] else ''))


if __name__ == '__main__':
    main()
