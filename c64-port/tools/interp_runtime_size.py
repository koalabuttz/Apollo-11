#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""C64 size of the runtime interpreter: each mission's interpreter linked as
one ABI 2 job-slot module with the flight store's flags
(llvm-mos -Oz, LTO, gc-sections), the rope read from the REU
(platform/c64/rope_image_reu.c), as module $0A01 (lane I,
docs/OVERLAY_ABI_V2.md). The compact interpreter links with its own module
entry (platform/c64/interp_module.c); --runtime links the verification
interpreter in a table module (tools/flight_store_build.py link_module). Prints the module's bytes against the
16 KB slot and writes build/interp-runtime/size.json.

  interp_runtime_size.py [--cc MOS_CC]"""
import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import re

import flight_store_build as F
from resident_flight_build import COMMON, MISSION, SERVICES

ROOT = F.ROOT
OUT = Path('build/interp-runtime')
COMMON_SRC = ['src/lm_interp_orders.c', 'src/lm_interp_steps.c', 'src/agc_image_ops.c', 'src/agc_word.c',
              'src/agc_multiword.c', 'src/agc_hardware_divide.c', 'src/agc_transfer.c', 'platform/c64/rope_image_reu.c']
SOURCES = {'lm': ['src/lm_interp_runtime.c'] + COMMON_SRC,
           'cm': ['src/cm_interp_runtime.c', 'src/cm_interp_orders.c', 'src/cm_interp_steps.c'] + COMMON_SRC[2:]}
# The compact interpreter, the flight build's (one unit; the image at $7000).
CORE_SRC = ['platform/c64/interp_module.c', 'src/agc_word.c', 'src/agc_multiword.c', 'src/agc_hardware_divide.c',
            'platform/c64/rope_image_reu.c']
CORE = {'lm': ['src/lm_interp_core.c'] + CORE_SRC, 'cm': ['src/cm_interp_core.c'] + CORE_SRC}


def link_core(cc, m, prg, sources, library):
    """The compact interpreter as job-slot module $0A01 (link_module's flags
    without its table wrapper)."""
    cmd = [cc, *COMMON, *MISSION[m], '-DAGC_FIXED_IMAGE=1', '-mllvm', '-inline-threshold=-50',
           '-ffunction-sections', '-fdata-sections', '-nostdlib', '-Lplatform/c64', f'-L{F.OUT}',
           '-T', 'platform/c64/overlay_v2_slot_job.ld', '-T', 'platform/c64/overlay_v2_payload.ld',
           f'-Wl,--gc-sections,--defsym=__ov2_slot=0,--defsym=__ov2_mission={1 if m == "lm" else 2},'
           f'--defsym=__ov2_id={0x0A01},--defsym=__ov2_services={SERVICES},--defsym=__ov2_selectors=1,'
           f'-Map,{prg}.map', *sources, 'platform/c64/overlay_v2_entry.S', '-lc', library, '-lcrt', '-o', prg]
    r = subprocess.run([str(c) for c in cmd], cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        over = [int(v) for v in re.findall(r"overflowed by (\d+) bytes", r.stderr)]
        return dict(ok=False, overflow=max(over) if over else None,
                    error=None if over else [l for l in r.stderr.splitlines() if 'error' in l][:4])
    return dict(ok=True, bytes=(ROOT / prg).stat().st_size - 2)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--runtime', action='store_true', help='the verification interpreter (lmi_ bodies) instead')
    ap.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    a = ap.parse_args()
    (ROOT / OUT).mkdir(parents=True, exist_ok=True)
    (ROOT / F.OUT).mkdir(parents=True, exist_ok=True)
    subprocess.run([sys.executable, 'tools/overlay_v2_services.py', '--out-dir', str(F.OUT)], cwd=ROOT, check=True)
    library = F.prepare(a.cc, ROOT / F.OUT)
    out = {}
    for m in ('lm', 'cm'):
        (ROOT / OUT / m).mkdir(exist_ok=True)
        prg = OUT / m / 'interp.prg'
        if a.runtime:
            r = F.link_module(a.cc, m, prg, 0, 0x0A01, SOURCES[m], ['%s_interp_runtime_basic' % m], [], library)
        else:
            r = link_core(a.cc, m, prg, CORE[m], library)
        out[m] = r
        if r['ok']:
            print('%s: %d bytes of %d (job slot)' % (m.upper(), r['bytes'], F.SLOT_BYTES[0]))
        else:
            print('%s: does not fit the job slot (over by %s bytes) %s' % (m.upper(), r.get('overflow'), r.get('error') or ''))
    (ROOT / OUT / ('size-runtime.json' if a.runtime else 'size.json')).write_text(json.dumps(out, indent=1) + '\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
