#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The second certificate reviewer's semantic checks for one milestone, as one
tool: run it before asking for review, and fix every GAP it prints.

  python3 tools/cert_selfcheck.py <spec> [--jobs N] [--keep] [--skip-differential] [--out DIR]

For an interp spec (tools/interp_specs.py):

  1. differential  tools/interp_routine_differential.py at the frozen cases per
                   routine (the proof's `cases` over the routines, rounded up);
                   every case must equal yaAGC, the regenerated case file must
                   hash to the proof's cases_sha256, and the frozen files
                   (tests/<id>_vectors.h, tests/<id>_c64_vectors.h,
                   tools/mk/<id>.groups, evidence/<id>/) must be committed.
                   --skip-differential reuses build/<id>-cases.bin when it
                   still hashes to cases_sha256.
  2. rows          every row of both vector headers is matched to its frozen
                   case (by seed and edits, or an edit prefix); a resumed
                   truncated row to its parent case and snapshot in
                   evidence/<id>/<id>-c64-cover.json. Each row, with its cut
                   (irc_c64_cut) and, for c64_segments rows, stopped at the
                   resume LOC (IRC_SNAPROW), is re-run on yaAGC and must give
                   the header's frozen digest. The oracle runs with the
                   differential's oracle_env (IRC_PENDING_DANZIG,
                   IRC_RTB_IN_FILE, IRC_INPUTS).
  3. branches      the native runner built -O0 --coverage over the cover
                   sources runs every frozen case (host), and each C64 group,
                   built with its KEEP mask from tools/mk/<id>.groups (and
                   IRC_CUTS for c64_cuts specs), runs only its own rows, which
                   must give their frozen digests. gcov -b runs per object, so
                   included sources do not overwrite each other. A branch
                   outcome (object, source, line, index) the host takes that no
                   group takes is a GAP; a line whose branch count differs in a
                   group build (#if'd arms) is judged by line execution. In
                   a source other cover records also pin (agc_transfer, ...),
                   an outcome that depends on table linkage (find() missing
                   inside a group's table range) is a NOTE when another spec's
                   self-check on the same source took it on C64
                   (build/selfcheck-arcs and the shared
                   /mnt/project-files/selfcheck-arcs); otherwise a GAP.
                   Host-executed lines no group executes are listed too.

For a Lane F module (tools/lane_f_c64_rows.py MODULES) only the branch check
runs: the module's src/ files built with gcc --coverage, all host cases
against each C64 PRG/group build's rows.

Exit 1 when any gap is found. Scratch goes to build/selfcheck/<id> and is
removed unless --keep.
"""
import argparse
import collections
import concurrent.futures
import glob
import hashlib
import json
import os
import pathlib
import re
import shutil
import struct
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
B = os.path.join(R, 'build')
GAPS = []


def gap(msg):
    GAPS.append(msg)
    print('GAP ' + msg, flush=True)


def shared_sources():
    """src/ paths more than one c64-cover record pins (agc_transfer, interp orders/steps, ...)."""
    n = collections.Counter()
    for f in glob.glob(os.path.join(R, 'evidence', '*', '*-c64-cover.json')):
        op = json.load(open(f)).get('linked_sources')
        if isinstance(op, dict):
            n.update(op.keys())
    return {k for k, v in n.items() if v > 1}


# Shared-runtime arcs taken on C64, one file per (source sha256, spec):
# <root>/<sha>/<id>.json. build/selfcheck-arcs is this container's cache; lanes
# run in separate containers, so each run also writes to a folder every lane
# reads ($SELFCHECK_ARCS, default /mnt/project-files/selfcheck-arcs when present).
ARCS = [os.path.join(B, 'selfcheck-arcs')]
_shared = os.environ.get('SELFCHECK_ARCS') or ('/mnt/project-files/selfcheck-arcs'
                                               if os.path.isdir('/mnt/project-files') else '')
if _shared:
    ARCS.append(_shared)


def src_sha(base):
    f = os.path.join(R, 'src', base)
    return hashlib.sha256(open(f, 'rb').read()).hexdigest() if os.path.exists(f) else None


def shared_arcs(sid, c64_taken, miss, shared):
    """Shared-runtime arcs (steward, Lane F LLGE), in src/agc_*.c and the interp
    orders/steps (not a milestone's own files). Whether find() takes an arc can
    depend on table linkage (a KEEP-mask group's table holds only its own
    routines). Record the shared-source arcs this spec takes on C64 (ARCS); a
    host arc no group takes in a shared source is a NOTE when another spec's
    run on the same source sha256 took it on C64, else it stays a GAP. Returns
    the misses still gaps."""
    base = {os.path.basename(x) for x in shared if re.match(r'src/(agc_\w+|\w+_interp_(orders|steps))\.c$', x)}
    mine = collections.defaultdict(list)
    for k, ln, bi in c64_taken:
        b = k.split('>')[-1]
        if b in base:
            mine[b].append([ln, bi])
    for b, v in mine.items():
        sha = src_sha(b)
        for root in ARCS:
            try:
                os.makedirs(os.path.join(root, sha), exist_ok=True)
                with open(os.path.join(root, sha, sid + '.json'), 'w') as f:
                    json.dump({'source': 'src/' + b, 'spec': sid, 'taken': sorted(v)}, f)
            except OSError as e:
                say('    (could not write %s: %s)' % (root, e))
    seen = collections.defaultdict(set)
    for b in {x[0].split('>')[-1] for x in miss} & base:
        for root in ARCS:
            for f in glob.glob(os.path.join(root, src_sha(b), '*.json')):
                o = json.load(open(f))
                if o.get('spec') != sid and o.get('source') == 'src/' + b:
                    for ln, bi in o['taken']:
                        seen[(b, ln, bi)].add(o['spec'])
    left, wit = [], collections.defaultdict(list)
    for x in miss:
        b = x[0].split('>')[-1]
        if b in base and seen.get((b, x[1], x[2])):
            wit[b].append('%d/%d (%s)' % (x[1], x[2], ','.join(sorted(seen[(b, x[1], x[2])])[:3])))
        else:
            left.append(x)
    for b in sorted(wit):
        say('    NOTE %s: host outcomes no group takes, taken on C64 by another spec on this source '
            '(table linkage, not a gap): %s' % (b, ' '.join(wit[b][:20])))
    for b in sorted({x[0].split('>')[-1] for x in left} & base):
        say('    (%s is shared: an outcome another spec takes on C64 becomes a NOTE once that spec has '
            'run cert_selfcheck on this source; searched %s)' % (b, ', '.join(ARCS)))
    return left


def say(msg):
    print(msg, flush=True)


# ---------------------------------------------------------------- headers

EDIT = re.compile(r'\{0?([0-7]+),0?([0-7]+)\}')
CASE = re.compile(r'\{0x([0-9a-f]+)ul,(\d+),(\d+),(0[0-7]*|\d+),(\d+),(\w+)\}')


def header_rows(text):
    """[(rows, cuts)] per header block: rows are (seed, cm, ie, sb, edits,
    expected digest)."""
    arrays = {n: tuple((int(a, 8), int(w, 8)) for a, w in EDIT.findall(body))
              for n, body in re.findall(r'static const irc_edit (\w+)\[\d+\]=\{(.*?)\};', text)}
    cs = re.search(r'static const irc_case \w+\[\d+\]=\{(.*?)\};', text, re.S)
    ex = re.search(r'static const uint32_t \w+\[\d+\]=\{(.*?)\};', text, re.S)
    if not cs:
        return [], []
    exp = [int(x, 16) for x in re.findall(r'0x([0-9a-f]+)ul', ex.group(1))]
    rows = []
    for (seed, cm, ie, sb, ne, name), d in zip(CASE.findall(cs.group(1)), exp):
        e = arrays[name]
        assert len(e) == int(ne), name
        rows.append((int(seed, 16), int(cm), int(ie), int(sb, 8) if sb.startswith('0') else int(sb), e, d))
    cutm = re.search(r'irc_c64_cut\[\d+\]=\{(.*?)\};', text)
    cuts = [int(x, 8) for x in cutm.group(1).split(',')] if cutm else [0] * len(rows)
    return rows, cuts


# ---------------------------------------------------------------- case file

def ebytes(edits):
    return b''.join(struct.pack('<HH', a, w) for a, w in edits)


class CaseFile:
    """The frozen case file: its header bytes, and each record's offset;
    keyed by md5(seed, edits) for matching header rows."""

    def __init__(self, path):
        self.b = open(path, 'rb').read()
        b, o = self.b, 4
        for _ in range(2):
            _, nr = struct.unpack_from('<2H', b, o)
            o += 4 + 4 * nr
            for _ in range(2):
                (n,) = struct.unpack_from('<H', b, o)
                o += 2 + 2 * n
        self.head = b[:o]
        (nc,) = struct.unpack_from('<I', b, o)
        o += 4
        self.off, self.key = [], {}
        for i in range(nc):
            seed, cm, ie, sb, stop, ne = struct.unpack_from('<IBBHHH', b, o)
            s = o
            o += 12 + (2 if ie & 0x80 else 0)
            self.off.append((s, o, o + 4 * ne))
            self.key.setdefault(hashlib.md5(struct.pack('<I', seed) + b[o:o + 4 * ne]).digest(), i)
            o += 4 * ne

    def __len__(self):
        return len(self.off)

    def fields(self, i):
        s, e0, e1 = self.off[i]
        seed, cm, ie, sb, stop, ne = struct.unpack_from('<IBBHHH', self.b, s)
        edits = [struct.unpack_from('<HH', self.b, e0 + 4 * k) for k in range(ne)]
        return seed, cm, ie & 0x7f, sb, stop, edits

    def match(self, seed, edits):
        """The case whose edits are edits, or a prefix of them (the header
        may carry extra register edits after the case's own)."""
        for k in range(len(edits), max(len(edits) - 6, 0), -1):
            i = self.key.get(hashlib.md5(struct.pack('<I', seed) + ebytes(edits[:k])).digest())
            if i is not None:
                return i
        return None


def record(seed, cm, ie, sb, stop, edits, cut=0):
    r = struct.pack('<IBBHHH', seed, cm, ie | (0x80 if cut else 0), sb, stop, len(edits))
    return r + (struct.pack('<H', cut) if cut else b'') + ebytes(edits)


def write_bin(path, head, recs):
    with open(path, 'wb') as f:
        f.write(head + struct.pack('<I', len(recs)) + b''.join(recs))


# ---------------------------------------------------------------- gcov

def gcov_dir(d):
    """gcov -b per object (per .gcda), each into its own directory."""
    for gc in glob.glob(d + '/*.gcda'):
        b = os.path.basename(gc)[:-5]
        g = os.path.join(d, 'g', b)
        os.makedirs(g, exist_ok=True)
        subprocess.run(['gcov', '-b', '-c', '-p', '-o', d, gc], cwd=g, capture_output=True)


def gcov_parse(d):
    br, ln = {}, set()
    for f in glob.glob(d + '/g/*/*.gcov'):
        obj = f.split('/')[-2].replace('runner-', '')
        src = os.path.basename(f).split('#')[-1].replace('.gcov', '')
        k = obj + '>' + src
        line = bi = 0
        for row in open(f, errors='replace'):
            m = re.match(r'\s*([^:]+):\s*(\d+):', row)
            if m:
                line, bi = int(m.group(2)), 0
                c = m.group(1).strip().rstrip('*')
                if c.isdigit() and int(c) > 0:
                    ln.add((k, line))
                continue
            m = re.match(r'branch\s+(\d+)\s+(taken (\d+)|never executed)', row)
            if m:
                br[(k, line, bi)] = int(m.group(3) or 0)
                bi += 1
    return br, ln


# ---------------------------------------------------------------- interp spec

def run_out(cmd, env=None, cwd=R):
    p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, env=env)
    if p.returncode:
        sys.exit('FAIL: %s exited %d\n%s' % (' '.join(cmd), p.returncode, p.stderr[-2000:]))
    return [x.split() for x in p.stdout.splitlines()]


def interp(sid, a, out):
    import interp_compile as IC
    import interp_routine_differential as D
    import interp_specs
    spec = interp_specs.SPECS[sid]
    sym = sid.replace('-', '_')
    ev = os.path.join(R, 'evidence', sid)
    proof_p = os.path.join(ev, '%s-proof.json' % sid)
    if not os.path.exists(proof_p):
        subprocess.run([sys.executable, 'tools/evidence.py', 'fetch', sid], cwd=R, check=True)
    proof = json.load(open(proof_p))
    cover_p = os.path.join(ev, '%s-c64-cover.json' % sid)
    cover = json.load(open(cover_p)) if os.path.exists(cover_p) else {}
    pins = cover.get('linked_sources')
    if isinstance(pins, dict):
        # A source other cover records also pin (interp orders/steps, interp_case/cut,
        # agc_transfer, ...) goes stale whenever another lane changes it: report it,
        # and let the branch re-derivation below decide (steward, #450). A stale pin
        # on the spec's own sources means this record was not refrozen.
        shared = shared_sources()
        for src, h in sorted(pins.items()):
            f = os.path.join(R, src)
            now = hashlib.sha256(open(f, 'rb').read()).hexdigest() if os.path.exists(f) else 'missing'
            if now == h:
                continue
            msg = '%s pins %s at %s, the tree has %s' % (os.path.relpath(cover_p, R), src, h[:8], now[:8])
            if src in shared:
                say('NOTE %s (a shared source; the branch check below re-derives it)' % msg)
            else:
                gap(msg + ': refreeze so the cover record matches this tree')
    if not all(os.path.exists(p) for p in D.ROPES):
        from fixed_data_generate import assemble
        for m in ('Luminary099', 'Comanche055'):
            assemble(m, pathlib.Path(B) / 'fixed-data')
    env = dict(os.environ, **D.oracle_env(spec))
    missions = [D.Mission(spec, m) if m in spec['missions'] else D.Absent(m) for m in ('lm', 'cm')]
    npts = sum(len(mi.case_points) for mi in missions if mi.m in spec['missions'])
    per = -(-proof['cases'] // npts)
    flags = spec_keys(spec)
    say('%s: %d frozen cases, %d routines -> --cases %d%s' % (sid, proof['cases'], npts, per,
                                                             (' (%s)' % ', '.join(flags)) if flags else ''))

    # 1. differential
    frozen = ['tests/%s_vectors.h' % sym, 'tests/%s_c64_vectors.h' % sym, 'tools/mk/%s.groups' % sid,
              'evidence/%s' % sid]
    dirty = subprocess.run(['git', 'status', '--porcelain', '--', *frozen], cwd=R, capture_output=True,
                           text=True).stdout.strip()
    if dirty:
        gap('frozen files differ from the commit: ' + '; '.join(dirty.splitlines()))
    path = os.path.join(B, '%s-cases.bin' % sid)
    sha = proof.get('cases_sha256')
    t0 = time.time()
    if a.skip_differential and os.path.exists(path):
        say('[1] differential skipped: reusing %s' % os.path.relpath(path, R))
    else:
        log = os.path.join(out, 'differential.log')
        with open(log, 'w') as f:
            p = subprocess.run([sys.executable, 'tools/interp_routine_differential.py', sid, '--cases', str(per)],
                               cwd=R, stdout=f, stderr=subprocess.STDOUT, env=dict(env, IRC_SALT=''))
        text = open(log).read()
        m = re.search(r'^PASS: (\d+) \S+ cases equal the original programs \((\d+) routines\)', text, re.M)
        say('[1] differential (%.0f s): %s' % (time.time() - t0, m.group(0) if m else 'no PASS line'))
        if p.returncode or not m:
            gap('differential failed (exit %d): %s' % (p.returncode, ' | '.join(
                x for x in text.splitlines() if x.startswith(('FAIL', 'mismatches', 'case ')))[:600] or text[-600:]))
        elif int(m.group(1)) != proof['cases']:
            gap('differential compared %s cases, the proof froze %d' % (m.group(1), proof['cases']))
        for x in text.splitlines():
            if x.startswith(('both sides loop', 'refused', 'waiting for', 'resume_flow:', 'flow states:')):
                say('    ' + x)
    if not os.path.exists(path):
        gap('no case file %s' % os.path.relpath(path, R))
        return
    got = hashlib.sha256(open(path, 'rb').read()).hexdigest()
    if not sha:
        gap('the proof pins no cases_sha256')
    elif got != sha:
        gap('case set sha256 %s.. is not the proof\'s %s..' % (got[:16], sha[:16]))
    else:
        say('    case set sha256 %s.. = proof' % sha[:16])
    after = subprocess.run(['git', 'status', '--porcelain', '--', *frozen], cwd=R, capture_output=True,
                           text=True).stdout.strip()
    if after != dirty:
        gap('the differential changed frozen files: ' + '; '.join(after.splitlines()))
    cf = CaseFile(path)
    if len(cf) != proof['cases']:
        gap('case file has %d cases, the proof %d' % (len(cf), proof['cases']))

    # 2. rows
    hv = open(os.path.join(R, 'tests/%s_vectors.h' % sym)).read()
    host_rows, _ = header_rows(hv)
    ctext = open(os.path.join(R, 'tests/%s_c64_vectors.h' % sym)).read()
    sect = {int(g): body for g, body in re.findall(r'#if IRC_GROUP == (\d+)\n(.*?)#endif', ctext, re.S)}
    groups = {g: header_rows(body) for g, body in sect.items()}
    nc64 = sum(len(r) for r, _ in groups.values())
    if proof.get('frozen_vectors') is not None and len(host_rows) != proof['frozen_vectors']:
        gap('tests/%s_vectors.h has %d vectors, the proof %d' % (sym, len(host_rows), proof['frozen_vectors']))
    want = sum(g.get('cases', 0) for g in proof.get('c64_vectors', []))
    if want and want != nc64:
        gap('tests/%s_c64_vectors.h has %d rows, the proof\'s c64_vectors %d' % (sym, nc64, want))
    resumed = [t for t in cover.get('truncated_rows', []) if 'resumed_at' in t]
    # A segment row is a C64 row of its segment routine's own group
    # (c64_vectors entry, in header group order, holding only that routine's
    # segment rows): only there does it stop at the resume LOC. The same case
    # as a host vector or in another group is a full run.
    cv = proof.get('c64_vectors', [])
    if len(cv) != len(groups) or any(len(groups[g + 1][0]) != v.get('cases') for g, v in enumerate(cv)
                                     if g + 1 in groups):
        cv = []
    segs = {}   # (group, case) -> resume LOC
    seg_rows = (proof.get('c64_segments') or {}).get('rows', [])
    seg_of = collections.defaultdict(set)
    for s in seg_rows:
        seg_of[s['mission'], s['from']].add(s['case'])
    for s in seg_rows:
        mi = next(mi for mi in missions if mi.m == s['mission'])
        hit = [g + 1 for g, v in enumerate(cv) if v.get('mission') == s['mission'] and s['case'] in v.get('case_ids', [])
               and set(v['case_ids']) <= seg_of[s['mission'], s['from']]]
        loc = D.loc_of(mi, s['to'])
        if len(hit) != 1 or loc is None:
            gap('c64_segments row %s %s to %s (case %d): %s' % (
                s['mission'], s['from'], s['to'], s['case'],
                'no resume LOC %s' % s['to'] if loc is None else '%d C64 groups hold it with only its routine\'s segment rows' % len(hit)))
            continue
        segs[hit[0], s['case']] = loc
    stats = collections.Counter()

    def rebuild(row, cut, what, g=None):
        """(record, snaprow) of a header row (g: its C64 group), or None."""
        seed, cm, ie, sb, edits, _ = row
        i = cf.match(seed, edits)
        if i is not None:
            stop = cf.fields(i)[4]
            stats['matched'] += 1
            if len(edits) != len(cf.fields(i)[5]):
                stats['prefix'] += 1
            if (g, i) in segs:
                stats['segment'] += 1
                return record(seed, cm, ie, sb, segs[g, i], edits, cut), True
            return record(seed, cm, ie, sb, stop, edits, cut), False
        for t in resumed:   # a resumed row: the parent case run on to its snapshot
            if t['case'] >= len(cf):
                continue
            pseed, pcm, _, _, _, pe = cf.fields(t['case'])
            E = collections.OrderedDict(pe)
            E.update((x, w) for x, w in t['snapshot'])
            if pseed == seed and tuple(E.items()) == edits:
                if hashlib.sha256(json.dumps(t['snapshot']).encode()).hexdigest() != t.get('snapshot_sha256'):
                    gap('%s: resumed row %d snapshot does not hash to its snapshot_sha256' % (what, t['row']))
                stats['resumed'] += 1
                return record(seed, cm, ie, sb, 0xffff, edits, cut), False
        gap('%s: row (seed %08x, %d edits) matches no frozen case%s' % (
            what, seed, len(edits), ' nor resumed row' if resumed else ''))
        return None

    checks = []   # (what, record, expected, snaprow)
    for k, row in enumerate(host_rows):
        r = rebuild(row, 0, 'host vector %d' % k)
        if r:
            checks.append(('host vector %d' % k, r[0], row[5], r[1]))
    grecs = {}
    for g, (rows, cuts) in sorted(groups.items()):
        grecs[g] = []
        for k, (row, cut) in enumerate(zip(rows, cuts)):
            stats['cut'] += bool(cut)
            r = rebuild(row, cut, 'group %d row %d' % (g, k), g)
            if r:
                checks.append(('group %d row %d' % (g, k), r[0], row[5], r[1]))
                grecs[g].append((k, r[0], row[5]))
    ref = os.path.join(B, 'interp-routine-reference%s' % (
        '-flight-device' if spec.get('engine') == 'flight-device' else ''))
    if not os.path.exists(ref):
        ref = D.build(sid, spec)[0]
    bad = 0
    for snaprow in (False, True):
        sel = [c for c in checks if c[3] == snaprow]
        if not sel:
            continue
        p = os.path.join(out, 'oracle%s.bin' % ('-snaprow' if snaprow else ''))
        write_bin(p, cf.head, [c[1] for c in sel])
        e = dict(env, IRC_SNAP='1', IRC_SNAPROW='1') if snaprow else env
        for (what, _, exp, _), o in zip(sel, run_out([ref, *D.ROPES, p], e)):
            if o[1] != '0' or int(o[3]) != exp:
                bad += 1
                if bad <= 20:
                    gap('%s: yaAGC gives kind %s digest %s, the header %d' % (what, o[1], o[-1], exp))
    if bad > 20:
        gap('... %d rows in all differ from yaAGC' % bad)
    if stats['segment'] != len(seg_rows):
        gap('%d C64 rows run as segment rows, the proof\'s c64_segments %d' % (stats['segment'], len(seg_rows)))
    say('[2] rows: %d host vectors, %d C64 rows in %d groups; %d matched to frozen cases (%d by an edit prefix), '
        '%d resumed, %d truncated, %d segment rows; %d of %d re-run on yaAGC equal their frozen digests' % (
            len(host_rows), nc64, len(groups), stats['matched'], stats['prefix'], stats['resumed'], stats['cut'],
            stats['segment'], len(checks) - bad, len(checks)))

    # 3. per-group gcov
    srcs = [os.path.join(R, s) for s in D.cover_sources(spec) + ['tools/interp_routine_native.c']]
    base = ['cc', '-O0', '--coverage'] + [
        x.replace('-Iinclude', '-I' + os.path.join(R, 'include')).replace('-Itests', '-I' + os.path.join(R, 'tests'))
        for x in D.FLAGS if x != '-O2'] + ['-DIRC_MAX_STEPS=%d' % spec.get('max_steps', 200)] + D.WRAP
    grp = open(os.path.join(R, 'tools/mk/%s.groups' % sid)).read()
    pre = sid.replace('-', '_').upper()
    gflags = {int(g): f.split() for g, f in re.findall(r'%s_GROUP_FLAGS_(\d+) = (.*)' % pre, grp)}

    def build_run(tag, extra, bin_path):
        d = os.path.join(out, tag)
        shutil.rmtree(d, ignore_errors=True)
        os.makedirs(d)
        p = subprocess.run(base + extra + srcs + ['-o', os.path.join(d, 'runner')], cwd=d, capture_output=True,
                           text=True)
        if p.returncode:
            sys.exit('FAIL: %s build\n%s' % (tag, p.stderr[-2000:]))
        rows = run_out([os.path.join(d, 'runner'), bin_path], env, cwd=d)
        gcov_dir(d)
        res = gcov_parse(d)
        shutil.rmtree(os.path.join(d, 'g'), ignore_errors=True)
        return rows, res

    host_bin = path
    if spec.get('partial'):
        # (cases waiting for an I/O channel model, never compared: a native
        # refusal in a routine with a channel instruction, off the host side
        # as in the proof's c64_gcov_branches and --cover-check)
        nat = os.path.join(B, '%s-native' % sid)
        if not os.path.exists(nat):
            nat = D.build(sid, spec)[1]
        chan = set()
        for i, r in enumerate(run_out([nat, path], env)):
            mi = missions[cf.fields(i)[1]]
            if r[1] != '0' and any(mi.cp.logical_of(q) == int(r[2]) for q in mi.cp.channel_blocks):
                chan.add(i)
        host_bin = os.path.join(out, 'host.bin')
        write_bin(host_bin, cf.head, [cf.b[cf.off[i][0]:cf.off[i][2]] for i in range(len(cf)) if i not in chan])
        say('    partial: %d cases waiting for an I/O channel model left off the host side' % len(chan))
    t0 = time.time()
    jobs = {}
    with concurrent.futures.ThreadPoolExecutor(max(1, a.jobs)) as ex:
        jobs['host'] = ex.submit(build_run, 'host', [], host_bin)
        for g in sorted(grecs):
            bp = os.path.join(out, 'grp%d.bin' % g)
            write_bin(bp, cf.head, [r for _, r, _ in grecs[g]])
            keep = [x for x in gflags.get(g, []) if '_KEEP' in x]
            if g not in gflags:
                gap('group %d has no flags in tools/mk/%s.groups' % (g, sid))
            jobs[g] = ex.submit(build_run, 'grp%d' % g, keep + D.cut_flags(spec), bp)
    _, (hbr, hln) = jobs['host'].result()
    hcount = collections.Counter((k, ln) for k, ln, _ in hbr)
    host_taken = {x for x, v in hbr.items() if v > 0}
    c64_taken, c64_lines = set(), set()
    per_group = {}
    for g in sorted(grecs):
        rows, (br, ln) = jobs[g].result()
        nbad = 0
        for (k, _, exp), o in zip(grecs[g], rows):
            if o[1] != '0' or int(o[3]) != exp:
                nbad += 1
                gap('group %d row %d: the KEEP-mask native build gives kind %s digest %s, the header %d' % (
                    g, k, o[1], o[3], exp))
        cnt = collections.Counter((k, ln_) for k, ln_, _ in br)
        mine = {x for x, v in br.items() if v > 0 and cnt[x[:2]] == hcount.get(x[:2])}
        mine |= {x for x in host_taken if x[:2] in cnt and cnt[x[:2]] != hcount.get(x[:2]) and x[:2] in ln}
        per_group[g] = (len(grecs[g]), len(mine & host_taken), nbad)
        c64_taken |= mine
        c64_lines |= ln
    miss = sorted(host_taken - c64_taken)
    lmiss = sorted(hln - c64_lines)
    say('[3] gcov (%.0f s): host %d branch outcomes, %d lines over %d cases; C64 groups take %d outcomes' % (
        time.time() - t0, len(host_taken), len(hln), len(cf), len(host_taken & c64_taken)))
    for g, (n, t, nbad) in sorted(per_group.items()):
        say('    group %d: %d rows, %d host outcomes%s' % (g, n, t, (', %d rows differ' % nbad) if nbad else ''))
    # (The runner's own driver is no certified code: a c64_cuts group build
    # runs irc_run_cut where the host build runs irc_run.)
    drv = 'interp_routine_native>interp_routine_native.c'
    for k, what in ((drv, [x for x in miss if x[0] == drv]), (drv, [x for x in lmiss if x[0] == drv])):
        if what:
            say('    NOTE %s (runner driver, not a gap): %s' % (k, ' '.join(str(x[1:]) for x in what[:20])))
    miss = shared_arcs(sid, c64_taken, [x for x in miss if x[0] != drv], shared_sources())
    lmiss = [x for x in lmiss if x[0] != drv]
    by = collections.defaultdict(list)
    for k, ln, bi in miss:
        by[k].append('%d/%d' % (ln, bi))
    for k in sorted(by):
        taken = sum(1 for x in host_taken if x[0] == k)
        gap('%s: %d of %d host branch outcomes no C64 row takes (line/branch): %s' % (
            k, len(by[k]), taken, ' '.join(by[k][:40]) + (' ...' if len(by[k]) > 40 else '')))
    lby = collections.defaultdict(list)
    for k, ln in lmiss:
        lby[k].append(ln)
    for k in sorted(lby):
        gap('%s: host lines no C64 row runs: %s' % (k, ' '.join(map(str, lby[k][:40])) + (' ...' if len(lby[k]) > 40 else '')))
    rec = proof.get('c64_gcov_branches') or {}
    if rec.get('host_taken') is not None:
        say('    (the proof\'s c64_gcov_branches: host %s, C64 %s, missing %s)' % (
            rec.get('host_taken'), rec.get('c64_taken'), rec.get('host_taken_not_c64')))


def spec_keys(spec):
    return [k for k in ('pending_danzig', 'resume_flow', 'flow_states', 'c64_cuts', 'c64_segments', 'channel_inputs',
                        'partial') if spec.get(k)] + (['c64_rows=' + spec['c64_rows']] if spec.get('c64_rows') else [])


# ---------------------------------------------------------------- Lane F

DRV = r'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "%(stem)s_cases.h"
#include "lm_flight_io.h"
#ifdef ROWS
#include "%(stem)s_c64_vectors.h"
#endif
int main(void){apollo_overlay_state s;lm_flight_io io;uint16_t steps;unsigned i,n;(void)io;(void)steps;
#ifdef ROWS
 for(i=0;i<%(mac)s_C64_COUNT;++i){n=%(sym)s_c64_case[i];
#else
 for(i=0;i<%(mac)s_CASES;++i){n=i;
#endif
  (void)RUN(&s,&io,(uint16_t)n,&steps);}
 return 0;}
'''


def lanef(m, a, out):
    """Lane F module: gcc --coverage over its src/ files, all host cases
    against each C64 PRG/group build's rows (tests/<stem>_c64_vectors.h)."""
    import lane_f_c64_rows as L
    stem, mac, sym, _, _, srcvar, run = L.MODULES[m]
    srcs = L.mk_var(m, srcvar)
    groups, macro = [None], None
    if m in L.GROUPS:
        macro = L.GROUPS[m][0]
    elif m in L.SEGMENTS:
        macro = L.SEGMENTS[m][0]
    if macro:
        txt = (L.R / f'tests/{stem}_c64_vectors.h').read_text()
        groups = sorted({int(g) for g in re.findall(rf'#if {macro}==(\d+)', txt)})
    out = pathlib.Path(out)

    def build_run(tag, defs):
        d = out / tag
        shutil.rmtree(d, ignore_errors=True)
        d.mkdir(parents=True)
        (d / 'drv.c').write_text(DRV % dict(stem=stem, mac=mac, sym=sym))
        base = ['gcc', '-std=gnu99', '-O0', '-w', '-I', str(L.R / 'include'), '-I', str(L.R / 'tests'),
                '-I', str(L.R / 'src'), '-DP66_NATIVE_DISPATCH_ONLY', f'-DRUN(s,io,n,steps)={run.format(s=sym)}', *defs]
        objs = []
        for src in srcs:
            o = d / (pathlib.Path(src).stem + '.o')
            cov = ['--coverage'] if src.startswith('src/') else []
            subprocess.run(base + cov + ['-c', str(L.R / src), '-o', str(o)], cwd=d, check=True)
            objs.append(str(o))
        subprocess.run(base + ['-c', str(d / 'drv.c'), '-o', str(d / 'drv.o')], cwd=d, check=True)
        link = ['gcc', '--coverage', *objs, str(d / 'drv.o'), '-lm', '-o', str(d / 'run')]
        r = subprocess.run(link, cwd=d, capture_output=True, text=True)
        dead = sorted(set(re.findall(r"undefined reference to `(\w+)'", r.stderr)))
        if r.returncode:
            if not dead:
                sys.exit(r.stderr)
            (d / 'stubs.c').write_text('#include <stdlib.h>\n' + ''.join(f'void {f}(void){{abort();}}\n' for f in dead))
            subprocess.run(['gcc', '-w', '-c', str(d / 'stubs.c'), '-o', str(d / 'stubs.o')], check=True)
            subprocess.run(link + [str(d / 'stubs.o')], cwd=d, check=True)
        subprocess.run([str(d / 'run')], cwd=L.R, check=True, timeout=7200)
        taken, lines, nbr = set(), set(), {}
        for src in srcs:
            if not src.startswith('src/'):
                continue
            st = pathlib.Path(src).stem
            g = d / 'g' / st
            g.mkdir(parents=True)
            subprocess.run(['gcov', '-b', '-c', '-o', str(d / (st + '.o')), str(L.R / src)], cwd=g, capture_output=True)
            for f in g.glob('*.gcov'):
                name, line = f.name[:-5], None
                for ln in f.read_text(errors='replace').splitlines():
                    mm = re.match(r'\s*[^:]+:\s*(\d+):', ln)
                    if mm:
                        line = int(mm.group(1))
                        cnt = ln.split(':')[0].strip().rstrip('*')
                        if cnt.isdigit() and int(cnt) > 0:
                            lines.add((name, line))
                        continue
                    mb = re.match(r'branch\s+(\d+) (taken (\d+)|never executed)', ln)
                    if mb and line:
                        nbr[(name, line)] = max(nbr.get((name, line), 0), int(mb.group(1)) + 1)
                        if mb.group(3) and int(mb.group(3)) > 0:
                            taken.add((name, line, int(mb.group(1))))
        return taken, lines, nbr

    say('%s: Lane F module; no differential here (its own make targets run it); branch check only' % m)
    with concurrent.futures.ThreadPoolExecutor(max(1, a.jobs)) as ex:
        host_j = ex.submit(build_run, 'host', [])
        gj = {g: ex.submit(build_run, f'c64-g{g}', ['-DROWS'] + ([f'-D{macro}={g}'] if g is not None else []))
              for g in groups}
    host, hl, hn = host_j.result()
    c64, cl = set(), set()
    for g in groups:
        t, ln, n = gj[g].result()
        mine = {x for x in t if n.get(x[:2]) == hn.get(x[:2])}
        mine |= {x for x in host if x[:2] in n and n[x[:2]] != hn.get(x[:2]) and x[:2] in ln}
        say('    build %s: %d host outcomes' % (g, len(mine & host)))
        c64 |= mine
        cl |= ln
    miss, lmiss = shared_arcs(m, c64, sorted(host - c64), shared_sources()), sorted(hl - cl)
    say('[3] gcov: host %d branch outcomes, C64 rows take %d; host lines %d' % (len(host), len(host & c64), len(hl)))
    for x in miss:
        gap('%s:%d branch %d: host takes it, no C64 row does' % x)
    for x in lmiss:
        gap('%s:%d: host runs the line, no C64 row does' % x)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('spec', help='interp spec (tools/interp_specs.py) or Lane F milestone')
    ap.add_argument('--jobs', type=int, default=2, help='parallel group builds (default 2)')
    ap.add_argument('--keep', action='store_true', help='keep the scratch directory')
    ap.add_argument('--skip-differential', action='store_true',
                    help='reuse build/<id>-cases.bin (still checked against cases_sha256)')
    ap.add_argument('--out', help='scratch directory (default build/selfcheck/<id>)')
    a = ap.parse_args()
    import interp_specs
    out = os.path.abspath(a.out or os.path.join(B, 'selfcheck', a.spec))
    shutil.rmtree(out, ignore_errors=True)
    os.makedirs(out)
    t0 = time.time()
    try:
        if a.spec in interp_specs.SPECS:
            interp(a.spec, a, out)
        else:
            import lane_f_c64_rows as L
            if a.spec not in L.MODULES:
                sys.exit('%s: neither an interp spec nor a Lane F module' % a.spec)
            lanef(a.spec, a, out)
    finally:
        if not a.keep:
            shutil.rmtree(out, ignore_errors=True)
    say('')
    if GAPS:
        say('REJECT %s: %d gap%s (%.0f s)' % (a.spec, len(GAPS), 's' if len(GAPS) > 1 else '', time.time() - t0))
        for x in GAPS[:60]:
            say('  - ' + x[:400])
        sys.exit(1)
    say('OK %s: no gaps (%.0f s)' % (a.spec, time.time() - t0))


if __name__ == '__main__':
    main()
