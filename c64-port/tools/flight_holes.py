#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Name the source file and label of each unregistered point that
build/flight-store/flight-host-run-<m> reports (docs/FLIGHT-STORE.md), and
the lane that owns the file (docs/overlay-allocation.json).

  tools/flight_holes.py lm|cm < flight-host-run output
"""
import json, re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MISSION = {'lm': 'Luminary099', 'cm': 'Comanche055'}
HOLE = re.compile(r'^\s+(basic |interp) (\d\d),(\d{4}) .*: (\d+) arrivals, (\d+) instructions(?:, refused by (\S+))?', re.M)
# Points a scenario bridges only because its core snapshot starts mid-way
# through code, not port gaps: the NASSP LM cores (lm-p63-nassp, lm-p52)
# start inside the self-check idle loop with its state half set, so the
# host bridges the loop's first pass (1 arrival each). From a real start
# the loop runs native (milestone self-check). Steward's ruling for P63,
# /mnt/project-files/flight-scenarios/lm-p52-stops.md.
CORE_START = {('lm', '43,3303'), ('lm', '43,3332'), ('lm', '43,3334')}


def listing(mission):
    """{(bank, address): (file, label)} for every fixed word, with the label
    the nearest one at or before it in the same file."""
    words, name, label = {}, None, None
    for text in (ROOT / 'build/fixed-data' / mission / 'assembly.lst').read_text(errors='replace').splitlines():
        m = re.search(r'#\s*Filename:\s*(\S+\.agc)', text)
        if m:
            name, label = m.group(1), None
            continue
        code = text.split('\t#')[0]
        m = re.match(r'^\d+,\d+:\s+(?:(\d\d),)?([0-7]{4})\s+([0-7]{5})', code)
        if not m or name is None:
            continue
        bank, addr, _ = m.groups()
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
    return words


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


def label(words, point):
    """'LABEL+n' for a 'BB,OOOO' point."""
    bank, offset = point.split(',')
    key = (int(bank, 8), int(offset, 8))
    _, lab = words.get(key, ('?', None))
    return f'{lab[0]}+{key[1] - lab[1]:o}' if lab and key[1] != lab[1] else (lab[0] if lab else '?')


def describe(h, words, owners, m):
    """A HOLE match as a dict naming the file, label and owning lane."""
    kind, bank, offset, arrivals, instructions, refused = h.groups()
    key = (int(bank, 8), int(offset, 8))
    name, label = words.get(key, ('?', None))
    where = f'{label[0]}+{key[1] - label[1]:o}' if label and key[1] != label[1] else (label[0] if label else '?')
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
