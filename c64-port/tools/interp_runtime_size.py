#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""C64 size of the runtime interpreter: each mission's interpreter linked as
one ABI 2 job-slot module with the flight store's flags
(tools/flight_store_build.py link_module: llvm-mos -Oz, LTO, gc-sections),
its basic table registered, the rope read from the REU
(platform/c64/rope_image_reu.c), as module $0A01 (lane I,
docs/OVERLAY_ABI_V2.md). Prints the module's bytes against the
16 KB slot and writes build/interp-runtime/size.json.

  interp_runtime_size.py [--cc MOS_CC]"""
import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import flight_store_build as F

ROOT = F.ROOT
OUT = Path('build/interp-runtime')
COMMON_SRC = ['src/lm_interp_orders.c', 'src/lm_interp_steps.c', 'src/agc_image_ops.c', 'src/agc_word.c',
              'src/agc_multiword.c', 'src/agc_hardware_divide.c', 'src/agc_transfer.c', 'platform/c64/rope_image_reu.c']
SOURCES = {'lm': ['src/lm_interp_runtime.c'] + COMMON_SRC,
           'cm': ['src/cm_interp_runtime.c', 'src/cm_interp_orders.c', 'src/cm_interp_steps.c'] + COMMON_SRC[2:]}


def main():
    ap = argparse.ArgumentParser()
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
        r = F.link_module(a.cc, m, prg, 0, 0x0A01, SOURCES[m], ['%s_interp_runtime_basic' % m], [], library)
        out[m] = r
        if r['ok']:
            print('%s: %d bytes of %d (job slot)' % (m.upper(), r['bytes'], F.SLOT_BYTES[0]))
        else:
            print('%s: does not fit the job slot (over by %s bytes) %s' % (m.upper(), r.get('overflow'), r.get('error') or ''))
    (ROOT / OUT / 'size.json').write_text(json.dumps(out, indent=1) + '\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
