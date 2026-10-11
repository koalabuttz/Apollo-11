#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain actual original fixed-word caller proof without passive algorithm credit."""
import pathlib, hashlib, json, gzip
R = pathlib.Path(__file__).resolve().parents[1]
B, E = R/'build', R/'evidence'
prefix = 'cm-fixed-compat'
proof = json.loads((B/(prefix+'-proof.json')).read_text())
assert proof['snapshot_sha256'] == '3d1f53fedcc1c1cd107c16b9034609552bfe4f7a17d9451d45d21b0f7a14fd0c'
assert (B/(prefix+'-vice-mailbox.bin')).read_bytes() == bytes.fromhex('a5000080004001')
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
for suffix in ['proof.json', 'source-audit.json', 'validation.log', 'sanitize-validation.log',
               'freeze.log', 'build-validation.log', 'c64-result.log', 'modules.json',
               'production-allocation.json', 'final-source-sample.bin', 'vice-mailbox.bin',
               'vice-shared.bin', 'vice-stack.bin', 'vice-monitor.log', 'vice.log']:
    retain(B/(prefix+'-'+suffix))
for role in ['reference', 'native']:
    for suffix in ['trace', 'stream.json']:
        retain(B/(prefix+'-'+role+'.'+suffix))
for suffix in ['prg', 'map', 'd64']:
    retain(B/(prefix+'.'+suffix))
retain(B/(prefix+'-production.d64'))
retain(B/'cm_fixed_compat_modules.h')
retain(B/'cm-fixed-compat-sdk/overlay-sdk-runtime/provenance.txt', prefix+'-sdk-provenance.txt')
for m in modules:
    for ext in ['prg', 'map']:
        retain(B/pathlib.Path(m['prg']).with_suffix('.'+ext))
report = {
    'host_actual_caller_cases': 81920, 'host_complete_checkpoints': 163840,
    'source_snapshot_sha256': proof['snapshot_sha256'],
    'target_actual_caller_sequences': 64, 'target_source_checkpoints': 128,
    'target_complete_samples': 4, 'target_final_physical_words': 2048,
    'mailbox_hex': 'a5000080004001', 'software_stack_bytes': extent,
    'passive_exactword_target_dependency': 'evidence/passive-storage-cm-target-proof.json',
    'passive_exactword_count': 81920, 'modules': modules,
    'scope': 'actual DSPFMEM/COMADRS lead-ins and SUPDACAL control through actual source returns; passive original fixed words only; later display/checksum consumers and production registration remain separate',
    'timeout_trial': 'evidence/cm-fixed-compat-timeout-80/trial.json',
}
data = (json.dumps(report, indent=2)+'\n').encode()
target = E/(prefix+'-report.json')
if target.exists():
    assert target.read_bytes() == data
else:
    target.write_bytes(data)
owned.append(target.relative_to(R).as_posix())
files = owned + [p.relative_to(R).as_posix() for pattern in [
    'include/cm_fixed_compat.h', 'include/cm_fixed_callers.h', 'src/cm_fixed_compat*',
    'src/cm_fixed_callers.c', 'tests/cm_fixed_compat*', 'platform/host/cm_fixed_compat_snapshot.c',
    'platform/c64/cm_fixed_compat*', 'tools/cm_fixed_compat*', 'tools/cm_freeze_fixed_compat.py',
    'docs/COMANCHE_FIXED_CALLERS.md', 'evidence/cm-fixed-compat-timeout-80/*']
    for p in R.glob(pattern) if p.is_file()]
files += ['include/agc_word.h', 'src/agc_word.c', 'include/overlay_runtime.h',
          'src/overlay_runtime.c', 'platform/c64/overlay_platform.c', 'platform/c64/overlay_platform.h',
          'platform/c64/overlay_payload.ld', 'platform/c64/overlay_resident.ld',
          'platform/c64/overlay_entry.S', 'tools/overlay_runtime_library.py', 'tools/overlay_build.py',
          'evidence/fixed-data-cm-original.rope.bin', 'evidence/fixed-data-audit.json.gz',
          'evidence/passive-storage-cm-target-proof.json', 'evidence/passive-storage-cm-returned.words.bin.xz']
ledger = E/(prefix+'-evidence.sha256')
ledger.write_text(''.join(hashlib.sha256((R/f).read_bytes()).hexdigest()+'  '+f+'\n' for f in sorted(set(files))))
files.append(ledger.relative_to(R).as_posix())
(B/(prefix+'-commit-files.txt')).write_text('\n'.join(sorted(set(files)))+'\n')
print(json.dumps(report, indent=2))
print('Commit list:', B/(prefix+'-commit-files.txt'))
