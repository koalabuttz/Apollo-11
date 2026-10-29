#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Measure the resident kernel on the C64 (make resident-kernel-size).

Links platform/c64/resident_kernel_size.c, which keeps only one mission's
kernel routine tables, with LLVM-MOS at -Oz and -Os, LTO and gc-sections, on
the canonical image at $7000 (RK_FIXED_IMAGE), and prints the bytes the
kernel adds over an empty program's C runtime: in total, and without the
transfer-ABI operand reader (agc_transfer_operand, inlined as inline_word),
which every resident program links anyway.

  resident_kernel_size.py [--json]
"""
import json
import os
import pathlib
import re
import subprocess
import sys

R = pathlib.Path(__file__).resolve().parents[1]
CC = os.environ.get('MOS_CC', str(R / 'deps/llvm-mos/bin/mos-c64-clang'))
COMMON = ['src/agc_image_ops.c', 'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c']
SHARED = ('inline_word', 'agc_transfer_operand')


def link(tag, opt, defines, sources):
    out = R / 'build' / f'resident-kernel-size-{tag}-{opt}.prg'
    cmd = [CC, '-Iinclude', '-Itests', '-std=c99', f'-{opt}', '-flto', '-DRK_FIXED_IMAGE=1', *defines,
           '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections', '-fno-static-stack', '-mlto-zp=0',
           '-Wall', '-Wextra', '-Werror', f'-Wl,-Map,{out}.map', 'platform/c64/resident_kernel_size.c',
           *sources, '-o', str(out)]
    subprocess.run(cmd, cwd=R, check=True)
    sizes = {}
    for line in open(f'{out}.map'):
        m = re.match(r'\s*[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+(\S+)$', line)
        if m and ':' not in m.group(2):
            sizes.setdefault(m.group(2), int(m.group(1), 16))
    return out.stat().st_size - 2, sizes


def main(argv):
    (R / 'build').mkdir(exist_ok=True)
    report = {}
    for opt in ('Oz', 'Os'):
        base, _ = link('empty', opt, ['-DRK_EMPTY=1', '-DRK_LM=1'], [])
        # Without RK_DEVICE_CH13 (tools/resident_flight_build.py MISSION): the CM
        # timers here use rk_hardware.channel13; this build runs no timer check.
        for m, defines in (('lm', ['-DRK_LM=1', '-DP66_NATIVE_DISPATCH_ONLY']), ('cm', ['-DRK_CM=1'])):
            total, sizes = link(m, opt, defines, [f'src/{m}_kernel.c', *COMMON])
            shared = sum(sizes.get(s, 0) for s in SHARED)
            report[f'{m}-{opt}'] = {'prg_bytes': total, 'runtime_bytes': base, 'kernel_bytes': total - base,
                                    'kernel_bytes_without_operand_reader': total - base - shared}
    if '--json' in argv:
        print(json.dumps(report, indent=1, sort_keys=True))
    else:
        for k, v in sorted(report.items()):
            print(f"{k}: kernel {v['kernel_bytes']} bytes ({v['kernel_bytes_without_operand_reader']} without the "
                  f"operand reader) over a {v['runtime_bytes']}-byte empty program")
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
