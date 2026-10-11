#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Link five independently loadable, explicit native RODCOMP continuation groups."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from overlay_build import crc16

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--cc', default='mos-c64-clang')
    p.add_argument('--output', type=Path, default=ROOT / 'build')
    args = p.parse_args()
    build = args.output.resolve()
    build.mkdir(parents=True, exist_ok=True)
    sources = ['src/agc_word.c', 'src/agc_multiword.c', 'src/p66_native.c']
    sources += [str(file.relative_to(ROOT)) for file in sorted((ROOT / 'src').glob('guidance*.c'))]
    groups = [(1, 'prepare', '03275 -> before INTPRET at03353'),
              (2, 'scale-time', 'VXSC/PDDL THISTPIP -> DANZIG03356'),
              (3, 'elapsed-store', 'DSU PIPTIME/STORE30D -> DANZIG03361'),
              (4, 'divide-gravity', 'DDV4SEC(28)/PDVL GDT/2 -> DANZIG03364'),
              (5, 'velocity', 'VSU/GDT-VEL/TEMP/R/V -> DANZIG03373 before UNIT')]
    modules = []
    for number, name, boundary in groups:
        prg = build / f'overlay-p66-{number}.prg'
        command = [args.cc, '-Iinclude', '-std=c99', '-Os', '-fno-static-stack',
            '-mlto-zp=0', '-Wall', '-Wextra', '-Werror', '-nostdlib',
            '-DP66_NATIVE_ARITH_ONLY', f'-DP66_NATIVE_ONLY_GROUP={number}',
            '-T', 'platform/c64/overlay_payload.ld',
            f'-Wl,--defsym=__overlay_module={number},-Map,{build}/overlay-p66-{number}.map',
            'platform/c64/p66_overlay_payload.c', 'platform/c64/overlay_entry.S', *sources,
            '-lc', '-lcrt', '-o', str(prg)]
        subprocess.run(command, cwd=ROOT, check=True)
        file = prg.read_bytes()
        data = file[2:]
        assert file[:2] == b'\x00\x80' and data[:4] == b'NATV' and data[5] == number
        assert int.from_bytes(data[14:16], 'little') == len(data)
        bss = int.from_bytes(data[10:12], 'little')
        bss_size = int.from_bytes(data[12:14], 'little')
        assert bss >= 32768 + len(data) and bss + bss_size <= 49152
        module = dict(id=number, filename=f'P66G{number}', name=name, selector=number,
            boundary=boundary, byte_count=len(data), crc16=crc16(data), bss_size=bss_size,
            sha256=hashlib.sha256(file).hexdigest(), prg=prg.name,
            shared_state='canonical2048word erasable image', mission_translation=True,
            native_sources=['platform/c64/p66_overlay_payload.c', *sources],
            compile_defines=['P66_NATIVE_ARITH_ONLY', f'P66_NATIVE_ONLY_GROUP={number}'],
            scope='one bounded Luminary099 RODCOMP group; no complete-program claim')
        modules.append(module)
        print(f'P66 group{number} {name}: {len(data)} loaded bytes + {bss_size} BSS; entry$8010; {boundary}')
    (build / 'overlay-p66-modules.json').write_text(json.dumps(modules, indent=2) + '\n')
    header = '#include "overlay_runtime.h"\nstatic const apollo_overlay_module overlay_p66_modules[5]={\n'
    header += ''.join('    {%d,"%s",%d,%d},\n' %
        (m['id'], m['filename'], m['byte_count'], m['crc16']) for m in modules)
    header += '};\n'
    (build / 'overlay_p66_modules.h').write_text(header)


if __name__ == '__main__':
    main()
