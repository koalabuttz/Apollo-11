#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze independent source/target mixed lifecycle qualification."""
import hashlib
import json
import lzma
from pathlib import Path
import shutil
import subprocess
from lm_idle_dispatch_differential import CORE

ROOT = Path(__file__).resolve().parents[1]
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
owned = ['include/lm_idle_dispatch.h', 'src/lm_idle_dispatch.c',
         'tests/lm_idle_dispatch_cases.h', 'tests/lm_idle_dispatch_cases.c',
         'tests/test_lm_idle_dispatch.c', 'tools/lm_idle_dispatch_snapshot.c',
         'tools/lm_idle_dispatch_reference.c', 'tools/lm_idle_dispatch_differential.py',
         'tools/lm_idle_dispatch_inputs.c', 'tools/lm_idle_dispatch_target_build.py',
         'tools/lm_idle_dispatch_vice_test.py', 'tools/lm_idle_dispatch_port_host.py',
         'tools/lm_idle_dispatch_port_integration.mk',
         'platform/c64/lm_idle_dispatch_fixture.h',
         'platform/c64/lm_idle_dispatch_diagnostic.c', 'docs/LM-CONTEXT-LIFECYCLE.md',
         'tools/package_lm_idle_dispatch.py']


def keep(name):
    source = ROOT / 'build' / name
    destination = ROOT / 'evidence' / name
    assert source.is_file(), name
    shutil.copyfile(source, destination)
    owned.append(str(destination.relative_to(ROOT)))


def main():
    assert (ROOT / 'build/lm-idle-dispatch-target-vice-mailbox.bin').read_bytes() == bytes.fromhex('a50000400002')
    for mode in ('strict', 'sanitize'):
        assert 'PASS:retained mixed lifecycle ' in (ROOT / ('build/lm-idle-dispatch-port-'+mode+'.log')).read_text()
    for name in ('reference.snapshots.xz', 'differential.log', 'sanitize.log',
                 'guards-strict.log', 'guards-sanitize.log', 'port-strict.log',
                 'port-sanitize.log', 'target-build.log', 'target.prg', 'target.map',
                 'target.d81', 'target-full.snapshots.gz', 'c64-result.log',
                 'target-vice.log', 'target-vice-monitor.log', 'target-vice.mon',
                 'target-vice-mailbox.bin', 'target-vice-shared.bin',
                 'target-vice-stack.bin', 'target-vice-hardware-stack.bin'):
        keep('lm-idle-dispatch-'+name)
    for identifier in range(1, 33):
        for extension in ('.c', '.prg', '.map'):
            keep('lm-idle-dispatch-fixture-%d%s' % (identifier, extension))
    keep('lm_idle_dispatch_target.h')
    records = json.loads((ROOT / 'build/lm-idle-dispatch-target-modules.json').read_text())
    for record in records:
        record['retained_prg'] = 'evidence/' + record['prg']
        assert sha(ROOT / record['retained_prg']) == record['sha256']
    path = ROOT / 'evidence/lm-idle-dispatch-target-modules.json'
    path.write_text(json.dumps(records, indent=2)+'\n')
    owned.append(str(path.relative_to(ROOT)))
    counts = {}
    digest = hashlib.sha256()
    n = 0
    with lzma.open(ROOT / 'evidence/lm-idle-dispatch-reference.snapshots.xz', 'rb') as source:
        for line in source:
            digest.update(line)
            number, status, interpretive = map(int, line.split(maxsplit=3)[:3])
            key = '%s/%s/%s' % (number//2048, status, interpretive)
            counts[key] = counts.get(key, 0)+1
            n += 1
    assert n == 16384 and digest.hexdigest() == '426ebc9bd21a5f4c864d907aa6e369ef93e1bf023159c527d1eed4d6deb5b979'
    deps = set(CORE) | {'src/lm_idle_start.c',
        'src/overlay_runtime.c', 'platform/c64/overlay_platform.c', 'platform/c64/overlay_platform.h',
        'platform/c64/overlay_entry.S', 'platform/c64/overlay_payload.ld',
        'platform/c64/overlay_resident.ld', 'tools/overlay_build.py',
        'tools/native_resident_port_host.py', 'tools/reference_runner.c', 'tests/scenarios.c',
        'deps/fixture/MAIN.agc.bin', 'deps/fixture/EXECUTIVE.agc',
        '../Luminary099/EXECUTIVE.agc', 'deps/virtualagc/yaAGC/agc_engine.c',
        'deps/virtualagc/yaAGC/agc_engine_init.c', 'deps/virtualagc/yaAGC/rfopen.c'}
    source = subprocess.check_output(['cc', '-MM', '-Iinclude', '-Itests']+CORE+
        ['src/lm_idle_start.c', 'tests/test_lm_idle_dispatch.c'],
        cwd=ROOT, text=True)
    for line in source.replace('\\\n', ' ').splitlines():
        deps.update(line.split(':', 1)[1].split())
    resources = {}
    for name in ('stack', 'hardware-stack'):
        data = (ROOT / ('build/lm-idle-dispatch-target-vice-'+name+'.bin')).read_bytes()
        changed = [i for i, byte in enumerate(data) if byte != 0xcd]
        resources[name] = len(data)-min(changed)
    proof = {'scope': 'Literal LM ADVAN/NUCHANG2/NUDIRECT with selected BASIC, INTRSM NEWOPS or external SELFRET; ordinary canonical domain, not whole Executive closure',
        'source_entries': {'ADVAN': '03214', 'NUCHANG2': '03011', 'NUDIRECT': '03225'},
        'terminal_entries': {'interpreter': '06070', 'selfcheck': 'SELFRET/066102',
                             'basic': 'selected positive DTCB'},
        'full_source_words': 2048, 'explicit_io_fields': ['SUPER', 'IRQ', 'channel11'],
        'explicit_transfer_fields': ['location', 'bank', 'interpretive'],
        'original_states': n, 'original_decoded_sha256': digest.hexdigest(),
        'entry_status_kind_counts': counts, 'strict_full_sanitizers': 'PASS',
        'historical_ready_basic_states_unchanged': 1024, 'all_object_guard_cases': 14,
        'selfcheck_pair': 'SELFRET01361/066102',
        'nudirect_pending': 'literal source has no NEWJOB check; newly pending request retained',
        'c64': {'stock_pal_roms_virtual_iec_d81': True, 'original_full_cases': 64,
                'fixture_images': 32, 'mailbox': 'a50000400002', 'resources': resources,
                'additional_zero_page': 0, 'cap_cycles': 16000000000,
                'source_normalization': False, 'native_i_and_port_restored': True},
        'files': {p:sha(ROOT/p) for p in owned},
        'dependencies': {p:sha(ROOT/p) for p in sorted(deps)}}
    path = ROOT / 'evidence/lm-idle-dispatch-proof.json'
    path.write_text(json.dumps(proof, indent=2)+'\n')
    owned.append(str(path.relative_to(ROOT)))
    allow = ROOT / 'build/lm-idle-dispatch-commit-files.txt'
    allow.write_text('\n'.join(owned)+'\n')
    print('Frozen', len(owned), 'files; allowlist SHA', sha(allow))
    print('Proof SHA', sha(path))


if __name__ == '__main__':
    main()
