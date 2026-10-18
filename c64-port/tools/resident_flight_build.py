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
import functools
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from overlay_runtime_library import prepare  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
# RK_FLIGHT_OUT: another build directory (the x0.5 check builds beside the
# default one).
OUT = Path(os.environ.get('RK_FLIGHT_OUT') or 'build/resident-flight')
SERVICES = 0x0203
COMMON = ['-Iinclude', '-Itests', '-Iplatform/c64', '-std=c99', '-Oz', '-fno-static-stack', '-mlto-zp=0',
          '-DRK_FIXED_IMAGE=1', '-DRK_SEAMS=1', '-Wall', '-Wextra', '-Werror',
          # The resident and every flight module resolve a pending DANZIG
          # (RTB right-hand order) through the mission's marker table.
          '-DAGC_TRANSFER_DANZIG_MARKER',
          # rk_boot verifies the whole store (apollo_reu_store_verify), so a
          # module load checks only the header's id (src/overlay_v2.c). Valid
          # only where apollo_ov2_init gets verify=0 after a successful
          # apollo_reu_store_verify, as rk_boot does: a build that skips that
          # check must not define it.
          '-DAPOLLO_OV2_STORE_VERIFIED',
          # The Executive's site tables are addresses only, the words read
          # from the REU rope image (include/agc_transfer.h): every image the
          # resident or a flight module runs with carries the rope
          # (flight_store_build.py add_rope, rope_check_image below).
          '-DAGC_SITES_ROPE']
# Scaled time (docs/RESIDENT-FLIGHT.md), with RK_WALL_CLOCK: RK_TICK_LAG, the clock ticks one
# step may catch up before the rest are dropped (default 2), and
# RK_TIME_DIVISOR, AGC time at 1/n of wall time (default 1; the nightly
# x0.5 run builds with 2). make passes its command-line variables in the
# environment.
COMMON += [f'-D{k}={os.environ[k]}u' for k in ('RK_TICK_LAG', 'RK_TIME_DIVISOR') if os.environ.get(k)]
# The work clock is the default; RK_WALL_CLOCK=1 builds the CIA wall clock.
COMMON += ['-DRK_WALL_CLOCK=1'] if os.environ.get('RK_WALL_CLOCK') else []
MISSION = {'lm': ['-DRK_LM=1', '-DP66_NATIVE_DISPATCH_ONLY'], 'cm': ['-DRK_CM=1']}


@functools.lru_cache(maxsize=None)
def _mct_data():
    """docs/flight-routine-mct.json (tools/x86_flight_mct.py, shared with
    the x86_64 build) by mission: 'table interp key' -> n, sum, sq, least,
    most of the yaAGC MCT per run."""
    path = Path(__file__).resolve().parents[1] / 'docs/flight-routine-mct.json'
    if not path.exists():
        return {}
    return {m: {f'{t} {i} {k}': dict(n=n, sum=total, sq=sq, least=lo, most=hi)
                for i, k, _median, n, total, sq, lo, hi, t in d['points']}
            for m, d in json.loads(path.read_text()).items()}


def _calibrated(table, named):
    """The calibration's name for a table: a hot subset
    (lm_p_axis_hot_routines, lm_t6rupt_jets_hot_routines) runs its full
    table's routines, which the calibration keys by the full table."""
    if table in named or '_hot_' not in table:
        return table
    full = table.replace('_hot_', '_')
    return full if full in named else re.sub(r'_[a-z0-9]+_hot_', '_', table)


def mct_weight(m, tables, interp=False):
    """The work clock's weight for a module running these tables
    (platform/c64/resident_flight.h): the mean yaAGC MCT per routine run
    over the calibration (docs/flight-routine-mct.json), of
    their basic keys (interp: their interpretive keys; tables None: every
    table's). Tables with no calibrated run take the mission's mean; no
    calibration at all, 24. Between 4 and 255 (rk_module_tables.mct is a byte; rk_module_limit).
    0 marks the background self-check loop (any *_self_check_idle(_hot)_routines
    table): idle, whose run takes the rest of the tick (RK_MCT_IDLE)."""
    if tables and any(t.endswith(('_self_check_idle_routines', '_self_check_idle_hot_routines')) for t in tables):
        return 0
    keys = _mct_data().get(m, {})
    named = {k.split()[0] for k in keys}
    mine = [v for k, v in keys.items()
            if (tables is None or k.split()[0] in {_calibrated(t, named) for t in tables})
            and k.split()[1] == str(int(interp))]
    pool = mine if sum(v['n'] for v in mine) else list(keys.values())
    n = sum(v['n'] for v in pool)
    return min(255, max(4, round(sum(v["sum"] for v in pool) / n))) if n else 24


# The work clock's weights (resident_flight.h): the trampoline's tables',
# but for the Executive's idle loop (*_kernel_idle_routines, 16 MCT a run
# against 59-83 for the others): its runs outnumber the rest 100 to 1 in the
# calibration, and since the self-check idle module's pass takes the rest of
# the tick (RK_MCT_IDLE) its own weight changes nothing.
KERNEL_TABLES = {'lm': ['lm_kernel_routines', 'lm_kernel_job_routines', 'lm_kernel_task_routines', 'lm_sp_routines'],
                 'cm': ['cm_kernel_routines', 'cm_kernel_job_routines', 'cm_sp_routines']}
for _m in MISSION:
    MISSION[_m] += [f'-DRK_RESIDENT_MCT={mct_weight(_m, KERNEL_TABLES[_m])}u',
                    # 0x01E1-0x01E4 (WRAPPED, below), and the interpreter's per slot.
                    '-DRK_WRAP_MCT_LIST=' + ','.join(
                        f'{mct_weight(_m, [f"{_m}_{t}"])}u' for t in ('lead_in_routines', 't4rupt_core_routines',
                                                                      'keyrupt_routines', 't6rupt_routines')),
                    f'-DRK_INTERP_MCT={mct_weight(_m, None, interp=True)}u']

# The resident's sources, by component (the size report's classes).
COMPONENTS = {
    'lm': [
        ('kernel', ['src/lm_kernel.c', 'src/agc_image_ops.c']),
        ('transfer ABI trampoline and Executive call sites', ['src/agc_transfer.c', 'src/lm_exec_routines.c',
                                                              'platform/c64/agc_find.S']),
        ('ABI 2 runtime and REU store', ['src/overlay_v2.c', 'src/overlay_reu.c', 'src/overlay_runtime.c',
                                         'platform/c64/overlay_reu_platform.c']),
        ('single-precision subroutines', ['src/sp_routines.c', 'src/lm_single_trig.c']),
        ('service table exports', ['src/agc_multiword.c']),
        ('word helpers', ['src/agc_word.c']),
        ('AGC timers and interrupt entry', ['src/resident_rupt.c']),
        ('resident state and step', ['platform/c64/resident_flight.c', 'platform/c64/rk_clock.S']),
        ('IRQ entry', ['platform/c64/overlay_v2_irq.S']),
    ],
    'cm': [
        ('kernel', ['src/cm_kernel.c', 'src/agc_image_ops.c']),
        ('transfer ABI trampoline and Executive call sites', ['src/agc_transfer.c', 'src/cm_exec_routines.c',
                                                              'platform/c64/agc_find.S']),
        ('ABI 2 runtime and REU store', ['src/overlay_v2.c', 'src/overlay_reu.c', 'src/overlay_runtime.c',
                                         'platform/c64/overlay_reu_platform.c']),
        ('single-precision subroutines', ['src/sp_routines.c', 'src/cm_quictrig.c', 'src/cm_single_return.c']),
        ('service table exports', ['src/agc_multiword.c']),
        ('word helpers', ['src/agc_word.c']),
        ('AGC timers and interrupt entry', ['src/resident_rupt.c']),
        ('resident state and step', ['platform/c64/resident_flight.c', 'platform/c64/rk_clock.S']),
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
# 0x01E5 is the hardware restart (platform/c64/resident_gojam_module.c,
# docs/FLIGHT-RESTART.md), which a resident trigger runs by id.
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
    (0x01E5, 1, [], {m: ['platform/c64/resident_gojam_module.c'] for m in ('lm', 'cm')}),
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


# The resident's find() is platform/c64/agc_find.S (agc_transfer.c
# AGC_TRANSFER_FIND_ASM): defined with the resident's sources rather than in
# COMMON, as the flight modules keep agc_transfer.c's C find. So is
# agc_transfer_run_at (AGC_TRANSFER_RUN_AT, include/agc_transfer.h), which
# rk_direct calls: other programs keep the smaller agc_transfer_run alone.
RESIDENT_DEFINES = ['-DAGC_TRANSFER_FIND_ASM', '-DAGC_TRANSFER_RUN_AT']


def resident_sources(m):
    return [*RESIDENT_DEFINES, *(s for _, files in COMPONENTS[m] for s in files)]


# The flight check: one program per RK_PART (each part's test code must fit
# beside the resident); part 4 is the AGC clock and T3RUPT, part 5 the
# boundary-word table and the interrupt-body wrappers.
CHECK_PARTS = (1, 2, 3, 4, 5)
CHECK_SOURCES = {1: ['tests/resident_kernel_cases.c'], 2: ['tests/resident_kernel_cases.c'],
                 3: ['tests/resident_kernel_cases.c'], 4: ['tests/resident_rupt_cases.c'],
                 5: []}


# The flight checks and their case modules use only the first three
# services (version $0202): the check programs carry test code beside the
# resident, which has no room for the flight modules' shared runtime.
CHECK_SERVICES, CHECK_SERVICE_COUNT = 0x0202, 3
CHECKS = OUT / 'checks'


def check_services():
    spec = json.loads((ROOT / 'docs/overlay-v2-services.json').read_text())
    spec.update(minor=CHECK_SERVICES & 0xFF, services=spec['services'][:CHECK_SERVICE_COUNT])
    (ROOT / CHECKS).mkdir(parents=True, exist_ok=True)
    (ROOT / CHECKS / 'services.json').write_text(json.dumps(spec, indent=2) + '\n')
    run([sys.executable, 'tools/overlay_v2_services.py', '--spec', CHECKS / 'services.json', '--out-dir', CHECKS])


# The flight program's resident (resident_flight_main.c) with its DSKY,
# which runs from $0200 and costs no resident bytes.
FLIGHT_MAIN = ['platform/c64/resident_flight_main.c', 'platform/c64/resident_dsky.c', 'platform/c64/resident_dsky.S']


def link_resident(cc, m, name, sources, defines=(), services=OUT):
    out = OUT / f'{m}-{name}.prg'
    run([cc, *COMMON, *MISSION[m], *defines, '-T', 'platform/c64/resident_flight.ld', f'-Wl,-Map,{out}.map',
         *sources, *resident_sources(m), services / 'overlay_v2_services.S', '-o', out])
    return out


def defined_symbols(cc, m, source):
    """Names a source defines (functions and objects, static included)."""
    obj = OUT / 'symbols' / (source.replace('/', '_') + f'.{m}.o')
    obj.parent.mkdir(parents=True, exist_ok=True)
    run([cc, *[f for f in COMMON if f != '-mlto-zp=0'], *MISSION[m], *RESIDENT_DEFINES, '-c', source, '-o', obj])
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
    owners.insert(0, ('transfer ABI trampoline and Executive call sites', {'rk_trampoline', 'agc_find'}))
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
         '-nostdlib', '-Lplatform/c64', f'-L{CHECKS}', '-T', f'platform/c64/overlay_v2_slot_{"job" if slot == 0 else "task"}.ld',
         '-T', 'platform/c64/overlay_v2_payload.ld',
         f'-Wl,--gc-sections,--defsym=__ov2_slot={slot},--defsym=__ov2_mission={1 if m == "lm" else 2},'
         f'--defsym=__ov2_id={ident},--defsym=__ov2_services={CHECK_SERVICES},--defsym=__ov2_selectors={selectors},'
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
    from resident_boundary_words import tail_bytes
    tail = OUT / f'{m}-boundary.bin'
    tail.write_bytes(tail_bytes((ROOT / f'platform/c64/resident_boundary_words_{m}.h').read_text()))
    run([sys.executable, 'tools/overlay_v2_pack.py', *prgs, '--mission', m.upper(), '--services', hex(CHECK_SERVICES),
         '--resolver', resolver, '--tail', tail, '--output', OUT / f'{m}.reu', '--manifest', OUT / f'{m}-manifest.json'])
    rope_check_image(m)
    return prgs


def rope_check_image(m):
    """The check's store image padded to 2 MB with the mission's rope at its
    REU base (include/rope_image.h), as flight_store_build.py add_rope places
    it in the flight store: the resident reads the Executive's inline operand
    words there (AGC_SITES_ROPE)."""
    from rope_image_pack import BASES, physical_words
    base = BASES[{'lm': 'Luminary099', 'cm': 'Comanche055'}[m]]
    reu = ROOT / OUT / f'{m}.reu'
    image = bytearray(reu.read_bytes())
    if len(image) > base:
        raise SystemExit(f'{m}: the check store reaches the rope image at {base:#x}')
    image += bytes(2 * 1024 * 1024 - len(image))
    for k, w in enumerate(physical_words(ROOT / f'evidence/fixed-data-{m}-original.rope.bin')):
        image[base + 2 * k:base + 2 * k + 2] = w.to_bytes(2, 'little')
    reu.write_bytes(image)
    manifest = ROOT / OUT / f'{m}-manifest.json'
    data = json.loads(manifest.read_text())
    data.update(reu_size_kb=2048, image_sha256=hashlib.sha256(image).hexdigest(), rope_reu_base=hex(base))
    manifest.write_text(json.dumps(data, indent=1) + '\n')


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
    check_services()
    library = prepare(a.cc, ROOT / OUT)
    report = {}
    for m in a.missions.split(','):
        build_modules(a.cc, m, make_cases(m), library)
        for part in CHECK_PARTS:
            link_resident(a.cc, m, f'check-{part}', ['platform/c64/resident_flight_check.c', *CHECK_SOURCES[part]],
                          ['-DRK_RUNS_ONLY', '-DRK_NO_DIRECT', f'-DRK_PART={part}', f'-DRK_SERVICES_VERSION={CHECK_SERVICES:#06x}u'],
                          CHECKS)
        report[m] = measure(a.cc, m, link_resident(a.cc, m, 'resident', FLIGHT_MAIN, ['-DRK_DSKY=1']))
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
