#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/reu_layout.py: the declared runtime REU regions do not overlap,
and a run's changed bytes are sorted into writable regions or reported
as outside, on made-up images (no build or VICE needed)."""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import reu_layout as rl  # noqa: E402

# The fixed regions from their sources are in order and apart.
pin, undo = rl.pin_save(), rl.undo_area()
assert pin[1] <= 0x1e0000 <= undo[0] and undo[1] <= rl.REU_BYTES - rl.SCRATCH_BYTES, (pin, undo)

regions = [('store', 0, 0x100, False), ('save', 0x200, 0x300, True), ('rope', 0x300, 0x400, False)]
before = bytes(0x400)

# Nothing changed.
assert rl.writes_outside(regions, before, before) == ([], [])
# A write inside the writable region.
after = bytearray(before)
after[0x210:0x220] = b'\1' * 16
assert rl.writes_outside(regions, before, bytes(after)) == ([], ['save'])
# One run of changed bytes across save into rope: the rope part is outside.
after[0x2f0:0x310] = b'\2' * 32
out, touched = rl.writes_outside(regions, before, bytes(after))
assert out == [(0x300, 0x310, 'rope')] and touched == ['save'], out
# A write between regions is undeclared.
after = bytearray(before)
after[0x180] = 9
assert rl.writes_outside(regions, before, bytes(after))[0] == [(0x180, 0x181, 'undeclared')]
# changed() merges runs over grain boundaries.
after = bytearray(before)
after[0xf0:0x110] = b'\3' * 32
assert rl.changed(before, bytes(after)) == [(0xf0, 0x110)]

# An overlap between declared regions fails the layout.
manifest = {'rope_reu_base': '0x1e0000', 'image_bytes': 0x1d0000}
rl.rope_words = lambda m: 36 * 1024
try:
    rl.layout('lm', manifest, (0x1de000, 0x1de800, 0x1df800))
except SystemExit as e:
    assert 'overlaps' in str(e), e
else:
    raise AssertionError('replay states over the Pinball save not refused')
assert rl.free_top('lm', manifest) == pin[0]
print('reu layout: ok')

# The DMA trace: one span per run of adjacent regions, every operation in
# parentheses (VICE's monitor gives * no precedence over +).
t = rl.dma_trace([('a', 0, 0x100, False), ('b', 0x100, 0x200, True), ('c', 0x300, 0x400, True)])
assert t.startswith('trace store df01 if ') and t.count('(') == t.count(')'), t
assert t.count('< $') == 3 and '>= $200)' in t and '>= $400)' in t, t
assert rl.dma_misses('#1 (Stop on store c000)\n#2 (Trace store df01)    4/$004\n#2 (Trace store df01)  8/$08\n') == 2
log = ('#2 (Trace store df01)  259/$103,  12/$0c\n'
       '.C:4358  8E 01 DF    STX $DF01      - A:1F X:90 Y:70 SP:d1 N.-..I.C  266591001\n'
       'Executing: io $df00\n'
       'RAM Expansion Module:\n'
       '>C:df00  10 90 00 70  00 30 1f 00  01 1f 3f ff  ff ff ff ff   ................\n')
assert rl.dma_hits(log) == [dict(pc='$4358', command='$90', c64='$7000', start='0x1f3000', end='0x1f3100')], rl.dma_hits(log)
assert rl.dma_misses(log) == 1
assert '\ncommand 2 "io $df00"\n' in t and t.endswith('trace store df01 if (@io:$df01 < $80)'), t
assert rl.dma_misses('#3 (Trace store df01)  1/$01\n', 3) == 1
print('reu dma trace: ok')
