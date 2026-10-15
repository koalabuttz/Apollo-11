#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build the whole-program flight store: every lane's routine tables in ABI 2
modules, for the flight resident (docs/FLIGHT-STORE.md).

Steps, per mission:

1. Index. Every src/*.c is compiled on the host (cc -c) and read with nm:
   which file defines each symbol, which symbols each file needs, and which
   defined objects are routine tables (names ending _routines, _basic,
   _interpretive or _marker).
2. Keys. A host program built from all objects prints every table's keys.
   A table belongs to the LM or CM by its name (lm/cm token, p66_*,
   standard_lead_*) or else by its file's name.
3. Selection. The resident's own tables (the thin kernel, SPSIN/SPCOS, the
   interrupt bodies its wrappers run) and the certified Executive tables the
   thin kernel replaces are not packed. A key two packed tables both hold is
   a collision: the resolver sends it to the table of the file
   docs/port-coverage.json lists first for that AGC file (else the first by
   name), and the report lists every collision for its lanes.
4. Modules. One module per source file that defines packed tables: the
   generic wrapper platform/c64/flight_table_module.c around that file's
   tables, linked with the file's link closure (every src/*.c that defines
   a symbol the file needs, transitively) at -Oz with LTO and gc-sections.
   The lane and slot come from docs/overlay-allocation.json through the AGC
   file docs/port-coverage.json maps the source to: task slot where the
   allocation says task, otherwise the job slot. Module ids are the lane's
   high byte and a sequence number (lane A stops below its resident ids
   0xE0-0xFF).
5. Resolver. One record per packed key in the canonical form
   (tools/resident_flight_build.py canonical_record): basic keys by Z,
   interpretive keys by LOC under BANKSET. Erasable keys (continuations in
   erasable memory) get no record; the report counts them.
6. Pack. tools/overlay_v2_pack.py writes build/flight-store/<m>.reu and its
   manifest; the report (build/flight-store/report.json) has the store size
   against the 2 MB REU, every module that does not fit its slot or fails to
   link, the collisions, and the tables by mission.

  flight_store_build.py [--cc MOS_CC] [--missions lm,cm] [--jobs N] [--host-cc CC] [--host-only]
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import collections
import fcntl
import functools
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

sys.path.insert(0, str(Path(__file__).resolve().parent))
from overlay_runtime_library import prepare  # noqa: E402
from resident_flight_build import (COMMON, MISSION, KERNEL_TABLES, RESIDENT_MODULES, SERVICES, WRAPPED, canonical_record, mct_weight,  # noqa: E402
                                   resident_sources, wrapper_records)
from resident_boundary_words import header_text, tail_bytes  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
OUT = Path('build/flight-store')
NM = ROOT / 'deps/llvm-mos/bin/llvm-nm'
PIN_LTO = ['-Wl,-mllvm,-inline-threshold=-50']
REU_LIMIT = 0x1c0000          # tools/overlay_allocation.py REU_STORE_LIMIT
SLOT_BYTES = {0: 0x4000, 1: 0x1FFA}
LANE_BYTE = {'A': 0x01, 'B': 0x02, 'C': 0x03, 'D1': 0x04, 'D2': 0x05, 'E': 0x06, 'F': 0x07, 'G': 0x08, 'H': 0x09}
LANE_LIMIT = {'A': 0xDF}       # lane A 0xE0-0xFF: the resident's and the flight check's modules
TABLE_NAME = re.compile(r'(_routines|_basic|_interpretive|_marker)$')
# Held by the resident (tools/resident_flight_build.py COMPONENTS and its
# wrapper modules), or replaced by the thin kernel (docs/RESIDENT-FLIGHT.md).
RESIDENT = {
    'lm': ['lm_kernel_routines', 'lm_kernel_job_routines', 'lm_kernel_task_routines', 'lm_kernel_idle_routines',
           'lm_sp_routines', 'lm_lead_in_routines', 'lm_t4rupt_core_routines', 'lm_keyrupt_routines',
           'lm_t6rupt_routines', 'agc_lm_danzig_marker'],
    'cm': ['cm_kernel_routines', 'cm_kernel_job_routines', 'cm_kernel_idle_routines', 'cm_sp_routines',
           'cm_lead_in_routines', 'cm_t4rupt_core_routines', 'cm_keyrupt_routines', 'agc_cm_danzig_marker'],
}
REPLACED = {'lm': ['lm_exec_routines', 'lm_exec_job_routines', 'lm_exec_task_routines', 'lm_exec_idle_routines'],
            'cm': ['cm_exec_routines', 'cm_exec_job_routines', 'cm_exec_idle_routines']}
ERASABLE_BASE = 40960
# Modules over their slot by more than this are not split (a split needs
# about two trial links per routine).
SPLIT_LIMIT = 65536
# The io record each source's tables take (include/flight_io.h), where it is
# not the mission's base record (LM p66_bias_io, CM cm_native_io).
IO_KIND = {
    'lm': {'lm_fresh_start': 'LM_FRESH', 'lm_servicer': 'LM_SV', 'lm_lunar_landing': 'LM_LLD',
           'lm_imu_performance': 'LM_IPT', 'lm_burn_baby': 'LM_BB', 'lm_ags_init': 'LM_AGS',
           'imu_mode_switching': 'LM_IMU', 'lm_downtelemetry': 'LM_DNTM', 'lm_dntm_rupt': 'LM_DNTM',
           'lanec_ev_routines': 'LM_DNTM',
           'lanec_pin_routines': 'LM_DNTM',
           'lanec_pin_keys_num_routines': 'LM_DNTM', 'lanec_pin_keys_cmd_routines': 'LM_DNTM',
           'lanec_pin_keys_release_routines': 'LM_DNTM',
           'lanec_pin_keys_sign_routines': 'LM_DNTM', 'lanec_pin_keys_clear_routines': 'LM_DNTM'},
    'cm': {'cm_fresh_start': 'CM_FRESH', 'cm_v37': 'CM_FRESH', 'cm_downtelemetry': 'CM_DNTM', 'cm_dntm_rupt': 'CM_DNTM',
           'lanec_ev_routines': 'CM_DNTM', 'lanec_pin_routines': 'CM_DNTM',
           'lanec_pin_keys_num_routines': 'CM_DNTM', 'lanec_pin_keys_cmd_routines': 'CM_DNTM',
           'lanec_pin_keys_release_routines': 'CM_DNTM',
           'lanec_pin_keys_sign_routines': 'CM_DNTM', 'lanec_pin_keys_clear_routines': 'CM_DNTM', 'cm_p21_routines': 'P66', 'cm_p76_routines': 'P66', 'cm_stateup': 'P66'},
}
BASE_IO = {'lm': 'P66', 'cm': 'CM_NATIVE'}
TABLE_DEF = re.compile(r'^const agc_routine_table\s+([A-Za-z0-9_]+)\s*=', re.M)


# The record type of each kind (include/flight_io.h flight_io_buffer).
IO_TYPE = {'P66': 'p66_bias_io', 'CM_NATIVE': 'cm_native_io', 'LM_FRESH': 'lm_fresh_io',
           'LM_SV': 'lm_sv_io', 'LM_LLD': 'lm_lld_io', 'LM_IPT': 'lm_ipt_io',
           'LM_BB': 'lm_bb_io', 'LM_AGS': 'lm_ags_io', 'LM_IMU': 'lm_imu_io', 'LM_DNTM': 'lm_dntm_io',
           'CM_FRESH': 'cm_fresh_io', 'CM_DNTM': 'cm_dntm_io', 'LM_CHANNEL': 'lm_channel_io',
           'CM_CHANNEL': 'cm_channel_io'}
# Record types a routine never takes as its io (component records, the
# flight record itself).
IO_INTERNAL = {'flight_io', 'apollo_noun_reader_io', 'dsky_charin_io', 'dsky_key_irq_io', 'dsky_lamp_io',
               'dsky_relay_io', 'dsky_reset_io', 'enema_io', 'cm_enema_io', 'gimbal_io', 'imu_pulse_io',
               'imu_standby_io', 'lanec_io', 'lm_torque_io', 'p66_task_io'}


@functools.lru_cache(maxsize=None)
def io_types():
    """Every struct typedef named *_io in include/."""
    return {t for h in (ROOT / 'include').glob('*.h')
            for t in re.findall(r'\}\s*([A-Za-z0-9_]+_io)\s*;', h.read_text(errors='replace'))}


def io_kind(m, stem):
    """The io record a source's tables take: from IO_KIND, else the base
    record followed by the input channels (include/agc_input_channels.h) for
    a source whose wrappers take lm_channel_io or cm_channel_io. Refuses a
    source naming an io record no kind maps field by field, and a kind whose
    record the source never names, so a new layout fails here rather than
    running with a mismatched record."""
    path = ROOT / 'src' / f'{stem}.c'
    text = path.read_text(errors='replace') if path.exists() else ''
    if stem in IO_KIND[m]:
        kind = IO_KIND[m][stem]
    elif f'{m}_channel_io' in text:
        kind = f'{m.upper()}_CHANNEL'
    else:
        kind = BASE_IO[m]
    if text:
        named = set(re.findall(r'\b([A-Za-z0-9_]+_io)\b', text)) & io_types()
        unknown = named - set(IO_TYPE.values()) - IO_INTERNAL
        if unknown:
            raise SystemExit(f'src/{stem}.c: io record {", ".join(sorted(unknown))} has no flight_io kind '
                             '(add it to include/flight_io.h, src/flight_io.c and IO_TYPE)')
        if IO_TYPE[kind] not in named and kind != BASE_IO[m]:
            raise SystemExit(f'src/{stem}.c: mapped to {kind} but never names {IO_TYPE[kind]}')
        # A file shared by both missions names the other mission's record too.
        special = {t for t in named & set(IO_TYPE.values()) if t.startswith(f'{m}_') and t != 'cm_native_io'}
        if kind == BASE_IO[m] and special:
            raise SystemExit(f'src/{stem}.c: names {", ".join(sorted(special))} but maps to the base record '
                             f'{IO_TYPE[kind]} (add it to IO_KIND)')
    return kind


def table_names():
    names = set()
    for f in list((ROOT / 'src').glob('*.c')) + list((ROOT / 'src').glob('*.inc')):
        names |= set(TABLE_DEF.findall(f.read_text(errors='replace')))
    return names


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], cwd=ROOT, check=True, **kw)


def host_index(cc, jobs):
    """{object: (defined symbols by kind, needed symbols)} for every src/*.c."""
    objs = ROOT / OUT / 'host'
    objs.mkdir(parents=True, exist_ok=True)
    sources = sorted((ROOT / 'src').glob('*.c'))

    def stale(obj, dep):
        # The object and the headers and .inc files it includes (-MMD), so an
        # edit to a shared body (src/self_check_body.inc) rebuilds its includers.
        if not obj.exists() or not dep.exists():
            return True
        made = obj.stat().st_mtime
        words = dep.read_text().replace('\\\n', ' ').split()[1:]
        return any(not (ROOT / w).exists() or (ROOT / w).stat().st_mtime > made for w in words)

    def build(src):
        obj = objs / (src.stem + '.o')
        dep = objs / (src.stem + '.d')
        if stale(obj, dep):
            run([cc, '-std=c99', '-O0', '-Iinclude', '-Itests', '-MMD', '-MF', dep, '-c', src, '-o', obj])
        return src.stem, obj
    with ThreadPoolExecutor(jobs) as pool:
        built = list(pool.map(build, sources))
    index = {}
    for stem, obj in built:
        defined, needed = {}, set()
        for line in run(['nm', obj], capture_output=True, text=True).stdout.splitlines():
            p = line.split()
            if len(p) == 2 and p[0] == 'U':
                needed.add(p[1])
            elif len(p) == 3 and p[1] in 'TDRBC':
                defined[p[2]] = p[1]
        index[stem] = (defined, needed)
    return index, [obj for _, obj in built]


def table_keys(cc, tables, objects):
    """{table: [keys]} from a host program over all objects."""
    src = ROOT / OUT / 'keys.c'
    names = sorted(tables)
    src.write_text('\n'.join(
        ['#include <stdio.h>', '#include <stdint.h>',
         'typedef struct { uint16_t physical; void *call; } E;',
         'typedef struct { const E *entries; uint16_t count; } T_;']
        + [f'extern const T_ {n};' for n in names]
        + ['static const struct { const char *n; const T_ *t; } T[] = {'
           + ''.join(f'{{"{n}",&{n}}},' for n in names) + '};',
           'int main(void){unsigned i,j;for(i=0;i<sizeof T/sizeof T[0];++i)for(j=0;j<T[i].t->count;++j)'
           'printf("%s %u\\n",T[i].n,(unsigned)T[i].t->entries[j].physical);return 0;}']) + '\n')
    exe = ROOT / OUT / 'keys'
    run([cc, '-std=c99', src, *objects, '-Wl,--unresolved-symbols=ignore-all', '-o', exe],
        capture_output=True)
    keys = collections.defaultdict(list)
    for line in run([exe], capture_output=True, text=True).stdout.splitlines():
        n, k = line.split()
        keys[n].append(int(k))
    # agc_transfer.c's find() binary-searches each table and skips one whose
    # first..last range misses the point, so keys must strictly ascend.
    unsorted = [n for n, v in keys.items() if any(a >= b for a, b in zip(v, v[1:]))]
    if unsorted:
        raise SystemExit(f'routine tables not in ascending key order: {" ".join(sorted(unsorted))}')
    return {n: keys.get(n, []) for n in names}


def mission_of(table, stem):
    for text in (table, stem):
        if re.search(r'(^|_)lm(_|$)', text) or text.startswith(('p66', 'standard_lead')):
            return 'lm'
        if re.search(r'(^|_)cm(_|$)', text):
            return 'cm'
    return None


# Host-only sources (file I/O) and their C64 counterparts: the rope images
# the REU holds above the store (include/rope_image.h).
HOST_ONLY = {'rope_image_host': 'platform/c64/rope_image_reu.c'}


def closure(index, *stems):
    owner = {s: o for o, (d, _) in index.items() for s in d if o not in HOST_ONLY}
    seen, stack = set(stems), list(stems)
    while stack:
        for s in index[stack.pop()][1]:
            o = owner.get(s)
            if o and o not in seen:
                seen.add(o)
                stack.append(o)
    return sorted(seen)


def module_io(m, stems):
    """The io kind a module's tables run on (platform/c64/flight_table_module.c
    FS_IO_KIND): None for the mission's base record, else the one kind every
    other stem takes. One run hands every routine the same record, so a
    module may mix the base record only with a kind that starts with it
    (LM_* records open with p66_bias_io, CM_* with cm_native_io); any other
    mix is refused."""
    kinds = {io_kind(m, st) for st in stems}
    other = kinds - {BASE_IO[m]}
    if len(other) > 1 or (other and len(kinds) > 1 and not next(iter(other)).startswith(f'{m.upper()}_')):
        raise SystemExit(f'{m}: one module mixes io kinds {sorted(kinds)} ({", ".join(stems)})')
    if not other:
        return None
    kind = other.pop()
    # The module converts through src/flight_io.c: a kind with no case
    # there would run on a cleared record.
    io_c = (ROOT / 'src/flight_io.c').read_text()
    enter, leave = io_c.split('void flight_io_leave', 1)
    if f'case FLIGHT_IO_{kind}:' not in enter or f'case FLIGHT_IO_{kind}:' not in leave:
        raise SystemExit(f'{m}: io kind {kind} ({", ".join(stems)}) has no flight_io_enter/leave case in src/flight_io.c')
    return kind


def c64_sources(index, *stems):
    """The link closure as C64 source paths, host-only files replaced."""
    owner_all = {s: o for o, (d, _) in index.items() for s in d}
    seen = closure(index, *stems)
    extra = {HOST_ONLY[owner_all[s]] for o in seen for s in index[o][1] if owner_all.get(s) in HOST_ONLY}
    return [f'src/{s}.c' for s in seen] + sorted(extra)


def source_owners():
    """src stem -> (AGC path, order of the stem in that file's native_files) for every coverage entry."""
    cov = json.loads((ROOT / 'docs/port-coverage.json').read_text())
    owners = collections.defaultdict(list)
    for f in cov['files']:
        for c in f.get('native_coverage', []):
            for i, nf in enumerate(c.get('native_files', [])):
                owners[Path(nf).stem].append((f['path'], i))
    return owners


def placements():
    alloc = json.loads((ROOT / 'docs/overlay-allocation.json').read_text())
    return {f['path']: (f['lane'], f['placement']) for m in alloc['missions'] for f in m['files']}


def lane_slot(stem, mission, owners, place):
    program = 'Luminary099' if mission == 'lm' else 'Comanche055'
    cands = sorted((i, path) for path, i in owners.get(stem, []) if path.startswith(program))
    if not cands:
        return 'H', 0, None
    path = cands[0][1]
    lane, placement = place.get(path, ('H', 'job'))
    return lane, 1 if placement == 'task' else 0, path


# Image ops a flight module takes from the resident kernel's k_* services
# (platform/c64/flight_image_op_services.c, docs/overlay-v2-services.json).
SHARED_OPS = ('ca caf cs csf ad adf maskf ts xch lxch ads dxch ccs tc dcaf bzf bzmf').split()
# Modules use the resident's rk_hardware and rk_flight records and its
# interrupt count (platform/c64/resident_flight.h RK_HARDWARE, RK_FLIGHT_IO,
# RK_IRQ_COUNT), not copies of their own. Compiled routines reach the
# channels in rk_hardware by name (tools/interp_compile.py AGC_CHANNELS_HW).
CHANNELS = {m: ['-DAGC_CHANNELS_HW=rk_hardware',
                '-Wl,--defsym=rk_hardware=0x6fbe,--defsym=rk_flight=0x6fdc,--defsym=ov2_irq_count=0x6fbc']
            for m in ('lm', 'cm')}


# Slot classes a flight module links for (include/overlay_v2.h): job, task,
# pinned (class 3, $C000) and the pinned task region (class 4, $E000), both
# loaded once at boot and only called directly.
SLOT_NAME = {0: 'job', 1: 'task', 3: 'pinned', 4: 'pinned_task'}
SLOT_BYTES = {0: 16384, 1: 8186, 3: 3072, 4: 8186}


def map_symbols(path):
    """The symbol names an lld map lists (address, VMA, size, align, name);
    none when the link wrote no map (lld writes none on undefined symbols)."""
    if not Path(path).exists():
        return set()
    return {p[4] for p in (l.split() for l in Path(path).read_text().splitlines())
            if len(p) == 5 and re.fullmatch(r'[0-9a-f]+', p[0]) and not p[4].startswith(('.', '<', '/', 'build'))
            and '(' not in p[4] and '.' not in p[4]}


def host_mct(m, t):
    """The work clock's weight the host flight run gives table t
    (FLIGHT_SCALED=W): the resident's or a wrapper's for the tables they run,
    otherwise the table's own calibrated MCT per routine (the C64 module
    averages its tables', tools/flight_host_run.c)."""
    if t in RESIDENT[m]:
        wrap = [i for w, i in WRAPPED.items() if t == f'{m}_{w}']
        tables = [f'{m}_{w}' for w, i in WRAPPED.items() if wrap and i == wrap[0]] or KERNEL_TABLES[m]
        return mct_weight(m, tables)
    return mct_weight(m, [t], interp='interpret' in t)


def link_module(cc, m, prg, slot, ident, sources, basic, interp, library, chunk=None, sites=None, fixed_data=None,
                io=None, direct_only=False, defines=(), link_flags=(), pick=None, keys=None,
                key_range=(0, 0xFF)):
    """chunk: (table, first, last) links only entries first..last-1 of one
    basic table (platform/c64/flight_table_module.c FS_CHUNK). pick:
    [(table, index)] in key order instead of basic and chunk (FS_PICK). io: the
    module's io kind (module_io). direct_only: no transfer engine, the
    resident runs the routines (FS_DIRECT_ONLY; pinned modules). defines:
    the module's own -D flags (FLIGHT_PARTS). link_flags: extra linker
    flags (a pinned pair's --undefined and --defsym, pin_pair). keys: the
    table keys ({table: [physical]}); with chunk or pick, the entries'
    keys go in as data (FS_KEYS), so the module's main stores only calls.
    key_range: the high bytes of the lowest and highest key the module's
    tables hold (FS_KEY_LO and FS_KEY_SPAN, so the resident skips a module
    that cannot hold the step's point; resident_flight.h)."""
    wrapper = prg.with_suffix('.c')
    head = (f'#define FS_CHUNK_TABLE {chunk[0]}\n#define FS_CHUNK(X) '
            + ' '.join(f'X({i})' for i in range(chunk[1], chunk[2])) + '\n') if chunk else ''
    if pick is not None:
        head, basic = '#define FS_PICK(X) ' + ' '.join(f'X({t},{i})' for t, i in pick) + '\n', []
    if keys is not None and (chunk or pick is not None):
        picked = pick if pick is not None else [(chunk[0], i) for i in range(chunk[1], chunk[2])]
        head += '#define FS_KEYS ' + ','.join(f'{keys[t][i]}u' for t, i in picked) + '\n'
    if direct_only:
        head += '#define FS_DIRECT_ONLY 1\n'
    if io:
        head += (f'#define FS_IO_KIND FLIGHT_IO_{io}\n'
                 f'#define FS_IO_CM {int(io.startswith("CM_"))}\n')
        sources = [*sources, 'src/flight_io.c']
    if sites:
        head += '#define FS_SITES ' + ','.join(f'{{{k}u,0{w:o}u}}' for k, w in sorted(sites.items())) + '\n'
    if fixed_data == 'rope':
        head += '#define FS_ROPE_DATA 1\n'
        sources = [*sources, 'platform/c64/rope_image_reu.c']
    elif fixed_data:
        head += f'#define FS_FIXED_DATA {fixed_data}\n'
    # The work clock's weight (resident_flight.h): its tables' calibrated
    # MCT per routine.
    head += f'#define FS_KEY_LO {key_range[0]}u\n#define FS_KEY_SPAN {key_range[1] - key_range[0]}u\n'
    head += f'#define FS_MCT {mct_weight(m, [chunk[0]] if chunk else list(basic) + list(interp))}u\n'
    wrapper.write_text(
        head + f'#define FS_BASIC(X) {" ".join(f"X({t})" for t in basic)}\n'
        f'#define FS_INTERP(X) {" ".join(f"X({t})" for t in interp)}\n'
        + (f'#define FS_HAS_INTERP 1\n#define FS_DANZIG agc_{m}_danzig_marker\n' if interp else '')
        + '#include "flight_table_module.c"\n')
    # Interpretive groups register the mission's DANZIG marker, which
    # agc_transfer.c defines on the C64 only when asked to.
    cmd = [cc, *COMMON, *MISSION[m], '-DAGC_FLIGHT_INTERPRETER', *defines,
           *(['-DAGC_TRANSFER_DANZIG_MARKER'] if interp else []),
           '-mllvm', '-inline-threshold=-50',
           '-ffunction-sections', '-fdata-sections', '-nostdlib', '-Lplatform/c64', f'-L{OUT}',
           '-T', f'platform/c64/overlay_v2_slot_{SLOT_NAME[slot]}.ld',
           '-T', 'platform/c64/overlay_v2_payload.ld',
           f'-Wl,--gc-sections,--defsym=__ov2_slot={slot},--defsym=__ov2_mission={1 if m == "lm" else 2},'
           f'--defsym=__ov2_id={ident},--defsym=__ov2_services={SERVICES},--defsym=__ov2_selectors=1,'
           f'-Map,{prg}.map', *(f'-Wl,--wrap=ai_{op}' for op in SHARED_OPS), *CHANNELS[m], *link_flags,
           wrapper, *sources, 'platform/c64/flight_image_op_services.c', 'platform/c64/overlay_v2_entry.S', '-lc', library,
           '-lcrt', '-o', prg]
    r = subprocess.run([str(c) for c in cmd], cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        over = [int(v) for v in re.findall(r"overflowed by (\d+) bytes", r.stderr)]
        undefined = sorted(set(re.findall(r'undefined symbol: (\S+)', r.stderr)))
        return dict(ok=False, overflow=max(over) if over else None, undefined=undefined,
                    error=None if over else [l for l in r.stderr.splitlines() if 'error' in l][:4])
    return dict(ok=True, bytes=(ROOT / prg).stat().st_size - 2)



def resident_modules(cc, host_cc, m, resolver, library, pin_task=False):
    """Link the resident's modules (tools/resident_flight_build.py
    RESIDENT_MODULES) for this store, and write the boundary-word table the
    store ends with (build/flight-store/<m>-boundary.bin, overlay_v2_pack.py
    --tail): the word at every key of the resident's tables and of these
    resolver records. pin_task: a class-4 module holds $E000, so the
    task-slot ones link for the job slot."""
    text = header_text(host_cc, m, resolver)
    if text is None:
        raise SystemExit(f'{m}: boundary words disagree with the listing')
    (ROOT / OUT / f'{m}-boundary-words.h').write_text(text)
    (ROOT / OUT / f'{m}-boundary.bin').write_bytes(tail_bytes(text))
    prgs = []
    for ident, slot, defines, sources in RESIDENT_MODULES:
        if m not in sources:
            continue
        slot = 0 if pin_task else slot
        prg = OUT / m / f'resident-{ident:04x}.prg'
        extra = list(defines)
        subprocess.run([str(c) for c in [
            cc, *COMMON, *MISSION[m], *extra, '-ffunction-sections', '-fdata-sections', '-nostdlib',
            '-Lplatform/c64', f'-L{OUT}', '-T', f'platform/c64/overlay_v2_slot_{"job" if slot == 0 else "task"}.ld',
            '-T', 'platform/c64/overlay_v2_payload.ld',
            f'-Wl,--gc-sections,--defsym=__ov2_slot={slot},--defsym=__ov2_mission={1 if m == "lm" else 2},'
            f'--defsym=__ov2_id={ident},--defsym=__ov2_services={SERVICES},--defsym=__ov2_selectors=1,'
            f'-Map,{prg}.map', *sources[m], 'platform/c64/overlay_v2_entry.S', '-lc', library, '-lcrt', '-o', prg]],
            cwd=ROOT, check=True)
        prgs.append(prg)
    return prgs


def add_rope(m):
    """Put the mission's rope image where include/rope_image.h reads it on the
    C64 (the top of a 2 MB REU, above the store), for the modules whose
    fixed_data reads the rope (self-check, FIXED_DATA 'rope')."""
    from rope_image_pack import BASES, physical_words
    program = {'lm': 'Luminary099', 'cm': 'Comanche055'}[m]
    reu = ROOT / OUT / f'{m}.reu'
    image = bytearray(reu.read_bytes())
    base = BASES[program]
    if len(image) > base and any(image[base:]):
        raise SystemExit(f'{m}: the store reaches the rope image at {base:#x}')
    image += bytes(2 * 1024 * 1024 - len(image))
    for k, w in enumerate(physical_words(ROOT / f'build/fixed-data/{program}/MAIN.agc.bin')):
        image[base + 2 * k:base + 2 * k + 2] = w.to_bytes(2, 'little')
    reu.write_bytes(image)
    manifest = ROOT / OUT / f'{m}-manifest.json'
    data = json.loads(manifest.read_text())
    data.update(reu_size_kb=2048, image_sha256=hashlib.sha256(image).hexdigest(), rope_reu_base=hex(base))
    manifest.write_text(json.dumps(data, indent=1) + '\n')


SCRATCH_BYTES = 32768


def link_load_check(cc, m, resolver, modules, keys, pinned=None):
    """build/flight-store/<m>-load-check.prg: the flight resident booting on
    this store, which resolves every record and times each flight module's
    load (platform/c64/flight_load_check.c, tools/flight_vice_load.py).
    pinned: the store's class-3 module, {id, bank, loc, module}: one of its
    points and the module that packs the same table; task_id, when a class-4
    module holds $E000, its id."""
    idle = next(z for z in range(0o7777, 0o4000, -1) if 3 * 1024 + (z & 0o1777) not in keys)
    # The direct-call check keeps the boot image in the store file's unused tail.
    reu = (ROOT / OUT / f'{m}.reu').read_bytes()
    scratch = len(reu) - SCRATCH_BYTES
    if any(reu[scratch:]):
        raise SystemExit(f'{m}: no unused {SCRATCH_BYTES} bytes at the end of {m}.reu for the load check')
    first = {}
    for r in resolver:
        first.setdefault(r['module'], r)
    a = b = 0
    for r in sorted(resolver, key=lambda r: (r['bank'], r['loc'])):
        for w in (r['bank'], r['loc'], r['module']):
            a = (a + w) & 0xffff
            b = (b + a) & 0xffff
    data = OUT / f'{m}-load-check.h'
    (ROOT / data).write_text(
        f'/* Generated by tools/flight_store_build.py: the {m.upper()} flight store\'s resolver record count and checksum and '
        'flight modules. */\n'
        f'#define FL_RECORDS {len(resolver)}u\n#define FL_RECORD_SUM 0x{b << 16 | a:08x}ul\n'
        f'#define FL_MODULES {len(modules)}u\n#define FL_IDLE_Z 0{idle:o}u\n'
        f'#define FL_SCRATCH 0x{scratch:x}ul\n#define FL_SCRATCH_BYTES {SCRATCH_BYTES}u\n'
        'static const fl_module fl_modules[FL_MODULES] BOOT_DATA={'
        + ','.join(f'{{0x{mod["id"]:04x}u,{mod["bytes"]}u,0{first.get(mod["id"], {}).get("bank", 0):o}u,'
                   f'0{first.get(mod["id"], {}).get("loc", 0):o}u}}' for mod in modules) + '};\n'
        + (f'#define FL_PINNED 0x{pinned["id"]:04x}u\n#define FL_PINNED_BANK 0{pinned["bank"]:o}u\n'
           f'#define FL_PINNED_LOC 0{pinned["loc"]:o}u\n#define FL_PINNED_MODULE 0x{pinned["module"]:04x}u\n'
           + (f'#define FL_PINNED_TASK 0x{pinned["task_id"]:04x}u\n' if pinned.get('task_id') else '')
           if pinned else ''))
    out = OUT / f'{m}-load-check.prg'
    subprocess.run([str(c) for c in [
        cc, *COMMON, *MISSION[m], f'-DFL_DATA="{ROOT / data}"', '-T', 'platform/c64/resident_flight.ld',
        f'-Wl,-Map,{out}.map', 'platform/c64/flight_load_check.c', *resident_sources(m),
        OUT / 'overlay_v2_services.S', '-o', out]], cwd=ROOT, check=True)
    return out


# Tables also packed into another module, so the routines they hold run there
# without a module swap (docs/FLIGHT-STORE.md). The background self-check loop
# leaves SMODECHK through TC POSTJUMP; the interbank tables ride with it,
# and with the LM idle loop's own module (CM pins its idle loop), so an idle
# pass loads nothing (that module's runs are the work clock's idle,
# RK_MCT_IDLE: the interbank routines run there only from the idle loop).
# The DOWNRUPT vector and RESUME ride with the down-telemetry tables
# (src/lm_dntm_rupt.c, src/cm_dntm_rupt.c), so one module run takes a whole
# DOWNRUPT, which runs inhibited, from the vector to the resumed Z; the
# vector's wrapper record goes to that module (DOWNRUPT_HOST).
CO_LOCATE = {'lm': {'lm_self_check': ['lm_interbank_routines'], 'lm_self_check_idle': ['lm_interbank_routines'],
                    'lm_downtelemetry': ['lm_dntm_rupt_routines']},
             'cm': {'cm_self_check': ['cm_interbank_routines'], 'cm_downtelemetry': ['cm_dntm_rupt_routines']}}
# The module that takes the DOWNRUPT vector (04040) from the lead-in wrapper:
# the down-telemetry file's, when it links with the DOWNRUPT ends.
DOWNRUPT_HOST = {'lm': ('lm_downtelemetry', 'lm_dntm_rupt_routines'),
                 'cm': ('cm_downtelemetry', 'cm_dntm_rupt_routines')}
DOWNRUPT_VECTOR = 2048 + 0o40   # physical 04040
# Tables pinned in a slot (docs/FLIGHT-STORE.md): loaded once and kept, so
# a cycle that runs them all costs no swaps. The builder links each slot's
# set as one module and reports its size. A class-3 set (slot 3, $C000)
# that fits direct-only goes into the store: the resident loads it at boot
# and its routines run from there through rk_direct; the resolver still
# names their packed modules. A class-4 set (slot 4, the task slot's $E000)
# goes in beside a packed class-3 set, whose table list names its tables;
# every task-slot module then links for the job slot. LM: the RCS DAP hold
# cycle (Lane F, /mnt/project-files/lane-f/dap-cycle-trace.md) across both
# regions (D2 splits it into the hot subsets that fit); the self-check idle
# loop is an ordinary job-slot module there. CM: the self-check idle loop.
# The LM set is the hot subsets D2 measured (docs/FLIGHT-STORE.md): class 4
# holds TJETLAW, the RESUME family every interrupt ends in, and the runtime
# both regions share (the runtime home, pin_pair); class 3 holds DAPIDLER's
# hot entries and the table list naming both. P-AXIS, Q,R-AXIS and RATELOOP
# run from the excursion.
PIN = {'lm': {4: ['lm_tjet_law_routines', 'lm_t6rupt_hot_routines'], 3: ['lm_dapidler_hot_routines']},
       'cm': {3: ['cm_self_check_idle_routines']}}
def packed_pins(pinned):
    """{table: slot class} for the pinned sets that went into the store (the
    report's "pinned", packed): their own tables, not the home's that a
    class-3 table list names too."""
    return {t: {'pinned': 3, 'pinned_task': 4}[p['slot']] for p in pinned if p.get('packed') for t in p['own']}


def host_pins(m, packed):
    """The pinned sets a host-only build models: the last full build's report
    (build/flight-store/report.json) when its sets are still PIN's, else PIN
    as configured (class 4 only beside class 3, as the store packs it)."""
    try:
        pinned = json.loads((ROOT / OUT / 'report.json').read_text())[m]['pinned']
        if all('packed' in p for p in pinned) and \
                sorted(map(sorted, (p['own'] for p in pinned))) == sorted(map(sorted, PIN[m].values())):
            return packed_pins(pinned)
    except (OSError, KeyError, ValueError):
        pass
    classes = [c for c in sorted(PIN[m]) if c == 3 or 3 in PIN[m]]
    return {t: c for c in classes for t in PIN[m][c] if t in packed}



def pack_report(packs):
    """The report's "packs": each job-slot pack (PACKS) with its tables, the
    ride ones and whether it linked."""
    return [dict(name=p['prg_name'], id=f"0x{p['id']:04x}", tables=p['basic'],
                 ride=[t for t in p['basic'] if t in RIDE], ok=bool(p.get('ok'))) for p in packs]


def pack_tables(packs):
    """{table: (pack name, ride)} for the packs that linked."""
    return {t: (p['name'], t in p['ride']) for p in packs if p['ok'] for t in p['tables']}


def host_packs(m, packed):
    """The packs a host-only build models: the last full build's report when
    its packs are still PACKS', else PACKS as configured (only beside a
    class-4 pin, as the store packs them)."""
    want = sorted((n, sorted(t for t in ts if t in packed)) for n, ts, _ in PACKS[m] if any(t in packed for t in ts))
    try:
        packs = json.loads((ROOT / OUT / 'report.json').read_text())[m]['packs']
        if sorted((p['name'], sorted(p['tables'])) for p in packs) == want:
            return pack_tables(packs)
    except (OSError, KeyError, ValueError):
        pass
    if 4 not in host_pins(m, packed).values():
        return {}
    return pack_tables([dict(name=n, tables=ts, ride=[t for t in ts if t in RIDE], ok=True) for n, ts in want])


# The excursion: hot subsets one job-slot module packs together, so a DAP
# pass that leaves the pinned sets loads one module, not one per file. It
# takes the runtime from the class-4 home (pin_pair) and is packed only
# beside it; the resolver sends its keys to it before their files' modules.
# RATELOOP (lm_kalman) runs in the same pass, so it rides here and leaves
# the home room for RESUME.
EXCURSION = {'lm': ['lm_p_axis_hot_routines', 'lm_q_r_axis_hot_routines', 'lm_kalman_basic',
                    'lm_t6rupt_jets_hot_routines', 'lm_interbank_dap_hot_routines'], 'cm': []}
# Tables a pack carries for rk_direct only: the DAP pass calls them while
# the pack is loaded, but other callers keep loading their own (smaller)
# modules, so the resolver does not send their keys to the pack.
RIDE = {'lm_t6rupt_jets_hot_routines', 'lm_interbank_dap_hot_routines'}
# A ride table whose operands are another file's sites (IBNKCALL's CADR is
# its caller's word, compiled into lm_interbank_routines' site list): the
# pack takes that file's sites in the banks its own tables' keys are in.
RIDE_SITES = {'lm_interbank_dap_hot_routines': 'lm_interbank_routines'}
# T4RUPT the same way: its core (the resident's wrapper 0x01E2 holds the
# whole table too) and the radar step in one job-slot module, so a radar
# T4RUPT loads one module, not two (steward/cpu-budget.md). RCSMON stays
# out: it takes lm_channel_io, which would put the pack's every step on the
# transfer engine instead of rk_direct.
T4PACK = {'lm': ['lm_t4rupt_core_hot_routines', 'lm_t4rupt_rr_routines'], 'cm': []}
# Job-slot modules beside the pinned home: (name, tables, module id). Lane
# A's 0xE0-0xFF are the resident's, so the ids are fixed in lane F's.
PACKS = {m: [('excursion', EXCURSION[m], 0x07F5), ('t4rupt', T4PACK[m], 0x07F6)] for m in ('lm', 'cm')}
# Hot subsets (a file's *_hot_routines: the entries one DAP pass reaches) are
# for PIN only: the file's modules pack its full table, which holds the same
# keys and routines.
def hot_full(t):
    return t[:-len('_hot_routines')] + '_routines' if t.endswith('_hot_routines') else None
# The compact interpreter (Interpreter lane, platform/c64/interp_module.c):
# module $0A01 in the job slot, its cold bodies module $0A02 at $D000 (slot
# class 2, loaded at boot), linked by tools/interp_runtime_size.py link_core.
# The resolver sends the interpreter's fixed entry points (logical Z, include/
# interp_runtime_tables.h) to $0A01; a table or the resident that holds one
# of them keeps it. CHANG2 is left out: the Executive's job switch, where the
# interpreter hands back.
INTERP_MODULE = 0x0A01
INTERP_POINTS = {
    'lm': {'NEWOPS': 0o6070, 'DANZIG': 0o6061, 'NOIBNKSW': 0o6063, 'DIRADRES': 0o6106, 'DODLOAD': 0o6460,
           'DODLOADX': 0o6144, 'DOVLOAD': 0o6463, 'DOVLOADX': 0o6466, 'B15BITADR': 0o6252, 'GOTO': 0o6646,
           'SWRETURN': 0o4631, 'INTPRET': 0o6037},
    'cm': {'NEWOPS': 0o6037, 'DANZIG': 0o6030, 'NOIBNKSW': 0o6032, 'DIRADRES': 0o6055, 'DODLOAD': 0o6427,
           'DODLOADX': 0o6113, 'DOVLOAD': 0o6432, 'DOVLOADX': 0o6435, 'B15BITADR': 0o6221, 'GOTO': 0o6615,
           'SWRETURN': 0o4675, 'INTPRET': 0o6006}}


# Files whose step machines stop at NEWOPS under -DAGC_FLIGHT_INTERPRETER
# (docs/FLIGHT-INTERP-STOP.md): the C64 store leaves their interpretive
# tables to the interpreter module. INTERP_ONLY tables hold interpretive
# frontiers only and are left out whole, as are the compiled interpreter's
# own entry tables (SWRETURN, INTPRET, DANZIG: the interbank tables, the
# interpreter module's records and the DANZIG marker hold them). The host
# index and host.inc keep every compiled table (other targets choose
# compiled routines).
FLIGHT_INTERP_STOP = {'lm': {'lm_ags_init', 'lm_burn_baby', 'lm_lunar_landing', 'lm_servicer', 'lm_imu_performance'},
                      'cm': set()}
INTERP_ONLY = {'lm': {'lm_ephemeris_routines', 'lm_s3435_routines', 'lm_s3435b_routines',
                      'lm_interp_core_basic', 'lm_interp_runtime_basic',
                      'lanec_ev_lm_intpret'},
               'cm': {'cm_interp_core_basic', 'cm_interp_runtime_basic', 'lanec_ev_cm_intpret'}}

# The host index's interpretive flag: a table keyed by LOC under BANKSET.
# Generated interpretive tables end in _interpretive(_routines); the DANZIG
# markers are looked up at interpretive points too; INTERP_KEYED tables hold
# compiled interpretive code under other names. The INTERPRETER.agc basic
# tables (lm/cm_interpreter_basic, _routines) are basic, though their names
# say interpret.
INTERP_KEYED = {'lm_ephemeris_routines', 'lm_s3435_routines', 'lm_s3435b_routines'}


def interpretive_table(t):
    return bool(re.search(r'_interpretive(_routines)?$|_marker$', t)) or t in INTERP_KEYED


# Files linked as several modules, one per define set
# (docs/LM-SERVICER-FLIGHT-PARTS.md). A part's keys are what its own build's
# tables hold (part_keys), which can include hand-off words the whole file
# does not key.
FLIGHT_PARTS = {'lm': {'lm_servicer': [['-DLMSV_FLIGHT_PART=1'], ['-DLMSV_FLIGHT_PART=2']],
                       'lm_aostask': [['-DLM_AOSTASK_FLIGHT_PART=%d' % j] for j in range(1, 4)],
                       'lm_interpreter': [['-DLM_INTERPRETER_FLIGHT_PART=%d' % j] for j in (1, 2)]},
                'cm': {'cm_p40_p47': [['-DCM_P40_P47_FLIGHT_PART=%d' % j] for j in range(1, 7)],
                       'cm_extended_verbs_file': [['-DCM_EXTENDED_VERBS_FILE_FLIGHT_PART=%d' % j] for j in (1, 2)],
                       'cm_interpreter': [['-DCM_INTERPRETER_FLIGHT_PART=%d' % j] for j in (1, 2)],
                       'cm_imu_calibration': [['-DCM_IMU_CALIBRATION_FLIGHT_PART=%d' % j] for j in (1, 2, 3)]}}


# Files whose over-slot table splits greedily (split_greedy): each chunk
# takes as many consecutive routines as fit, instead of halving until every
# chunk fits. The CM entry DAP pass runs its table front to back across the
# chunks; halving gave 4 chunks of about 9-10 KB and a pass loaded about 5
# of them (lane-g/cm-entry-profile-80342904.md); greedy chunks of about
# 15 KB cut its job loads by a third.
GREEDY_SPLIT = {'lm': set(), 'cm': {'cm_entry_dap'}}

def part_keys(cc, m, stem, tables):
    """[{table: [keys]}] per FLIGHT_PARTS build of stem: its host object
    with the part's defines and each table renamed, read by table_keys."""
    parts = []
    # Not in host/: tools/flight_host_differential.py links every object there.
    (ROOT / OUT / 'host-parts').mkdir(parents=True, exist_ok=True)
    for j, defines in enumerate(FLIGHT_PARTS[m][stem], 1):
        obj = ROOT / OUT / 'host-parts' / f'{stem}.part{j}.o'
        run([cc, '-std=c99', '-O0', '-Iinclude', '-Itests', '-DAGC_FLIGHT_INTERPRETER', *defines,
             *(f'-D{t}={t}_part{j}' for t in tables), '-c', ROOT / 'src' / f'{stem}.c', '-o', obj])
        got = table_keys(cc, [f'{t}_part{j}' for t in tables], [obj])
        parts.append({t: got[f'{t}_part{j}'] for t in tables})
    return parts


def flight_keys(cc, stem, tables):
    """{table: [keys]} of a FLIGHT_INTERP_STOP file built as its flight
    module is (-DAGC_FLIGHT_INTERPRETER): the flight build can key words the
    host build does not, such as the basic word an EXIT returns to
    (tools/flight_exit_returns.py)."""
    (ROOT / OUT / 'host-parts').mkdir(parents=True, exist_ok=True)
    obj = ROOT / OUT / 'host-parts' / f'{stem}.flight.o'
    run([cc, '-std=c99', '-O0', '-Iinclude', '-Itests', '-DAGC_FLIGHT_INTERPRETER',
         *(f'-D{t}={t}_flight' for t in tables), '-c', ROOT / 'src' / f'{stem}.c', '-o', obj])
    got = table_keys(cc, [f'{t}_flight' for t in tables], [obj])
    return {t: got[f'{t}_flight'] for t in tables}


def interp_modules(cc, m, library):
    """Link the interpreter's two modules for the store; their PRGs and
    link_core's result."""
    import interp_runtime_size
    prg = OUT / m / 'interp.prg'
    res = interp_runtime_size.link_core(cc, m, prg, interp_runtime_size.CORE[m], library, ext=True)
    return ([prg, prg.parent / 'interp-ext.prg'] if res['ok'] else []), res


# Modules whose fixed_data is not the resident's boundary words: self-check's
# own hook (include/self_check.h), or 'rope', every rope word from the REU
# image (include/rope_image.h; LM TRIM_GIMBAL's GTS reads DOSHIFT's indexed
# CA operands through it).
FIXED_DATA = {'lm_self_check': 'lm_self_check_rope_data', 'cm_self_check': 'cm_self_check_rope_data',
              'lm_trim_gimbal': 'rope'}
FIXED_KIND = {None: 0, 'rope': 2}
# Where the resident's own tables read their inline operands: the thin
# kernel's program holds the Executive's site table.
RESIDENT_SITES = {'lm': 'lm_exec_routines', 'cm': 'cm_exec_routines'}


def c_value(text):
    """A preprocessed C integer constant expression."""
    text = re.sub(r'\(\s*(?:u?int\d+_t|unsigned|agc_word)\s*\)', '', text)
    text = re.sub(r'\b0([0-7]+)[uU]?\b', r'0o\1', text)
    text = re.sub(r'\b(\d+)[uU]\b', r'\1', text)
    if not re.fullmatch(r'[0-9a-fA-Fxo+\-*/()<>|&^~ \t\n]*', text):
        raise SystemExit(f'site operand {text!r} is not a constant expression')
    return int(eval(text, {'__builtins__': {}}))  # noqa: S307 - digits and operators only


@functools.lru_cache(maxsize=None)
def file_sites(cc, m, stem):
    """{physical: word}: the agc_site_operand arrays src/<stem>.c defines for
    mission m (arrays named for the other mission skipped), preprocessed."""
    out = subprocess.run([cc, '-E', '-P', '-Iinclude', '-Itests', f'-DRK_{m.upper()}=1', f'src/{stem}.c'],
                         cwd=ROOT, capture_output=True, text=True, check=True).stdout
    sites = {}
    for name, body in re.findall(r'agc_site_operand\s+(\w+)\s*\[[^]]*\]\s*=\s*\{(.*?)\}\s*;', out, re.S):
        tokens = name.split('_')
        if ('lm' in tokens or 'cm' in tokens) and m not in tokens:
            continue
        for entry in re.findall(r'\{([^{}]*)\}', body):
            k, w = (c_value(x) for x in entry.split(','))
            if sites.setdefault(k, w) != w:
                raise SystemExit(f'src/{stem}.c: two operands for {k // 1024:02o},{k % 1024 + 0o2000:04o}')
    return sites


def check_site_includers():
    """Each operand file (an .inc defining or listing agc_site_operand
    entries) has exactly one including source, whose module carries it."""
    users = collections.defaultdict(list)
    for c in sorted((ROOT / 'src').glob('*.c')):
        for inc in re.findall(r'#\s*include\s+"([^"]+\.inc)"', c.read_text(errors='replace')):
            users[inc].append(c.name)
    for inc in sorted((ROOT / 'src').glob('*.inc')):
        text = inc.read_text(errors='replace')
        if re.search(r'agc_site_operand\s+\w+\s*\[', text) or re.search(r'_(sites|operands)\.inc$', inc.name):
            if len(users[inc.name]) != 1:
                raise SystemExit(f'src/{inc.name}: operand file included by {users[inc.name] or "nothing"}, '
                                 'not exactly one source')


# Everything the build reads: sources, headers, platform code, the tools it
# imports and runs, the docs tables and the assembled listings. The C
# compilers are named by path, size and mtime.
STAMP_INPUTS = ('src', 'include', 'platform', 'tools', 'docs', 'build/fixed-data')


def input_digest(a):
    h = hashlib.sha256(repr((a.cc, a.host_cc, a.missions, a.host_only, os.environ.get('RK_FLIGHT_OUT'))).encode())
    for tool in (a.cc, a.host_cc, NM):
        try:
            st = os.stat(tool if os.sep in str(tool) else subprocess.run(
                ['sh', '-c', f'command -v {tool}'], capture_output=True, text=True).stdout.strip() or tool)
            h.update(f'{tool}:{st.st_size}:{st.st_mtime_ns}'.encode())
        except OSError:
            h.update(f'{tool}:missing'.encode())
    for top in STAMP_INPUTS:
        for f in sorted((ROOT / top).rglob('*')):
            if f.is_file() and '__pycache__' not in f.parts:
                h.update(f.relative_to(ROOT).as_posix().encode() + b'\0' + f.read_bytes())
    return h.hexdigest()


def output_digests(since):
    return {f.relative_to(ROOT / OUT).as_posix(): hashlib.sha256(f.read_bytes()).hexdigest()
            for f in sorted((ROOT / OUT).rglob('*'))
            if f.is_file() and not f.name.startswith(('.stamp-', '.lock')) and f.stat().st_mtime >= since}


def up_to_date(stamp, digest):
    """The last build with these arguments read the same inputs, and every file
    it wrote is still as it left it (a --missions lm build rewrites the LM
    outputs a full build recorded, so the full build runs again)."""
    try:
        record = json.loads(stamp.read_text())
    except (OSError, ValueError):
        return False
    if record.get('inputs') != digest:
        return False
    for name, sha in record.get('outputs', {}).items():
        f = ROOT / OUT / name
        if not f.is_file() or hashlib.sha256(f.read_bytes()).hexdigest() != sha:
            return False
    return True


UNPLACED = 'docs/flight-store-unplaced.json'


def check_unplaced(m, unplaced):
    """The ratchet on routine keys no packed module holds (the C64 stops
    there; the host bridges them through yaAGC): per table, at most the
    count docs/flight-store-unplaced.json allows. A lower count asks for the
    file to be lowered with it. Returns the error, raised once report.json
    is written."""
    allowed = json.loads((ROOT / UNPLACED).read_text()).get(m, {})
    over = {t: (n, allowed.get(t, 0)) for t, n in unplaced.items() if n > allowed.get(t, 0)}
    lower = {t: (unplaced.get(t, 0), k) for t, k in allowed.items() if unplaced.get(t, 0) < k}
    for t, (n, k) in sorted(lower.items()):
        print(f'{m}: {t} has {n} unplaced keys, {k} allowed: lower {UNPLACED} to {n}')
    if over:
        return (f'{m}: routine keys with no module in the store (the C64 stops at them): '
                + ', '.join(f'{t} {n} (allowed {k})' for t, (n, k) in sorted(over.items()))
                + f'. Make them fit a slot (report.json "failed" says how each module failed), '
                f'or raise the count in {UNPLACED} with the reason in the PR.')
    return None


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--missions', default='lm,cm')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    p.add_argument('--host-only', action='store_true',
                   help='steps 1-3 only: the host index and build/flight-store/<m>-host.inc')
    p.add_argument('--force', action='store_true', help='build even when the last build is up to date')
    a = p.parse_args()
    (ROOT / OUT).mkdir(parents=True, exist_ok=True)
    # One build at a time in the tree: ci_run runs flight-store targets in
    # parallel, and two builds rewriting build/flight-store/keys at once fail
    # ("Text file busy"). The one that waits then finds the stamp current.
    lock = open(ROOT / OUT / '.lock', 'w')
    fcntl.flock(lock, fcntl.LOCK_EX)
    stamp = ROOT / OUT / ('.stamp-' + re.sub(r'[^a-z,]', '', a.missions).replace(',', '-')
                          + ('-host' if a.host_only else ''))
    digest = input_digest(a)
    if not a.force and up_to_date(stamp, digest):
        print(f'{OUT}: up to date ({stamp.name}; --force rebuilds)')
        return 0
    stamp.unlink(missing_ok=True)
    started = time.time() - 2
    index, objects = host_index(a.host_cc, a.jobs)
    declared = table_names()
    tables = {s: o for o, (d, _) in index.items() for s, k in d.items()
              if k in 'DR' and (s in declared or TABLE_NAME.search(s))}
    keys = table_keys(a.host_cc, tables, objects)
    owners, place = source_owners(), placements()
    if not a.host_only:
        run([sys.executable, 'tools/overlay_v2_services.py', '--out-dir', OUT])
        library = prepare(a.cc, ROOT / OUT)
    report = {}
    errors = []
    for m in a.missions.split(','):
        mine = {t: o for t, o in tables.items() if mission_of(t, o) == m}
        skipped = {t: 'resident' for t in RESIDENT[m]} | {t: 'replaced by the thin kernel' for t in REPLACED[m]}
        packed = {t: o for t, o in mine.items() if t not in skipped and keys[t]}
        empty = sorted(t for t, o in mine.items() if t not in skipped and not keys[t])
        for stem in sorted(FLIGHT_INTERP_STOP[m] - set(FLIGHT_PARTS[m])):
            keys.update(flight_keys(a.host_cc, stem, sorted(t for t, o in mine.items()
                                                             if o == stem and not interpretive_table(t))))
        resident_keys = {('i' if interpretive_table(t) else 'b', k) for t in RESIDENT[m] for k in keys.get(t, [])}
        # Key -> tables, in resolution preference order.
        holders = collections.defaultdict(list)
        parts = {stem: part_keys(a.host_cc, m, stem, sorted(t for t, o in packed.items()
                                                           if o == stem and not interpretive_table(t)))
                 for stem in FLIGHT_PARTS[m] if stem in packed.values()}
        for t in sorted(packed, key=lambda t: (min((i for _, i in owners.get(packed[t], [])), default=99), t)):
            if hot_full(t):
                continue
            kind = 'i' if interpretive_table(t) else 'b'
            more = [k for p in parts.get(packed[t], []) for k in p.get(t, [])]
            for k in sorted(set(keys[t]) | set(more)):
                holders[(kind, k)].append(t)
        collisions = collections.Counter(tuple(v) for v in holders.values() if len(v) > 1)
        shadowed = sum(1 for k in holders if k in resident_keys)
        order = list(dict.fromkeys(RESIDENT[m] + sorted(
            (t for t in packed if not hot_full(t)), key=lambda t: (min((i for _, i in owners.get(packed[t], [])), default=99), t))))
        check_site_includers()
        also = collections.defaultdict(list)
        for host, ts in CO_LOCATE[m].items():
            for t in ts:
                also[t].append(host)

        def site_stem(t):
            if t in RIDE_SITES and RIDE_SITES[t] in tables:
                return tables[RIDE_SITES[t]]
            return RESIDENT_SITES[m] if t in RESIDENT[m] else tables[t]
        stems = sorted({site_stem(t) for t in order + [t for ts in PIN[m].values() for t in ts]
                        + [t for _, ts, _ in PACKS[m] for t in ts] if t in tables})
        (ROOT / OUT / f'{m}-sites.inc').write_text(
            f'/* Generated by tools/flight_store_build.py: the inline operands each {m.upper()} source '
            'compiles (its agc_site_operand arrays), sorted; a module carries its sources\' sets. */\n'
            + ''.join(f'static const agc_site_operand sites_{st}[]={{' + ','.join(
                f'{{{k}u,0{w:o}u}}' for k, w in sorted(file_sites(a.host_cc, m, st).items()))
                      + (f'}};\n' if file_sites(a.host_cc, m, st) else '{AGC_TRANSFER_NONE,0u}};\n') for st in stems))
        def write_host(pins, packs):
            """build/flight-store/<m>-host.inc. pins: {table: slot class 3
            or 4} for the pinned sets the C64 loads at boot and runs through
            rk_direct before any module: they go right after the resident,
            never load and never take a slot. With a class-4 set in, every
            task-slot module links for the job slot (task_pinned_out).
            packs: {table: (pack name, ride)} for the job-slot packs
            (PACKS) that linked. The resolver sends a pack table's keys to
            the pack first (a hot subset ahead of its full table, the
            resident's included, as the wrappers move); a ride table runs
            only while its pack is loaded (slot 5: the host skips it
            otherwise)."""
            task_out = 4 in pins.values()
            pin_order = [t for t in pins if t in tables]
            ahead = [t for t in packs if t in tables and (packs[t][1] or hot_full(t))]
            first = [t for t in ahead if hot_full(t) in RESIDENT[m]]

            def slot_of(t):
                if t in pins:
                    return pins[t]
                if t in packs:
                    return 5 if packs[t][1] else 0
                if t not in packed or t in RESIDENT[m]:
                    return 0
                return 0 if task_out else lane_slot(packed[t], m, owners, place)[1]

            def module_of_table(t):
                if t in pins:
                    return 'pin-' + SLOT_NAME[pins[t]]
                if t in packs:
                    return packs[t][0]
                return packed.get(t, '') if t not in RESIDENT[m] else ''
            (ROOT / OUT / f'{m}-host.inc').write_text(
                f'/* Generated by tools/flight_store_build.py: the {m.upper()} flight program\'s tables, '
                'resident first, then the pinned sets, in resolution order: table, interpretive, io kind, '
                'module (empty when resident), slot (0 job, 1 task, 3 and 4 pinned: never loaded), '
                'operand set, its size, other modules holding the table, '
                'fixed_data (0 boundary words, 1 self-check, 2 rope), work clock MCT per routine. */\n'
                + ''.join(f'FLIGHT_TABLE({t}, {int(interpretive_table(t))}, '
                          f'FLIGHT_IO_{io_kind(m, tables[t])}, '
                          f'"{module_of_table(t)}", '
                          f'{slot_of(t)}, '
                          f'sites_{site_stem(t)}, {len(file_sites(a.host_cc, m, site_stem(t)))}u, '
                          f'"{" ".join(also[t])}", '
                          f'{FIXED_KIND.get(FIXED_DATA.get(packed.get(t)), 1) if t not in RESIDENT[m] else 0}, '
                          f'{host_mct(m, t)}u)\n'
                          for t in list(dict.fromkeys(first + RESIDENT[m] + pin_order + ahead + order)) if t in tables))
        write_host(host_pins(m, packed), host_packs(m, packed))
        if a.host_only:
            continue
        to_interp = {t for t, o in packed.items()
                     if t in INTERP_ONLY[m] or (interpretive_table(t) and not t.endswith('_marker'))
                     or (o in FLIGHT_INTERP_STOP[m] and interpretive_table(t))}
        # Shadowed entries: a basic table's keys that the resident or an
        # earlier linked table in resolution order (holders, less the
        # interpreter's) holds never resolve to it, so its module links only
        # its live entries (FS_PICK, live). Flight parts keep their key
        # slices.
        resolves_to = {k: next((t for t in ts if t not in to_interp), None) for k, ts in holders.items()}
        live = {}
        for t in sorted({t for (kind, _), ts in holders.items() if kind == 'b' for t in ts} - to_interp):
            if packed[t] in FLIGHT_PARTS[m]:
                continue
            keep = {i for i, k in enumerate(keys[t])
                    if k >= ERASABLE_BASE or ('b', k) not in resident_keys and resolves_to[('b', k)] == t}
            if len(keep) < len(keys[t]):
                live[t] = keep
        by_stem = collections.defaultdict(list)
        for t, o in packed.items():
            if t not in to_interp and not hot_full(t):
                by_stem[o].append(t)
        modules, seq = [], collections.Counter()
        for stem in sorted(by_stem):
            lane, slot, agc = lane_slot(stem, m, owners, place)
            seq[lane] += 1
            if seq[lane] > LANE_LIMIT.get(lane, 0xFF):
                raise SystemExit(f'{m}: lane {lane} has more than {LANE_LIMIT.get(lane, 0xFF)} modules')
            ts = sorted(by_stem[stem])
            extra = [t for t in CO_LOCATE[m].get(stem, []) if t in packed]
            sources = list(dict.fromkeys([stem] + [packed[t] for t in extra]))
            sites = {}
            for st in sources:
                sites.update(file_sites(a.host_cc, m, st))
            mod = dict(stem=stem, agc=agc, lane=lane, slot=slot,
                       basic=[t for t in ts if not interpretive_table(t)] + [t for t in extra if not interpretive_table(t)],
                       interp=[t for t in ts if interpretive_table(t)] + [t for t in extra if interpretive_table(t)],
                       also=extra, sites=sites, fixed_data=FIXED_DATA.get(stem), io=module_io(m, sources),
                       sources=c64_sources(index, *sources, 'agc_transfer'))
            for i, defines in enumerate(FLIGHT_PARTS[m].get(stem, [()])):
                if i:
                    seq[lane] += 1
                    if seq[lane] > LANE_LIMIT.get(lane, 0xFF):
                        raise SystemExit(f'{m}: lane {lane} has more than {LANE_LIMIT.get(lane, 0xFF)} modules')
                part = dict(mod, id=LANE_BYTE[lane] << 8 | seq[lane])
                if defines:
                    if mod['interp'] or extra:
                        raise SystemExit(f'{m}: {stem} parts link basic tables only')
                    part.update(defines=list(defines), part=i + 1, prg_name=f'{stem}-part{i + 1}',
                                key_slice=parts[stem][i])
                modules.append(part)
        (ROOT / OUT / m).mkdir(exist_ok=True)

        def pick_of(mod):
            """The module's live entries in key order, or None when it links
            every entry of its tables (FS_BASIC or FS_CHUNK as before)."""
            if mod.get('defines') or mod.get('excursion') or mod['interp']:
                return None
            if mod.get('chunk'):
                t, lo, hi = mod['chunk']
                spans = [(t, range(lo, hi))]
            else:
                spans = [(t, range(len(keys[t]))) for t in mod['basic']]
            if not any(t in live and t not in mod['also'] for t, _ in spans):
                return None
            return sorted(((t, i) for t, r in spans for i in r
                           if t in mod['also'] or t not in live or i in live[t]), key=lambda e: keys[e[0]][e[1]])

        def link(mod):
            prg = OUT / m / f'{mod.get("prg_name") or "module-%04x" % mod["id"]}.prg'
            pick = pick_of(mod)
            if pick == []:
                return mod, prg, dict(ok=False, overflow=None, error=None, empty=True)
            # A module on the base record links no transfer engine: the
            # resident runs its routines through rk_direct (FS_DIRECT_ONLY).
            direct_only = mod.get('direct_only', not mod.get('io'))
            # The high bytes of the keys its basic tables hold, for
            # rk_direct's range check, which sees an erasable Z as 0xff; a
            # module with interpretive tables can run at an interpretive
            # point, so it takes every point.
            c = mod.get('chunk')
            ks = ([keys[t][i] for t, i in pick] if pick is not None else keys[c[0]][c[1]:c[2]] if c
                  else [k for t in mod['basic'] for k in keys.get(t, [])])
            rng = ((min(ks) >> 8, 0xFF if max(ks) >= ERASABLE_BASE else max(ks) >> 8)
                   if ks and not mod['interp'] else (0, 0xFF))
            res = link_module(a.cc, m, prg, mod['slot'], mod['id'], mod['sources'], mod['basic'], mod['interp'],
                              library, mod.get('chunk'), mod.get('sites'), mod.get('fixed_data'), mod.get('io'),
                              direct_only, mod.get('defines', ()), mod.get('link_flags', ()), pick, keys, rng)
            if not res['ok'] and res.get('overflow') and mod['slot'] == 1:
                # A task-slot module that does not fit tries the job slot.
                res2 = link_module(a.cc, m, prg, 0, mod['id'], mod['sources'], mod['basic'], mod['interp'], library,
                                   mod.get('chunk'), mod.get('sites'), mod.get('fixed_data'), mod.get('io'),
                                   direct_only, mod.get('defines', ()), (), pick, keys, rng)
                if res2['ok']:
                    res = dict(res2, moved_to_job_slot=True, task_overflow=res['overflow'])
                    mod['slot'] = 0
            return mod, prg, res
        # The packs (excursion, T4RUPT) link once on their own first, so
        # pin_pair knows the names they link: those the class-4 home links
        # anyway stay global there (--undefined), and the packs take them
        # from the home as class 3 does.
        packs, exc_ask = [], set()
        for name, pts, ident in PACKS[m]:
            pts = [t for t in pts if t in packed]
            if not pts:
                continue
            stems = list(dict.fromkeys(packed[t] for t in pts))
            lane = lane_slot(stems[0], m, owners, place)[0]
            sites = {}
            for st in stems:
                sites.update(file_sites(a.host_cc, m, st))
            banks = {k >> 10 for t in pts if t not in RIDE for k in keys[t] if k < ERASABLE_BASE}
            for t in pts:
                if RIDE_SITES.get(t) in packed:
                    sites.update({k: w for k, w in file_sites(a.host_cc, m, packed[RIDE_SITES[t]]).items()
                                  if k >> 10 in banks})
            pack = dict(stem=stems[0], lane=lane, slot=0, id=ident, basic=pts,
                        interp=[], also=[], sites=sites, io=module_io(m, stems), excursion=True,
                        sources=c64_sources(index, *stems, 'agc_transfer'), prg_name=name)
            (ROOT / OUT / m / f'{name}.prg.map').unlink(missing_ok=True)
            link(dict(pack))
            exc_ask |= {n for n in map_symbols(ROOT / OUT / m / f'{name}.prg.map')
                        if not n.startswith(('apollo_ov2_', 'rk_module_tables', '__ov2')) and n not in pts}
            packs.append(pack)
        # The pinned sets, measured: one module per slot with the set's
        # tables and its files' operand sites (a fit as linked; an overflow
        # beyond the slot otherwise). Class 4 links first: beside it, class
        # 3 links only its own files and takes the rest from class 4's map
        # (pin_pair), and its table list names both sets.
        pinned, params = [], {}

        def pin_pair(home, ts, measure, sources):
            """Class 3 beside the class-4 module, which homes the runtime
            both use: class 3 links only its own files (sources less the
            home's) and its table list names the home's tables too. Every
            global function class 3 links is the home's: the home links
            class 3's files too but keeps only those (--undefined; LTO and
            --gc-sections drop the rest), and class 3 takes their addresses
            from the home's map (--defsym). That moves the runtime and the
            DAP files' shared helpers (include/lm_dap_shared.h) into class 4,
            where the job-slot DAP modules can share them as well.
            Relinks both until class 3 resolves. Returns class 3's link
            result, its bytes and the shared symbols."""
            hp = params[f'pin-{SLOT_NAME[4]}']
            own = [f for f in sources if f not in set(hp['sources'])]
            basic = [t for t in ts if not interpretive_table(t)] + hp['basic']
            (ROOT / OUT / m / f'pin-{SLOT_NAME[3]}.prg.map').unlink(missing_ok=True)
            res, nbytes = measure(None, True, f'pin-{SLOT_NAME[3]}', (), own, basic)

            def asked():
                # Names class 3 linked (its map; a link over the slot still
                # writes one), less the module's own entry and table list.
                return {n for n in map_symbols(ROOT / OUT / m / f'pin-{SLOT_NAME[3]}.prg.map')
                        if not n.startswith(('apollo_ov2_', 'rk_module_tables', '__ov2')) and n not in ts}
            ask = asked()
            want, exc = set(), set()
            for _ in range(8):
                hres = link(dict(hp, sources=list(dict.fromkeys(hp['sources'] + own)),
                                 link_flags=PIN_LTO + [f'-Wl,--undefined={u}' for u in sorted(ask | want | exc)]))[2]
                home.update(ok=hres['ok'], bytes=hres.get('bytes') or (
                    SLOT_BYTES[4] + hres['overflow'] if hres.get('overflow') else None))
                if not hres['ok']:
                    return hres, None, want
                nm = subprocess.run([str(NM), str(ROOT / home['prg']) + '.elf'], capture_output=True,
                                    text=True, check=True).stdout
                sym = {p[2]: p for p in (l.split() for l in nm.splitlines()) if len(p) == 3 and p[1].isupper()}
                addr = {n: p[0] for n, p in sym.items()}
                # Asked: functions only (a same-named object is each
                # module's own); wanted: whatever class 3 left undefined.
                share = sorted({n for n in ask if n in sym and sym[n][1] in 'TW'} | (want & set(addr)))
                home['shared'] = len(share)
                flags = ['-Wl,--error-limit=0'] + (
                    ['-Wl,' + ','.join(f'--defsym={u}=0x{addr[u]}' for u in share)] if share else [])
                res, nbytes = measure(None, True, f'pin-{SLOT_NAME[3]}', flags, own, basic)
                # A failed link may leave the last link's map: read only this one's.
                more = (asked() if res['ok'] or res.get('overflow') else set()) - ask - set(share)
                # The packs' names the home links anyway stay global there;
                # names only a pack uses stay in the pack.
                held = exc_ask & map_symbols(ROOT / OUT / m / f'pin-{SLOT_NAME[4]}.prg.map') - exc
                exc |= held
                if not set(res.get('undefined') or ()) - want and not more and not held:
                    return res, nbytes, share
                want |= set(res.get('undefined') or ())
                ask |= more
            return res, nbytes, share
        for slot in sorted(PIN[m], reverse=True):
            ts = [t for t in PIN[m][slot] if t in packed]
            if not ts:
                continue
            stems = list(dict.fromkeys(packed[t] for t in ts))
            lane = lane_slot(stems[0], m, owners, place)[0]
            sites = {}
            for st in stems:
                sites.update(file_sites(a.host_cc, m, st))
            # A pinned module runs only through rk_direct (no transfer
            # engine), which hands it the base record; one whose tables take
            # another io kind links its conversion and engine until that kind
            # folds into agc_hardware, and direct_bytes projects the set
            # without them.
            io = module_io(m, stems)
            size = SLOT_BYTES[slot]
            home = next((p for p in pinned if p['slot'] == 'pinned_task' and p['ok'] and not p['io']), None) \
                if slot == 3 else None
            if home:
                for st in home['stems']:
                    sites.update(file_sites(a.host_cc, m, st))

            def measure(io, direct_only, name, flags=(), sources=None, basic=None):
                mod = dict(stem=stems[0], slot=slot, id=LANE_BYTE[lane] << 8 | 0xF0 | slot,
                           basic=basic or [t for t in ts if not interpretive_table(t)],
                           interp=[t for t in ts if interpretive_table(t)], also=[], sites=sites,
                           sources=sources or c64_sources(index, *stems, 'agc_transfer'), io=io,
                           direct_only=direct_only, prg_name=name, link_flags=PIN_LTO + list(flags),
                           defines=['-DFS_PIN_HOME=1'] if slot == 4 and 3 in PIN[m] else [])
                params[name] = mod
                res = link(dict(mod))[2]
                if res.get('moved_to_job_slot'):  # fits the job slot, not the task slot it is pinned in
                    res = dict(ok=False, overflow=res['task_overflow'])
                return res, res.get('bytes') or (size + res['overflow'] if res.get('overflow') else None)
            if home:
                res, nbytes, shared = pin_pair(home, ts, measure, c64_sources(index, *stems, 'agc_transfer'))
                direct = nbytes
            else:
                res, nbytes = measure(io, not io, f'pin-{SLOT_NAME[slot]}')
                direct = measure(None, True, f'pin-{SLOT_NAME[slot]}-direct')[1] if io else nbytes
                shared = []
            pinned.append(dict(slot=SLOT_NAME[slot], tables=ts + (home['tables'] if home else []), own=ts,
                               stems=stems, ok=res['ok'], bytes=nbytes,
                               direct_only=not io, io=io, direct_bytes=direct, slot_bytes=size,
                               error=res.get('error'), id=LANE_BYTE[lane] << 8 | 0xF0 | slot,
                               prg=str(OUT / m / f'pin-{SLOT_NAME[slot]}.prg'), shared=len(shared),
                               packed=slot in (3, 4) and res['ok'] and not io))
        # Class 4 runs only through the class-3 module's table list, so it
        # goes into the store only beside a packed class-3 module. Then it
        # holds $E000, and every task-slot module links for the job slot.
        if not any(p['packed'] and p['slot'] == 'pinned' for p in pinned):
            for p in pinned:
                if p['slot'] == 'pinned_task':
                    p['packed'] = False
        pin_task = any(p['packed'] and p['slot'] == 'pinned_task' for p in pinned)
        if pin_task:
            for mod in modules:
                if mod['slot'] == 1:
                    mod.update(slot=0, task_pinned_out=True)
        home = next((p for p in pinned if p['packed'] and p['slot'] == 'pinned_task'), None)
        if not home:
            packs = []
        for pack in packs:
            # Every function it links that the home holds as a global is the
            # home's (--defsym from the home's map), as for class 3.
            nm = subprocess.run([str(NM), str(ROOT / home['prg']) + '.elf'], capture_output=True,
                                text=True, check=True).stdout
            addr = {p[2]: p[0] for p in (l.split() for l in nm.splitlines()) if len(p) == 3 and p[1] in 'TW'}
            share = sorted(n for n in map_symbols(ROOT / OUT / m / f"{pack['prg_name']}.prg.map") & set(addr)
                           if not n.startswith(('apollo_ov2_', 'rk_module_tables', '__ov2')))
            pack['shared'] = len(share)
            if share:
                pack['link_flags'] = ['-Wl,' + ','.join(f'--defsym={u}=0x{addr[u]}' for u in share)]
            modules.append(pack)
        with ThreadPoolExecutor(a.jobs) as pool:
            linked = list(pool.map(link, modules))
        # A basic-only module over its slot is split into chunks of
        # consecutive routines, halving until each chunk fits. Splitting stops
        # (the module stays whole and failed) when a chunk of one routine
        # does not fit: its routines share one body (a file-wide step
        # machine) that every chunk links. Modules over SPLIT_LIMIT are not
        # tried. Trial links use a scratch id; kept chunks are relinked with
        # their own. Interpretive modules wait on the interpreter decision.
        chunks = []

        def own(mod):
            """A chunk's module: its file alone, without co-located tables."""
            return dict(mod, also=[], sources=c64_sources(index, mod['stem'], 'agc_transfer'),
                        sites=file_sites(a.host_cc, m, mod['stem']))

        def trial(c, mod):
            return link(dict(own(mod), id=LANE_BYTE[mod['lane']] << 8 | 0xFF, basic=[], chunk=c, chunk_of=mod['id'],
                             prg_name=f'trial-{c[0]}-{c[1]}'))[2]

        def split_greedy(mod):
            """As split, but each chunk takes as many consecutive routines as fit
            (binary search on its end), so a pass crosses fewer chunks."""
            kept, over = [], []
            for t in [t for t in mod['basic'] if t not in mod['also']]:
                lo, n = 0, len(keys[t])
                while lo < n:
                    r = trial((t, lo, n), mod)
                    if r['ok'] or r.get('empty'):
                        kept.append((t, lo, n)); break
                    if not r.get('overflow'):
                        return None
                    good, bad = lo, n          # (t, lo, good) fits (empty when good == lo), (t, lo, bad) does not
                    while bad - good > 1:
                        mid = (good + bad) // 2
                        r = trial((t, lo, mid), mod)
                        if r['ok'] or r.get('empty'):
                            good = mid
                        elif not r.get('overflow'):
                            return None
                        else:
                            bad = mid
                    if good == lo:
                        over.append(((t, lo, lo + 1), r.get('overflow', 0))); lo += 1
                    else:
                        kept.append((t, lo, good)); lo = good
            return (kept, over) if kept else None

        def split(mod):
            """(chunks that fit, [(one-routine chunk, overflow)] that do not), or None."""
            if mod['stem'] in GREEDY_SPLIT[m]:
                return split_greedy(mod)
            # Co-located tables stay whole in their own module.
            todo, kept, over = [(t, 0, len(keys[t])) for t in mod['basic'] if t not in mod['also']], [], []
            while todo:
                with ThreadPoolExecutor(a.jobs) as pool:
                    results = list(pool.map(lambda c: trial(c, mod), todo))
                nxt = []
                for c, r in zip(todo, results):
                    if r['ok'] or r.get('empty'):
                        kept.append(c)
                    elif not r.get('overflow'):
                        return None
                    elif c[2] - c[1] == 1:
                        over.append((c, r['overflow']))
                    else:
                        half = (c[1] + c[2]) // 2
                        nxt += [(c[0], c[1], half), (c[0], half, c[2])]
                todo = nxt
            return (kept, over) if kept else None
        for mod, prg, res in linked:
            if res['ok'] or not res.get('overflow') or mod['interp'] or mod.get('defines') or mod.get('excursion'):
                continue
            if res['overflow'] > SPLIT_LIMIT:
                mod['chunking'] = 'too big to split'
                continue
            got = split(mod)
            if not got or seq[mod['lane']] + len(got[0]) - 1 > LANE_LIMIT.get(mod['lane'], 0xFF):
                mod['chunking'] = 'every routine over its slot' if got is None else 'no module ids left'
                continue
            kept, over = got
            subs = []
            for i, c in enumerate(sorted(kept)):
                if i:
                    seq[mod['lane']] += 1
                ident = mod['id'] if i == 0 else LANE_BYTE[mod['lane']] << 8 | seq[mod['lane']]
                subs.append(dict(own(mod), id=ident, basic=[], chunk=c, chunk_of=mod['id']))
            with ThreadPoolExecutor(a.jobs) as pool:
                chunks += list(pool.map(link, subs))
            # A routine over its slot alone stays unplaced and is reported.
            for c, overflow in over:
                chunks.append((dict(own(mod), basic=[], chunk=c, chunk_of=mod['id'],
                                    chunking='one routine over its slot'),
                               None, dict(ok=False, overflow=overflow)))
            mod['chunked'] = True
        linked = [x for x in linked if not x[0].get('chunked')] + sorted(chunks, key=lambda x: x[0]['id'])
        # Modules (or chunks) whose every entry is shadowed are left out.
        pruned = [x[0]['id'] for x in linked if x[2].get('empty')]
        linked = [x for x in linked if not x[2].get('empty')]
        modules = [mod for mod, _, _ in linked]
        module_of = {}
        for mod, prg, res in linked:
            mod.update(res)
            mod['prg'] = str(prg)
            if res['ok']:
                if mod.get('chunk'):
                    t, first, last = mod['chunk']
                    for i in range(first, last):
                        if t not in live or i in live[t]:
                            module_of[(t, keys[t][i])] = mod['id']
                    continue
                for t in mod['basic'] + mod['interp']:
                    if t in mod.get('also', []):
                        continue  # resolved to the table's own module
                    if mod.get('key_slice'):
                        for k in mod['key_slice'][t]:
                            module_of[(t, k)] = mod['id']
                        continue
                    for i, k in enumerate(keys[t]):
                        if t not in live or i in live[t] or mod.get('excursion'):
                            module_of[(t, k)] = mod['id']
        # A pack's keys resolve to it first: its hot entries are its files'
        # tables' (hot_full), so the same routines run.
        for pack in packs:
            if pack.get('ok'):
                for t in pack['basic']:
                    if t in RIDE:
                        continue
                    for k in keys[t]:
                        module_of[(hot_full(t) or t, k)] = pack['id']
        write_host(packed_pins(pinned), pack_tables(pack_report(packs)))
        records, erasable, unplaced, interpreted = [], 0, 0, 0
        unplaced_by = {}
        for (kind, k), ts in sorted(holders.items()):
            if (kind, k) in resident_keys:
                continue
            if all(t in to_interp for t in ts):
                interpreted += 1  # the interpreter's: reached by Z at NEWOPS
                continue
            if k >= ERASABLE_BASE:
                erasable += 1
                continue
            owner = next((module_of[(t, k)] for t in ts if (t, k) in module_of), None)
            if owner is None:
                unplaced += 1
                unplaced_by.setdefault(sorted(ts)[0], []).append(k)
                continue
            bank, loc = canonical_record(k)
            records.append(dict(bank=bank, loc=loc, module=owner, selector=0, interpretive=kind == 'i'))
        # One record per (bank, loc): a basic and an interpretive key at the
        # same address cannot both be resolved by Z (docs/FLIGHT-STORE.md).
        seen, clash = {}, []
        for r in records:
            k = (r['bank'], r['loc'])
            if k in seen:
                clash.append((seen[k], r))
                continue
            seen[k] = r
        resolver = [dict(bank=r['bank'], loc=r['loc'], module=r['module'], selector=0) for r in seen.values()]
        (ROOT / OUT / f'{m}-resolver.json').write_text(json.dumps(resolver) + '\n')
        good = [prg for mod, prg, res in linked if res['ok']]
        # The class-3 set, packed, and one of its points for the load check:
        # a fixed key of its first table and the module that packs that
        # table (the same routines; the resolver may prefer another holder,
        # which can run on past them).
        pin_check = None
        for p in pinned:
            if not p['packed']:
                continue
            good.append(Path(p['prg']))
            if p['slot'] == 'pinned_task':
                continue  # its tables run through the class-3 list
            t = p['tables'][0]
            full = hot_full(t) or t  # a hot subset's keys are its full table's
            k = next((k for k in keys[t] if k < ERASABLE_BASE and (full, k) in module_of), None)
            if k is not None:
                bank, loc = canonical_record(k)
                pin_check = dict(id=p['id'], bank=bank, loc=loc, module=module_of[(full, k)])
        if pin_check and pin_task:
            pin_check['task_id'] = next(p['id'] for p in pinned if p['packed'] and p['slot'] == 'pinned_task')
        # The resident's own modules, so the store boots the flight resident:
        # the interrupt-body wrappers and their resolver records, and the
        # boundary words for every key the resident can stop at.
        wrappers = wrapper_records(a.host_cc, m)
        # A pack holding a wrapped table's hot subset takes its keys from
        # the wrapper (T4RUPT's core from 0x01E2).
        moved = {canonical_record(k): pack['id'] for pack in packs if pack.get('ok')
                 for t in pack['basic'] if t not in RIDE and (hot_full(t) or t) in RESIDENT[m] for k in keys[t]}
        wrappers = [dict(r, module=moved.get((r['bank'], r['loc']), r['module'])) for r in wrappers]
        # DOWNRUPT's vector goes to the down-telemetry module that carries
        # its ends; RESUME stays with its wrappers (only that run reaches the
        # module's copy).
        stem, ends = DOWNRUPT_HOST[m]
        host = next((mod['id'] for mod, _, res in linked if mod['stem'] == stem and res['ok']
                     and ends in mod.get('also', []) and not mod.get('part')), None)
        if host is not None:
            vector = canonical_record(DOWNRUPT_VECTOR)
            wrappers = [dict(r, module=host) if (r['bank'], r['loc']) == vector else r for r in wrappers]
            # A run that stops on the work-clock budget inside the chain
            # resumes in the same module. Its fixed points have records; an
            # unswitched erasable point (DNTMGOTO) gets one under each FB its
            # table runs in (bank 05), where no fixed key can be. The resolver
            # keys it by the FB at the stop: every TC DNTMGOTO is in bank 05
            # code (DODOWNTM after the vector's CAF DNRPTBB; XCH BBANK, and the
            # phases), none of which writes FB.
            table = f'{m}_downtelemetry_routines'
            fbs = sorted({k >> 10 for k in keys[table] if k < ERASABLE_BASE})
            taken = {(r['bank'], r['loc']) for r in resolver + wrappers}
            for k in keys[table]:
                if ERASABLE_BASE <= k < ERASABLE_BASE + 0o1400:
                    for fb in fbs:
                        record = dict(bank=fb << 10, loc=k - ERASABLE_BASE, module=host, selector=0)
                        if (record['bank'], record['loc']) in taken:
                            raise SystemExit(f'{m}: DOWNRUPT point {record} already has a record')
                        wrappers.append(record)
        # The interpreter's modules and its points' records, where no
        # table record or resident table already holds the point.
        interp_prgs, interp = interp_modules(a.cc, m, library)
        good += interp_prgs
        # INTPRET goes to the interpreter even where a table record holds
        # it: such a table (lanec_ev_lm_intpret) enters only its own file's
        # callers and refuses the rest, which stops the step, while the
        # interpreter enters every caller. The table still runs from its
        # module when that is loaded (rk_direct).
        if interp_prgs:
            z = INTERP_POINTS[m]['INTPRET']
            resolver = [r for r in resolver if (r['bank'], r['loc']) != (0o6000 if z >= 0o6000 else 0o4000, z)]
        taken = {(r['bank'], r['loc']) for r in resolver + wrappers}
        # The DANZIG marker is not a routine: the flight resident's own
        # program has no marker, so DANZIG still needs the interpreter's record.
        held = {k for t in RESIDENT[m] if not t.endswith('danzig_marker') for k in keys.get(t, [])
                if ('b', k) in resident_keys}
        interp_records, interp_kept = [], {}
        for name, z in INTERP_POINTS[m].items():
            bank = 0o6000 if z >= 0o6000 else 0o4000
            physical = (bank >> 10) * 1024 + (z & 0o1777)
            if not interp_prgs or (bank, z) in taken or physical in held:
                interp_kept[name] = 'resident' if physical in held else 'table' if (bank, z) in taken else 'unlinked'
                continue
            interp_records.append(dict(bank=bank, loc=z, module=INTERP_MODULE, selector=0))
        wrappers = wrappers + interp_records
        boot = resident_modules(a.cc, a.host_cc, m, resolver + wrappers, library, pin_task)
        (ROOT / OUT / f'{m}-boot-resolver.json').write_text(json.dumps(resolver + wrappers) + '\n')
        pack = subprocess.run([sys.executable, 'tools/overlay_v2_pack.py', *good, *boot, '--mission', m.upper(),
                    '--services', hex(SERVICES), '--resolver', OUT / f'{m}-boot-resolver.json',
                    '--tail', OUT / f'{m}-boundary.bin',
                    '--output', OUT / f'{m}.reu', '--manifest', OUT / f'{m}-manifest.json'],
                   cwd=ROOT, capture_output=True, text=True)
        if pack.returncode:
            raise SystemExit(f'{m}: overlay_v2_pack.py failed: {pack.stderr.strip()}')
        add_rope(m)
        store = json.loads((ROOT / OUT / f'{m}-manifest.json').read_text())
        link_load_check(a.cc, m, resolver + wrappers, [mod for mod, _, res in linked if res['ok']],
                        {k for t in packed for k in keys[t]} | {k for t in RESIDENT[m] for k in keys.get(t, [])}
                        | {k for ps in parts.values() for p in ps for v in p.values() for k in v},
                        pin_check)
        failed = [dict(stem=mod['stem'], id=f'0x{mod["id"]:04x}', slot=mod['slot'], overflow=mod.get('overflow'),
                       error=mod.get('error'), chunking=mod.get('chunking'), chunk=mod.get('chunk'),
                       routine_key=keys[mod['chunk'][0]][mod['chunk'][1]]
                       if mod.get('chunk') and mod['chunk'][2] - mod['chunk'][1] == 1 else None)
                  for mod in modules if not mod['ok']]
        report[m] = dict(
            tables=len(mine), packed_tables=len(packed), empty_tables=empty, skipped=skipped,
            keys=len(holders), keys_held_by_resident=shadowed, erasable_keys=erasable,
            unplaced_keys=unplaced, interpreter_keys=interpreted,
            unplaced={t: len(v) for t, v in sorted(unplaced_by.items())},
            shadowed_entries={t: len(keys[t]) - len(v) for t, v in sorted(live.items())},
            modules_left_out_shadowed=[f'0x{i:04x}' for i in pruned],
            left_to_interpreter=sorted(to_interp), resolver_records=len(resolver),
            basic_interpretive_same_address=len(clash),
            collisions=[dict(tables=list(k), keys=v) for k, v in collisions.most_common()],
            modules=len(modules), modules_linked=len(good), failed=failed, boot_modules=[p.name for p in boot],
            pinned=pinned, packs=pack_report(packs),
            interpreter=dict(interp, records=len(interp_records), points_kept_elsewhere=interp_kept),
            store_bytes=store.get('store_bytes') or store.get('image_bytes'), pack=pack.stdout.strip(),
            module_list=[{k: mod.get(k) for k in ('stem', 'agc', 'lane', 'slot', 'id', 'bytes', 'basic', 'interp',
                                                    'chunk', 'chunk_of', 'also', 'fixed_data', 'moved_to_job_slot',
                                                    'task_overflow', 'task_pinned_out')} | dict(sites=len(mod.get('sites') or {}))
                         for mod in modules])
        if not a.host_only:
            errors += [e for e in [check_unplaced(m, report[m]['unplaced'])] if e]
        print(f"{m}: {len(packed)} tables in {len(modules)} modules ({len(good)} linked, {len(failed)} failed); "
              f"{len(resolver)} resolver records; {sum(r['keys'] for r in report[m]['collisions'])} colliding keys; "
              f"{pack.stdout.strip()}")
        print(f"{m}: interpreter " + (f"{interp['bytes']} + {interp['extension_bytes']} bytes, "
                                      f"{len(interp_records)} records" if interp['ok'] else f"does not link: {interp}")
              + (f" (held elsewhere: {', '.join(f'{k} {v}' for k, v in interp_kept.items())})" if interp_kept else ''))
        for p in pinned:
            where = {'pinned': 'class-3', 'pinned_task': 'class-4'}.get(p['slot'], p['slot'])
            print(f"{m}: pinned {where} slot set {' '.join(p['tables'])}: {p['bytes']} of {p['slot_bytes']} bytes"
                  f"{'' if p['ok'] else ' (does not fit)'}"
                  + (f"; {p['direct_bytes']} direct-only once {p['io']} folds into agc_hardware" if p['io'] else ''))
    if not a.host_only:
        (ROOT / OUT / 'report.json').write_text(json.dumps(report, indent=1) + '\n')
    if errors:
        raise SystemExit('\n'.join(errors))
    stamp.write_text(json.dumps(dict(inputs=digest, outputs=output_digests(started)), indent=1) + '\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
