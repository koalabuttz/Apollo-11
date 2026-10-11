#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain source-qualified canonical service state proof and actual C64 execution."""
import pathlib, hashlib, json, gzip
R = pathlib.Path(__file__).resolve().parents[1]
B, E = R/'build', R/'evidence'
prefix = 'cm-priority-full'
proof = json.loads((B/(prefix+'-proof.json')).read_text())
assert proof['snapshot_sha256'] == '68787a8266dbbef6c50db6ccbb4ddf7a40c61958a001eea36583ab678eacc69e'
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
               'source-audit.json', 'preflight-validation.log', 'final-source-sample.bin', 'vice-mailbox.bin', 'vice-shared.bin',
               'vice-stack.bin', 'vice-monitor.log', 'vice.log']:
    retain(B/(prefix+'-'+suffix))
for role in ['reference', 'native']:
    for suffix in ['trace', 'stream.json']:
        retain(B/(prefix+'-'+role+'.'+suffix))
for suffix in ['prg', 'map', 'd64']:
    retain(B/(prefix+'.'+suffix))
retain(B/'cm_priority_full_modules.h')
retain(B/'cm-priority-full-sdk/overlay-sdk-runtime/provenance.txt', prefix+'-sdk-provenance.txt')
for m in modules:
    for ext in ['prg', 'map']:
        retain(B/pathlib.Path(m['prg']).with_suffix('.'+ext))
report = dict(actual_executed_source_states=75264,
              source_stream_sha256=proof['snapshot_sha256'], target_contexts=64,
              target_full_samples=2, target_full_sample_fields=2064, target_final_physical_words=2048,
              mailbox_hex='a500004000', software_stack_bytes=extent, modules=modules,
              complete_file_qualified=False,
              scope='all32 bare PRIOCHNG levels including+0, six authentic resource fractions, all+0candidate positions and actual basic/interpreter/caller/CCSHOLE transfer; selected job/alarm bodies external')
data = (json.dumps(report, indent=2)+'\n').encode()
target = E/(prefix+'-report.json')
if target.exists():
    assert target.read_bytes() == data
else:
    target.write_bytes(data)
owned.append(target.relative_to(R).as_posix())
files = owned + [p.relative_to(R).as_posix() for pattern in [
    'include/cm_priority_full.h', 'src/cm_priority_full.c', 'tests/cm_priority_full*',
    'platform/host/cm_priority_full_snapshot.c', 'platform/c64/cm_priority_full*',
    'tools/cm_priority_full*', 'tools/cm_freeze_priority_full.py',
    'docs/COMANCHE_PRIORITY_FULL.md', 'tests/test_cm_priority_full.c', 'evidence/cm-priority-full-first-oracle-frontier-failure/*'] for p in R.glob(pattern) if p.is_file()]
files += ['include/cm_admission_complete.h', 'tests/cm_context_change_cases.h', 'tests/cm_context_change_cases.c', 'tools/cm_priority_full_dump.h', 'tests/cm_admission_complete_cases.h', 'tests/cm_admission_complete_cases.c', 'include/cm_native_image.h', 'src/cm_native_image.c', 'include/cm_executive.h', 'src/cm_executive.c', 'include/cm_dispatch.h', 'src/cm_dispatch.c', 'include/agc_word.h', 'src/agc_word.c', 'include/overlay_runtime.h', 'src/overlay_runtime.c', 'platform/c64/overlay_platform.c', 'platform/c64/overlay_platform.h', 'platform/c64/overlay_payload.ld', 'platform/c64/overlay_resident.ld', 'platform/c64/overlay_entry.S', 'tools/overlay_runtime_library.py', 'tools/overlay_build.py', 'tools/cm_fixed_compat_generate.py', 'tools/prepare_cm_original_rope.py', 'tools/cm_executive_reference.c', 'tests/cm_executive_cases.h', 'tests/cm_executive_cases.c', 'evidence/fixed-data-cm-original.rope.bin', 'evidence/fixed-data-audit.json.gz', 'tools/cm_alarm2_image_dump.h','tools/cm_alarm2_image_dump.h']
ledger = E/(prefix+'-evidence.sha256')
ledger.write_text(''.join(hashlib.sha256((R/f).read_bytes()).hexdigest()+'  '+f+'\n' for f in sorted(set(files))))
files.append(ledger.relative_to(R).as_posix())
(B/(prefix+'-commit-files.txt')).write_text('\n'.join(sorted(set(files)))+'\n')
print(json.dumps(report, indent=2))
print('Commit list:', B/(prefix+'-commit-files.txt'))
