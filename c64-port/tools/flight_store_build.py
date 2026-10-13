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
import functools
import json
import os
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from overlay_runtime_library import prepare  # noqa: E402
from resident_flight_build import COMMON, MISSION, SERVICES, canonical_record  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
OUT = Path('build/flight-store')
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
# The io record each source's tables take (include/flight_io.h), where it is
# not the mission's base record (LM p66_bias_io, CM cm_native_io).
IO_KIND = {
    'lm': {'lm_fresh_start': 'LM_FRESH', 'lm_dapidler': 'LM_FLIGHT', 'lm_q_r_axis': 'LM_FLIGHT',
           'lm_trim_gimbal': 'LM_FLIGHT', 'lm_landing_displays': 'LM_FLIGHT', 'lm_p_axis': 'LM_FLIGHT',
           'lm_throttle': 'LM_FLIGHT', 'lm_servicer': 'LM_SV', 'lm_lunar_landing': 'LM_LLD',
           'lm_imu_performance': 'LM_IPT', 'lm_burn_baby': 'LM_BB', 'lm_ags_init': 'LM_AGS',
           'imu_mode_switching': 'LM_IMU', 'lm_downtelemetry': 'LM_DNTM', 'lanec_ev_routines': 'LM_DNTM',
           'lanec_pin_routines': 'LM_DNTM'},
    'cm': {'cm_fresh_start': 'CM_FRESH', 'cm_v37': 'CM_FRESH', 'cm_downtelemetry': 'CM_DNTM',
           'lanec_ev_routines': 'CM_DNTM', 'lanec_pin_routines': 'CM_DNTM', 'cm_p21_routines': 'P66', 'cm_p76_routines': 'P66', 'cm_stateup': 'P66'},
}
BASE_IO = {'lm': 'P66', 'cm': 'CM_NATIVE'}
TABLE_DEF = re.compile(r'^const agc_routine_table\s+([A-Za-z0-9_]+)\s*=', re.M)


# The record type of each kind (include/flight_io.h flight_io_buffer).
IO_TYPE = {'P66': 'p66_bias_io', 'CM_NATIVE': 'cm_native_io', 'LM_FRESH': 'lm_fresh_io',
           'LM_FLIGHT': 'lm_flight_io', 'LM_SV': 'lm_sv_io', 'LM_LLD': 'lm_lld_io', 'LM_IPT': 'lm_ipt_io',
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

    def build(src):
        obj = objs / (src.stem + '.o')
        if not obj.exists() or obj.stat().st_mtime < src.stat().st_mtime:
            run([cc, '-std=c99', '-O0', '-Iinclude', '-Itests', '-c', src, '-o', obj])
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


def link_module(cc, m, prg, slot, ident, sources, basic, interp, library, chunk=None, sites=None, fixed_data=None):
    """chunk: (table, first, last) links only entries first..last-1 of one
    basic table (platform/c64/flight_table_module.c FS_CHUNK)."""
    wrapper = prg.with_suffix('.c')
    head = (f'#define FS_CHUNK_TABLE {chunk[0]}\n#define FS_CHUNK(X) '
            + ' '.join(f'X({i})' for i in range(chunk[1], chunk[2])) + '\n') if chunk else ''
    if sites:
        head += '#define FS_SITES ' + ','.join(f'{{{k}u,0{w:o}u}}' for k, w in sorted(sites.items())) + '\n'
    if fixed_data:
        head += f'#define FS_FIXED_DATA {fixed_data}\n'
    wrapper.write_text(
        head + f'#define FS_BASIC(X) {" ".join(f"X({t})" for t in basic)}\n'
        f'#define FS_INTERP(X) {" ".join(f"X({t})" for t in interp)}\n'
        + (f'#define FS_HAS_INTERP 1\n#define FS_DANZIG agc_{m}_danzig_marker\n' if interp else '')
        + '#include "../../../platform/c64/flight_table_module.c"\n')
    # Interpretive groups register the mission's DANZIG marker, which
    # agc_transfer.c defines on the C64 only when asked to.
    cmd = [cc, *COMMON, *MISSION[m], *(['-DAGC_TRANSFER_DANZIG_MARKER'] if interp else []),
           '-mllvm', '-inline-threshold=-50',
           '-ffunction-sections', '-fdata-sections', '-nostdlib', '-Lplatform/c64', f'-L{OUT}',
           '-T', f'platform/c64/overlay_v2_slot_{"job" if slot == 0 else "task"}.ld',
           '-T', 'platform/c64/overlay_v2_payload.ld',
           f'-Wl,--gc-sections,--defsym=__ov2_slot={slot},--defsym=__ov2_mission={1 if m == "lm" else 2},'
           f'--defsym=__ov2_id={ident},--defsym=__ov2_services={SERVICES},--defsym=__ov2_selectors=1,'
           f'-Map,{prg}.map',
           wrapper, *sources, 'platform/c64/overlay_v2_entry.S', '-lc', library,
           '-lcrt', '-o', prg]
    r = subprocess.run([str(c) for c in cmd], cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        over = [int(v) for v in re.findall(r"overflowed by (\d+) bytes", r.stderr)]
        return dict(ok=False, overflow=max(over) if over else None,
                    error=None if over else [l for l in r.stderr.splitlines() if 'error' in l][:4])
    return dict(ok=True, bytes=(ROOT / prg).stat().st_size - 2)



# Tables also packed into another module, so the routines they hold run there
# without a module swap (docs/FLIGHT-STORE.md). The background self-check loop
# leaves SMODECHK through TC POSTJUMP; the interbank tables ride with it.
CO_LOCATE = {'lm': {'lm_self_check': ['lm_interbank_routines']},
             'cm': {'cm_self_check': ['cm_interbank_routines']}}
# The self-check module's fixed_data hook (include/self_check.h).
FIXED_DATA = {'lm_self_check': 'lm_self_check_rope_data', 'cm_self_check': 'cm_self_check_rope_data'}
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


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--missions', default='lm,cm')
    p.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    p.add_argument('--host-only', action='store_true',
                   help='steps 1-3 only: the host index and build/flight-store/<m>-host.inc')
    a = p.parse_args()
    (ROOT / OUT).mkdir(parents=True, exist_ok=True)
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
    for m in a.missions.split(','):
        mine = {t: o for t, o in tables.items() if mission_of(t, o) == m}
        skipped = {t: 'resident' for t in RESIDENT[m]} | {t: 'replaced by the thin kernel' for t in REPLACED[m]}
        packed = {t: o for t, o in mine.items() if t not in skipped and keys[t]}
        empty = sorted(t for t, o in mine.items() if t not in skipped and not keys[t])
        resident_keys = {('i' if 'interpret' in t else 'b', k) for t in RESIDENT[m] for k in keys.get(t, [])}
        # Key -> tables, in resolution preference order.
        holders = collections.defaultdict(list)
        for t in sorted(packed, key=lambda t: (min((i for _, i in owners.get(packed[t], [])), default=99), t)):
            kind = 'i' if 'interpret' in t else 'b'
            for k in keys[t]:
                holders[(kind, k)].append(t)
        collisions = collections.Counter(tuple(v) for v in holders.values() if len(v) > 1)
        shadowed = sum(1 for k in holders if k in resident_keys)
        order = list(dict.fromkeys(RESIDENT[m] + sorted(
            packed, key=lambda t: (min((i for _, i in owners.get(packed[t], [])), default=99), t))))
        check_site_includers()
        also = collections.defaultdict(list)
        for host, ts in CO_LOCATE[m].items():
            for t in ts:
                also[t].append(host)

        def site_stem(t):
            return RESIDENT_SITES[m] if t in RESIDENT[m] else tables[t]
        stems = sorted({site_stem(t) for t in order if t in tables})
        (ROOT / OUT / f'{m}-sites.inc').write_text(
            f'/* Generated by tools/flight_store_build.py: the inline operands each {m.upper()} source '
            'compiles (its agc_site_operand arrays), sorted; a module carries its sources\' sets. */\n'
            + ''.join(f'static const agc_site_operand sites_{st}[]={{' + ','.join(
                f'{{{k}u,0{w:o}u}}' for k, w in sorted(file_sites(a.host_cc, m, st).items()))
                      + (f'}};\n' if file_sites(a.host_cc, m, st) else '{AGC_TRANSFER_NONE,0u}};\n') for st in stems))
        (ROOT / OUT / f'{m}-host.inc').write_text(
            f'/* Generated by tools/flight_store_build.py: the {m.upper()} flight program\'s tables, '
            'resident first, in resolution order: table, interpretive, io kind, module (empty when '
            'resident), slot (0 job, 1 task), operand set, its size, other modules holding the table. */\n'
            + ''.join(f'FLIGHT_TABLE({t}, {int("interpret" in t or t.endswith("_marker"))}, '
                      f'FLIGHT_IO_{io_kind(m, tables[t])}, "{packed.get(t, "") if t not in RESIDENT[m] else ""}", '
                      f'{lane_slot(packed[t], m, owners, place)[1] if t in packed and t not in RESIDENT[m] else 0}, '
                      f'sites_{site_stem(t)}, {len(file_sites(a.host_cc, m, site_stem(t)))}u, '
                      f'"{" ".join(also[t])}")\n'
                      for t in order if t in tables))
        if a.host_only:
            continue
        by_stem = collections.defaultdict(list)
        for t, o in packed.items():
            by_stem[o].append(t)
        modules, seq = [], collections.Counter()
        for stem in sorted(by_stem):
            lane, slot, agc = lane_slot(stem, m, owners, place)
            seq[lane] += 1
            if seq[lane] > LANE_LIMIT.get(lane, 0xFF):
                raise SystemExit(f'{m}: lane {lane} has more than {LANE_LIMIT.get(lane, 0xFF)} modules')
            ident = LANE_BYTE[lane] << 8 | seq[lane]
            ts = sorted(by_stem[stem])
            extra = [t for t in CO_LOCATE[m].get(stem, []) if t in packed]
            sources = list(dict.fromkeys([stem] + [packed[t] for t in extra]))
            sites = {}
            for st in sources:
                sites.update(file_sites(a.host_cc, m, st))
            modules.append(dict(stem=stem, agc=agc, lane=lane, slot=slot, id=ident,
                                basic=[t for t in ts if 'interpret' not in t] + [t for t in extra if 'interpret' not in t],
                                interp=[t for t in ts if 'interpret' in t] + [t for t in extra if 'interpret' in t],
                                also=extra, sites=sites, fixed_data=FIXED_DATA.get(stem),
                                sources=c64_sources(index, *sources, 'agc_transfer')))
        (ROOT / OUT / m).mkdir(exist_ok=True)

        def link(mod):
            prg = OUT / m / f'{mod.get("prg_name") or "module-%04x" % mod["id"]}.prg'
            res = link_module(a.cc, m, prg, mod['slot'], mod['id'], mod['sources'], mod['basic'], mod['interp'],
                              library, mod.get('chunk'), mod.get('sites'), mod.get('fixed_data'))
            if not res['ok'] and res.get('overflow') and mod['slot'] == 1:
                # A task-slot module that does not fit tries the job slot.
                res2 = link_module(a.cc, m, prg, 0, mod['id'], mod['sources'], mod['basic'], mod['interp'], library,
                                   mod.get('chunk'), mod.get('sites'), mod.get('fixed_data'))
                if res2['ok']:
                    res = dict(res2, moved_to_job_slot=True, task_overflow=res['overflow'])
                    mod['slot'] = 0
            return mod, prg, res
        with ThreadPoolExecutor(a.jobs) as pool:
            linked = list(pool.map(link, modules))
        # A basic-only module over its slot is split into chunks of
        # consecutive routines, halving until each chunk fits. Splitting stops
        # (the module stays whole and failed) when a chunk of one routine
        # does not fit, or when halving leaves the overflow nearly unchanged:
        # the routines share one body (a file-wide step machine) that every
        # chunk links. Trial links use a scratch id; kept chunks are relinked
        # with their own. Interpretive modules wait on the interpreter
        # decision.
        chunks = []

        def trial(c, mod):
            return link(dict(mod, id=LANE_BYTE[mod['lane']] << 8 | 0xFF, basic=[], chunk=c, chunk_of=mod['id'],
                             prg_name=f'trial-{c[0]}-{c[1]}'))[2]

        def split(mod):
            todo, kept = [((t, 0, len(keys[t])), None) for t in mod['basic']], []
            while todo:
                with ThreadPoolExecutor(a.jobs) as pool:
                    results = list(pool.map(lambda x: trial(x[0], mod), todo))
                nxt = []
                for (c, parent), r in zip(todo, results):
                    if r['ok']:
                        kept.append(c)
                    elif not r.get('overflow') or c[2] - c[1] == 1 or (
                            parent and r['overflow'] > 0.75 * parent):
                        return None
                    else:
                        half = (c[1] + c[2]) // 2
                        nxt += [((c[0], c[1], half), r['overflow']), ((c[0], half, c[2]), r['overflow'])]
                todo = nxt
            return kept
        for mod, prg, res in linked:
            if res['ok'] or not res.get('overflow') or mod['interp']:
                continue
            kept = split(mod)
            if not kept or seq[mod['lane']] + len(kept) - 1 > LANE_LIMIT.get(mod['lane'], 0xFF):
                mod['chunking'] = 'routines share one body' if kept is None else 'no module ids left'
                continue
            subs = []
            for i, c in enumerate(sorted(kept)):
                if i:
                    seq[mod['lane']] += 1
                ident = mod['id'] if i == 0 else LANE_BYTE[mod['lane']] << 8 | seq[mod['lane']]
                subs.append(dict(mod, id=ident, basic=[], chunk=c, chunk_of=mod['id']))
            with ThreadPoolExecutor(a.jobs) as pool:
                chunks += list(pool.map(link, subs))
            mod['chunked'] = True
        linked = [x for x in linked if not x[0].get('chunked')] + sorted(chunks, key=lambda x: x[0]['id'])
        modules = [mod for mod, _, _ in linked]
        module_of = {}
        for mod, prg, res in linked:
            mod.update(res)
            mod['prg'] = str(prg)
            if res['ok']:
                if mod.get('chunk'):
                    t, first, last = mod['chunk']
                    for k in keys[t][first:last]:
                        module_of[(t, k)] = mod['id']
                    continue
                for t in mod['basic'] + mod['interp']:
                    if t in mod.get('also', []):
                        continue  # resolved to the table's own module
                    for k in keys[t]:
                        module_of[(t, k)] = mod['id']
        records, erasable, unplaced = [], 0, 0
        for (kind, k), ts in sorted(holders.items()):
            if (kind, k) in resident_keys:
                continue
            if k >= ERASABLE_BASE:
                erasable += 1
                continue
            owner = next((module_of[(t, k)] for t in ts if (t, k) in module_of), None)
            if owner is None:
                unplaced += 1
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
        pack = subprocess.run([sys.executable, 'tools/overlay_v2_pack.py', *good, '--mission', m.upper(),
                    '--services', hex(SERVICES), '--resolver', OUT / f'{m}-resolver.json',
                    '--output', OUT / f'{m}.reu', '--manifest', OUT / f'{m}-manifest.json'],
                   cwd=ROOT, capture_output=True, text=True)
        if pack.returncode:
            raise SystemExit(f'{m}: overlay_v2_pack.py failed: {pack.stderr.strip()}')
        store = json.loads((ROOT / OUT / f'{m}-manifest.json').read_text())
        failed = [dict(stem=mod['stem'], id=f'0x{mod["id"]:04x}', slot=mod['slot'], overflow=mod.get('overflow'),
                       error=mod.get('error'), chunking=mod.get('chunking'), chunk=mod.get('chunk'))
                  for mod in modules if not mod['ok']]
        report[m] = dict(
            tables=len(mine), packed_tables=len(packed), empty_tables=empty, skipped=skipped,
            keys=len(holders), keys_held_by_resident=shadowed, erasable_keys=erasable,
            unplaced_keys=unplaced, resolver_records=len(resolver),
            basic_interpretive_same_address=len(clash),
            collisions=[dict(tables=list(k), keys=v) for k, v in collisions.most_common()],
            modules=len(modules), modules_linked=len(good), failed=failed,
            store_bytes=store.get('store_bytes') or store.get('image_bytes'), pack=pack.stdout.strip(),
            module_list=[{k: mod.get(k) for k in ('stem', 'agc', 'lane', 'slot', 'id', 'bytes', 'basic', 'interp',
                                                    'chunk', 'chunk_of', 'also', 'fixed_data', 'moved_to_job_slot',
                                                    'task_overflow')} | dict(sites=len(mod.get('sites') or {}))
                         for mod in modules])
        print(f"{m}: {len(packed)} tables in {len(modules)} modules ({len(good)} linked, {len(failed)} failed); "
              f"{len(resolver)} resolver records; {sum(r['keys'] for r in report[m]['collisions'])} colliding keys; "
              f"{pack.stdout.strip()}")
    if not a.host_only:
        (ROOT / OUT / 'report.json').write_text(json.dumps(report, indent=1) + '\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
