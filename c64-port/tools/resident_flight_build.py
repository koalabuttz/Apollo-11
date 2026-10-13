#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build and measure the ABI 2 flight resident with the thin kernel.

For each mission (lm, cm) this links, with LLVM-MOS at -Oz, LTO and
gc-sections, on platform/c64/resident_flight.ld:

  build/resident-flight/<m>-resident.prg  the flight resident (resident_flight_main.c)
  build/resident-flight/<m>-check-<p>.prg the same resident with the flight check
                                          (resident_flight_check.c, RK_PART p) as main
  build/resident-flight/<m>-module-*.prg  the check's modules (case generators in the
                                          job slot; rows and LM RESTARTS in the task slot)
  build/resident-flight/<m>.reu           the check's REU store (modules + probe resolver)

and writes build/resident-flight/sizes.json: the flight resident's bytes in
$0801-$6EFF by component (classified by the source file that defines each
linked symbol), the service table, and the boot-only bytes linked into the
job slot.

  resident_flight_build.py [--cc MOS_CC] [--missions lm,cm] [--json]
"""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from overlay_runtime_library import prepare  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
OUT = Path('build/resident-flight')
SERVICES = 0x0201
COMMON = ['-Iinclude', '-Itests', '-Iplatform/c64', '-std=c99', '-Oz', '-fno-static-stack', '-mlto-zp=0',
          '-DRK_FIXED_IMAGE=1', '-DRK_SEAMS=1', '-Wall', '-Wextra', '-Werror']
MISSION = {'lm': ['-DRK_LM=1', '-DP66_NATIVE_DISPATCH_ONLY'], 'cm': ['-DRK_CM=1']}

# The resident's sources, by component (the size report's classes).
COMPONENTS = {
    'lm': [
        ('kernel', ['src/lm_kernel.c', 'src/agc_image_ops.c']),
        ('transfer ABI trampoline and Executive call sites', ['src/agc_transfer.c', 'src/lm_exec_routines.c']),
        ('ABI 2 runtime and REU store', ['src/overlay_v2.c', 'src/overlay_reu.c', 'src/overlay_runtime.c',
                                         'platform/c64/overlay_reu_platform.c']),
        ('single-precision subroutines', ['src/sp_routines.c', 'src/lm_single_trig.c']),
        ('service table exports', ['src/agc_multiword.c']),
        ('word helpers', ['src/agc_word.c']),
        ('resident state and step', ['platform/c64/resident_flight.c']),
        ('IRQ entry', ['platform/c64/overlay_v2_irq.S']),
    ],
    'cm': [
        ('kernel', ['src/cm_kernel.c', 'src/agc_image_ops.c']),
        ('transfer ABI trampoline and Executive call sites', ['src/agc_transfer.c', 'src/cm_exec_routines.c']),
        ('ABI 2 runtime and REU store', ['src/overlay_v2.c', 'src/overlay_reu.c', 'src/overlay_runtime.c',
                                         'platform/c64/overlay_reu_platform.c']),
        ('single-precision subroutines', ['src/sp_routines.c', 'src/cm_quictrig.c', 'src/cm_single_return.c']),
        ('service table exports', ['src/agc_multiword.c']),
        ('word helpers', ['src/agc_word.c']),
        ('resident state and step', ['platform/c64/resident_flight.c']),
        ('IRQ entry', ['platform/c64/overlay_v2_irq.S']),
    ],
}
# The check's modules: (part, LM_EXJ_PART, module id, selectors). Case
# modules (job slot) link the resident-kernel milestone's case sources; part
# None is the check's data module (task slot; resident_flight_data.c).
MODULES = {
    'lm': [(1, None, 0x01F1, 2), (2, 1, 0x01F2, 1), (2, 2, 0x01F3, 1), (2, 3, 0x01F4, 1), (3, None, 0x01F5, 1),
           (None, None, 0x01F6, 1)],
    'cm': [(1, None, 0x01F1, 2), (2, None, 0x01F2, 1), (3, None, 0x01F5, 1), (None, None, 0x01F6, 1)],
}
DATA_SOURCES = {'lm': ['platform/c64/resident_flight_data.c', 'src/lm_restart_routines.c', 'src/lm_restart_select.c',
                       'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c'],
                'cm': ['platform/c64/resident_flight_data.c']}
PROBE = dict(bank=0o24000, loc=0o2000, module=0x01F1, selector=1)


def run(cmd):
    subprocess.run([str(c) for c in cmd], cwd=ROOT, check=True)


def resident_sources(m):
    return [s for _, files in COMPONENTS[m] for s in files]


# The flight check: one program per RK_PART (each part's test code must fit
# beside the resident).
CHECK_PARTS = (1, 2, 3)
CHECK_SOURCES = ['platform/c64/resident_flight_check.c', 'tests/resident_kernel_cases.c']


def link_resident(cc, m, name, sources, defines=()):
    out = OUT / f'{m}-{name}.prg'
    run([cc, *COMMON, *MISSION[m], *defines, '-T', 'platform/c64/resident_flight.ld', f'-Wl,-Map,{out}.map',
         *sources, *resident_sources(m), OUT / 'overlay_v2_services.S', '-o', out])
    return out


def defined_symbols(cc, m, source):
    """Names a source defines (functions and objects, static included)."""
    obj = OUT / 'symbols' / (source.replace('/', '_') + f'.{m}.o')
    obj.parent.mkdir(parents=True, exist_ok=True)
    run([cc, *[f for f in COMMON if f != '-mlto-zp=0'], *MISSION[m], '-c', source, '-o', obj])
    nm = Path(cc).parent / 'llvm-nm'
    names = set()
    for line in subprocess.run([str(nm), str(obj)], cwd=ROOT, check=True, capture_output=True,
                               text=True).stdout.splitlines():
        parts = line.split()
        if len(parts) >= 2 and parts[-2] in 'TtDdBbRrVv':
            names.add(parts[-1])
    return names


MAP_LINE = re.compile(r'\s*([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)\s+\d+\s+(.*)$')


def sections(map_path):
    """(address, size, input section) for each input section in the map."""
    rows = []
    for line in open(ROOT / map_path):
        mt = MAP_LINE.match(line)
        if mt and ':(' in mt.group(3):
            rows.append((int(mt.group(1), 16), int(mt.group(2), 16), mt.group(3)))
    return rows


def symbol(section):
    name = section.rsplit(':(', 1)[1].rstrip(')')
    name = re.sub(r'^\.(text|rodata|data|bss|noinit|zp|boot\.text)\.?', '', name)
    return re.sub(r'(\.llvm\.\d+|\.\d+)$', '', name)


def measure(cc, m, prg):
    owners = []
    for comp, files in COMPONENTS[m]:
        names = set()
        for f in files:
            if not f.endswith('.S'):
                names |= defined_symbols(cc, m, f)
        owners.append((comp, names))
    owners.append(('IRQ entry', {'ov2_irq', 'ov2_irq_entry', 'ov2_nmi_entry', 'ov2_irq_count'}))
    # resident_flight.c keeps the trampoline and the module runner out of
    # line under these names; count each with its component.
    owners.insert(0, ('transfer ABI trampoline and Executive call sites', {'rk_trampoline'}))
    owners.insert(0, ('ABI 2 runtime and REU store', {'rk_run_module'}))
    text = Path(f'{prg}.map').read_text()
    end = int(re.search(r'([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__rk_resident_end = \.', text).group(1), 16)
    boot0 = int(re.search(r'([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__rk_boot_start = \.', text).group(1), 16)
    boot1 = int(re.search(r'([0-9a-f]+)\s+[0-9a-f]+\s+0\s+1\s+__rk_boot_end = \.', text).group(1), 16)
    by = {}
    unclassified = {}
    for address, size, section in sections(f'{prg}.map'):
        if not (0x0801 <= address < 0x6F00) or not size:
            continue
        name = symbol(section)
        comp = next((c for c, names in owners if name in names), None)
        if comp is None:
            comp = 'C runtime and library'
            unclassified[name] = unclassified.get(name, 0) + size
        by[comp] = by.get(comp, 0) + size
    total = end - 0x0801
    by['BASIC header and alignment'] = total - sum(by.values())
    return dict(resident_bytes=total, resident_capacity=0x6F00 - 0x0801, free_bytes=0x6F00 - end,
                service_table_bytes=3 * len(json.loads((ROOT / 'docs/overlay-v2-services.json').read_text())['services']),
                boot_only_bytes=boot1 - boot0, components=dict(sorted(by.items(), key=lambda kv: -kv[1])),
                library_symbols=dict(sorted(unclassified.items(), key=lambda kv: -kv[1])))


def build_modules(cc, m, cases, library):
    prgs = []
    for part, exj, ident, selectors in MODULES[m]:
        prg = OUT / f'{m}-module-{ident:04x}.prg'
        if part:
            extra = [f'-DRK_PART={part}', '-DRK_INPUTS_ONLY'] + ([f'-DLM_EXJ_PART={exj}'] if exj else [])
            sources = ['platform/c64/resident_flight_cases.c', 'tests/resident_kernel_cases.c', *cases]
        else:
            extra, sources = [], DATA_SOURCES[m]
        slot = 0 if part else 1
        run([cc, *COMMON, *MISSION[m], *extra, '-ffunction-sections', '-fdata-sections',
             '-nostdlib', '-Lplatform/c64', f'-L{OUT}', '-T', f'platform/c64/overlay_v2_slot_{"job" if slot == 0 else "task"}.ld',
             '-T', 'platform/c64/overlay_v2_payload.ld',
             f'-Wl,--gc-sections,--defsym=__ov2_slot={slot},--defsym=__ov2_mission={1 if m == "lm" else 2},'
             f'--defsym=__ov2_id={ident},--defsym=__ov2_services={SERVICES},--defsym=__ov2_selectors={selectors},'
             f'-Map,{prg}.map',
             *sources, 'platform/c64/overlay_v2_entry.S', '-lc', library, '-lcrt', '-o', prg])
        prgs.append(prg)
    resolver = OUT / f'{m}-resolver.json'
    resolver.write_text(json.dumps([PROBE], indent=1) + '\n')
    run([sys.executable, 'tools/overlay_v2_pack.py', *prgs, '--mission', m.upper(), '--services', hex(SERVICES),
         '--resolver', resolver, '--output', OUT / f'{m}.reu', '--manifest', OUT / f'{m}-manifest.json'])
    return prgs


def make_cases(m):
    """The case sources of tools/mk/resident-kernel.mk (RK_LM_CASES, RK_CM_CASES)."""
    text = (ROOT / 'tools/mk/resident-kernel.mk').read_text()
    return re.search(rf'^RK_{m.upper()}_CASES = (.*)$', text, re.M).group(1).split()


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--missions', default='lm,cm')
    p.add_argument('--json', action='store_true')
    a = p.parse_args()
    (ROOT / OUT).mkdir(parents=True, exist_ok=True)
    run([sys.executable, 'tools/overlay_v2_services.py', '--out-dir', OUT])
    library = prepare(a.cc, ROOT / OUT)
    report = {}
    for m in a.missions.split(','):
        build_modules(a.cc, m, make_cases(m), library)
        for part in CHECK_PARTS:
            link_resident(a.cc, m, f'check-{part}', CHECK_SOURCES, ['-DRK_RUNS_ONLY', f'-DRK_PART={part}'])
        report[m] = measure(a.cc, m, link_resident(a.cc, m, 'resident', ['platform/c64/resident_flight_main.c']))
    (ROOT / OUT / 'sizes.json').write_text(json.dumps(report, indent=1) + '\n')
    if a.json:
        print(json.dumps(report, indent=1))
    else:
        for m, r in report.items():
            print(f"{m}: resident {r['resident_bytes']} of {r['resident_capacity']} bytes ({r['free_bytes']} free), "
                  f"service table {r['service_table_bytes']}, boot-only {r['boot_only_bytes']} in the job slot")
            for comp, size in r['components'].items():
                print(f'  {size:6d}  {comp}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
