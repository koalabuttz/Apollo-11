#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Retain a source/target packet only after the complete raw proof passes."""
import gzip
import hashlib
import json
from pathlib import Path
import re
import shutil

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build'
OUT = ROOT / 'evidence/p66-complete'
RAW_SHA = '9480366d0d7203683dd1d35ae06736dca9ef93646fd795bf210d405ae6689c52'
PENDING_SHA = '513b4674a0414fab4bce8f42bf9755bc404978eb7ed4d6ef5f11ff682b1cc8a9'
OWNED = '''
include/p66_descent_control.h include/p66_descent_invoke.h
include/p66_descent_prefix_control.h include/p66_descent_runtime.h
include/p66_native_descent.h
src/p66_descent_control.c src/p66_descent_invoke.c
src/p66_descent_prefix_control.c src/p66_descent_runtime.c
src/p66_descent_view.h src/p66_native_descent.c
platform/c64/p66_descent_control_payload.c
platform/c64/p66_descent_control_payload.h
platform/c64/p66_descent_overlay_payload.c
platform/c64/p66_complete_fixture.h
platform/c64/p66_complete_fixture_input.c
platform/c64/p66_complete_fixture_check.c
platform/c64/p66_complete_runtime_diagnostic.c
platform/host/p66_complete_live_snapshot.c
platform/host/p66_descent_pending_snapshot.c
tests/p66_complete_live_cases.c tests/p66_complete_live_cases.h
tests/p66_complete_live_inputs.h tests/p66_complete_live_vectors.h
tests/p66_descent_live_cases.c tests/p66_descent_live_cases.h
tools/p66_descent_overlay_build.py tools/p66_descent_control_overlay_build.py
tools/p66_complete_build.py tools/p66_complete_vice_test.py
tools/p66_complete_live_inputs.c tools/p66_complete_live_reference.c
tools/p66_complete_live_differential.py tools/freeze_p66_complete_live.py
tools/p66_descent_pending_reference.c tools/p66_descent_pending_differential.py
tools/freeze_p66_complete_packet.py tools/p66_complete.mk
docs/P66-COMPLETE-DESCENT.md
'''.split()


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def local_closure(files):
    """Conservative local source/include closure, including conditional arms."""
    result = set()
    pending = list(files)
    while pending:
        path = pending.pop()
        if path in result or not path.is_file():
            continue
        result.add(path)
        for name in re.findall(r'^\s*#\s*include\s*"([^"]+)"', path.read_text(), re.M):
            for parent in (path.parent, ROOT / 'include', ROOT / 'tests',
                           ROOT / 'platform/c64', BUILD):
                candidate = parent / name
                if candidate.is_file():
                    pending.append(candidate)
                    break
    return result


def main():
    assert (BUILD / 'p66-complete-vice-mailbox.bin').read_bytes() == bytes([0xa5, 0, 0, 148, 0, 2])
    for stem, expected in (('p66-complete-live', RAW_SHA), ('p66-descent-pending', PENDING_SHA)):
        suffix = '-raw' if stem == 'p66-complete-live' else ''
        for role in ('reference', 'native', 'sanitize'):
            path = BUILD / f'{stem}-{role}{suffix}.snapshots'
            assert digest(path) == expected, str(path)
    target_log = BUILD / 'p66-complete-target-vice-check.log'
    assert 'PASS: stock PAL C64 ROMs' in target_log.read_text()
    OUT.mkdir(parents=True, exist_ok=True)
    artifacts = {}

    def retain(path, name=None, compressed=False):
        target = OUT / (name or path.name)
        if compressed:
            target.write_bytes(gzip.compress(path.read_bytes(), mtime=0))
        else:
            shutil.copyfile(path, target)
        artifacts[str(target.relative_to(ROOT))] = dict(sha256=digest(target), bytes=target.stat().st_size)

    for name in ('p66-complete.prg', 'p66-complete.map', 'p66-complete.d81',
                 'p66-complete-modules.json', 'p66_complete_modules.h',
                 'p66-complete-fixture-248.prg', 'p66-complete-fixture-248.map',
                 'p66-complete-fixture-249.prg', 'p66-complete-fixture-249.map',
                 'p66-complete-vice-mailbox.bin', 'p66-complete-vice-shared.bin',
                 'p66-complete-vice-stack.bin', 'p66-complete-vice-hardware-stack.bin',
                 'p66-complete-vice.log', 'p66-complete-vice-monitor.log',
                 'p66-complete-target-build.log', 'p66-complete-target-vice-check.log',
                 'p66-complete-live-raw-check.log', 'p66-complete-live-raw-sanitize-check.log',
                 'p66-descent-pending-current-check.log', 'p66-descent-pending-current-sanitize-check.log',
                 'p66-complete-live-reference.trace', 'p66-complete-live-inputs.snapshots'):
        retain(BUILD / name)
    retain(BUILD / 'p66-complete-live-reference-raw.snapshots', compressed=True,
           name='p66-complete-live-reference-raw.snapshots.gz')
    retain(BUILD / 'p66-descent-pending-reference.snapshots', compressed=True,
           name='p66-descent-pending-reference.snapshots.gz')

    sources = {ROOT / f for f in OWNED if f.endswith(('.c', '.h'))}
    scripts = ('p66_complete_build', 'p66_descent_overlay_build',
               'p66_descent_control_overlay_build', 'native_scheduler_overlay_build',
               'p66_display_job_overlay_build')
    for script in scripts:
        text = (ROOT / f'tools/{script}.py').read_text()
        sources.update(ROOT / name for name in re.findall(r"['\"]((?:src|platform/c64)/[^'\"]+\.(?:c|h))['\"]", text))
    sources.update((ROOT / 'src').glob('guidance*.c'))
    for name in ('prepare', 'scale', 'push', 'elapsed', 'store', 'divide', 'gravity',
                 'velocity', 'radial', 'unit', 'trig', 'projection', 'p66_guidance_live'):
        sources.add(ROOT / f'tests/{name}_cases.c')
    job_sources = (ROOT / 'tools/display_job.mk').read_text().split('DJOB_CORE = ', 1)[1].split('\n', 1)[0].split()
    sources.update(ROOT / name for name in job_sources if name.startswith('src/'))
    dependencies = local_closure(sources)
    dependencies.update(ROOT / name for name in ('platform/c64/overlay_entry.S',
                         'platform/c64/overlay_payload.ld', 'platform/c64/overlay_resident.ld',
                         'tools/reference_runner.c', 'tools/prepare_hardware_dv.py',
                         'tools/overlay_build.py', 'tools/overlay_runtime_library.py'))
    source_hashes = {name: digest(ROOT / name) for name in OWNED}
    dependency_hashes = {str(path.relative_to(ROOT)): digest(path) for path in sorted(dependencies)}
    fixture_hashes = {str(path.relative_to(ROOT)): digest(path)
                      for path in sorted((ROOT / 'deps/fixture').glob('*.agc'))}
    fixture_hashes['deps/fixture/MAIN.agc.bin'] = digest(ROOT / 'deps/fixture/MAIN.agc.bin')
    for name in ('EXECUTIVE.agc', 'INTERPRETER.agc', 'LUNAR_LANDING_GUIDANCE_EQUATIONS.agc'):
        assert (ROOT.parent / 'Luminary099' / name).read_bytes() == (ROOT / 'deps/fixture' / name).read_bytes()

    modules = json.loads((BUILD / 'p66-complete-modules.json').read_text())
    for module in modules:
        assert digest(BUILD / module['prg']) == module['sha256']
    assert len(modules) == 96 and len({m['id'] for m in modules}) == 96
    software = (BUILD / 'p66-complete-vice-stack.bin').read_bytes()
    hardware = (BUILD / 'p66-complete-vice-hardware-stack.bin').read_bytes()

    def extent(data):
        used = [n for n, byte in enumerate(data) if byte != 0xcd]
        return len(data) - min(used) if used else 0

    report = dict(scope='Luminary099 manual P66 RODCOMP through enabled V06N60 MAKEPLAY and original ENDOFJOB idle',
                  host_full_raw_checkpoints=1204, host_full_raw_words=2052,
                  host_enabled_paths=16, c64_full_raw_checkpoints=148, c64_paths=2,
                  c64_comparison='fingerprint every raw source word; no normalization',
                  pending_injections=404, pending_checkpoints=808, pending_modeled_words=535,
                  retained_snapshot_sha256=dict(complete_raw=RAW_SHA, pending=PENDING_SHA),
                  software_stack_bytes=extent(software), hardware_stack_bytes=extent(hardware),
                  extra_zp_bytes=0, c64_irq_super_cases=[dict(native_irq='enabled', source_superbank=0), dict(native_irq='masked', source_superbank=0o60)],
                  hardware_stack_unfilled_top_bytes=14, modules=modules, source_files=source_hashes,
                  dependencies=dependency_hashes, dependency_method='conservative local include closure including conditional arms',
                  original_fixture=fixture_hashes, artifacts=artifacts,
                  limits=['Explicit upstream producer inputs and lawful active P66 job required',
                          'Flight launch, waitlist recurrence and keyboard integration are separate',
                          'Pending multi-job corpus has bounded modeled coverage, not full raw CHANG2 CPU/debris qualification',
                          'Other landing programs and whole Luminary/Comanche coverage remain separate'])
    manifest = ROOT / 'evidence/p66-complete-proof.json'
    manifest.write_text(json.dumps(report, indent=2) + '\n')
    files = OWNED + list(artifacts) + [str(manifest.relative_to(ROOT))]
    (BUILD / 'p66-complete-integration-files.txt').write_text('\n'.join(files) + '\n')
    print('Frozen', len(files), 'integration files:', len(source_hashes), 'source files,',
          len(dependency_hashes), 'dependencies, 96 actual disk modules, 148 raw C64 checkpoints')


if __name__ == '__main__':
    main()
