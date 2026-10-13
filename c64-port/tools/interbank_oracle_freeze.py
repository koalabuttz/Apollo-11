#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""INTER-BANK_COMMUNICATION of both missions (lane A, milestone
interbank-oracle): every genuine inline-CADR site and every registered entry,
through agc_transfer_run. Writes the SUPDACAL data words from the ropes
(tests/interbank_oracle_data.h), builds the native snapshot
(platform/host/interbank_oracle_snapshot.c) and the yaAGC oracle
(tools/interbank_oracle_reference.c), requires byte-identical output on every
case of tests/interbank_oracle_cases.c, and freezes one digest per case (the
reach flag, then the interrupt inhibit, SUPERBNK and the 2048 erasable words,
Fletcher as ibo_digest) into tests/interbank_oracle_vectors.h and every third
into tests/interbank_oracle_c64_vectors.h. Writes build/interbank-oracle-proof.json.
"""
import collections, hashlib, json, pathlib, subprocess

R = pathlib.Path(__file__).resolve().parents[1]
E = R / 'deps/virtualagc/yaAGC'
NATIVE = ['tests/interbank_oracle_cases.c', 'src/agc_transfer.c', 'src/lm_interbank_routines.c', 'src/lm_interbank.c',
          'src/cm_interbank_routines.c', 'src/cm_interbank_complete.c', 'src/agc_word.c']
HEADERS = ['include/agc_transfer.h', 'include/lm_interbank_routines.h', 'include/lm_interbank.h',
           'include/cm_interbank_routines.h', 'include/cm_interbank_complete.h', 'src/lm_interbank_sites.inc',
           'src/cm_interbank_sites.inc', 'tests/interbank_oracle_cases.h', 'tests/interbank_oracle_data.h']
FLAGS = ['cc', '-std=c99', '-O2', '-Iinclude', '-Itests', '-Itools', '-Iplatform/host']
ENTRIES = ['BANKCALL', 'SWCALL', 'SWRETURN', 'POSTJUMP', 'BANKJUMP', 'QJUMP', 'MAKECADR', 'SUPDACAL', 'IBNKCALL',
           'ISWCALL', 'ISWRETRN', 'USPRCADR', 'SUPERSW', 'SWCALL -1']
SITES = {'LM': 448, 'CM': 354}


def digest(fields):
    a = b = 0
    for v in fields:
        a = (a + v) & 0xffff
        b = (b + a) & 0xffff
    return b << 16 | a


def build_reference():
    subprocess.run(FLAGS + [f'-I{E}', 'tools/interbank_oracle_reference.c', 'src/cm_executive.c',
                            'tests/cm_executive_cases.c'] + [s for s in NATIVE if s != 'src/agc_word.c'] +
                   ['src/agc_word.c', str(E / 'agc_engine.o'), str(E / 'agc_engine_init.o'), str(E / 'rfopen.o'),
                    '-o', 'build/interbank-oracle-reference'], cwd=R, check=True)


def kind(i):
    if i < SITES['LM']:
        return 'LM sites'
    if i < SITES['LM'] + SITES['CM']:
        return 'CM sites'
    j = i - SITES['LM'] - SITES['CM']
    return ('LM', 'CM')[j % 2] + ' ' + ENTRIES[j // 2 % len(ENTRIES)]


def main():
    ropes = [R / 'build/fixed-data' / m / 'MAIN.agc.bin' for m in ('Luminary099', 'Comanche055')]
    # The data slots' addresses do not depend on their words: build once, write the words, rebuild.
    data = R / 'tests/interbank_oracle_data.h'
    if not data.exists():
        data.write_text('static const uint16_t ibo_data[2][IBO_DATA]={{0},{0}};\n')
    build_reference()
    data.write_text(subprocess.run(['./build/interbank-oracle-reference'] + [str(r) for r in ropes] + ['--data'],
                                   cwd=R, check=True, capture_output=True, text=True).stdout)
    build_reference()
    subprocess.run(FLAGS + NATIVE + ['platform/host/interbank_oracle_snapshot.c',
                                     '-o', 'build/interbank-oracle-snapshot'], cwd=R, check=True)
    native = subprocess.run(['./build/interbank-oracle-snapshot'], cwd=R, check=True, capture_output=True,
                            text=True).stdout
    oracle = subprocess.run(['./build/interbank-oracle-reference'] + [str(r) for r in ropes], cwd=R,
                            check=True, capture_output=True, text=True).stdout
    nl, ol = native.splitlines(), oracle.splitlines()
    assert len(nl) == len(ol), (len(nl), len(ol))
    cases = len(ol)
    differ = [i for i, (x, y) in enumerate(zip(nl, ol)) if x != y]
    assert not differ, f'native differs from yaAGC on cases {differ[:10]}'
    digests, reached, per_kind = [], 0, collections.Counter()
    for i, line in enumerate(ol):
        v = list(map(int, line.split()))
        assert v[0] == i
        reached += v[1]
        per_kind[kind(i)] += v[1]
        digests.append(digest(v[1:] if v[1] else [0]))
    proof = {'cases': cases, 'reached_continuation': reached, 'native_equals_yaagc': True,
             'reached_by_entry': dict(sorted(per_kind.items())), 'cases_per_entry': 48, 'operand_sites': SITES,
             'ropes': {r.parent.name: hashlib.sha256(r.read_bytes()).hexdigest() for r in ropes},
             'oracle': 'yaAGC agc_engine from the original ropes',
             'oracle_sha256': hashlib.sha256(oracle.encode()).hexdigest(),
             'dependencies': {f: hashlib.sha256((R / f).read_bytes()).hexdigest() for f in NATIVE + HEADERS},
             'boundary': ('from every genuine inline-CADR site (Q at the operand word) and every registered entry of '
                          'INTER-BANK_COMMUNICATION, with a random erasable image, to the first continuation outside '
                          'the file; yaAGC stops when Z and FB first equal the native ones')}
    head = ['/* SPDX-License-Identifier: MIT; generated by tools/interbank_oracle_freeze.py from the original',
            ' * Luminary099 and Comanche055 programs (yaAGC oracle): per case of tests/interbank_oracle_cases.c,',
            ' * the Fletcher digest of the reach flag, the inhibit flag, SUPERBNK and the 2048 erasable words. */',
            f'#define IBO_VECTORS {cases}u',
            f'static const uint32_t ibo_digests[{cases}]={{']
    head += [','.join(f'0x{d:08x}u' for d in digests[i:i + 8]) + ',' for i in range(0, cases, 8)] + ['};']
    (R / 'tests/interbank_oracle_vectors.h').write_text('\n'.join(head) + '\n')
    # Every third case (both missions, every entry) for the C64 diagnostic.
    c64 = digests[::3]
    head = ['/* SPDX-License-Identifier: MIT; generated by tools/interbank_oracle_freeze.py: every',
            ' * third digest of tests/interbank_oracle_vectors.h, for the C64 diagnostic. */',
            '#define IBO_C64_STRIDE 3u', f'#define IBO_C64_COUNT {len(c64)}u',
            f'static const uint32_t ibo_c64[{len(c64)}]={{']
    head += [','.join(f'0x{d:08x}ul' for d in c64[i:i + 6]) + ',' for i in range(0, len(c64), 6)] + ['};']
    (R / 'tests/interbank_oracle_c64_vectors.h').write_text('\n'.join(head) + '\n')
    (R / 'build/interbank-oracle-proof.json').write_text(json.dumps(proof, indent=1) + '\n')
    print(f'PASS: {cases} interbank-oracle cases identical to yaAGC, {reached} reach a continuation')


if __name__ == '__main__':
    main()
