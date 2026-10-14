#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The compact interpreter's flight build on a PAL C64 in VICE.

tools/interp_core_vice.c runs the slot records of a fuzz run
(tools/interp_core_fuzz.c, FUZZ_DUMP) and checks each 6502 result against
the verification interpreter's on the host. The build is the flight
module's: AGC_FIXED_IMAGE, INTERP_K_OPS with the resident's ops, and the
$D000 extension, module $0A02, which it loads from the REU as the
resident's boot does. The REU holds the rope and the records; the CIA timer IRQ
runs throughout.

  interp_core_vice_test.py lm|cm [--records N] [--seed S]"""
import argparse
import os
import struct
import subprocess
import sys

R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(R, 'tools'))
import overlay_v2_pack as P  # noqa: E402
from resident_flight_build import SERVICES  # noqa: E402
from rope_image_pack import BASES, physical_words  # noqa: E402

B = os.path.join(R, 'build')
CC = os.environ.get('MOS_CC', os.path.join(R, 'deps/llvm-mos/bin/mos-c64-clang'))
ROPES = {'lm': 'Luminary099', 'cm': 'Comanche055'}
STRIDE = 0x2400
# Module $0A02 in the REU, between the two ropes (as interp_core_vice.c).
EXT_REU = 0x1d8000
MAX_RECORDS = (BASES['Luminary099'] - STRIDE) // STRIDE
FLAGS = ['-Oz', '-fno-static-stack', '-mlto-zp=0', '-DAGC_FIXED_IMAGE=1', '-DINTERP_K_OPS=1', '-DINTERP_EXTENSION=1', '-Wall', '-Wextra', '-Werror',
         '-Iinclude', '-Itests', '-Iplatform/c64', '-mllvm', '-inline-threshold=-50']
SOURCES = ['src/agc_image_ops.c', 'src/agc_word.c', 'src/agc_multiword.c',
           'src/agc_hardware_divide.c', 'platform/c64/rope_image_reu.c', 'src/overlay_reu.c',
           'platform/c64/overlay_reu_platform.c']


def records(m, count, seed):
    """Up to count slot records of the mission (pre image, rope reads, result)."""
    dump = os.path.join(B, 'interp-core-vice-%s.dump' % m)
    subprocess.run(['make', '-s', 'build/interp-core-fuzz'], cwd=R, check=True)
    subprocess.run([os.path.join(B, 'interp-core-fuzz'),
                    *[os.path.join(B, 'fixed-data/%s/MAIN.agc.bin' % p) for p in ('Luminary099', 'Comanche055')],
                    str(count * 4), str(seed), '0' if m == 'lm' else '1'],
                   cwd=R, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                   env=dict(os.environ, FUZZ_DUMP=dump))
    data = open(dump, 'rb').read()
    os.remove(dump)
    out, at = [], 0
    while at < len(data) and len(out) < count:
        def words(n):
            nonlocal at
            v = struct.unpack_from('<%dH' % n, data, at)
            at += 2 * n
            return v
        magic, = words(1)
        if magic != 0xa6c1:
            sys.exit('bad fuzz record')
        pre = words(2048)
        words(2)                       # interrupts enabled, superbank 0
        n, = words(1)
        reads = words(2 * n)
        words(1)                       # the word at LOC
        out.append((pre, reads, words(2048)))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('mission', choices=('lm', 'cm'))
    ap.add_argument('--records', type=int, default=180)
    ap.add_argument('--seed', type=int, default=71)
    a = ap.parse_args()
    if a.records > MAX_RECORDS:
        sys.exit('at most %d records' % MAX_RECORDS)
    recs = records(a.mission, a.records, a.seed)
    reu = bytearray(2 * 1024 * 1024)
    struct.pack_into('<H', reu, 0, len(recs))
    for k, (pre, reads, post) in enumerate(recs):
        at = (k + 1) * STRIDE
        struct.pack_into('<2048H', reu, at, *pre)
        struct.pack_into('<2048H', reu, at + 4096, *post)
        struct.pack_into('<%dH' % (1 + len(reads)), reu, at + 8192, len(reads) // 2, *reads)
    for p in ROPES.values():
        for i, w in enumerate(physical_words(os.path.join(R, 'build/fixed-data/%s/MAIN.agc.bin' % p))):
            struct.pack_into('<H', reu, BASES[p] + 2 * i, w)
    prg = os.path.join(B, 'interp-core-vice-%s.prg' % a.mission)
    core = ['src/%s_interp_core.c' % a.mission]
    # The harness (with the k_ ops) outside LTO: in flight the ops are the
    # resident's, which the module's code cannot inline.
    harness = prg + '.o'
    subprocess.run([CC, *[f for f in FLAGS if f != '-mlto-zp=0'], *(['-DINTERP_VICE_CM=1'] if a.mission == 'cm' else []), '-fno-lto', '-c',
                    'tools/interp_core_vice.c', '-o', harness], cwd=R, check=True)
    mission = 1 if a.mission == 'lm' else 2
    subprocess.run([CC, *FLAGS, '-T', 'tools/interp_core_vice.ld', '-Wl,-Map,%s.map' % prg,
                    f'-Wl,--defsym=__ov2_mission={mission},--defsym=__ov2_services={SERVICES}', *core, harness,
                    *SOURCES, '-o', prg], cwd=R, check=True)
    # Module $0A02 from the ELF, checked as the store packer checks it, at
    # EXT_REU in the REU (tools/interp_core_vice.c loads it to $D000).
    subprocess.run([os.path.join(os.path.dirname(CC), 'llvm-objcopy'), '-O', 'binary', '-j', '.interp_ext',
                    prg + '.elf', prg + '.ext'], cwd=R, check=True)
    ext = open(os.path.join(R, prg + '.ext'), 'rb').read()
    open(os.path.join(R, prg + '.ext.prg'), 'wb').write(b'\x00\xd0' + ext)
    P.parse_module(os.path.join(R, prg + '.ext.prg'), mission, SERVICES)
    reu[EXT_REU:EXT_REU + len(ext)] = ext
    image = os.path.join(B, 'interp-core-vice-%s.reu' % a.mission)
    open(image, 'wb').write(reu)
    print('%s: %d records' % (a.mission.upper(), len(recs)))
    return subprocess.run([sys.executable, 'tools/vice_run.py', prg, '--reu', image, '--cycles', '2000000000',
                           '--timeout', '900'], cwd=R).returncode


if __name__ == '__main__':
    sys.exit(main())
