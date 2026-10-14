#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build the ABI 2 target proof: service table, resident, three proof
modules (two job-slot, one task-slot) and the packed REU store image."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
OUT = Path('build/overlay-v2')
SERVICES = 0x0202
MODULES = ((1, 0, 4), (2, 0, 2), (3, 1, 2))   # number, slot class, selectors


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default='mos-c64-clang')
    args = p.parse_args()
    (ROOT / OUT).mkdir(parents=True, exist_ok=True)
    subprocess.run([sys.executable, 'tools/overlay_v2_services.py', '--out-dir', str(OUT),
                    '--version', hex(SERVICES)],
                   cwd=ROOT, check=True)
    common = [args.cc, '-Iinclude', '-Iplatform/c64', '-std=c99', '-Os', '-fno-static-stack',
              '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
    prgs = []
    for number, slot, selectors in MODULES:
        prg = OUT / f'm{number}.prg'
        script = 'overlay_v2_slot_task.ld' if slot else 'overlay_v2_slot_job.ld'
        subprocess.run(common + ['-nostdlib', f'-DOV2_MODULE={number}', '-Lplatform/c64', f'-L{OUT}',
            '-T', f'platform/c64/{script}', '-T', 'platform/c64/overlay_v2_payload.ld',
            f'-Wl,--defsym=__ov2_slot={slot},--defsym=__ov2_mission=1,'
            f'--defsym=__ov2_id={0x0F00 | number},--defsym=__ov2_services={SERVICES},'
            f'--defsym=__ov2_selectors={selectors},-Map,{OUT}/m{number}.map',
            'tests/overlay_v2_payload.c', 'src/agc_word.c', 'platform/c64/overlay_v2_entry.S', '-lcrt',
            '-o', str(prg)], cwd=ROOT, check=True)
        prgs.append(str(prg))
    subprocess.run(common + ['-T', 'platform/c64/overlay_v2_resident.ld', f'-Wl,-Map,{OUT}/resident.map',
        'src/overlay_runtime.c', 'src/overlay_v2.c', 'src/overlay_reu.c', 'src/agc_word.c',
        'src/agc_multiword.c', 'platform/c64/overlay_reu_platform.c',
        'platform/c64/overlay_v2_diagnostic.c', 'platform/c64/overlay_v2_irq.S',
        f'{OUT}/overlay_v2_services.S', '-o', f'{OUT}/resident.prg'], cwd=ROOT, check=True)
    resolver = [dict(bank=0o24000, loc=0o2100, module=0x0F02, selector=0),
                dict(bank=0o24000, loc=0o2000, module=0x0F01, selector=0)]
    (ROOT / OUT / 'resolver.json').write_text(json.dumps(resolver, indent=1) + '\n')
    subprocess.run([sys.executable, 'tools/overlay_v2_pack.py', *prgs, '--mission', 'LM',
        '--services', hex(SERVICES), '--resolver', f'{OUT}/resolver.json',
        '--output', f'{OUT}/lm.reu'], cwd=ROOT, check=True)


if __name__ == '__main__':
    main()
