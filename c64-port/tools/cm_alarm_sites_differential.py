#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""cm-alarm-sites: native site-table ALARM/ALARM2/abort-word entries against
the original assembly, one full 2048-word image per case.

The native runner links no rope reader. The reference runs the unmodified
Comanche055 rope in the pinned Virtual AGC engine (host oracle only)."""
import gzip, hashlib, json, pathlib, subprocess, sys
R = pathlib.Path(__file__).resolve().parents[1]
B = R / 'build'
E = R / 'deps/virtualagc/yaAGC'
ROPE = B / 'fixed-data/Comanche055/MAIN.agc.bin'
ROPE_SHA = '6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
RECORD = 4130

assert hashlib.sha256(ROPE.read_bytes()).hexdigest() == ROPE_SHA, 'unexpected Comanche055 rope'
subprocess.run(['python3', 'tools/agc_sites_generate.py', 'cm-alarm', '--check',
                '--json', str(B / 'cm-alarm-sites.json')], cwd=R, check=True)
subprocess.run(['cc', '-std=c99', '-O2', '-Iinclude', '-Itests', '-Itools', '-I' + str(E),
                'tools/cm_alarm_sites_reference.c', 'tests/cm_alarm_sites_cases.c',
                'tests/cm_alarm_complete_cases.c', 'tests/cm_alarm_image_cases.c',
                'tests/cm_alarm2_image_cases.c', 'tests/cm_abort_word_cases.c',
                'tests/cm_alarm_record_cases.c', 'tests/cm_admission_complete_cases.c',
                'tests/cm_executive_cases.c', 'src/cm_alarm_sites.c', 'src/agc_transfer.c',
                'src/cm_alarm_complete.c', 'src/cm_alarm_image.c', 'src/cm_alarm2_image.c',
                'src/cm_abort_word.c', 'src/cm_alarm_record.c', 'src/cm_native_image.c',
                'src/cm_executive.c', 'src/agc_word.c',
                str(E / 'agc_engine.o'), str(E / 'agc_engine_init.o'), str(E / 'rfopen.o'),
                '-o', 'build/cm-alarm-sites-reference'], cwd=R, check=True)
streams = {}
for role, cmd in (('native', ['./build/cm-alarm-sites-native']),
                  ('reference', ['./build/cm-alarm-sites-reference', str(ROPE)])):
    h = hashlib.sha256(); count = 0
    with gzip.open(B / f'cm-alarm-sites-{role}.snapshots.gz', 'wb', compresslevel=6) as out:
        p = subprocess.Popen(cmd, cwd=R, stdout=subprocess.PIPE)
        for data in iter(lambda: p.stdout.read(1 << 20), b''):
            h.update(data); count += len(data); out.write(data)
        assert p.wait() == 0, role + ' runner failed'
    streams[role] = {'bytes': count, 'sha256': h.hexdigest()}
if streams['native'] != streams['reference']:
    with gzip.open(B / 'cm-alarm-sites-native.snapshots.gz') as a, \
         gzip.open(B / 'cm-alarm-sites-reference.snapshots.gz') as b:
        n = 0
        while True:
            x, y = a.read(RECORD), b.read(RECORD)
            if x != y:
                words = [i for i in range(0, min(len(x), len(y)), 2) if x[i:i + 2] != y[i:i + 2]]
                sys.exit(f'FAIL: case {n} differs at byte offsets {words[:12]}')
            if not x:
                break
            n += 1
sites = json.loads((B / 'cm-alarm-sites.json').read_text())['sites']
cases = streams['native']['bytes'] // RECORD
assert cases == 3 * len(sites) * 64, cases
proof = {'milestone': 'cm-alarm-sites', 'sites': len(sites), 'cases': cases,
         'kinds': ['ALARM 05650', 'ALARM2 05652', 'BAILOUT 07762 / POODOO 05726 word'],
         'contexts_per_site_and_kind': 64, 'image_words': 2048,
         'rope_sha256': ROPE_SHA, 'snapshot_sha256': streams['native']['sha256'],
         'snapshot_bytes': streams['native']['bytes'],
         'native_link_has_rope_reader': False}
(B / 'cm-alarm-sites-proof.json').write_text(json.dumps(proof, indent=1) + '\n')
print('PASS: cm-alarm-sites native equals original for', cases, 'cases')
print(json.dumps(proof, indent=1))
