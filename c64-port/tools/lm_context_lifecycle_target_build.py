#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build64 raw original fixtures and separately linked resident lifecycle."""
import argparse
import gzip
import hashlib
import json
import lzma
import os
from pathlib import Path
import subprocess
from overlay_build import crc16

ROOT = Path(__file__).resolve().parents[1]


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--cc', default='mos-c64-clang')
    p.add_argument('--c1541', default='c1541')
    a = p.parse_args()
    build = ROOT / 'build'
    stem = 'lm-context-lifecycle'
    subprocess.run(['cc', '-std=c99', '-O2', '-Wall', '-Wextra', '-Werror', '-Iinclude',
        '-Itests', 'src/agc_word.c', 'tests/throttle_bridge_cases.c',
        'tests/lm_priority_cases.c', 'tests/lm_sleep_cases.c',
        'tests/lm_context_lifecycle_cases.c', 'tools/lm_context_lifecycle_inputs.c',
        '-o', 'build/' + stem + '-inputs'], cwd=ROOT, check=True)
    raw = subprocess.check_output(['build/' + stem + '-inputs'], cwd=ROOT)
    inputs = [list(map(int, line.split())) for line in raw.splitlines()]
    wanted = {row[0] for row in inputs}
    outputs = {}
    with lzma.open(build / (stem + '-reference.snapshots.xz'), 'rt') as source:
        for line in source:
            n = int(line.split(' ', 1)[0])
            if n in wanted:
                outputs[n] = list(map(int, line.split()))
    assert len(outputs) == 64
    flags = [a.cc, '-Iinclude', '-Iplatform/c64', '-Ibuild', '-std=c99', '-Oz',
             '-fno-static-stack', '-mlto-zp=0', '-Wall', '-Wextra', '-Werror']
    manifest = []
    for group in range(1, 33):
        rows = inputs[(group-1)*2:group*2]
        payload = build / ('%s-fixture-%d.c' % (stem, group))
        prg = build / ('%s-fixture-%d.prg' % (stem, group))
        payload.write_text('#include "lm_context_lifecycle_fixture.h"\n'
            'static const uint16_t input[2][2051]={\n' +
            ''.join('{' + ','.join(map(str, row[1:])) + '},\n' for row in rows) +
            '};\nuint16_t apollo_overlay_main(apollo_overlay_state *image,uint16_t pointer){'
            'lm_context_lifecycle_fixture *f=(lm_context_lifecycle_fixture *)(uintptr_t)pointer;'
            'uint16_t i;if(!f||f->row>1)return 1;for(i=0;i<2048;++i)image->words[i]=input[f->row][i];'
            'f->io.superbank=input[f->row][2048];f->io.interrupt_enabled=input[f->row][2049];'
            'f->channel11=input[f->row][2050];f->entry=f->row?%d:%d;return 0;}\n' %
            (rows[1][0]&1, rows[0][0]&1))
        subprocess.run(flags + ['-nostdlib', '-T', 'platform/c64/overlay_payload.ld',
            '-Wl,--defsym=__overlay_module=%d,-Map,build/%s-fixture-%d.map' % (group, stem, group),
            str(payload), 'platform/c64/overlay_entry.S', '-lcrt', '-o', str(prg)], cwd=ROOT, check=True)
        image = prg.read_bytes()
        assert image[:2] == b'\0\x80' and len(image)-2 <= 16384
        manifest.append(dict(id=group, filename='L%d' % group, byte_count=len(image)-2,
            crc16=crc16(image[2:]), prg=prg.name, sha256=hashlib.sha256(image).hexdigest()))
    def fnv(words):
        h = 2166136261
        for word in words:
            for byte in (word&255, word>>8):
                h = ((h ^ byte)*16777619)&0xffffffff
        return h
    header = '#include "overlay_runtime.h"\nstatic const apollo_overlay_module lm_context_modules[32]={\n'
    header += ''.join('{%d,"%s",%d,%d},\n' % (m['id'], m['filename'], m['byte_count'], m['crc16'])
                      for m in manifest) + '};\n'
    header += 'static const uint32_t lm_context_hash[64]={' + \
        ','.join(str(fnv(outputs[row[0]][5:]))+'UL' for row in inputs) + '};\n'
    header += 'static const uint16_t lm_context_next[64][4]={\n' + \
        ''.join('{%d,%d,%d,%d},\n' % (outputs[row[0]][1], outputs[row[0]][3],
            outputs[row[0]][4], outputs[row[0]][2]) for row in inputs) + '};\n'
    (build / 'lm_context_lifecycle_target.h').write_text(header)
    subprocess.run(flags + ['-T', 'platform/c64/overlay_resident.ld',
        '-Wl,-Map,build/' + stem + '-target.map', 'src/overlay_runtime.c',
        'platform/c64/overlay_platform.c', 'src/agc_word.c', 'src/lm_context_lifecycle.c',
        'platform/c64/lm_context_lifecycle_diagnostic.c', '-o', 'build/' + stem + '-target.prg'],
        cwd=ROOT, check=True)
    disk = build / (stem + '-target.d81')
    disk.unlink(missing_ok=True)
    command = [a.c1541, '-format', 'LM CONTEXT,11', 'd81', str(disk), '-write',
               str(build / (stem + '-target.prg')), 'resident']
    for record in manifest:
        command += ['-write', str(build / record['prg']), record['filename'].lower()]
    environment = dict(os.environ, XDG_CACHE_HOME=str(build / 'vice-cache'),
                       XDG_CONFIG_HOME=str(build / 'vice-config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        Path(environment[key]).mkdir(exist_ok=True)
    subprocess.run(command, cwd=ROOT, check=True, env=environment)
    frozen = ''.join(' '.join(map(str, row))+'\n'+' '.join(map(str, outputs[row[0]]))+'\n'
                     for row in inputs).encode()
    (build / (stem + '-target-full.snapshots.gz')).write_bytes(gzip.compress(frozen, mtime=0))
    (build / (stem + '-target-modules.json')).write_text(json.dumps(manifest, indent=2)+'\n')
    print('Built64 full original lifecycle fixtures; SHA256', hashlib.sha256(frozen).hexdigest())


if __name__ == '__main__':
    main()
