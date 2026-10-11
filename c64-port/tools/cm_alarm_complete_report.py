#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain source-qualified canonical service state proof and actual C64 execution."""
import pathlib, hashlib, json, gzip
R = pathlib.Path(__file__).resolve().parents[1]
B, E = R/'build', R/'evidence'
prefix = 'cm-alarm-complete'
proof = json.loads((B/(prefix+'-proof.json')).read_text())
assert proof['snapshot_sha256'] == '1760b5a9c8b7bfa94efb11ec2d29b6b284ad4194f8d5a8ce66692e39567dc9f0'
assert (B/(prefix+'-vice-mailbox.bin')).read_bytes() == bytes.fromhex('a500004000')
assert 'PASS:' in (B/(prefix+'-c64-result.log')).read_text()
assert (B/(prefix+'-vice-shared.bin')).read_bytes() == (B/(prefix+'-final-source-sample.bin')).read_bytes()
modules = json.loads((B/(prefix+'-modules.json')).read_text())
for m in modules:
    raw = (B/m['prg']).read_bytes()
    assert len(raw)-2 == m['byte_count'] <= 16384
    assert hashlib.sha256(raw).hexdigest() == m['sha256']
stack = (B/(prefix+'-vice-stack.bin')).read_bytes()
changed = [i for i, x in enumerate(stack) if x != 0xcd]
extent = 2048-min(changed) if changed else 0
assert extent < 2048
owned = []
def retain(source, name=None):
    target = E/(name or source.name)
    data = source.read_bytes()
    if target.exists():
        assert target.read_bytes() == data, 'Published identity changed: '+str(target)
    else:
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
    owned.append(target.relative_to(R).as_posix())
for role in ['reference', 'native']:
    source = B/(prefix+'-'+role+'.snapshots.bin.gz')
    h, count = hashlib.sha256(), 0
    with gzip.open(source, 'rb') as f:
        while True:
            data = f.read(1024*1024)
            if not data:
                break
            h.update(data)
            count += len(data)
    assert count == proof['binary_bytes'] and h.hexdigest() == proof['snapshot_sha256']
    retain(source)
for suffix in ['proof.json', 'validation.log', 'sanitize-validation.log', 'freeze.log',
               'build-validation.log', 'c64-result.log', 'modules.json',
               'final-source-sample.bin', 'vice-mailbox.bin', 'vice-shared.bin',
               'vice-stack.bin', 'vice-monitor.log', 'vice.log']:
    retain(B/(prefix+'-'+suffix))
for suffix in ['.c','.json','-initial-metadata.json','-validation.log','-frozen-helper.bin','-corrected-helper.bin','-original-source.bin']:
    retain(B/('cm-alarm-return-collision'+suffix))
for role in ['reference', 'native']:
    for suffix in ['trace', 'stream.json']:
        retain(B/(prefix+'-'+role+'.'+suffix))
for suffix in ['prg', 'map', 'd64']:
    retain(B/(prefix+'.'+suffix))
retain(B/'cm_alarm_complete_modules.h')
retain(B/'cm-alarm-complete-sdk/overlay-sdk-runtime/provenance.txt', prefix+'-sdk-provenance.txt')
for m in modules:
    for ext in ['prg', 'map']:
        retain(B/pathlib.Path(m['prg']).with_suffix('.'+ext))
report = dict(actual_executed_source_states=32240,
              source_stream_sha256=proof['snapshot_sha256'], target_contexts=64,
              target_full_samples=2, target_final_physical_words=2048,
              mailbox_hex='a500004000', software_stack_bytes=extent, modules=modules,
              complete_file_qualified=False,
              scope='actual four alarm entries through true source return or overflow POODOO handoff; normalreturn-to-POODOO-address IRQ collision corrected additively; no decoder or fixed-word executable credit, selected recovery/display bodies separate')
data = (json.dumps(report, indent=2)+'\n').encode()
target = E/(prefix+'-report.json')
if target.exists():
    assert target.read_bytes() == data
else:
    target.write_bytes(data)
owned.append(target.relative_to(R).as_posix())
files = owned + [p.relative_to(R).as_posix() for pattern in [
    'include/cm_alarm_complete.h', 'src/cm_alarm_complete.c', 'tests/cm_alarm_complete*',
    'platform/host/cm_alarm_complete_snapshot.c', 'platform/c64/cm_alarm_complete*',
    'tools/cm_alarm_complete*', 'tools/cm_freeze_alarm_complete.py',
    'docs/COMANCHE_ALARM_RETURNS.md'] for p in R.glob(pattern) if p.is_file()]
files += ['include/cm_native_image.h', 'src/cm_native_image.c', 'include/cm_executive.h', 'src/cm_executive.c', 'include/cm_dispatch.h', 'src/cm_dispatch.c', 'include/agc_word.h', 'src/agc_word.c', 'include/overlay_runtime.h', 'src/overlay_runtime.c', 'platform/c64/overlay_platform.c', 'platform/c64/overlay_platform.h', 'platform/c64/overlay_payload.ld', 'platform/c64/overlay_resident.ld', 'platform/c64/overlay_entry.S', 'tools/overlay_runtime_library.py', 'tools/overlay_build.py', 'tools/cm_fixed_compat_dump.h', 'tools/cm_fixed_compat_generate.py', 'tools/prepare_cm_original_rope.py', 'tools/cm_executive_reference.c', 'tests/cm_executive_cases.h', 'tests/cm_executive_cases.c', 'evidence/fixed-data-cm-original.rope.bin', 'evidence/fixed-data-audit.json.gz', 'tests/cm_admission_complete_cases.h', 'tests/cm_admission_complete_cases.c', 'include/cm_admission_complete.h', 'tests/cm_context_change_cases.h', 'tests/cm_context_change_cases.c', 'include/cm_context_change.h']
files += ['evidence/cm-alarm2-image-producer-first-target-failure/src/cm_alarm2_image.c','tools/cm_alarm_return_collision_audit.py','include/cm_alarm_image.h','src/cm_alarm_image.c','tests/cm_alarm_image_cases.h','tests/cm_alarm_image_cases.c','include/cm_alarm_record.h','src/cm_alarm_record.c','tests/cm_alarm_record_cases.h','tests/cm_alarm_record_cases.c','tools/cm_alarm_complete_dump.h','include/cm_fixed_compat.h','src/cm_fixed_compat.c','src/cm_fixed_compat_words.inc','platform/c64/cm_fixed_compat_data.c']
ledger = E/(prefix+'-evidence.sha256')
ledger.write_text(''.join(hashlib.sha256((R/f).read_bytes()).hexdigest()+'  '+f+'\n' for f in sorted(set(files))))
files.append(ledger.relative_to(R).as_posix())
(B/(prefix+'-commit-files.txt')).write_text('\n'.join(sorted(set(files)))+'\n')
print(json.dumps(report, indent=2))
print('Commit list:', B/(prefix+'-commit-files.txt'))
