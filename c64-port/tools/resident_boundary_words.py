#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Generate the flight resident's boundary-word tables.

The interrupt entry (src/resident_rupt.c) stores the instruction word at Z in
BRUPT and refuses to vector at RELINT, INHINT or EXTEND, so it needs the word
at every fixed point where the resident can stop between two routines. Those
points are the keys of the routine tables (the resident's, and the interrupt
bodies its task-slot wrappers run), the store's resolver records, and the
interpreter's fetch point and DANZIG (where Z stands at an interpretive
order, whose key is LOC's). This
tool collects them (tools/resident_boundary_keys.c, built with the host cc),
takes each word from the unmodified rope
(evidence/fixed-data-<m>-original.rope.bin), checks it against the assembly
listing when build/fixed-data/<program>/assembly.lst exists, and writes

  platform/c64/resident_boundary_words_<m>.h   sorted {physical key, 15-bit word}

whose entries end the REU store image (tail_bytes, overlay_v2_pack.py --tail:
the entries, little-endian, then their count and a zero word). The resident
binary-searches them there; a key that is not in it refuses the interrupt,
which stays pending.

  resident_boundary_words.py [--cc CC] [--resolver lm|cm=JSON ...] [--check]

--resolver adds a store's resolver records (overlay_v2_pack.py's JSON list)
to a mission's keys; the default is the flight check's probe records
(tools/resident_flight_build.py PROBES). --check regenerates and fails if a
committed header differs.
"""
import argparse
import textwrap
import json
from pathlib import Path
import re
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from resident_flight_build import PROBES  # noqa: E402
PROGRAM = {'lm': 'Luminary099', 'cm': 'Comanche055'}
# Without RK_DEVICE_CH13 (tools/resident_flight_build.py MISSION): the CM
# timers here use rk_hardware.channel13; this build runs no timer check.
FLAGS = {'lm': ['-DRK_LM=1', '-DP66_NATIVE_DISPATCH_ONLY'], 'cm': ['-DRK_CM=1']}
EXTRA = {
    'lm': ['src/lm_kernel.c', 'src/sp_routines.c', 'src/lm_single_trig.c', 'src/lm_t6rupt.c'],
    'cm': ['src/cm_kernel.c', 'src/sp_routines.c', 'src/cm_quictrig.c', 'src/cm_single_return.c'],
}
# Where Z stands when the resident resolves an interpretive order: the
# interpreter's fetch point (NEWOPS, include/standard_lead_routines.h) and
# DANZIG with a right-hand order pending (include/agc_transfer.h). An
# interrupt there takes the word at Z, not at the order's LOC key.
INTERPRETER_POINTS = {'lm': [2048 + 0o6070 - 0o4000, 3121], 'cm': [2048 + 0o6037 - 0o4000, 3096]}
COMMON = ['src/agc_image_ops.c', 'src/agc_hardware_divide.c', 'src/agc_word.c', 'src/agc_multiword.c',
          'src/agc_transfer.c', 'src/rupt_lead_ins.c', 'src/t4rupt_core.c', 'src/keyrupt_uprupt.c']


def kernel_sources(m):
    """The kernel's routine sources: the src/ files of RK_<M>_CASES (tools/mk/resident-kernel.mk)."""
    text = (ROOT / 'tools/mk/resident-kernel.mk').read_text()
    cases = re.search(rf'^RK_{m.upper()}_CASES = (.*)$', text, re.M).group(1).split()
    return [s for s in cases if s.startswith('src/')]


def table_keys(cc, m, out_dir):
    sources = sorted(set(kernel_sources(m) + EXTRA[m] + COMMON))
    exe = out_dir / f'resident-boundary-keys-{m}'
    subprocess.run([cc, '-std=c99', '-O1', '-Iinclude', '-Itests', *FLAGS[m],
                    '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections', *sources,
                    'tools/resident_boundary_keys.c', '-o', str(exe)], cwd=ROOT, check=True)
    lines = subprocess.run([str(exe)], cwd=ROOT, check=True, capture_output=True, text=True).stdout.split('\n')
    keys = {}
    for line in filter(None, lines):
        table, key = line.split()
        keys.setdefault(int(key), table)
    return keys


def fixed_physical(loc, bank):
    """agc_fixed_physical for a resolver record's (bank, loc)."""
    loc &= 0o7777
    if loc < 0o2000:
        return None
    if loc >= 0o4000:
        return (3 if loc >= 0o6000 else 2) * 1024 + (loc & 0o1777)
    b = (bank & 0o76000) >> 10
    if b >= 0o30 and bank & 0o100:
        b += 8
    return b * 1024 + (loc & 0o1777)


def rope_word(rope, physical):
    bank, offset = divmod(physical, 1024)
    position = 2 * ((bank ^ 2 if bank < 4 else bank) * 1024 + offset)
    return int.from_bytes(rope[position:position + 2], 'big') >> 1


def listing_words(path):
    """Physical fixed address -> assembled word, from a yaYUL listing."""
    words = {}
    for text in path.read_text().splitlines():
        if not re.match(r'^\d{6},\d{6}: ', text):
            continue
        address = text[14:23].strip()
        m = re.fullmatch(r'([0-7]{2}),([0-7]{4})', address)
        if m:
            base = int(m[1], 8) * 1024 + int(m[2], 8) - 0o2000
        elif re.fullmatch('[0-7]{4}', address) and 0o4000 <= int(address, 8) < 0o10000:
            base = int(address, 8)
        else:
            continue
        for i, w in enumerate(w for w in text[33:44].split() if re.fullmatch('[0-7]{5}', w)):
            words[base + i] = int(w, 8)
    return words


def render(m, rows, sources):
    note = (f'Generated by tools/resident_boundary_words.py: the {PROGRAM[m]} rope word at every fixed point '
            f'where the flight resident can stop between routines ({", ".join(sources)}), sorted by physical '
            'key. Do not edit.')
    lines = textwrap.wrap(note, 74)
    out = ['/* SPDX-License-Identifier: MIT */', '/* ' + lines[0]] + [' * ' + l for l in lines[1:]]
    out[-1] += ' */'
    out += [f'#define RK_BOUNDARY_WORDS {len(rows)}u',
            f'static const agc_site_operand rk_boundary_words[RK_BOUNDARY_WORDS]={{']
    for i in range(0, len(rows), 6):
        out.append(' ' + ''.join(f'{{{k},0{w:05o}}},' for k, _, w in rows[i:i + 6]))
    out.append('};')
    return '\n'.join(out) + '\n'


def tail_bytes(text):
    """The REU store's tail (overlay_v2_pack.py --tail) for a boundary-word
    header: each {physical key, word} as two little-endian 16-bit words
    (agc_site_operand), then the count and a zero word, which the resident
    reads from the image's last four bytes (platform/c64/resident_flight.c)."""
    rows = re.findall(r'\{(\d+),0([0-7]+)\},', text)
    return b''.join(struct.pack('<HH', int(k), int(w, 8)) for k, w in rows) + struct.pack('<HH', len(rows), 0)


def header_text(cc, m, resolver):
    """The mission's boundary-word header for these resolver records, or None
    when a word disagrees with the assembly listing."""
    out_dir = ROOT / 'build'
    out_dir.mkdir(exist_ok=True)
    keys = table_keys(cc, m, out_dir)
    for k in INTERPRETER_POINTS[m]:
        keys.setdefault(k, 'interpreter points')
    for r in resolver:
        k = fixed_physical(r['loc'], r['bank'])
        if k is not None:
            keys.setdefault(k, 'resolver')
    rope = (ROOT / f'evidence/fixed-data-{m}-original.rope.bin').read_bytes()
    rows = [(k, keys[k], rope_word(rope, k)) for k in sorted(keys)]
    listing = ROOT / f'build/fixed-data/{PROGRAM[m]}/assembly.lst'
    if listing.exists():
        words = listing_words(listing)
        bad = [(k, t, w, words.get(k)) for k, t, w in rows if words.get(k) != w]
        if bad:
            for k, t, w, lw in bad:
                print(f'{m}: {t} key {k} ({k // 1024:02o},{k % 1024 + 0o2000:04o}): rope {w:05o}, listing {lw}',
                      file=sys.stderr)
            return None
        print(f'{m}: {len(rows)} boundary words match the rope and {listing.relative_to(ROOT)}')
    else:
        print(f'{m}: SKIP listing check (no {listing.relative_to(ROOT)}); {len(rows)} words from the rope')
    tables = sorted({t for _, t, _ in rows}, key=lambda t: (t == 'resolver', t))
    return render(m, rows, [t if t != 'resolver' else 'resolver records' for t in tables])


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default='cc')
    p.add_argument('--resolver', action='append', default=[], help='lm=PATH or cm=PATH')
    p.add_argument('--check', action='store_true')
    a = p.parse_args()
    resolvers = {m: PROBES for m in PROGRAM}
    for spec in a.resolver:
        m, path = spec.split('=', 1)
        resolvers[m] = json.loads(Path(path).read_text())
    failed = False
    for m in PROGRAM:
        text = header_text(a.cc, m, resolvers[m])
        if text is None:
            return 1
        header = ROOT / f'platform/c64/resident_boundary_words_{m}.h'
        if a.check:
            if not header.exists() or header.read_text() != text:
                print(f'{header.relative_to(ROOT)} is stale: rerun tools/resident_boundary_words.py', file=sys.stderr)
                failed = True
        else:
            header.write_text(text)
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
