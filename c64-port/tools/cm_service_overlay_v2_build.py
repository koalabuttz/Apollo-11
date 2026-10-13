#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build the CM service overlay proof on overlay ABI 2.

The four native service bodies and two proof-data modules are the unchanged
ABI 1 sources (tools/cm_service_overlay_build.py), linked through
platform/c64/overlay_v2_adapter.c into ABI 2 slots. They import multiword
arithmetic from the resident service table instead of linking their own copy
(single-word helpers stay private so LTO inlines them), and the packer puts
them in one CM REU store image."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from overlay_runtime_library import prepare  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
OUT = Path('build/cm-service-overlay-v2')
SERVICES = 0x0202
# ABI 1 number -> (ABI 2 id, slot class, selector count, sources)
MODULES = {
    1: (0x0101, 1, 64, ['platform/c64/cm_service_overlay_payload.c', 'src/cm_task_transfer.c', 'src/cm_waitlist.c']),
    2: (0x0102, 1, 64, ['platform/c64/cm_service_overlay_payload.c', 'src/cm_entry_tasks.c', 'src/cm_clocktask.c',
                        'src/cm_tig5.c', 'src/cm_svct3.c', 'src/cm_phase.c']),
    3: (0x0103, 1, 64, ['platform/c64/cm_service_overlay_payload.c', 'src/cm_waitlist_image.c',
                        'src/cm_delay_image.c', 'src/cm_waitlist.c']),
    4: (0x0104, 0, 64, ['platform/c64/cm_service_overlay_payload.c', 'src/cm_admission_image.c', 'src/cm_executive.c']),
    250: (0x0F01, 0, 6, ['platform/c64/cm_service_overlay_input.c', 'tests/cm_service_loop_cases.c',
                         'src/cm_executive.c', 'src/cm_waitlist.c']),
    251: (0x0F02, 0, 16, ['platform/c64/cm_service_overlay_check.c']),
}


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default='mos-c64-clang')
    args = p.parse_args()
    (ROOT / OUT).mkdir(parents=True, exist_ok=True)
    library = prepare(args.cc, ROOT / OUT)
    subprocess.run([sys.executable, 'tools/overlay_v2_services.py', '--out-dir', str(OUT)], cwd=ROOT, check=True)
    common = [args.cc, '-Iinclude', '-Itests', '-Iplatform/c64', '-Ibuild', '-std=c99', '-Oz', '-fno-static-stack',
              '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
    prgs, sizes = [], {}
    for number, (ident, slot, selectors, sources) in MODULES.items():
        prg = OUT / f'cm-service-{ident:04x}.prg'
        script = 'overlay_v2_slot_task.ld' if slot else 'overlay_v2_slot_job.ld'
        extra = [f'-DCM_SERVICE_MODULE={number}'] if number < 250 else []
        base = ['src/cm_native_image.c', 'src/agc_word.c'] if number != 251 else []
        subprocess.run(common + extra + ['-nostdlib', '-Lplatform/c64', f'-L{OUT}',
            '-T', f'platform/c64/{script}', '-T', 'platform/c64/overlay_v2_payload.ld',
            f'-Wl,--defsym=__ov2_slot={slot},--defsym=__ov2_mission=2,--defsym=__ov2_id={ident},'
            f'--defsym=__ov2_services={SERVICES},--defsym=__ov2_selectors={selectors},'
            f'-Map,{OUT}/cm-service-{ident:04x}.map']
            + sources + base + ['platform/c64/overlay_v2_adapter.c', 'platform/c64/overlay_v2_entry.S',
                                '-lc', library, '-lcrt', '-o', str(prg)], cwd=ROOT, check=True)
        prgs.append(str(prg))
        sizes[f'{ident:04x}'] = dict(abi1_module=number, slot='task' if slot else 'job',
                                     loaded_bytes=(ROOT / prg).stat().st_size - 2)
    subprocess.run(common + ['-T', 'platform/c64/overlay_v2_resident.ld', f'-Wl,-Map,{OUT}/resident.map',
        'src/cm_service_loop.c', 'src/overlay_runtime.c', 'src/overlay_v2.c', 'src/overlay_reu.c',
        'src/agc_word.c', 'src/agc_multiword.c', 'platform/c64/overlay_reu_platform.c',
        'platform/c64/overlay_v2_irq.S', 'platform/c64/cm_service_overlay_v2_diagnostic.c',
        f'{OUT}/overlay_v2_services.S', '-o', f'{OUT}/resident.prg'], cwd=ROOT, check=True)
    subprocess.run([sys.executable, 'tools/overlay_v2_pack.py', *prgs, '--mission', 'CM',
        '--services', hex(SERVICES), '--output', f'{OUT}/cm.reu'], cwd=ROOT, check=True)
    (ROOT / OUT / 'sizes.json').write_text(json.dumps(sizes, indent=1) + '\n')
    for ident, info in sizes.items():
        print(ident, info['slot'], info['loaded_bytes'], 'bytes (ABI 1 module', info['abi1_module'], end=')\n')


if __name__ == '__main__':
    main()
