#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run the original-program oracle for lm-servicer and freeze digests.

Builds the host-only yaAGC driver tools/lm_servicer_reference.c (on yaAGC
with the hardware divide, build/agc_engine_hardware_dv.o), runs it on the
unmodified Luminary099 rope, and checks every case the native snapshot
(build/lm-servicer-native) runs is byte-identical to it; a case the native
routine refuses must leave its image unchanged. Writes
tests/lm_servicer_vectors.h (one digest per case: two 16-bit running sums
over the kind and, unless refused, all 2048 erasable words and the
channels 11, 12, 32 and 33 and the interrupt-enable state) and
build/lm-servicer-proof.json.

The C64 build is split into segment groups (LMSV_C64_GROUP in
src/lm_servicer.c), each holding part of the file's code and ending a run
before the first instruction or order pair outside it. For each group the
host build of that group runs its keys' cases, and the oracle runs them
with the same segment ends (tools/lm_servicer_reference.c with a segment
file); every case must again be byte-identical. Their digests
(build/lm-servicer-g<n>-digests.txt) give tests/lm_servicer_c64_vectors.h,
the C64 rows chosen per group by tools/lane_f_c64_rows.py.
"""
import collections
import hashlib
import json
import pathlib
import re
import subprocess
import sys

R = pathlib.Path(__file__).resolve().parents[1]
B = R / 'build'
E = R / 'deps/virtualagc/yaAGC'
ROPE = B / 'fixed-data/Luminary099/MAIN.agc.bin'
NATIVE = ['src/lm_servicer.c', 'src/lm_interp_orders.c', 'src/lm_interp_steps.c', 'src/agc_image_ops.c',
          'src/agc_hardware_divide.c', 'src/agc_transfer.c', 'src/agc_word.c', 'src/agc_multiword.c',
          'tests/lm_servicer_cases.c']
CORE = NATIVE + ['tests/cm_executive_cases.c', str(B / 'agc_engine_hardware_dv.o'),
                 str(E / 'agc_engine_init.o'), str(E / 'rfopen.o')]
FLAGS = ['-std=c99', '-O2', '-DP66_NATIVE_DISPATCH_ONLY', '-ffunction-sections', '-fdata-sections',
         '-Wl,--gc-sections', '-Iinclude', '-Itests', '-Isrc', '-Itools', '-I' + str(E)]
REFUSED = 2
STOPS = {0o5353: 'PHASCHNG', 0o5327: '2PHSCHNG', 0o4616: 'BANKCALL', 0o4674: 'IBNKCALL',
         0o4635: 'POSTJUMP', 0o4640: 'BANKJUMP', 0o4645: 'MAKECADR', 0o5155: 'ENDOFJOB', 0o5261: 'TASKOVER',
         0o5072: 'NOVAC', 0o5105: 'FINDVAC', 0o5146: 'PRIOCHNG', 0o5203: 'WAITLIST', 0o5224: 'VARDELAY',
         0o5504: 'UPFLAG', 0o5516: 'DOWNFLAG', 0o5567: 'ALARM', 0o5644: 'WHIMPER', 0o6001: 'GOTOPOOH',
         0o5126: 'CHANG2', 0o5726: 'POODOO1 (alarm 1302)', 0o4631: 'SWRETURN',
         0o7307: 'SHORTMP', 0o7313: 'SHORTMP2', 0o7544: 'ALSIGNAG', 0o6723: 'BRANCH', 0o6727: 'BRANCH +4',
         0o6070: 'NEWOPS outside the file (a CALL)', 0o2021: 'TCF R10,R11A', 0o2045: 'TCF R29',
         0o3612: 'SGNAGREE (through RTB, in the RTB bank)'}
# Any other Z is the return of a subroutine entered from outside: TC Q
# (NOULLAGE, ONULLAGE) or KILLTASK's DTCB to the caller.
RETURN = "the caller's return (TC Q, TC A, TC LRADRET, DTCB through AVGEXIT, BANKJUMP's CADR)"


def digest(values):
    a = b = 0
    for v in values:
        a = (a + v) & 0xffff
        b = (b + a) & 0xffff
    return b << 16 | a


def table(name, values):
    out = [f'static const uint32_t {name}[{len(values)}]={{']
    for i in range(0, len(values), 6):
        out.append(''.join(f'0x{v:08x}ul,' for v in values[i:i + 6]))
    out.append('};')
    return out


def keys():
    src = (R / 'src/lm_servicer.c').read_text()
    body = re.search(r'lmsv_routine_keys\[LMSV_ROUTINES\]=\{(.*?)\};', src, re.S).group(1)
    return [b + ',' + a[1:] for b, a in re.findall(r'K(\d\d|FF)\((0[0-7]+)\)', body)]


def groups():
    """Each instruction (B) and order-pair or store word (I) of the file, by
    physical address, with its C64 group; and each key slot's group."""
    src = (R / 'src/lm_servicer.c').read_text()
    of, g = {}, None
    for fn, kind in (('static int basic(', 'B'), ('static int interpretive(', 'I')):
        i = src.index(fn)
        for line in src[i:src.index('\n}\n', i)].splitlines():
            m = re.match(r'#if SV_G\((\d+)\)$', line)
            if m:
                g = int(m.group(1))
            m = re.match(r' (AT|IC)\((\d\d),(0[0-7]+)\);', line)
            if m:
                of[(kind, int(m.group(2), 8) * 1024 + int(m.group(3), 8) - 0o2000)] = g
    slots = {}
    for i, k in enumerate(keys()):
        b, a = k.split(',')
        ph = int(b, 8) * 1024 + int(a, 8) - 0o2000
        slots[i] = of.get(('B', ph), of.get(('I', ph)))
    return of, slots


def case_slots(n_keys):
    """Each case's key slot: n % n_keys, a continuation's from
    tests/lm_servicer_chains.h (tools/lm_servicer_chains.py)."""
    text = (R / 'tests/lm_servicer_chains.h').read_text()
    base = int(re.search(r'#define LMSV_BASE_CASES (\d+)u', (R / 'tests/lm_servicer_cases.h').read_text()).group(1))
    extra = [int(x) for x in re.findall(r'(\d+)u,', re.search(r'lmsv_chain_slot\[[^]]*\]=\{(.*?)\};', text).group(1))]
    return lambda n: n % n_keys if n < base else extra[n - base]


def segments(n_keys):
    """Checks every C64 group's segment runs against the oracle and writes
    their digests (build/lm-servicer-g<n>-digests.txt)."""
    of, slots = groups()
    slot = case_slots(n_keys)
    out = {}
    for g in sorted(set(slots.values())):
        exe = B / f'lm-servicer-native-g{g}'
        subprocess.run(['cc', *FLAGS, f'-DLMSV_C64_GROUP={g}', *NATIVE, 'platform/host/lm_servicer_snapshot.c',
                        '-o', str(exe)], cwd=R, check=True)
        seg = B / f'lm-servicer-g{g}.segment'
        mine = [k for k, v in slots.items() if v == g]
        seg.write_text(' '.join(map(str, mine)) + '\n' +
                       ''.join(f'{k} {p}\n' for (k, p), h in sorted(of.items()) if h != g))
        ref = subprocess.run(['./build/lm-servicer-reference', str(ROPE), str(seg)], cwd=R,
                             check=True, capture_output=True).stdout.decode().splitlines()
        nat = subprocess.run([str(exe)], cwd=R, check=True, capture_output=True).stdout.decode().splitlines()
        assert len(ref) == len(nat)
        bad, refused, lines = [], 0, []
        for a, b in zip(nat, ref):
            r = list(map(int, a.split()))
            assert r[0] == int(b.split()[0]) and slots[slot(r[0])] == g
            if r[1] == REFUSED:
                refused += 1
            elif a != b:
                bad.append(r[0])
                continue
            lines.append(f'{r[0]} {digest(r[1:2] if r[1] == REFUSED else r[1:]):08x}')
        if bad:
            sys.exit(f'C64 group {g}: {len(bad)} segment cases differ from the original program: {bad[:20]}')
        (B / f'lm-servicer-g{g}-digests.txt').write_text('\n'.join(lines) + '\n')
        out[g] = {'keys': len(mine), 'cases': len(nat), 'compared': len(nat) - refused, 'refused': refused}
        print(f'C64 group {g}: {len(nat) - refused} of {len(nat)} segment cases equal the original program')
    return out


def main():
    if not (B / 'agc_engine_hardware_dv.o').exists():
        subprocess.run([sys.executable, 'tools/prepare_hardware_dv.py'], cwd=R, check=True)
    subprocess.run(['cc', *FLAGS, '-w', 'tools/lm_servicer_reference.c', *CORE,
                    '-o', 'build/lm-servicer-reference'], cwd=R, check=True)
    ref = subprocess.run(['./build/lm-servicer-reference', str(ROPE)], cwd=R,
                         check=True, capture_output=True).stdout.decode().splitlines()
    nat = subprocess.run(['./build/lm-servicer-native'], cwd=R, check=True,
                         capture_output=True).stdout.decode().splitlines()
    names = keys()
    assert len(ref) == len(nat)
    slot = case_slots(len(names))
    bad = []
    rows, refused, stops = [], collections.Counter(), collections.defaultdict(collections.Counter)
    for i, (a, b) in enumerate(zip(nat, ref)):
        r = list(map(int, a.split()))
        assert r[0] == i
        key = names[slot(i)]
        if r[1] == REFUSED:
            refused[key] += 1
        else:
            if a != b:
                bad.append((i, key, a, b))
                continue
            z = r[2 + 5]
            stops[key][STOPS.get(z, RETURN)] += 1
        rows.append(r)
    if bad:
        for i, key, a, b in bad[:25]:
            x, y = list(map(int, a.split())), list(map(int, b.split()))
            d = [(j - 2, f'{x[j]:o}', f'{y[j]:o}') for j in range(2, len(x)) if x[j] != y[j]]
            print(f'case {i} ({key}) native kind {x[1]} ref kind {y[1]}: words (idx, native, ref) {d[:12]}')
        bykey = collections.Counter(k for _, k, _, _ in bad)
        sys.exit(f'{len(bad)} lm-servicer cases differ from the original program: {dict(bykey)}')
    digests = [digest(r[1:2] if r[1] == REFUSED else r[1:]) for r in rows]
    out = ['/* SPDX-License-Identifier: MIT; generated by tools/lm_servicer_freeze.py from the',
           ' * original Luminary099 program (yaAGC oracle). Per-case digests of the kind and, unless',
           ' * refused, all 2048 erasable words and channels 11, 12, 32 and 33 and the interrupt-enable state. */',
           f'#define LMSV_VECTORS {len(digests)}u', *table('lmsv_expected', digests)]
    (R / 'tests/lm_servicer_vectors.h').write_text('\n'.join(out) + '\n')
    seg = segments(len(names))
    subprocess.run([sys.executable, 'tools/lane_f_c64_rows.py', 'lm-servicer'], cwd=R, check=True)
    # Every branch outcome and line the whole-file host cases take, by gcov, is
    # taken by some group's C64 rows (#167).
    subprocess.run([sys.executable, 'tools/lane_f_c64_gcov.py', 'lm-servicer'], cwd=R, check=True)
    proof = {'mission': 'Luminary099', 'rope_sha256': hashlib.sha256(ROPE.read_bytes()).hexdigest(),
             'oracle_engine': 'yaAGC with the hardware divide (build/agc_engine_hardware_dv.o); scalers frozen',
             'cases': len(rows), 'compared': len(rows) - sum(refused.values()),
             'refused': dict(sorted(refused.items())),
             'snapshot_sha256': hashlib.sha256('\n'.join(nat).encode()).hexdigest(),
             'stops': {k: dict(sorted(v.items())) for k, v in sorted(stops.items())},
             'boundary': 'from each routine key to the first instruction outside SERVICER '
                         'and the interpreter (lmsv_leaves)',
             'c64_segment_groups': seg,
             'compared_words': '2048 erasable words (A, L 16-bit) and channels 11, 12, 32 and 33 and the interrupt-enable state'}
    (B / 'lm-servicer-proof.json').write_text(json.dumps(proof, indent=1) + '\n')
    print(f'PASS: {proof["compared"]} of {len(rows)} lm-servicer cases equal the original program '
          f'({len(rows) - proof["compared"]} refused)')


if __name__ == '__main__':
    main()
