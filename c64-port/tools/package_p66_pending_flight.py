#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Freeze the additive raw-state pending-job flight proof and target images."""
import hashlib
import json
import lzma
from pathlib import Path
import shutil
import subprocess
from native_registry_build import RESIDENT_DEFINES, RESIDENT_SOURCES
from p66_application_port_host import inventories

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build'
EVIDENCE = ROOT / 'evidence'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
owned = ['include/p66_application_pending.h', 'src/p66_application_pending.c',
         'platform/c64/p66_pending_payload.h', 'platform/c64/p66_pending_payload.c',
         'platform/c64/p66_pending_initializer.h', 'platform/c64/p66_pending_initializer.c',
         'platform/c64/p66_pending_flight_diagnostic.c', 'tools/p66_pending_layout.c',
         'tools/p66_pending_flight_snapshot.c', 'tools/p66_pending_flight_reference.c',
         'tools/p66_pending_adoption_snapshot.c', 'tools/p66_pending_flight_differential.py',
         'tools/p66_pending_flight_build.py', 'tools/p66_pending_flight_vice_test.py',
         'tools/p66_pending_directory_build.py', 'tools/p66_pending_directory_vice_test.py',
         'tools/p66_pending_port_host.py', 'tools/p66_pending_port_integration.mk',
         'tests/test_p66_pending_guards.c', 'docs/P66-APPLICATION-PENDING.md',
         'tools/package_p66_pending_flight.py']


def keep(name, output=None):
    source = BUILD / name
    destination = EVIDENCE / (output or name)
    assert source.is_file(), name
    shutil.copyfile(source, destination)
    owned.append(str(destination.relative_to(ROOT)))


def main():
    for log in ('p66-pending-port-strict.log', 'p66-pending-port-sanitize.log'):
        assert 'PASS:pending P66 port ' in (BUILD / log).read_text()
    progress = json.loads((BUILD / 'p66-pending-flight-progress.json').read_text())
    assert progress['per_input_completed_source_checkpoints'] == {'0': 154, '1': 158}
    assert progress['total_complete_checkpoints'] == 312 and progress['source_failures'] == 0
    for prefix, mailbox in [('p66-pending-flight', 'a50000380102'),
                            ('p66-pending-directory', 'a50000ec0002')]:
        assert (BUILD / (prefix + '-vice-mailbox.bin')).read_bytes() == bytes.fromhex(mailbox)
        for suffix in ('.prg', '.map', '-c64-result.log', '-vice.log',
                       '-vice-monitor.log', '-vice.mon', '-vice-mailbox.bin',
                       '-vice-shared.bin', '-vice-stack.bin', '-vice-hardware-stack.bin'):
            keep(prefix + suffix)
    keep('p66-pending-flight.d81')
    for identifier in (88, 89, 97, 98, 99, 148, 245):
        for extension in ('.prg', '.map'):
            keep('p66-pending-flight-%d%s' % (identifier, extension))
    for name in ('p66-pending-flight-progress.json', 'p66-pending-flight-build.log',
                 'p66-pending-directory-build.log', 'p66-pending-flight-differential.log',
                 'p66-pending-flight-sanitize.log', 'p66-pending-adoption-strict.log',
                 'p66-pending-adoption-sanitize.log', 'p66-pending-compatibility-strict.log',
                 'p66-pending-compatibility-sanitize.log', 'p66-pending-port-strict.log',
                 'p66-pending-port-sanitize.log', 'p66-pending-layout.ll',
                 'p66-pending-flight-prior-overflow.map', 'p66-pending-flight-prior-overflow.log',
                 'p66-pending-directory-prior-environment.log',
                 'p66-pending-flight-reference.snapshots.xz'):
        keep(name)
    for name in ('p66_pending_flight_vectors.h', 'checker.c',
                 'native_registry_table.h', 'native_registry_bootstrap.h',
                 'overlay-sdk-runtime/provenance.txt'):
        keep('p66-pending-flight/' + name,
             'p66-pending-flight-' + name.replace('/', '-'))
    records = json.loads((BUILD / 'p66-pending-flight-modules.json').read_text())
    new = {88, 89, 97, 98, 99, 148, 245}
    for record in records:
        if record['id'] in new:
            record['retained_prg'] = 'evidence/' + record['prg']
        path = ROOT / record.get('retained_prg', 'evidence/' + record['prg'])
        assert sha(path) == record['sha256']
    manifest = EVIDENCE / 'p66-pending-flight-modules.json'
    manifest.write_text(json.dumps(records, indent=2) + '\n')
    owned.append(str(manifest.relative_to(ROOT)))
    h = hashlib.sha256()
    count = 0
    decoded_bytes = 0
    with lzma.open(EVIDENCE / 'p66-pending-flight-reference.snapshots.xz', 'rb') as source:
        for line in source:
            h.update(line)
            count += 1
            decoded_bytes += len(line)
    assert count == 2476 and h.hexdigest() == '822b4b18d80e7cc4bf201b25862712bcc0a344d9b59c508e6d26a28a0dc5df48'
    _, source_files, inputs, defines = inventories()[2]
    source_files += ['src/p66_length_return_control.c', 'src/p66_application_recurrence.c',
                     'src/lm_clock.c', 'src/lm_idle_start.c', 'src/lm_taskover_resume.c',
                     'src/lm_waitlist_dispatch.c', 'src/lm_waitlist_irq_dispatch.c',
                     'src/p66_rodtask_image.c', 'src/p66_application_split.c',
                     'src/p66_application_pending.c', 'src/p66_elapsed_return_control.c',
                     'src/lm_danzig_change.c', 'src/lm_interpreter_change.c']
    inputs += ['tests/p66_recurrence_live_cases.c']
    sources = list(dict.fromkeys(source_files + inputs))
    dependency_text = subprocess.check_output(['cc', '-MM', '-Iinclude', '-Itests'] +
                                             defines + sources, cwd=ROOT, text=True)
    deps = set(sources)
    for line in dependency_text.replace('\\\n', ' ').splitlines():
        deps.update(line.split(':', 1)[1].split())
    # Keep source provenance for the resident and all explicit service stages,
    # including the shared original-assembly oracle. Generated target headers
    # and the altered hardware-DV engine are retained under this packet's names.
    deps.update(RESIDENT_SOURCES)
    deps.update(['platform/c64/overlay_platform.c', 'platform/c64/overlay_platform.h',
                 'platform/c64/overlay_entry.S', 'platform/c64/overlay_payload.ld',
                 'platform/c64/overlay_resident.ld', 'tools/overlay_build.py',
                 'tools/overlay_runtime_library.py', 'tools/native_registry_build.py',
                 'tools/native_registry_progress_audit.py', 'tools/reference_runner.c',
                 'tools/p66_application_port_host.py', 'tools/native_resident_port_host.py',
                 'deps/fixture/MAIN.agc.bin', 'deps/virtualagc/yaAGC/agc_engine.c',
                 'deps/virtualagc/yaAGC/agc_engine_init.c', 'deps/virtualagc/yaAGC/rfopen.c',
                 'evidence/p66-recurrence-live-modules.json',
                 'evidence/p66-application-split-modules.json',
                 'evidence/p66-recurrence-live-source.snapshots.xz'])
    keep('agc_engine_hardware_dv.c', 'p66-pending-flight-oracle-hardware-dv.c')
    resources = {}
    for prefix in ('p66-pending-flight', 'p66-pending-directory'):
        resources[prefix] = {}
        for suffix, field in [('vice-stack.bin', 'software_stack_bytes'),
                              ('vice-hardware-stack.bin', 'hardware_stack_bytes')]:
            data = (BUILD / (prefix + '-' + suffix)).read_bytes()
            changed = [i for i, value in enumerate(data) if value != 0xcd]
            resources[prefix][field] = len(data) - min(changed)
        resources[prefix]['additional_zero_page'] = 0
    proof = {
        'scope': 'Additive higher-priority deferred mode job and exact P66 resumption/one-shot recurrence; no whole-Executive or whole-application certificate',
        'stimulus': {'at_source_loc': '03353', 'deferred_job': '03534/060102',
                     'packed_priority': '037110', 'newjob_core_offset': '014'},
        'first_legal_exchange_loc': '03356', 'restored_source_q': '03206',
        'strict_full_sanitizers': 'PASS', 'full_source_fields': 2053,
        'original_states': count, 'original_decoded_sha256': h.hexdigest(),
        'original_decoded_bytes': decoded_bytes,
        'adoption_source_states': 64,
        'adoption_decoded_sha256': '8074608bd0fa95ea4b11d90684662b7844a7afff5de220e81660bcff10a7c854',
        'unchanged_old_states': 2460,
        'unchanged_old_decoded_sha256': 'd1aa5f4de9e02bf126731818227c4bc7592e62bddbbd66a425cc56346683f186',
        'transport_guards': 'enabled/masked exclusion; import failure before callback and export failure retaining completed source callback; caller stops on failure',
        'native_irq_seam': 'host queued ticket retained without additional poll; physical target arrival at that instant not claimed',
        'module_ids': sorted(new), 'retained_historical_images_unchanged': 112,
        'directory_records': 118, 'resident_end': 0x6fda, 'resident_spare_bytes': 38,
        'leases': {'executive_view': [0xc100, 0xc3d4], 'native_metadata': [0xc400, 0xc4c6],
                   'descriptor': [0xc600, 0xc640], 'canonical': [0x7000, 0x8000],
                   'slot': [0x8000, 0xc000], 'software_stack': [0xc800, 0xd000]},
        'target_abi_bytes': {'runtime': 36, 'binding': 130, 'queue': 32,
                             'metadata': 198, 'executive': 648, 'descriptor': 27},
        'c64': {'stock_pal_roms_virtual_iec_d81': True, 'source_checkpoints': 312,
                'per_input': [154, 158], 'flight_mailbox': 'a50000380102',
                'directory_complete_descriptors': 236, 'directory_mailbox': 'a50000ec0002',
                'cap_cycles': 48000000000, 'last_checkpoint_cycles': progress['last_event']['cycles'],
                'frames': 'each stage returns to resident before metadata or next stage replaces its slot',
                'resources': resources, 'source_normalization': False},
        'prior_failures': ['fixed resident link overflow; retained map/log',
                           'directory launcher missing private XDG directories before execution; target unchanged'],
        'target_defines': RESIDENT_DEFINES + ['P66_APPLICATION_SPLIT_DISPATCH_ONLY',
            'P66_PENDING_DISPATCH_ONLY', 'P66_PENDING_FIXED_BINDING', 'P66_PENDING_OBSERVE_INVOKER'],
        'files': {p: sha(ROOT / p) for p in owned},
        'dependencies': {p: sha(ROOT / p) for p in sorted(deps)}}
    path = EVIDENCE / 'p66-pending-flight-proof.json'
    path.write_text(json.dumps(proof, indent=2) + '\n')
    owned.append(str(path.relative_to(ROOT)))
    allowlist = BUILD / 'p66-pending-flight-commit-files.txt'
    allowlist.write_text('\n'.join(owned) + '\n')
    print('Frozen', len(owned), 'files; allowlist SHA', sha(allowlist))
    print('Proof SHA', sha(path))


if __name__ == '__main__':
    main()
