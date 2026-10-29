#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build additive pending-job stages, checker and resident on a separate D81."""
import argparse
import hashlib
import json
import lzma
import os
from pathlib import Path
import subprocess
from native_registry_build import RESIDENT_SOURCES, RESIDENT_DEFINES
from overlay_build import crc16
from overlay_runtime_library import prepare

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cc', default='mos-c64-clang')
    parser.add_argument('--c1541', default='c1541')
    args = parser.parse_args()
    build = ROOT / 'build'
    generated = build / 'p66-pending-flight'
    generated.mkdir(exist_ok=True)
    with lzma.open(build / 'p66-pending-flight-reference.snapshots.xz', 'rt') as source:
        rows = [list(map(int, line.split())) for line in source]
    def fingerprint(words):
        value = 2166136261
        for word in words:
            for byte in (word & 255, word >> 8):
                value = ((value ^ byte) * 16777619) & 0xffffffff
        return value
    sequences = [[row for row in rows if row[0] == n] for n in (0, 1)]
    counts = list(map(len, sequences))
    assert all(0 < count < 256 for count in counts)
    maximum = max(counts)
    header = ['#include <stdint.h>', 'static const uint8_t prl_counts[2]={%s};' %
              ','.join(map(str, counts)),
              'static const uint16_t prl_locations[2][%d]={' % maximum]
    header += ['{' + ','.join(str(row[2]) for row in sequence) + '},'
               for sequence in sequences]
    header += ['};', 'static const uint32_t prl_fingerprints[2][%d]={' % maximum]
    header += ['{' + ','.join('UINT32_C(%d)' % fingerprint(row[3:])
                             for row in sequence) + '},' for sequence in sequences]
    header += ['};', '']
    (generated / 'p66_pending_flight_vectors.h').write_text('\n'.join(header))
    checker = (ROOT / 'platform/c64/p66_recurrence_fixture_check.c').read_text()
    checker = checker.replace('p66_recurrence_live_vectors.h', 'p66_pending_flight_vectors.h')
    checker = '#include "p66_native.h"\n' + checker
    checker = checker.replace('for(i=0;i<2048u;++i)',
        'if(loc==03353u&&f->stage==1u){'
        'if(m->words[0200]!=03534u||m->words[0201]!=060102u)return 1;'
        'm->words[0203]=(uint16_t)(037000u|(m->words[0203]&0777u));'
        'm->words[067]=014;'
        'p66_native_import_executive((executive *)(uintptr_t)0xc100,m);}'
        'for(i=0;i<2048u;++i)')
    (generated / 'checker.c').write_text(checker)
    manifest = [record for record in json.loads((ROOT / 'evidence' /
        'p66-application-split-modules.json').read_text()) if record['id'] != 148]
    retained = {}
    for record in manifest:
        path = ROOT / record.get('retained_prg', 'evidence/' + record['prg'])
        assert hashlib.sha256(path.read_bytes()).hexdigest() == record['sha256']
        retained[record['id']] = path
    common = [args.cc, '-I' + str(generated), '-Iinclude', '-Itests',
              '-Iplatform/c64', '-Ibuild', '-std=c99', '-Oz', '-fno-static-stack',
              '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
    library = prepare(args.cc, generated)
    def module(identifier, filename, sources, defines):
        output = build / ('p66-pending-flight-%d.prg' % identifier)
        subprocess.run(common + defines + ['-nostdlib', '-T',
            'platform/c64/overlay_payload.ld', '-Wl,--defsym=__overlay_module=%d,-Map,%s' %
            (identifier, build / ('p66-pending-flight-%d.map' % identifier))] +
            sources + ['platform/c64/overlay_entry.S', '-lc', library, '-lcrt',
                       '-o', str(output)], cwd=ROOT, check=True)
        image = output.read_bytes()
        assert image[:2] == b'\x00\x80' and len(image) - 2 <= 16384
        return dict(id=identifier, filename=filename, byte_count=len(image)-2,
                    crc16=crc16(image[2:]), sha256=hashlib.sha256(image).hexdigest(),
                    prg=output.name, mission_translation=False, native_adapter=True)
    for identifier, filename in ((88, 'PENDEXCH'), (89, 'PENDQ'), (98, 'PENDIMP'), (99, 'PENDEXP')):
        record = module(identifier, filename, ['platform/c64/p66_pending_payload.c',
            'src/p66_application_pending.c', 'src/p66_native.c', 'src/agc_word.c',
            'src/agc_multiword.c', 'src/lm_danzig_change.c',
            'src/lm_interpreter_change.c', 'src/p66_elapsed_return_control.c'],
            ['-DP66_NATIVE_DISPATCH_ONLY', '-DP66_PENDING_KERNEL_ONLY',
             '-DP66_PENDING_ONLY_SERVICE=%d' % identifier])
        assert identifier not in retained
        manifest.append(record)
    manifest.append(module(97, 'PENDINIT', ['platform/c64/p66_pending_initializer.c',
                                             'src/dsky_display.c'], []))
    manifest = [record for record in manifest if record['id'] != 245]
    retained.pop(245)
    manifest.append(module(245, 'PENDCK', [str(generated / 'checker.c'), 'src/p66_native.c', 'src/agc_word.c'], ['-DP66_NATIVE_DISPATCH_ONLY']))
    (generated / 'native_registry_table.h').write_text('#include "overlay_runtime.h"\n'
        '#define NATIVE_REGISTRY_MODULES %du\n' % len(manifest) +
        'static const apollo_overlay_module native_registry_modules[%d]={\n' % len(manifest) +
        ''.join('{%d,"%s",%d,%d},\n' % (m['id'], m['filename'], m['byte_count'], m['crc16'])
                for m in manifest) + '};\n')
    bootstrap = module(148, 'PENDINFO', ['platform/c64/native_registry_payload.c',
                                      'src/native_module_registry.c'], [])
    (generated / 'native_registry_bootstrap.h').write_text('#include "overlay_runtime.h"\n'
        'static const apollo_overlay_module native_registry_bootstrap={148,"PENDINFO",%d,%d};\n' %
        (bootstrap['byte_count'], bootstrap['crc16']))
    sources = [source for source in RESIDENT_SOURCES if source != 'src/p66_direct_launch.c'
               and not source.endswith('native_registry_recurrence_diagnostic.c')]
    sources += ['src/p66_application_split.c', 'src/p66_application_pending.c',
                'platform/c64/p66_pending_flight_diagnostic.c']
    defines = RESIDENT_DEFINES + ['P66_APPLICATION_SPLIT_DISPATCH_ONLY',
                                 'P66_PENDING_DISPATCH_ONLY', 'P66_PENDING_FIXED_BINDING', 'P66_PENDING_OBSERVE_INVOKER']
    subprocess.run(common + ['-D' + define for define in defines] +
        ['-T', 'platform/c64/overlay_resident.ld',
         '-Wl,-Map,build/p66-pending-flight.map'] + sources +
        ['-o', 'build/p66-pending-flight.prg'], cwd=ROOT, check=True)
    disk = build / 'p66-pending-flight.d81'
    disk.unlink(missing_ok=True)
    command = [args.c1541, '-format', 'APOLLO PENDING,11', 'd81', str(disk),
               '-write', str(build / 'p66-pending-flight.prg'), 'resident']
    for record in manifest + [bootstrap]:
        command += ['-write', str(retained.get(record['id'], build / record['prg'])),
                    record['filename'].lower()]
    subprocess.run(command, cwd=ROOT, check=True,
        env=dict(os.environ, XDG_CACHE_HOME=str(build / 'vice-cache'),
                 XDG_CONFIG_HOME=str(build / 'vice-config')))
    (build / 'p66-pending-flight-modules.json').write_text(
        json.dumps(manifest + [bootstrap], indent=2) + '\n')
    print('Built', len(manifest), 'stages; original source checkpoints', counts,
          'total', sum(counts), flush=True)


if __name__ == '__main__':
    main()
