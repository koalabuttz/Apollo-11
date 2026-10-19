#!/usr/bin/env python3
"""Untaken branch arms of each spec's generated C over its frozen host cases.

A certificate shows the frozen host cases equal yaAGC, but an arm no case
takes is never compared. This tool keeps the list of such arms, each one
triaged with a reason, in tools/cert_arms_allow.py next to the specs:

  IMPOSSIBLE  no AGC state reaches it (the reason says why)
  OFFNOM      only alarms, aborts, restarts, failed sensors or bad data
  FLIGHT      ordinary flight reaches it: an open gap (the reason says
              how); a case that takes it is still owed
  UNSURE      not settled either way (the reason says what would settle it)

  cert_arms.py measure SPEC...   regenerate SPEC's frozen cases (as the
      differential does), build its own sources with gcc --coverage, run
      every case, and write the untaken arms into the allowlist. Arms
      already there keep their class and reason; new ones are UNTRIAGED.
      Needs the oracle toolchain (deps/env.sh), like a refreeze.
  cert_arms.py check             (CI: ledgers-and-coverage; no toolchain)
      fails on a certified spec with no record and on an UNTRIAGED arm
      or one with no reason; warns on FLIGHT and UNSURE arms, on a spec whose
      record is stale (its proof's cases_sha256 or an own source changed
      since it was measured: run measure again; tools/cert_precheck.py
      fails on that, so a re-cite carries a fresh record).

An arm is an AGC word's decision outcome that no copy of the word takes
(the word's '/* bb,aaaa OP */' comment in the generated C): a BZF/BZMF
branch or fall-through, a CCS exit, BON/BOFF/BPL/BMN/BZE/BHIZ/TIX
outcomes, a BOV/BOVB overflow (through GOTO for an interpretive target or
RTB for a basic one) or its no-overflow path, and words no case executes.
TS overflow skips, UNIT/SQRT/ACOS overflow arms and the runner's scope
checks are left out.
"""
import collections
import hashlib
import json
import os
import random
import re
import shutil
import subprocess
import sys

R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(R, 'tools'))
ALLOW = os.path.join(R, 'tools', 'cert_arms_allow.py')
CLASSES = ('IMPOSSIBLE', 'OFFNOM', 'FLIGHT', 'UNSURE', 'UNTRIAGED')
CCS = ('>0', '+0', '<0', '-0')

AGC = re.compile(r'/\*\s*(\d\d,\d{4})\s+(.*?)\s*\*/')
DEC = re.compile(r'(if|switch)\((ai_(ccs|bzf|bzmf|ovsk)|[lc]mi_(switch|bplbmn|bov|bzegoto|tix|bhiz|bze|bpl))\b')
SCOPE = re.compile(r'return EXEC_OUT_OF_SCOPE;?$|EXEC_INVALID')


def sha(path):
    return hashlib.sha256(open(os.path.join(R, path), 'rb').read()).hexdigest()


def own_sources(spec):
    return ['src/%s.c' % spec['base'][m] for m in spec['missions']]


def what(code, tk):
    """The untaken outcomes of one decision (counts summed over copies), or
    None when every outcome is taken or the decision is a scope check."""
    if '_bov' in code:
        ov = tk[1] + (tk[2] if len(tk) > 2 else 0)
        return None if tk[0] and ov else 'BOV never overflows' if not ov else 'BOV always overflows'
    if SCOPE.search(code):
        return None
    if 'ai_ccs' in code:
        miss = [CCS[i] for i in range(4) if i < len(tk) and tk[i] == 0]
        return 'CCS never ' + '/'.join(miss) if miss else None
    if len(tk) == 2 and 0 in tk:
        return 'branch never taken' if tk[0] == 0 else 'never falls through'
    return None


def parse_gcov(text, src):
    """(arms, unreached) of one source from gcov -b -c -t output: arms as
    {agc, op, what, line}, with each word's copies summed; unreached as the
    words no copy executes."""
    lines, order, cur = {}, [], None
    for l in text.splitlines():
        m = re.match(r'\s*-:\s*0:Source:(.*)', l)
        if m:
            cur = m.group(1)
            continue
        if cur is None or os.path.basename(cur) != os.path.basename(src):
            continue
        m = re.match(r'\s*([^:]+):\s*(\d+):(.*)', l)
        if m:
            c = m.group(1).strip().rstrip('*')
            ln = int(m.group(2))
            lines[ln] = {'count': int(c) if c.isdigit() else 0 if c[:1] in '#=' else None,
                         'code': m.group(3), 'br': []}
            order.append(ln)
            continue
        m = re.match(r'branch\s+(\d+)\s+(?:taken (\d+)|never executed)', l)
        if m and order:
            lines[order[-1]]['br'].append(int(m.group(2) or 0))
    decisions = collections.OrderedDict()   # (agc, code) -> [line, op, summed counts, code]
    words = collections.OrderedDict()       # agc -> [line, op, run]: True, False (code, never run) or None (data)
    last = pend = None
    for ln in order:
        L = lines[ln]
        mm = AGC.search(L['code'])
        if mm and 'part key' not in L['code']:
            last = pend = (ln, mm.group(1), mm.group(2))
            words.setdefault(mm.group(1), [ln, mm.group(2), None])
            continue
        if L['count'] is not None and pend:
            w = words[pend[1]]
            w[2] = True if L['count'] or w[2] else False
            pend = None
        if L['br'] and last and ln - last[0] <= 12 and DEC.search(L['code']):
            k = (last[1], L['code'].strip()[:40])
            d = decisions.setdefault(k, [ln, last[2], None, L['code'].strip()])
            d[2] = L['br'] if d[2] is None else [x + y for x, y in zip(d[2], L['br'])] \
                if len(L['br']) == len(d[2]) else d[2]
    arms, done = [], set()
    for (agc, _), (ln, op, tk, code) in decisions.items():
        w = what(code, tk) if tk and any(tk) else None
        if w and (agc, w) not in done and words.get(agc, [0, 0, True])[2] is not False:
            done.add((agc, w))
            arms.append({'agc': agc, 'op': op, 'what': w, 'line': ln})
    # (Data words, which the generator marks ': data, never executed', bare
    # labels and the rope tables read under INDEX ('LABEL+0 .. +N') have no
    # instruction to run.)
    unreached = [{'agc': a, 'op': op, 'what': 'never executed', 'line': ln}
                 for a, (ln, op, hit) in words.items()
                 if hit is False and 'never executed' not in op and ' ' in op.strip() and ' .. ' not in op]
    return arms + unreached


def measure_spec(sid):
    import interp_routine_differential as D
    import interp_specs
    spec = interp_specs.SPECS[sid]
    os.chdir(R)
    os.environ.update(D.oracle_env(spec))
    missions = [D.Mission(spec, m) if m in spec['missions'] else D.Absent(m) for m in ('lm', 'cm')]
    if spec.get('flow_states'):
        D.flow_states(sid, spec, missions)
    rng = random.Random('interp-routine:' + sid)
    proof = json.load(open(os.path.join(R, 'evidence', sid, sid + '-proof.json')))
    n = sum(len(mi.case_points) for mi in missions if mi.m in spec['missions'])
    per = max(1, -(-proof.get('cases', 64 * n) // n))
    cases = []
    for cm, mi in enumerate(missions):
        if mi.m not in spec['missions']:
            continue
        for p in mi.case_points:
            for k in range(per):
                seed, edits, ie, stop, sb = D.case(mi, p, k, rng)
                cases.append({'cm': cm, 'point': p, 'seed': seed, 'edits': D.Edits(edits), 'ie': ie,
                              'stop': stop, 'sb': sb})
    w = os.path.join(D.B, '%s-arms' % sid)
    shutil.rmtree(w, ignore_errors=True)
    os.makedirs(w)
    path = os.path.join(w, 'cases.bin')
    ref, nat = D.build(sid, spec)
    D.BUILT[:] = [ref, nat]
    if spec.get('resume_flow'):
        D.flow_seed(missions, cases, rng, ref, path, spec)
    D.write_cases(path, missions, cases)
    cases = D.frozen_only(missions, cases, ref, nat, path)
    cases_sha = hashlib.sha256(open(path, 'rb').read()).hexdigest()
    if cases_sha != proof.get('cases_sha256'):
        raise SystemExit('%s: the regenerated cases are not the frozen ones (refreeze first)' % sid)
    srcs = list(dict.fromkeys(own_sources(spec) + spec.get('native_sources', []) + D.common(spec) + [spec['target']]))
    defs = ['-DIRC_MAX_STEPS=%d' % spec.get('max_steps', 200)] + D.cut_flags(spec)
    objs = []
    for s in srcs:
        o = os.path.join(w, os.path.basename(s)[:-2] + '.o')
        subprocess.run(['gcc', '-std=c99', '-O0', '--coverage', '-Iinclude', '-Itests', *defs, '-c', s, '-o', o],
                       cwd=R, check=True)
        objs.append(o)
    subprocess.run(['gcc', '-O0', '--coverage', '-Iinclude', '-Itests', *defs, *objs, *D.WRAP,
                    'tools/interp_routine_native.c', '-o', os.path.join(w, 'run')], cwd=R, check=True)
    subprocess.run([os.path.join(w, 'run'), path], cwd=R, check=True, stdout=subprocess.DEVNULL)
    arms = []
    for s in own_sources(spec):
        o = os.path.join(w, os.path.basename(s)[:-2] + '.gcda')
        out = subprocess.run(['gcov', '-b', '-c', '-t', '-o', w, o], cwd=R, check=True,
                             capture_output=True, text=True).stdout
        arms += [dict(a, src=s) for a in parse_gcov(out, s)]
    shutil.rmtree(w, ignore_errors=True)
    return {'cases': len(cases), 'cases_sha256': cases_sha,
            'sources_sha256': {s: sha(s) for s in own_sources(spec)}, 'arms': arms}


def key(a):
    return (a['src'], a['agc'], a['what'])


def load():
    if not os.path.exists(ALLOW):
        return {}
    g = {}
    exec(compile(open(ALLOW).read(), ALLOW, 'exec'), g)
    return g['ARMS']


HEADER = '''"""Triaged untaken arms per spec (tools/cert_arms.py, which writes it).

Each spec: the frozen cases and own sources it was measured on, and every
branch arm or word of its generated C that no frozen host case takes, with
a class and the reason (IMPOSSIBLE, OFFNOM, FLIGHT: an open gap, UNSURE:
not settled, UNTRIAGED: not looked at yet; CI fails on UNTRIAGED). 'line' is
where the arm was when measured, for reading only. Edit 'class' and 'why';
'cert_arms.py measure SPEC' keeps them by (src, agc, what).
"""

'''


def save(allow):
    """One spec record per block and one arm per line, so a triage edit is a
    one-line diff."""
    out = [HEADER, 'ARMS = {\n']
    for sid in sorted(allow):
        r = allow[sid]
        out.append('    %r: {\n' % sid)
        out.append('        %r: %r, %r: %r,\n' % ('cases', r['cases'], 'cases_sha256', r['cases_sha256']))
        out.append('        %r: %r,\n' % ('sources_sha256', dict(sorted(r['sources_sha256'].items()))))
        out.append('        %r: [\n' % 'arms')
        for a in r['arms']:
            out.append('            {%s},\n' % ', '.join('%r: %r' % (k, a[k]) for k in
                                                       ('src', 'line', 'agc', 'op', 'what', 'class', 'why')))
        out.append('        ],\n    },\n')
    out.append('}\n')
    open(ALLOW, 'w').write(''.join(out))


def merge(old, new):
    prev = {key(a): a for a in (old or {}).get('arms', [])}
    for a in new['arms']:
        p = prev.get(key(a))
        a['class'] = p['class'] if p else 'UNTRIAGED'
        a['why'] = p['why'] if p else ''
    new['arms'].sort(key=lambda a: (a['src'], a['line']))
    return new


def certified():
    """Specs with a certificate (evidence/<spec>/<spec>-proof.json)."""
    import interp_specs
    return sorted(s for s in interp_specs.SPECS if os.path.exists(os.path.join(R, 'evidence', s, s + '-proof.json')))


def stale(sid, rec, spec):
    """What the record was measured on that has changed since."""
    proof = json.load(open(os.path.join(R, 'evidence', sid, sid + '-proof.json')))
    out = ['frozen cases'] if proof.get('cases_sha256') != rec['cases_sha256'] else []
    return out + [s for s in own_sources(spec) if not os.path.exists(os.path.join(R, s))
                  or rec['sources_sha256'].get(s) != sha(s)]


def problems(sid, rec, spec):
    """Why a certified spec's record is not current and fully triaged (for
    tools/cert_precheck.py), or None."""
    if rec is None:
        return 'is missing'
    old = stale(sid, rec, spec)
    if old:
        return 'was measured on other ' + ', '.join(old)
    n = sum(a.get('class') not in CLASSES or a['class'] == 'UNTRIAGED' or not a.get('why') for a in rec['arms'])
    return '%d arm(s) untriaged' % n if n else None


def check():
    import interp_specs
    allow, fails, warns = load(), [], []
    for sid in certified():
        rec = allow.get(sid)
        if rec is None:
            fails.append('%s: certified but not measured (run tools/cert_arms.py measure %s)' % (sid, sid))
            continue
        old = stale(sid, rec, interp_specs.SPECS[sid])
        if old:
            warns.append('%s: measured on other %s; run tools/cert_arms.py measure %s' % (sid, ', '.join(old), sid))
        for a in rec['arms']:
            if a.get('class') not in CLASSES:
                fails.append('%s %s:%d %s %s: class %r is not one of %s' % (sid, a['src'], a['line'], a['agc'],
                                                                            a['what'], a.get('class'), CLASSES))
            elif a['class'] == 'UNTRIAGED' or not a.get('why'):
                fails.append('%s %s:%d %s %s %s: untriaged (give it a class and a reason in tools/cert_arms_allow.py)'
                             % (sid, a['src'], a['line'], a['agc'], a['op'], a['what']))
            elif a['class'] in ('FLIGHT', 'UNSURE'):
                warns.append('%s %s:%d %s %s %s: %s (%s)' % (
                    sid, a['src'], a['line'], a['agc'], a['op'], a['what'],
                    'flight gap' if a['class'] == 'FLIGHT' else 'unsure', a['why']))
    for sid in sorted(set(allow) - set(certified())):
        warns.append('%s: in the allowlist but has no certificate' % sid)
    for w in warns:
        print('::warning::' + w if os.environ.get('GITHUB_ACTIONS') else 'warning: ' + w)
    for f in fails:
        print('::error::' + f if os.environ.get('GITHUB_ACTIONS') else 'error: ' + f)
    n = sum(len(r['arms']) for r in allow.values())
    cl = collections.Counter(a.get('class') for r in allow.values() for a in r['arms'])
    print('cert_arms: %d specs measured, %d untaken arms (%s), %d failures' % (
        len(allow), n, ', '.join('%d %s' % (cl[c], c) for c in CLASSES if cl[c]), len(fails)))
    return 1 if fails else 0


def main(argv):
    if argv[:1] == ['check'] and len(argv) == 1:
        return check()
    if argv[:1] == ['measure'] and len(argv) > 1:
        allow = load()
        for sid in argv[1:]:
            allow[sid] = merge(allow.get(sid), measure_spec(sid))
            save(allow)
            print('%s: %d untaken arms, %d untriaged' % (
                sid, len(allow[sid]['arms']), sum(a['class'] == 'UNTRIAGED' for a in allow[sid]['arms'])))
        return 0
    print(__doc__)
    return 2


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
