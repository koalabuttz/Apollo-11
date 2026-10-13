#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Routine tables of a Lane C file's modules for the native control-transfer
ABI (docs/NATIVE_TRANSFER_ABI.md).

  python3 tools/lanec/routines.py di|ev|pin [--check]

di is DISPLAY_INTERFACE_ROUTINES (both missions), ev is EXTENDED_VERBS, pin
is PINBALL_GAME_BUTTONS_AND_LIGHTS (the pw-* suites over the earlier Pinball
modules, and pin-gaps).
Each table entry is a point at which one of the file's Lane C suites enters
its native module and whose results match the unchanged original program
(tools/lanec/diff.py): build/lanec-native --entries lists the points each
suite's accepted cases start at. A basic point is keyed by the physical
address of Z, an interpretive point (Z at NEWOPS) by the physical address of
LOC under BANKSET. A point claimed by two modules is an error, except in pin,
where the earlier modules overlap: its routine tries each claiming module in
turn and keeps the first that accepts. Writes (or
with --check compares) src/lanec_<family>_routines.inc, sorted by physical
address (agc_fixed_physical) as the trampoline requires.

It also writes tests/lanec_<family>_transfer_cases.inc, the round-trip cases
of the <family>-transfer suite: every genuine BANKCALL/IBNKCALL/POSTJUMP site
(src/{lm,cm}_interbank_sites.inc) whose CADR names a registered basic point,
each with up to CASES_PER_SITE accepted cases of that point's suite (spread
over the suite's cases) whose state the round trip starts from.

It checks the tables against each mission's listing: every labelled
instruction of the file is registered, except internal subroutines that no
other source file names (they are reached only by a TC inside the file and
compiled into their callers), and so is every word where an external callee
returns into the file (the word after a TC/TCR to a label outside the file,
past its inline operands; ENDOFJOB never returns).
"""
import os, pathlib, re, subprocess, sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import wholefile  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parents[2]
CASES_PER_SITE = 4
FAMILIES = {
    'di': dict(file='DISPLAY_INTERFACE_ROUTINES.agc', title='DISPLAY_INTERFACE_ROUTINES',
               modules=('dsp', 'mark', 'play', 'sleep', 'idle', 'nvdsp'), c64='dsp', data=wholefile.DATA,
               # the C64 round trips: callers of the display-start and MAKEPLAY entries, both missions
               c64_modules=('dsp', 'play'), c64_missions=('LM', 'CM'),
               # di-transfer-c64m: the other modules' callers and points, one C64 program per module and mission
               c64m_modules=('mark', 'sleep', 'idle', 'nvdsp')),
    'ev': dict(file='EXTENDED_VERBS.agc', title='EXTENDED_VERBS',
               modules=('front', 'short', 'align', 'radar', 'dap', 'time', 'track', 'lmi'), c64='front',
               data=wholefile.DATA | {'REMADR', 'MM'}, fans=('LST2FAN',),
               # Comanche055's interpretive words wait for the CM interpreter library
               open_missions=('CM',), cases_per_site=64, file_ranges=True),
    'pin': dict(file='PINBALL_GAME_BUTTONS_AND_LIGHTS.agc', title='PINBALL_GAME_BUTTONS_AND_LIGHTS',
                modules=None, c64='decimal', data=wholefile.DATA, multi=True, file_ranges=True,
                # instruction words other code reads as constants (CAF/CS/AD/CA), never runs here
                constants={'ENDINST': 1, 'TCNOVAC': 1, 'TCWAIT': 1, 'TCTSKOVR': 1, 'TCFINDVC': 1, 'NVSBENDL': 1,
                           'MIXAD': 1, 'DISPLACE': 3},
                # callees that never return to the word after the call
                no_return=('CCSHOLE', 'POODOO', 'ENDOFJOB', 'ALMCYCLE', 'BLANKSUB', 'ENTEXIT', 'JOBSLEEP'),
                # labels whose name other files use only as an interpretive opcode
                opcodes=('CLEAR',)),
}


def suites(fam, cfg):
    """(module, suite name, lanec_run) of the family, in table order."""
    if cfg['modules'] is not None:
        return [(m, f'{fam}-{m}', f'lanec_{fam}_{m}_dispatch') for m in cfg['modules']]
    names = sorted(set(re.findall(r'"pw-([a-z0-9-]+)"', ''.join(
        p.read_text() for p in (ROOT / 'tests').glob('lanec_pw_*_cases.c')))))
    return [(n.replace('-', '_'), f'pw-{n}', f'lanec_pw_{n.replace("-", "_")}_run') for n in names] + \
        [('gaps', 'pin-gaps', 'lanec_pin_gaps_dispatch')]


def physical(bank, z):
    return 2048 + z - 0o4000 if z >= 0o4000 else bank * 1024 + z - 0o2000


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in FAMILIES:
        sys.exit('usage: routines.py di|ev|pin [--check]')
    fam = sys.argv[1]
    cfg = dict(FAMILIES[fam])
    mods = suites(fam, cfg)
    cfg['modules'] = tuple(m for m, _, _ in mods)
    suite_of = {m: s for m, s, _ in mods}
    run_of = {m: r for m, _, r in mods}
    multi = cfg.get('multi', False)
    out = ROOT / f'src/lanec_{fam}_routines.inc'
    cases_path = ROOT / f'tests/lanec_{fam}_transfer_cases.inc'
    up = fam.upper()
    subprocess.run(['make', '-s', 'SKIP_EVIDENCE_RESTORE=1', 'build/lanec-native']
                   + (['build/lanec-oracle'] if cfg.get('multi') or cfg.get('c64m_modules') else []),
                   cwd=ROOT, check=True)
    # mission -> kind ('' basic, 'I' interpretive) -> physical -> (module, bank, z)
    points = {m: {'': {}, 'I': {}} for m in ('LM', 'CM')}
    accepted = {'LM': {}, 'CM': {}}
    own, outside, exits, swret = {}, {}, {}, {}
    # the site tables name the repository file (older ones the listing's include name)
    listing = (cfg['file'][:-4], wholefile.LISTING.get(cfg['file'], cfg['file'])[:-4])
    if multi:   # pin: the earlier modules also run code of other files; only the file's own points count
        wholefile.FILE = cfg['file']
        for mission in ('LM', 'CM'):
            own[mission] = {physical(int(r['bank'] or '0', 8), r['addr']) for r in wholefile.listing_words(mission)}
    for mod in cfg['modules']:
        text = subprocess.run(['build/lanec-native', '--exits' if multi else '--entries', suite_of[mod]], cwd=ROOT,
                              check=True, capture_output=True, text=True).stdout
        for line in text.split('\n'):
            if not line:
                continue
            tag, bank, z, n, *stop = line.split()
            if stop:
                exits.setdefault(tag[:2], set()).add(tuple(int(x, 8) for x in stop))
            mission, kind = tag[:2], tag[2:]
            bank, z = int(bank, 8), int(z, 8)
            if multi and (z < 0o2000 or physical(bank, z) not in own[mission]):
                outside.setdefault(mission, set()).add((bank, z))
                if not kind and z == SWRETURN[mission] and mod not in swret.setdefault(mission, []):
                    swret[mission].append(mod)
                continue
            if z < 0o2000:
                sys.exit(f'FAIL: {mission} {suite_of[mod]} entry {z:o} is not in fixed memory')
            key = physical(bank, z)
            if multi:
                got = points[mission][kind].setdefault(key, ([], bank, z))[0]
                if mod not in got:
                    got.append(mod)
                if not kind and got[0] == mod:
                    accepted[mission].setdefault(key, []).append(int(n))
                continue
            if points[mission][kind].setdefault(key, (mod, bank, z))[0] != mod:
                sys.exit(f'FAIL: {mission} {bank:o},{z:04o} is claimed by {fam}-{points[mission][kind][key][0]} '
                         f'and {fam}-{mod}')
            if not kind:
                accepted[mission].setdefault(key, []).append(int(n))
    for mission, where in sorted(outside.items()):
        print(f'{mission}: {len(where)} entry points outside {cfg["file"]} (other files\' code) not registered')
    if multi:
        # pin: the return point after each of the file's BANKCALLs, which the
        # earlier modules enter only at INTER-BANK's SWRETURN: a routine that
        # gives the state back to SWRETURN and runs the modules from there
        for mission in ('LM', 'CM'):
            for phys, word, where, kind, form, label, source in interbank_sites(mission):
                if (kind in ('BANKCALL', 'IBNKCALL') and form == 'CADR' and source.startswith(listing)
                        and int(phys) + 1 not in points[mission]['']):
                    sb, sa = (int(x, 8) for x in where.split(','))
                    bank, z = (0, 0o4000 + (sb - 2) * 0o2000 + sa - 0o2000 + 1) if sb in (2, 3) else (sb, sa + 1)
                    points[mission][''][int(phys) + 1] = (['SWRETURN'] + swret[mission], bank, z)
    first, lists = {'LM': {}, 'CM': {}}, []
    if multi:   # one routine per distinct list of claiming modules
        lists = sorted({tuple(v[0]) for m in points for k in points[m] for v in points[m][k].values()})
        for m in points:
            for k in points[m]:
                for key, (got, bank, z) in points[m][k].items():
                    first[m][key] = got[0]
                    points[m][k][key] = (f'try{lists.index(tuple(got))}', bank, z)
    for mission in ('LM', 'CM'):
        check_listing(cfg, mission, points[mission], exits.get(mission, set()) if multi else None)
    text = ['/* SPDX-License-Identifier: MIT */',
            f'/* Generated by tools/lanec/routines.py {fam}: every point at which a Lane C',
            f' * {cfg["title"]} suite enters its native module, keyed by',
            ' * physical address and sorted. Do not edit. */']
    if multi:   # pin: the modules each routine tries, in order; the C64 program has the c64 module's points only
        text.append(f'#ifndef LANEC_{up}_C64')
        used = {m: {v[0] for v in points[m][''].values()} for m in points}
        for i, got in enumerate(lists):
            ret = got[0] == 'SWRETURN'
            mods = got[1:] if ret else got
            text.append(f'static const dispatch try{i}[{len(mods) + 1}] = {{ ' + ', '.join(run_of[g] for g in mods) + ', 0 };')
            for mission in ('LM', 'CM'):
                if f'try{i}' in used[mission]:
                    ml = mission.lower()
                    call = f'swreturn(p, s, v, try{i}, {ml}, 0{SWRETURN[mission]:o}u)' if ret else f'{ml}(p, s, v, try{i})'
                    text.append(f'static enum exec_status try{i}_{ml}(const agc_native_program *p, apollo_overlay_state *s, '
                                f'void *v) {{ return {call}; }}')
    for mission in ('LM', 'CM'):
        ml = mission.lower()
        for kind, name in (('', ''), ('I', '_INTERP')):
            rows = sorted(points[mission][kind].items())
            if kind and not rows:
                continue
            arr = f'{ml}_entries' if not kind else f'{ml}_interp_entries'
            text.append(f'#define LANEC_{up}_{mission}{name}_ROUTINES {len(rows)}u')
            text.append(f'static const agc_routine_entry {arr}[LANEC_{up}_{mission}{name}_ROUTINES] = {{')
            for key, (mod, bank, z) in rows:
                where = f'{bank:02o},{z:04o}' if z < 0o4000 else f'FF,{z:04o}'
                text.append(f'    {{ {key}u, {mod}_{ml} }},   /* {where}{" LOC" if kind else ""} */')
            text.append('};')
        # the basic points again, one table per module (a program may register a subset)
        rows = sorted(points[mission][''].items())
        if multi:
            continue
        for mod in cfg['modules']:
            mine = [key for key, (m, _, _) in rows if m == mod]
            if not mine:
                continue
            text.append(f'static const agc_routine_entry {ml}_{mod}_entries[{len(mine)}] = {{ '
                        + ', '.join(f'{{ {key}u, {mod}_{ml} }}' for key in mine) + ' };')
        for mod in cfg['modules']:
            n = sum(1 for m in rows if m[1][0] == mod)
            if n:
                text.append(f'const agc_routine_table lanec_{fam}_{ml}_{mod} = {{ {ml}_{mod}_entries, {n}u }};')
    if multi:
        # the C64 programs' tables: every point with the module that claims it
        # first (0377: a BANKCALL return, through SWRETURN); a program builds
        # one module's table from them and runs that module alone
        text.append('#endif')
        text.append('/* the C64 round trips: every point and the index of the module that claims it first (0377: a')
        text.append(' * BANKCALL return point, entered through SWRETURN); lanec_pin_c64_select builds one module\'s table */')
        for mission in ('LM', 'CM'):
            ml = mission.lower()
            rows = sorted(points[mission][''].items())
            idx = [255 if first[mission][k] == 'SWRETURN' else cfg['modules'].index(first[mission][k]) for k, _ in rows]
            text.append(f'#define LANEC_{up}_{mission}_C64_POINTS {len(rows)}u')
            text.append(f'static const lanec_pin_point c64_{ml}_points[LANEC_{up}_{mission}_C64_POINTS] = {{ '
                        + ', '.join(f'{{ {k}u, {i}u }}' for (k, _), i in zip(rows, idx)) + ' };')
        most = max(sum(1 for k in points[m][''] if first[m][k] == g) for m in ('LM', 'CM') for g in cfg['modules'])
        text += [f'#define LANEC_{up}_C64_MAX {most}u', '#ifndef LANEC_PIN_C64',
                 f'static const dispatch c64_runs[{len(cfg["modules"])}] = {{ '
                 + ', '.join(run_of[m] for m in cfg['modules']) + ' };', '#endif']
    text = '\n'.join(text) + '\n'
    cases = transfer_cases(fam, cfg, {m: points[m][''] for m in points}, accepted, first if multi else None, suite_of,
                           exits)
    stale = [p for p, body in ((out, text), (cases_path, cases)) if not p.exists() or p.read_text() != body]
    counts = ', '.join(f'{len(points[m][""])} {m}' + (f' + {len(points[m]["I"])} interpretive' if points[m]['I'] else '')
                       for m in ('LM', 'CM'))
    if '--check' in sys.argv:
        if stale:
            sys.exit(f'FAIL: {", ".join(str(p.relative_to(ROOT)) for p in stale)} stale; run tools/lanec/routines.py {fam}')
        print(f'PASS: {out.relative_to(ROOT)} current ({counts} routines), {cases_path.relative_to(ROOT)} current')
    else:
        out.write_text(text)
        cases_path.write_text(cases)
        print(f'wrote {out.relative_to(ROOT)} ({counts} routines) and {cases_path.relative_to(ROOT)}')


def check_listing(cfg, mission, points, exits=None):
    wholefile.FILE = cfg['file']
    rows = wholefile.listing_words(mission)
    wholefile.original_lines(mission, rows)
    origin = ROOT.parent / wholefile.ORIGINAL[mission]
    src = (origin / wholefile.FILE).read_text(errors='replace').splitlines()
    # Other files' code lines, without EQUALS definitions: a label counts as
    # named outside when it, or an EQUALS alias of it, appears there.
    others, alias = [], {}
    for p in sorted(origin.glob('*.agc')):
        for line in p.read_text(errors='replace').splitlines():
            f = line.split('#', 1)[0].split()
            if len(f) == 3 and f[1] == 'EQUALS':
                alias.setdefault(f[2], []).append(f[0])
            elif p.name != wholefile.FILE:
                others.append(line.split('#', 1)[0])
    others = '\n'.join(others)
    registered = set(points['']) | set(points['I'])
    fields = [r['text'].split('#', 1)[0].split() for r in rows]
    data = [any(f in cfg['data'] for f in fs) for fs in fields]
    for i, (r, fs) in enumerate(zip(rows, fields)):
        if fs and fs[0] in cfg.get('constants', {}) and not src[r['line'] - 1][:1].isspace():
            for j in range(i, i + cfg['constants'][fs[0]]):
                data[j] = True
    where = [physical(int(r['bank'] or '0', 8), r['addr']) for r in rows]
    labels = {fs[0] for r, fs in zip(rows, fields) if fs and not src[r['line'] - 1][:1].isspace()}
    # a fan table (INDEX MPAC; TC LST2FAN): its TC words are jumps, nothing returns after them
    fan = set()
    for i, (r, fs) in enumerate(zip(rows, fields)):
        if fs and fs[0] in cfg.get('fans', ()) and not src[r['line'] - 1][:1].isspace():
            j = i
            while j < len(rows) and len(fields[j]) >= 2 and fields[j][-2] in ('TC', 'TCF'):
                fan.add(j)
                j += 1
    internal, returns, missing = [], 0, []
    for i, (r, fs) in enumerate(zip(rows, fields)):
        if data[i] or not fs:
            continue
        if not src[r['line'] - 1][:1].isspace() and where[i] not in registered:
            if fs[0] not in cfg.get('opcodes', ()) and any(re.search(rf'(?<![\w+\-/*.]){re.escape(n)}(?![\w+\-/*.])', others)
                   for n in [fs[0]] + alias.get(fs[0], [])):
                missing.append(f'label {fs[0]} is named outside the file but not registered')
            else:
                internal.append(fs[0])
        if exits is not None:
            continue
        if len(fs) >= 2 and fs[-2] in ('TC', 'TCR') and fs[-1] not in labels and fs[-1] not in ('Q', 'ENDOFJOB', 'POSTJUMP') and fs[-1][0] not in '+-' \
                and i not in fan:
            j = i + 1
            while j < len(rows) and data[j]:
                j += 1
            returns += 1
            if j < len(rows) and where[j] not in registered:
                missing.append(f'return from {fs[-1]} at {r["bank"]},{rows[j]["addr"]:04o} not registered')
    if exits is not None:
        # pin: the earlier modules run their callees inline wherever they can,
        # so a return needs a routine only where a module stops at a call: an
        # exit outside the file whose Q is the word after a TC/TCR in the file
        # (past a BANKCALL's CADR) to a callee that returns
        at = {w: i for i, w in enumerate(where)}
        need = set()
        for fb, z, q, q0 in exits:
            if z == q0:
                continue   # returned to the caller the case started with
            stop = physical(fb, z) if 0o2000 <= z < 0o4000 else physical(0, z) if z >= 0o4000 else None
            if stop in at:   # stopped inside the file (pin-transfer checks those continuations)
                continue
            key = physical(fb, q) if 0o2000 <= q < 0o4000 else physical(0, q) if q >= 0o4000 else None
            if key not in at:
                continue
            i = at[key]
            k = i - 1
            while k >= 0 and data[k]:
                k -= 1
            call = fields[k] if k >= 0 else []
            if len(call) < 2 or call[-2] not in ('TC', 'TCR') or call[-1] in cfg.get('no_return', ()):
                continue
            j = i
            while j < len(rows) and data[j]:
                j += 1
            if j < len(rows):
                need.add((j, call[-1]))
        returns = len(need)
        for j, callee in sorted(need):
            if where[j] not in registered:
                missing.append(f'return from {callee} at {rows[j]["bank"]},{rows[j]["addr"]:04o} not registered')
    if missing and mission in cfg.get('open_missions', ()):
        print(f'OPEN: {mission} {len(missing)} entries not registered yet: ' + '; '.join(missing))
        return
    if missing and os.environ.get('ROUTINES_SURVEY'):
        print(f'MISSING: {mission} {len(missing)}: ' + '; '.join(missing))
        return
    if missing:
        sys.exit(f'FAIL: {mission} {len(missing)} missing: ' + '; '.join(missing))
    print(f'PASS: {mission} every label registered but the internal subroutines {", ".join(internal)}; '
          f'all {returns} external-callee returns registered')


# services that never return to their caller (the others run inline)
SERVICES = ('ENDOFJOB', 'JOBSLEEP', 'TASKOVER', 'POODOO', 'BAILOUT', 'CCSHOLE', 'CURTAINS')


def fixed_labels(mission):
    """label -> (bank or None for fixed-fixed, address) for every label in fixed memory (the listing's symbol table)."""
    text = (ROOT / f'deps/{mission.lower()}-powered-fixture/assembly.lst').read_text()
    return {n: (int(b, 8) if b else None, int(a, 8))
            for n, b, a in re.findall(r'\d+,F:\s+(\S+)\s+(?:([0-7]{2}),)?([0-7]{4})', text)}


def external_words(cfg, suite_of):
    """(bank or None for fixed-fixed, address) of each word outside cfg's file
    that the original program runs in the module suites (oracle LANEC_TRACE)."""
    own = {m: {(int(r['bank'], 8) if r['bank'] else None, r['addr'] + i)
               for r in wholefile.listing_words(m) for i in range(len(r['words']))} for m in ('LM', 'CM')}
    ext = {'LM': set(), 'CM': set()}
    trace = ROOT / 'build/lanec-pin-external.txt'
    for mod in cfg['modules']:
        if mod == 'gaps':
            continue
        subprocess.run(['build/lanec-oracle', suite_of[mod], 'deps/lm-powered-fixture/MAIN.agc.bin',
                        'deps/cm-powered-fixture/MAIN.agc.bin'], cwd=ROOT, check=True, stdout=subprocess.DEVNULL,
                       env=dict(os.environ, LANEC_TRACE=str(trace)))
        for line in trace.read_text().split('\n'):
            if line[:2] not in ext or line[2:3] == 'I':
                continue
            b, a = (int(x, 8) for x in line.split()[1].split(','))
            key = (None if a >= 0o4000 else b, a)
            if key not in own[line[:2]]:
                ext[line[:2]].add(key)
    trace.unlink()
    return ext


SWRETURN = {'LM': 0o4631, 'CM': 0o4675}


def interbank_sites(mission, callees=False):
    """The mission's INTER-BANK call sites (physical, word, where, kind, form, label, source), and their TC targets."""
    inc = (ROOT / f'src/{mission.lower()}_interbank_sites.inc').read_text()
    sites = re.findall(r'\{(\d+)u,(0[0-7]+)\}, /\* (\S+) (\w+) (\w+) (.*?) <- (.*?) \*/', inc)
    callee = re.search(r'_callee\[\d+\]=\{(.*?)\};', inc, re.S).group(1).replace('\n', '').split(',')
    callee = [int(c, 8) for c in callee if c.strip()]
    if len(callee) != len(sites):
        sys.exit(f'FAIL: {mission} site table not parsed')
    return (sites, callee) if callees else sites


def c64_frozen(fam, c64, names=()):
    """For a family whose C64 program has no room for every C64 case: the
    frozen rows only (tests/lanec_<fam>_transfer_c64_c64.h), with their case
    numbers, for the C64 build."""
    frozen = ROOT / f'tests/lanec_{fam}_transfer_c64_c64.h'
    if fam not in ('di', 'pin') or not frozen.exists():
        return []
    ns = [int(n) for n in re.findall(r'\{(\d+)u, 0x', frozen.read_text())]
    up = fam.upper()
    row = {n: f'    {{ {n}u, {c64[n].strip().split("/*")[0].strip().rstrip(",")} }},' for n in ns}
    if fam == 'pin':
        # one program per module and mission: each takes only its rows, here and
        # in the frozen hashes (tests/lanec_pin_transfer_c64_rows.h)
        hashes = dict(re.findall(r'\{(\d+)u, (0x[0-9a-f]+)u\}', frozen.read_text()))
        groups = {}
        for n in ns:
            mission = 'cm' if 'LANEC_CM' in c64[n] else 'lm'
            groups.setdefault(f"{names[int(c64[n].split(', ')[3].rstrip('u')) & 0o177]}-{mission}", []).append(n)
        out, rows_h = [f'#ifdef LANEC_{up}_TRANSFER_C64',
                       f'/* the frozen rows of the program\'s module (tests/lanec_{fam}_transfer_c64_c64.h): case number, case */'], \
            ['/* SPDX-License-Identifier: MIT */',
             f'/* Generated by tools/lanec/routines.py pin from tests/lanec_{fam}_transfer_c64_c64.h: each C64 program\'s',
             ' * frozen rows (LANEC_PIN_C64_MOD_<suite>_<lm|cm>: one module, one mission), for',
             ' * tools/lanec/c64_diag.c. Do not edit. */']
        for name, mine in sorted(groups.items()):
            mac = f'LANEC_PIN_C64_MOD_{name.replace("-", "_")}'
            out += [f'#ifdef {mac}', f'static const struct {{ uint16_t n; transfer_case c; }} c64_frozen[{len(mine)}] = {{']
            out += [row[n] for n in mine] + ['};', '#endif']
            rows_h += [f'#ifdef {mac}', 'static const struct { uint32_t n, hash; } lanec_frozen[] = {']
            rows_h += [f'    {{{n}u, {hashes[str(n)]}u}},' for n in mine] + ['};', '#endif']
        (ROOT / f'tests/lanec_{fam}_transfer_c64_rows.h').write_text('\n'.join(rows_h) + '\n')
        return out + ['#endif']
    lm = [row[n] for n in ns if 'LANEC_LM' in row[n]]
    cm = [row[n] for n in ns if 'LANEC_CM' in row[n]]
    return ([f'#ifdef LANEC_{up}_TRANSFER_C64',
             f'/* the frozen rows only (tests/lanec_{fam}_transfer_c64_c64.h), one mission per program',
             ' * (LANEC_DI_C64_CM: Comanche055): case number, case */',
             '#ifndef LANEC_DI_C64_CM',
             f'static const struct {{ uint16_t n; transfer_case c; }} c64_frozen[{len(lm)}] = {{'] + lm
            + ['};', '#else', f'static const struct {{ uint16_t n; transfer_case c; }} c64_frozen[{len(cm)}] = {{'] + cm
            + ['};', '#endif', '#endif'])


def c64m_block(fam, cfg, c64m, points):
    """di-transfer-c64m: its cases, each program's stops (every point of the
    other modules) and, once tests/lanec_<fam>_transfer_c64m_c64.h holds the
    rows tools/lanec/cover.py picked, each program's frozen rows (here, and with
    their hashes in tests/lanec_<fam>_transfer_c64m_rows.h)."""
    up = fam.upper()
    out = [f'/* {fam}-transfer-c64m: the cases whose destination is a {"/".join(cfg["c64m_modules"])} point (callers,',
           ' * then every point from its own entry); one C64 program per module and mission',
           f' * (LANEC_{up}_C64M_<MODULE>_<LM|CM>) registers INTER-BANK and that module only. */',
           f'#define LANEC_{up}_TRANSFER_C64M_CASES {len(c64m)}u']
    frozen = ROOT / f'tests/lanec_{fam}_transfer_c64m_c64.h'
    ns = [int(n) for n in re.findall(r'\{(\d+)u, 0x', frozen.read_text())] if frozen.exists() else []
    out += ([f'#ifndef LANEC_{up}_TRANSFER_C64M'] if ns else []) + \
        [f'static const transfer_case c64m_cases[LANEC_{up}_TRANSFER_C64M_CASES] = {{'] + c64m + ['};'] + \
        (['#endif'] if ns else [])
    if ns:
        hashes = dict(re.findall(r'\{(\d+)u, (0x[0-9a-f]+)u\}', frozen.read_text()))
        groups = {}
        for n in ns:
            f = c64m[n].strip().split('/*')[0].strip().rstrip(',').strip('{} ').split(', ')
            mod = cfg['modules'][int(f[3].rstrip('u')) & 0o177]
            groups.setdefault(f'{mod}_{f[4][len("LANEC_"):].lower()}', []).append(n)
        rows_h = ['/* SPDX-License-Identifier: MIT */',
                  f'/* Generated by tools/lanec/routines.py {fam} from tests/lanec_{fam}_transfer_c64m_c64.h: each',
                  f' * C64 program\'s frozen rows (LANEC_{up}_C64M_<MODULE>_<LM|CM>), for tools/lanec/c64_diag.c. Do not edit. */']
        out.append(f'#ifdef LANEC_{up}_TRANSFER_C64M')
        for name, mine in sorted(groups.items()):
            mac = f'LANEC_{up}_C64M_{name.upper()}'
            out += [f'#ifdef {mac}', f'static const struct {{ uint16_t n; transfer_case c; }} c64m_frozen[{len(mine)}] = {{']
            out += [f'    {{ {n}u, {c64m[n].strip().split("/*")[0].strip().rstrip(",")} }},' for n in mine] + ['};', '#endif']
            rows_h += [f'#ifdef {mac}', 'static const struct { uint32_t n, hash; } lanec_frozen[] = {']
            rows_h += [f'    {{{n}u, {hashes[str(n)]}u}},' for n in mine] + ['};', '#endif']
        out.append('#endif')
        (ROOT / f'tests/lanec_{fam}_transfer_c64m_rows.h').write_text('\n'.join(rows_h) + '\n')
    # A module's routines run on through some of the other modules' points (a
    # mark display runs the display-start code to its exits): those are not
    # stops. The others are where the module's own suite stops too.
    ran = {}
    trace = ROOT / f'build/lanec-{fam}-c64m-trace.txt'
    for mod in cfg['c64m_modules']:
        subprocess.run(['build/lanec-oracle', f'{fam}-{mod}', 'deps/lm-powered-fixture/MAIN.agc.bin',
                        'deps/cm-powered-fixture/MAIN.agc.bin'], cwd=ROOT, check=True, stdout=subprocess.DEVNULL,
                       env=dict(os.environ, LANEC_TRACE=str(trace)))
        ran[mod] = {(line[:2], *(int(x, 8) for x in line.split()[1].split(',')))
                    for line in trace.read_text().split('\n') if line[:2] in ('LM', 'CM') and line[2:3] != 'I'}
    trace.unlink()
    for mission in ('LM', 'CM'):
        for mod in cfg['c64m_modules']:
            v = [('LANEC_ANY_BANK' if z >= 0o4000 else f'0{bank:o}u', f'0{z:o}u')
                 for key, (m, bank, z) in sorted(points[mission].items())
                 if m != mod and (mission, 0 if z >= 0o4000 else bank, z) not in ran[mod]]
            out.append(f'/* the points of every module but {mod} that {fam}-{mod}\'s cases do not run on through, {mission}'
                       ' (the oracle stops there) */')
            out.append(f'#if !defined(LANEC_{up}_TRANSFER_C64M) || defined(LANEC_{up}_C64M_{mod.upper()}_{mission})')
            out.append(f'static const lanec_point c64m_stops_{mission.lower()}_{mod}[{len(v)}] = {{ '
                       + ', '.join(f'{{ {b}, {a} }}' for b, a in v) + ' };')
            out.append('#endif')
    return [f'#ifndef LANEC_{up}_TRANSFER_C64'] + out + ['#endif']


def transfer_cases(fam, cfg, points, accepted, first=None, suite_of=None, exits=None):
    rows = []
    for mission in ('LM', 'CM'):
        sites, callee = interbank_sites(mission, True)
        for (phys, word, where, kind, form, label, source), tc in zip(sites, callee):
            word = int(word, 8)
            bank = word >> 10
            if 0o30 <= bank < 0o40 and int(where.split(',')[0], 8) >= 0o40:
                bank += 0o10   # the caller runs with the superbank bit, which the transfer keeps
            target = bank * 1024 + (word & 0o1777)
            if form != 'CADR' or kind == 'USPRCADR' or word < 0o2000 or target not in points[mission]:
                continue
            per = cfg.get('cases_per_site', CASES_PER_SITE)
            if first and source.startswith((cfg['file'][:-4], wholefile.LISTING.get(cfg['file'], cfg['file'])[:-4])):
                continue    # pin: a call inside the file runs within its caller's routine (below)
            mod = first[mission][target] if first else points[mission][target][0]
            ns = accepted[mission][target]
            pick = sorted({ns[i * len(ns) // per] for i in range(min(per, len(ns)))})
            for n in pick:
                rows.append(f'    {{ {int(phys)}u, 0{tc:o}u, {n}u, {cfg["modules"].index(mod)}u, LANEC_{mission} }},'
                            f'   /* {where} {kind} {label.split()[0]} <- {source.split(":")[0]} */')
    if first:
        # pin: every routine from its own entry, with accepted cases of its
        # first module (index | 0200), chained through the routines that
        # follow it until the original leaves the file
        per = cfg.get('cases_per_entry', 4)
        for mission in ('LM', 'CM'):
            for key, (_, bank, z) in sorted(points[mission].items()):
                if key not in accepted[mission]:
                    continue    # a BANKCALL return point: its callers' cases reach it
                mod, ns = first[mission][key], accepted[mission][key]
                where = f'{bank:02o},{z:04o}' if z < 0o4000 else f'FF,{z:04o}'
                for n in sorted({ns[i * len(ns) // per] for i in range(min(per, len(ns)))}):
                    rows.append(f'    {{ {key}u, 0u, {n}u, {cfg["modules"].index(mod) | 0o200}u, LANEC_{mission} }},'
                                f'   /* {where} {mod} */')
    c64i = cfg['modules'].index(cfg['c64'])
    c64m = {f'{cfg["modules"].index(m) | f}u' for m in cfg.get('c64_modules', (cfg['c64'],)) for f in (0, 0o200)}
    c64 = [r for r in rows if r.split(', ')[3] in c64m
           and any(f'LANEC_{x} ' in r for x in cfg.get('c64_missions', ('LM',)))]
    stops = {}
    if 'c64_modules' in cfg and not first:
        # the other modules' points, where the oracle stops (the C64 program has no routine there)
        for mission in cfg['c64_missions']:
            stops[mission] = [('LANEC_ANY_BANK' if z >= 0o4000 else f'0{bank:o}u', f'0{z:o}u')
                              for key, (mod, bank, z) in sorted(points[mission].items())
                              if isinstance(mod, str) and mod in cfg['modules'] and mod not in cfg['c64_modules']]
        # di: also every C64 module's point from its own entry (index | 0200),
        # so that chains go on past the module's exit (BOTHJOBS to BANKJUMP)
        per = cfg.get('cases_per_entry', 4)
        for mission in cfg['c64_missions']:
            for key, (mod, bank, z) in sorted(points[mission].items()):
                if mod not in cfg['c64_modules'] or key not in accepted[mission]:
                    continue
                ns = accepted[mission][key]
                where = f'{bank:02o},{z:04o}' if z < 0o4000 else f'FF,{z:04o}'
                for n in sorted({ns[i * len(ns) // per] for i in range(min(per, len(ns)))}):
                    c64.append(f'    {{ {key}u, 0u, {n}u, {cfg["modules"].index(mod) | 0o200}u, LANEC_{mission} }},'
                               f'   /* {where} {mod} */')
    c64m, c64m_text = [], []
    if cfg.get('c64m_modules') and not first:
        # di-transfer-c64m: the callers of the other modules' entries and every
        # point of theirs from its own entry; a program registers INTER-BANK and
        # one module, and the oracle stops at every other module's points
        mm = {f'{cfg["modules"].index(m)}u' for m in cfg['c64m_modules']}
        c64m = [r for r in rows if r.split(', ')[3] in mm]
        per = cfg.get('cases_per_entry', 4)
        for mission in ('LM', 'CM'):
            for key, (mod, bank, z) in sorted(points[mission].items()):
                if mod not in cfg['c64m_modules'] or key not in accepted[mission]:
                    continue
                ns = accepted[mission][key]
                where = f'{bank:02o},{z:04o}' if z < 0o4000 else f'FF,{z:04o}'
                for n in sorted({ns[i * len(ns) // per] for i in range(min(per, len(ns)))}):
                    c64m.append(f'    {{ {key}u, 0u, {n}u, {cfg["modules"].index(mod) | 0o200}u, LANEC_{mission} }},'
                                f'   /* {where} {mod} */')
        c64m_text = c64m_block(fam, cfg, c64m, points)
    if first:
        # pin: every routine from its own entry, with cases of the module that
        # claims it first, both missions; each C64 program runs one module's
        per = cfg.get('c64_cases_per_entry', 4)
        c64 = []
        for mission in ('LM', 'CM'):
            for key, (_, bank, z) in sorted(points[mission].items()):
                if key not in accepted[mission]:
                    continue
                mod, ns = first[mission][key], accepted[mission][key]
                where = f'{bank:02o},{z:04o}' if z < 0o4000 else f'FF,{z:04o}'
                for n in sorted({ns[i * len(ns) // per] for i in range(min(per, len(ns)))}):
                    c64.append(f'    {{ {key}u, 0u, {n}u, {cfg["modules"].index(mod) | 0o200}u, LANEC_{mission} }},'
                               f'   /* {where} {mod} */')
    up = fam.upper()
    who = ' and '.join({'LM': 'Luminary099', 'CM': 'Comanche055'}[x] for x in cfg.get('c64_missions', ('LM',)))
    tabs = '/'.join(f'{x.lower()}_{m}' for x in cfg.get('c64_missions', ('LM',)) for m in cfg.get('c64_modules', (cfg['c64'],)))
    spans = []
    if cfg.get('file_ranges'):
        # the file's words per mission, as oracle ranges (bank, lo, hi)
        wholefile.FILE = cfg['file']
        for mission in ('LM', 'CM'):
            runs = []
            for r in sorted(wholefile.listing_words(mission), key=lambda r: (r['bank'] or '', r['addr'])):
                bank = int(r['bank'], 8) if r['bank'] else None
                if runs and runs[-1][0] == bank and runs[-1][2] + 1 >= r['addr']:
                    runs[-1][2] = r['addr'] + len(r['words']) - 1
                else:
                    runs.append([bank, r['addr'], r['addr'] + len(r['words']) - 1])
            spans.append(f'static const lanec_range {mission.lower()}_file_ranges[{len(runs)}] = {{ '
                         + ', '.join(f'{{ {"LANEC_ANY_BANK" if b is None else f"0{b:o}u"}, 0{lo:o}u, 0{hi:o}u }}'
                                     for b, lo, hi in runs) + ' };')
    if first:
        # pin: the code of other files the earlier modules run inline (SHORTMP,
        # JOBWAKE, ...): every word outside the file the original program ran
        # in the pw-* suites, whose oracle stops where the module stops
        ext = external_words(cfg, suite_of)
        own = {m: {(int(r['bank'], 8) if r['bank'] else None, r['addr'] + i)
                   for r in wholefile.listing_words(m) for i in range(len(r['words']))} for m in ('LM', 'CM')}
        for mission in ('LM', 'CM'):
            # the Executive's, Waitlist's and alarms' services where a job or
            # task hands control to another file's code for good: the next
            # routine is that file's, or the round trip ends there
            labels = fixed_labels(mission)
            out = sorted({labels[n] for n in SERVICES if n in labels}, key=lambda k: (-1 if k[0] is None else k[0], k[1]))
            spans.append(f'static const lanec_point {mission.lower()}_external_exits[{len(out)}] = {{ '
                         + ', '.join(f'{{ {"LANEC_ANY_BANK" if b is None else f"0{b:o}u"}, 0{z:o}u }}' for b, z in out)
                         + ' };')
            runs = []
            for b, a in sorted(ext[mission], key=lambda k: (-1 if k[0] is None else k[0], k[1])):
                if runs and runs[-1][0] == b and runs[-1][2] + 1 == a:
                    runs[-1][2] = a
                else:
                    runs.append([b, a, a])
            spans.append(f'static const lanec_range {mission.lower()}_external_ranges[{len(runs)}] = {{ '
                         + ', '.join(f'{{ {"LANEC_ANY_BANK" if b is None else f"0{b:o}u"}, 0{lo:o}u, 0{hi:o}u }}'
                                     for b, lo, hi in runs) + ' };')
    frozen_rows = c64_frozen(fam, c64, [suite_of[m] for m in cfg['modules']] if first else ())
    text = '\n'.join(['/* SPDX-License-Identifier: MIT */',
                      f'/* Generated by tools/lanec/routines.py {fam}: {fam}-transfer round-trip cases',
                      ' * (site physical address, its TC target, case of the target\'s suite, suite',
                      ' * index in module order, mission). Do not edit. */',
                      f'#define LANEC_{up}_TRANSFER_CASES {len(rows)}u',
                      f'#ifndef LANEC_{up}_TRANSFER_C64',
                      f'static const transfer_case cases[LANEC_{up}_TRANSFER_CASES] = {{'] + rows + ['};', '#endif',
                      (f'/* {fam}-transfer-c64: every routine from its own entry, both missions, with cases of the module'
                       '\n * that claims it first (module index | 0200); each C64 program runs its module\'s. */' if first else
                       f'/* {fam}-transfer-c64: the {who} cases whose destination is a\n * lanec_{fam}_{tabs} entry. */'),
                      f'#define LANEC_{up}_TRANSFER_C64_CASES {len(c64)}u']
                     + ([f'#ifndef LANEC_{up}_TRANSFER_C64'] if frozen_rows else [])
                     + [f'static const transfer_case c64_cases[LANEC_{up}_TRANSFER_C64_CASES] = {{'] + c64 + ['};']
                     + (['#endif'] if frozen_rows else []) + frozen_rows
                     + ([f'#ifndef LANEC_{up}_TRANSFER_C64M'] if c64m_text else [])
                     + [f'/* the other modules\' {m} points (the oracle stops there) */\n'
                        f'static const lanec_point c64_stops_{m.lower()}[{len(v)}] = {{ '
                        + ', '.join(f'{{ {b}, {a} }}' for b, a in v) + ' };' for m, v in stops.items()]
                     + (['#endif'] if c64m_text else []) + c64m_text
                     + ([f'/* the words of {cfg["file"]} (oracle ranges) */'] + spans if spans else [])
                     + (['/* the suites the cases come from, in module order */', f'#ifndef LANEC_{up}_TRANSFER_C64',
                         f'static const lanec_suite *const modules[{len(cfg["modules"])}] = {{ '
                         + ', '.join(f'&lanec_suite_{suite_of[m].replace("-", "_")}' for m in cfg['modules']) + ' };',
                         '#endif']
                        if first else [])) + '\n'
    if c64m_text:   # the host-only and di-transfer-c64 tables stay out of the di-transfer-c64m programs
        text = text.replace(f'#ifndef LANEC_{up}_TRANSFER_C64\n',
                            f'#if !defined(LANEC_{up}_TRANSFER_C64) && !defined(LANEC_{up}_TRANSFER_C64M)\n', 2)
    return text


if __name__ == '__main__':
    main()
