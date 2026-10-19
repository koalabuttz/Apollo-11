#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The declared REU layout of a flight run, and a check that a run wrote
nothing outside it (docs/FLIGHT-STORE.md, REU layout).

Every region's bounds come from the source that owns it, so a moved area
moves here too:

  store         0 .. image_bytes             read only (flight_store_build.py)
  replay rows   flight_scenario_vice.py --replay places them; read only
  replay states after the rows; the C64 writes them
  pinball save  LANEC_PIN_REU_SAVE, 4 KB     src/lanec_pin_trampoline.c
  rope          rope_reu_base, 2 B a word    include/rope_image.h
  undo          FLIGHT_UNDO_REU, _BYTES       include/flight_undo.h
  scratch       the last SCRATCH_BYTES       dap_pass_vice.py records

layout() fails when two declared regions overlap. writes_outside() compares
the REU image a run started from with the one VICE wrote back
(-reuimagerw) and lists every changed byte range that is not in a writable
region. A write of the bytes already there is not visible this way; the
verification build's DMA check (APOLLO_REU_BOUNDS) covers that case.

  reu_layout.py lm|cm            print the layout of the built store
  reu_layout.py lm|cm BEFORE AFTER   check one run's images
"""
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/flight-store'
REU_BYTES = 2 * 1024 * 1024
SCRATCH_BYTES = 32768        # tools/dap_pass_vice.py SCRATCH_BYTES
PIN_SAVE_BYTES = 0x1000      # sizeof(apollo_overlay_state), as the undo area


def _define(path, name):
    text = (ROOT / path).read_text()
    return int(re.search(rf'#define {name}\s+(0x[0-9A-Fa-f]+)ul', text).group(1), 16)


def pin_save():
    """Lane C's Pinball trampoline save: the state each claiming module
    starts from (src/lanec_pin_trampoline.c LANEC_PIN_REU_SAVE)."""
    at = _define('src/lanec_pin_trampoline.c', 'LANEC_PIN_REU_SAVE')
    return at, at + PIN_SAVE_BYTES


def undo_area():
    at = _define('include/flight_undo.h', 'FLIGHT_UNDO_REU')
    return at, at + _define('include/flight_undo.h', 'FLIGHT_UNDO_BYTES')


def rope_words(m):
    """Physical words in the mission's rope image (2 bytes each)."""
    program = {'lm': 'Luminary099', 'cm': 'Comanche055'}[m]
    return (ROOT / f'build/fixed-data/{program}/MAIN.agc.bin').stat().st_size // 2


def layout(m, manifest=None, replay=None, extra=()):
    """[(name, start, end, writable)], sorted by start. replay: (rows_at,
    states_at, states_end) for a --replay run. extra: more regions of one
    run, such as the check disk's boot stash below the replay rows
    (tools/flight_disk.py, Lane A)."""
    if manifest is None:
        manifest = json.loads((OUT / f'{m}-manifest.json').read_text())
    rope = int(manifest['rope_reu_base'], 0)
    regions = [('store', 0, manifest['image_bytes'], False),
               ('pinball save', *pin_save(), True),
               ('rope', rope, rope + 2 * rope_words(m), False),
               ('undo', *undo_area(), True),
               ('scratch', REU_BYTES - SCRATCH_BYTES, REU_BYTES, True)]
    if replay:
        rows_at, states_at, states_end = replay
        regions += [('replay rows', rows_at, states_at, False), ('replay states', states_at, states_end, True)]
    regions += list(extra)
    regions.sort(key=lambda r: r[1])
    for a, b in zip(regions, regions[1:]):
        if a[2] > b[1]:
            raise SystemExit(f'FAIL: {m} REU layout: {a[0]} ({a[1]:#x}-{a[2]:#x}) overlaps '
                             f'{b[0]} ({b[1]:#x}-{b[2]:#x})')
    return regions


def free_top(m, manifest=None):
    """The top of the zeros between the store and the next region above it:
    where flight_scenario_vice.py --replay may put its rows."""
    regions = layout(m, manifest)
    store_end = next(r[2] for r in regions if r[0] == 'store')
    return min(r[1] for r in regions if r[1] >= store_end and r[0] != 'store')


def changed(before, after, grain=256):
    """[(start, end)] of the bytes that differ, merged."""
    out = []
    for i in range(0, max(len(before), len(after)), grain):
        a, b = before[i:i + grain], after[i:i + grain]
        if a == b:
            continue
        lo = next(k for k in range(len(b)) if k >= len(a) or a[k] != b[k])
        hi = next(k for k in range(len(b), 0, -1) if k > len(a) or a[k - 1] != b[k - 1])
        if out and out[-1][1] == i + lo:
            out[-1] = (out[-1][0], i + hi)
        else:
            out.append((i + lo, i + hi))
    return out


def writes_outside(regions, before, after):
    """(outside, touched): the changed ranges no writable region holds, and
    the writable regions that changed."""
    outside, touched = [], set()
    for lo, hi in changed(before, after):
        at = lo
        while at < hi:
            r = next((r for r in regions if r[1] <= at < r[2]), None)
            end = min(hi, r[2]) if r else min([hi] + [x[1] for x in regions if x[1] > at])
            if r and r[3]:
                touched.add(r[0])
            else:
                outside.append((at, end, r[0] if r else 'undeclared'))
            at = end
    return outside, sorted(touched)


# Every DMA's REU range, from the controller's registers as the command is
# written ($DF01; a $FF00-triggered DMA arms its command there too): first
# byte S and last byte E (a length of 0, 65,536 bytes, reads as S - 1).
# VICE 3.7's monitor gives * no precedence over + (a * b + c * d is
# ((a * b) + c) * d), so every operation is in parentheses.
_S = '(((@io:$df06 * $10000) + (@io:$df05 * $100)) + @io:$df04)'
_E = f'((({_S} + (@io:$df08 * $100)) + @io:$df07) - 1)'


def dma_trace(regions):
    """A VICE monitor checkpoint that logs, without stopping, every DMA
    whose range is not inside one declared region: read or write, whether
    or not it changes a byte. VICE evaluates the condition itself, so a
    run in bounds costs nothing; tools/dap_pass_vice.py vice(monitor=)
    adds it after the $C000 watch, so it is checkpoint 2."""
    spans = []
    for _, lo, hi, _ in sorted(regions, key=lambda r: r[1]):
        if spans and spans[-1][1] == lo:
            spans[-1][1] = hi        # adjacent regions: one span (a read may cross)
        else:
            spans.append([lo, hi])
    terms = [f'(({_S} < ${lo:x}) || ({_E} >= ${hi:x}))' for lo, hi in spans]
    outside = terms[0]
    for t in terms[1:]:
        outside = f'({outside} && {t})'
    # On each hit VICE also prints the controller's registers (io $df00),
    # so the log names the DMA: dma_hits() reads them back.
    return f'trace store df01 if {outside}\ncommand 2 "io $df00"'


def dma_misses(monitor_log, checkpoint=2):
    """DMAs the trace logged (tools/dap_pass_vice.py's monitor.log)."""
    return len(re.findall(rf'^#{checkpoint} \(Trace store df01\)', monitor_log, re.M))


def dma_hits(monitor_log, checkpoint=2):
    """[dict(pc, command, c64, start, end)] of each DMA the trace logged,
    from the registers VICE printed after the hit."""
    out = []
    for m in re.finditer(rf'^#{checkpoint} \(Trace store df01\).*\n\.C:([0-9a-f]{{4}}).*\n'
                         r'(?:Executing:.*\n)?RAM Expansion Module:\n>C:df00((?:\s+[0-9a-f]{2}){11})',
                         monitor_log, re.M):
        b = [int(x, 16) for x in m.group(2).split()]
        start = b[4] | b[5] << 8 | b[6] << 16
        length = (b[7] | b[8] << 8) or 0x10000
        out.append(dict(pc=f'${m.group(1)}', command=f'${b[1]:02x}', c64=f'${b[2] | b[3] << 8:04x}',
                        start=hex(start), end=hex(start + length)))
    return out


def report(m, regions, before, after, label=''):
    """Prints the result; returns a dict for the run's JSON."""
    outside, touched = writes_outside(regions, before, after)
    tag = f'{label}: ' if label else ''
    if outside:
        print(f'FAIL: {tag}REU writes outside the {m} layout:')
        for lo, hi, name in outside[:16]:
            print(f'  {lo:#08x}-{hi:#08x} ({hi - lo} bytes) in {name}')
    else:
        print(f"{tag}REU writes inside the {m} layout (changed: {', '.join(touched) or 'nothing'})")
    return dict(ok=not outside, touched=touched,
                outside=[dict(start=hex(lo), end=hex(hi), region=n) for lo, hi, n in outside])


def main(argv):
    m = argv[1]
    regions = layout(m)
    if len(argv) == 2:
        for name, lo, hi, w in regions:
            print(f"{lo:#08x}-{hi:#08x} {hi - lo:8} {'rw' if w else 'ro'} {name}")
        return 0
    before, after = (pathlib.Path(p).read_bytes() for p in argv[2:4])
    return 0 if report(m, regions, before, after)['ok'] else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv))
