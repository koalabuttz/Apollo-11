#!/usr/bin/env python3
"""Verify actual bounded raster IRQ delivery and retain PAL timing samples."""
import argparse
import json
import os
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--vice', default='x64sc')
p.add_argument('--rom-dir', required=True, type=pathlib.Path)
p.add_argument('--data-dir', type=pathlib.Path)
a = p.parse_args()
rom = a.rom_dir.resolve()
build = ROOT / 'build'
prg = build / 'irq.prg'
entry = re.search(rb'\x9e\s*(\d+)\x00', prg.read_bytes()[:32])
if entry is None:
    sys.exit('FAIL: PRG has no BASIC SYS header')
mapping = (build / 'irq.map').read_text()
for section in ('zp.data', 'zp.bss', 'zp.noinit', 'zp'):
    allocation = re.search(r'^\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+1\s+\.' + re.escape(section) + r'$', mapping, re.M)
    if allocation is not None and int(allocation[1], 16) != 0:
        sys.exit('FAIL: IRQ build has unpreserved compiler zero-page allocation: ' + section)
stack_match = re.search(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+100\s+1\s+c64_irq_stack$', mapping, re.M)
if stack_match is None:
    sys.exit('FAIL: missing isolated IRQ stack in map')
irq_stack = int(stack_match[1], 16)
mailbox = build / 'irq-mailbox.bin'
monitor_log = build / 'irq-monitor.log'
mailbox.unlink(missing_ok=True)
monitor_log.unlink(missing_ok=True)
commands = build / 'irq.mon'
events = 179  # Nine IRQ pairs, callback/mask markers, and 74 benchmark pairs.
script = ('fill 0100 01f1 cd\nfill c100 cfff cd\n'
          f'load "{prg}" 0\nwatch store c030\nwatch store c020\n'
          f'ignore 2 1\nstopwatch reset\ng {int(entry[1]):04x}\n')
script += 'm c030 c032\nstopwatch\n' + 'g\nm c030 c032\nstopwatch\n' * (events - 1)
script += (f'delete 1\ng\nm c020 c026\nbsave "{mailbox}" 0 c020 c026\n'
           f'bsave "{build}/irq-hardware-stack.bin" 0 0100 01f1\n'
           f'bsave "{build}/irq-software-stack.bin" 0 c100 cfff\n'
           f'bsave "{build}/irq-isolated-stack.bin" 0 {irq_stack:04x} {irq_stack+255:04x}\n'
           'screen\nquit\n')
commands.write_text(script)
cmd = [a.vice, '-default', '-console', '-pal', '-warp', '+sound',
       '-kernal', str(rom / 'kernal-901227-03.bin'),
       '-basic', str(rom / 'basic-901226-01.bin'),
       '-chargen', str(rom / 'chargen-901225-01.bin'),
       '-directory', (str(a.data_dir.resolve()) + ':' if a.data_dir else '') + str(rom.parent),
       '-initbreak', 'ready', '-moncommands', str(commands),
       '-monlogname', str(monitor_log), '-monlog', '-limitcycles', '400000000']
env = os.environ.copy()
for key, directory in [('XDG_CACHE_HOME', 'vice-cache'), ('XDG_CONFIG_HOME', 'vice-config')]:
    env.setdefault(key, str(build / directory))
    pathlib.Path(env[key]).mkdir(parents=True, exist_ok=True)
with (build / 'irq-vice.log').open('w') as log:
    subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, check=True, timeout=60, env=env)
if not mailbox.exists():
    sys.exit('FAIL: VICE did not produce the IRQ mailbox; inspect build/irq-vice.log')
data = mailbox.read_bytes()
if data != bytes([0xa5, 0, 0, 8, 9, 74, 32]):
    sys.exit('FAIL: IRQ mailbox ' + data.hex())
log = monitor_log.read_text()
markers = re.findall(r'>C:c030\s+([0-9a-f]{2})\s+([0-9a-f]{2})\s+([0-9a-f]{2})', log, re.I)
times = [int(v) for v in re.findall(r'Stopwatch:\s+(\d+)', log)]
if len(markers) != events or len(times) != events:
    sys.exit(f'FAIL: expected {events} events; got {len(markers)} markers / {len(times)} times')
records = [dict(marker=int(m, 16), group=int(g, 16), case=int(c, 16), cycle=t)
           for (m, g, c), t in zip(markers, times)]
expected = [0x81, 0x82, 0x90, 0x81, 0x82, 0x94, 0x95, 0x96, 0x97]
expected += [0x90, 0x81, 0x82, 0x94, 0x95, 0x96, 0x97]
expected += [0x81, 0x82] * 5 + [0x91, 0x92, 0x81, 0x82, 0x93] + [0x10, 0x20] * 74
if [r['marker'] for r in records] != expected:
    sys.exit('FAIL: IRQ / callback / exclusion marker order')
names = ['admission/endjob scan', 'basic exchange', 'completion/dispatch',
         'idle startup', 'four-request safe point', 'exclusion enter/leave']
samples = []
irq_samples = []
for first, second in zip(records, records[1:]):
    cycles = second['cycle'] - first['cycle']
    if first['marker'] == 0x10:
        if second['marker'] != 0x20 or first['group'] != second['group']:
            sys.exit('FAIL: unmatched timing pair')
        samples.append(dict(group=names[first['group']], case=first['case'], cycles=cycles))
    if first['marker'] == 0x81:
        irq_samples.append(dict(group=first['group'], cycles=cycles))
def elapsed(start, end, group=None, case=None):
    selected = [r for r in records if (group is None or r['group'] == group) and
                (case is None or r['case'] == case)]
    return next(r['cycle'] for r in selected if r['marker'] == end) - next(r['cycle'] for r in selected if r['marker'] == start)
stacks = {}
for kind, top in [('hardware', 0x1f2), ('software', 0xd000), ('isolated', irq_stack+256)]:
    values = (build / f'irq-{kind}-stack.bin').read_bytes()
    touched = [i for i, b in enumerate(values) if b != 0xcd]
    extent = len(values) - min(touched) if touched else 0
    if touched and min(touched) == 0:
        sys.exit(f'FAIL: {kind} stack consumed the entire instrumented range')
    stacks[kind] = dict(top=top, sentinel_extent=extent, instrumented=len(values))
result = dict(mailbox=data.hex(), command=cmd, samples=samples, irq_samples=irq_samples,
              observed_latency=dict(irq_marker_to_poll_return=elapsed(0x81, 0x95, 7, 0),
                                    irq_return_marker_to_poll_return=elapsed(0x82, 0x95, 7, 0),
                                    callback_start_to_poll_return=elapsed(0x90, 0x95, 7, 0),
                                    poll_interval=elapsed(0x94, 0x95, 7, 0),
                                    poll_return_to_high_callback=elapsed(0x95, 0x97, 7, 0),
                                    masked_work=elapsed(0x91, 0x92),
                                    restore_marker_to_delivery_return=elapsed(0x92, 0x93)),
              stacks=stacks, records=records)
(build / 'irq-timing.json').write_text(json.dumps(result, indent=2) + '\n')
print('PASS: PAL C64 register/decimal canary, interrupted C arithmetic, eight reference checkpoints, real IRQ overflow and deferred masked delivery')
print('Mailbox:', data.hex())
for name in names:
    group = [s for s in samples if s['group'] == name]
    largest = max(group, key=lambda s: s['cycles'])
    print(f"{name}: {len(group)} samples, maximum observed {largest['cycles']} cycles (case {largest['case']})")
print('IRQ marker intervals:', [s['cycles'] for s in irq_samples])
print('Observed latency:', result['observed_latency'])
print('Stack sentinel extents:', {k: v['sentinel_extent'] for k, v in stacks.items()})
