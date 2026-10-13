#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build and measure the ABI 2 flight resident with the thin kernel.

For each mission (lm, cm) this links, with LLVM-MOS at -Oz, LTO and
gc-sections, on platform/c64/resident_flight.ld:

  build/resident-flight/<m>-resident.prg  the flight resident (resident_flight_main.c)
  build/resident-flight/<m>-check-<p>.prg the same resident with the flight check
                                          (resident_flight_check.c, RK_PART p 1-4) as main
  build/resident-flight/<m>-module-*.prg  the resident's boundary-word module and the check's
                                          modules (case generators in the job slot; rows and
                                          LM RESTARTS in the task slot)
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
SERVICES = 0x0202
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
        ('AGC timers and interrupt entry', ['src/resident_rupt.c']),
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
        ('AGC timers and interrupt entry', ['src/resident_rupt.c']),
        ('resident state and step', ['platform/c64/resident_flight.c']),
        ('IRQ entry', ['platform/c64/overlay_v2_irq.S']),
    ],
}
# The check's modules: (part, LM_EXJ_PART, module id, selectors). Case
# modules (job slot) link the resident-kernel milestone's case sources; part
# None is the check's data module (task slot; resident_flight_data.c).
MODULES = {
    'lm': [(1, None, 0x01F1, 2), (2, 1, 0x01F2, 1), (2, 2, 0x01F3, 1), (2, 3, 0x01F4, 1), (3, None, 0x01F5, 1),
           (None, None, 0x01F6, 1), (4, None, 0x01F7, 1), (5, None, 0x01F8, 2)],
    'cm': [(1, None, 0x01F1, 2), (2, None, 0x01F2, 1), (3, None, 0x01F5, 1), (None, None, 0x01F6, 1),
           (4, None, 0x01F7, 1), (5, None, 0x01F8, 2)],
}
# Part 4's case module: the T3RUPT cases (tests/resident_rupt_cases.c).
RUPT_SOURCES = ['platform/c64/resident_flight_rupt_cases.c', 'tests/resident_rupt_cases.c', 'src/agc_word.c',
                'src/agc_transfer.c']
# Part 5's case module: the other-vector cases (tests/resident_vector_cases.c),
# which take their states from the Kernel lane's interrupt-body generators.
VECTOR_SOURCES = {m: ['platform/c64/resident_flight_vector_cases.c', 'tests/resident_vector_cases.c',
                      'tests/resident_rupt_cases.c', 'tests/rupt_lead_ins_cases.c', 'tests/t4rupt_core_cases.c',
                      'tests/keyrupt_uprupt_cases.c', 'src/agc_word.c', 'src/agc_transfer.c']
                  + (['tests/lm_t6rupt_cases.c'] if m == 'lm' else []) for m in ('lm', 'cm')}
DATA_SOURCES = {'lm': ['platform/c64/resident_flight_data.c', 'src/lm_restart_routines.c', 'src/lm_restart_select.c',
                       'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c'],
                'cm': ['platform/c64/resident_flight_data.c']}
# The check's resolver records: the probe (bank 012) and the same probe at
# fixed-fixed 07753 under its canonical bank 06000, which the step must find
# whatever FB holds. tools/resident_boundary_words.py adds their keys.
PROBES = [dict(bank=0o24000, loc=0o2000, module=0x01F1, selector=1),
          dict(bank=0o6000, loc=0o7753, module=0x01F1, selector=1)]
# The resident's own modules, in every store it runs on: (module id, slot,
# defines, sources by mission). 0x01E0 is the boundary-word table
# (tools/resident_boundary_words.py); 0x01E1-0x01E4 wrap the interrupt
# bodies' tables (platform/c64/resident_rupt_module.c; 0x01E4 LM only).
# The wrappers link with a negative inline threshold: inlining the image
# operations into each routine is what overflows the task slot (LM T4RUPT
# core 8,506 B at -Oz alone, 7,731 B with it).
WRAP_FLAGS = ['-mllvm', '-inline-threshold=-50']
WRAP_COMMON = ['platform/c64/resident_rupt_module.c', 'src/agc_image_ops.c', 'src/agc_hardware_divide.c',
               'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c']
RESIDENT_MODULES = [
    (0x01E0, 1, [], {m: ['platform/c64/resident_boundary_module.c'] for m in ('lm', 'cm')}),
    (0x01E1, 1, ['-DRK_WRAP=1', *WRAP_FLAGS], {m: WRAP_COMMON + ['src/rupt_lead_ins.c'] for m in ('lm', 'cm')}),
    (0x01E2, 1, ['-DRK_WRAP=2', *WRAP_FLAGS], {m: WRAP_COMMON + ['src/t4rupt_core.c'] for m in ('lm', 'cm')}),
    (0x01E3, 1, ['-DRK_WRAP=3', *WRAP_FLAGS], {m: WRAP_COMMON + ['src/keyrupt_uprupt.c'] for m in ('lm', 'cm')}),
    (0x01E4, 1, ['-DRK_WRAP=4', *WRAP_FLAGS], {'lm': WRAP_COMMON + ['src/lm_t6rupt.c']}),
]
# Which wrapper runs each interrupt body table (tools/resident_boundary_keys.c names).
WRAPPED = {'lead_in_routines': 0x01E1, 't6rupt_routines': 0x01E4, 't4rupt_core_routines': 0x01E2,
           'keyrupt_routines': 0x01E3}


def canonical_record(physical):
    """(bank, loc) of a physical fixed address in the resolver's canonical form."""
    bank, offset = divmod(physical, 1024)
    if bank in (2, 3):
        return bank << 10, (bank << 10) + offset
    if bank >= 0o40:
        return ((bank - 8) << 10) | 0o100, 0o2000 + offset
    return bank << 10, 0o2000 + offset


def wrapper_records(cc, m):
    """A resolver record (selector 0) for every key of the wrapped tables the resident does not hold."""
    from resident_boundary_words import table_keys
    records = []
    for key, table in sorted(table_keys(cc, m, ROOT / 'build').items()):
        module = next((ident for suffix, ident in WRAPPED.items() if table.endswith(suffix)), None)
        if module is not None:
            bank, loc = canonical_record(key)
            records.append(dict(bank=bank, loc=loc, module=module, selector=0))
    return records


def run(cmd):
    subprocess.run([str(c) for c in cmd], cwd=ROOT, check=True)


def resident_sources(m):
    return [s for _, files in COMPONENTS[m] for s in files]


# The flight check: one program per RK_PART (each part's test code must fit
# beside the resident); part 4 is the AGC clock and T3RUPT, part 5 the
# boundary-word table and the interrupt-body wrappers.
CHECK_PARTS = (1, 2, 3, 4, 5)
CHECK_SOURCES = {1: ['tests/resident_kernel_cases.c'], 2: ['tests/resident_kernel_cases.c'],
                 3: ['tests/resident_kernel_cases.c'], 4: ['tests/resident_rupt_cases.c'],
                 5: []}


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
    # resident_flight.c keeps the trampoline, the module runner and the
    # clock and interrupt entry out of line under these names; count each
    # with its component.
    owners.insert(0, ('transfer ABI trampoline and Executive call sites', {'rk_trampoline'}))
    owners.insert(0, ('ABI 2 runtime and REU store', {'rk_run_module'}))
    owners.insert(0, ('AGC timers and interrupt entry', {'rk_rupts'}))
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


def link_module(cc, m, prg, slot, ident, selectors, sources, extra, library):
    run([cc, *COMMON, *MISSION[m], *extra, '-ffunction-sections', '-fdata-sections',
         '-nostdlib', '-Lplatform/c64', f'-L{OUT}', '-T', f'platform/c64/overlay_v2_slot_{"job" if slot == 0 else "task"}.ld',
         '-T', 'platform/c64/overlay_v2_payload.ld',
         f'-Wl,--gc-sections,--defsym=__ov2_slot={slot},--defsym=__ov2_mission={1 if m == "lm" else 2},'
         f'--defsym=__ov2_id={ident},--defsym=__ov2_services={SERVICES},--defsym=__ov2_selectors={selectors},'
         f'-Map,{prg}.map',
         *sources, 'platform/c64/overlay_v2_entry.S', '-lc', library, '-lcrt', '-o', prg])


def build_modules(cc, m, cases, library):
    prgs = []
    for ident, slot, defines, sources in RESIDENT_MODULES:
        if m not in sources:
            continue
        prg = OUT / f'{m}-module-{ident:04x}.prg'
        link_module(cc, m, prg, slot, ident, 1, sources[m], defines, library)
        prgs.append(prg)
    for part, exj, ident, selectors in MODULES[m]:
        prg = OUT / f'{m}-module-{ident:04x}.prg'
        if part == 4:
            extra, sources = [], RUPT_SOURCES
        elif part == 5:
            extra, sources = [], VECTOR_SOURCES[m]
        elif part:
            extra = [f'-DRK_PART={part}', '-DRK_INPUTS_ONLY'] + ([f'-DLM_EXJ_PART={exj}'] if exj else [])
            sources = ['platform/c64/resident_flight_cases.c', 'tests/resident_kernel_cases.c', *cases]
        else:
            extra, sources = [], DATA_SOURCES[m]
        link_module(cc, m, prg, 0 if part else 1, ident, selectors, sources, extra, library)
        prgs.append(prg)
    resolver = OUT / f'{m}-resolver.json'
    resolver.write_text(json.dumps(PROBES + wrapper_records(os.environ.get('CC', 'cc'), m), indent=1) + '\n')
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
            link_resident(a.cc, m, f'check-{part}', ['platform/c64/resident_flight_check.c', *CHECK_SOURCES[part]],
                          ['-DRK_RUNS_ONLY', f'-DRK_PART={part}'])
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
