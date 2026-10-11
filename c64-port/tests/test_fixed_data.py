#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare every accessible native table word with original assembled rope."""
import ctypes
import gzip
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


class Table(ctypes.Structure):
    _fields_ = [('address', ctypes.c_uint16), ('count', ctypes.c_uint16)]


def main():
    fresh = ROOT / 'build/fixed-data/audit.json'
    audit = json.loads(fresh.read_text() if fresh.exists() else
                       gzip.decompress((ROOT / 'evidence/fixed-data-audit.json.gz').read_bytes()))
    target = ROOT / 'build/fixed-data-host.so'
    subprocess.run(['cc', '-std=c99', '-O2', '-Wall', '-Wextra', '-Werror',
                    '-Wconversion', '-pedantic', '-shared', '-fPIC', '-Iinclude',
                    'src/fixed_data.c', '-o', str(target)], cwd=ROOT, check=True)
    lib = ctypes.CDLL(str(target))
    read = lib.apollo_fixed_data_read
    read.argtypes = [ctypes.c_int, ctypes.c_uint16, ctypes.POINTER(ctypes.c_uint16)]
    read.restype = ctypes.c_bool
    info = lib.apollo_noun_table_info
    info.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.POINTER(Table)]
    info.restype = ctypes.c_bool
    noun = lib.apollo_noun_table_read
    noun.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_uint16,
                     ctypes.POINTER(ctypes.c_uint16)]
    noun.restype = ctypes.c_bool
    count = 0
    for mission_id, mission in enumerate(audit['missions']):
        directory = ROOT / 'build/fixed-data' / mission['mission']
        candidate = directory / 'MAIN.agc.bin'
        if not candidate.exists():
            candidate = ROOT / f'evidence/fixed-data-{"lm" if mission_id == 0 else "cm"}-original.rope.bin'
        rope = candidate.read_bytes()
        assert hashlib.sha256(rope).hexdigest() == mission['assembled_rope_sha256']
        for source in mission['source_audit']:
            assert hashlib.sha256((ROOT.parent / source['path']).read_bytes()).hexdigest() == source['sha256']
        # Independent external-file decoder, with physical banks in the
        # original assembler's documented fixed-fixed-first file order.
        reference = {}
        order = [2, 3, 0, 1] + list(range(4, 36))
        for file_bank, physical_bank in enumerate(order):
            for offset in range(1024):
                position = (file_bank * 1024 + offset) * 2
                reference[physical_bank * 1024 + offset] = (
                    ((rope[position] << 8) | rope[position + 1]) >> 1)
        covered = {record['physical_fixed_word'] + offset
                   for record in mission['native_records']
                   for offset in range(len(record['words']))}
        assert len(covered) == mission['native_word_count']
        for address in range(65536):
            value = ctypes.c_uint16(0xa55a)
            supported = read(mission_id, address, ctypes.byref(value))
            assert supported == (address in covered), (mission['mission'], address)
            assert value.value == (reference[address] if supported else 0xa55a)
            count += int(supported)
        for table_id, table in enumerate(mission['noun_tables']):
            descriptor = Table(123, 456)
            assert info(mission_id, table_id, ctypes.byref(descriptor))
            assert (descriptor.address, descriptor.count) == (table['address'], table['word_count'])
            for index in range(descriptor.count):
                value = ctypes.c_uint16(0xa55a)
                assert noun(mission_id, table_id, index, ctypes.byref(value))
                assert value.value == reference[descriptor.address + index]
            value = ctypes.c_uint16(0xa55a)
            assert not noun(mission_id, table_id, descriptor.count, ctypes.byref(value))
            assert value.value == 0xa55a
        assert not read(mission_id, mission['noun_tables'][0]['address'] - 1,
                        ctypes.byref(ctypes.c_uint16()))
    print(f'PASS: all {count} native data words match independently decoded original ropes; '
          'all 131072 address probes and six tables per mission obey coverage/bounds')


if __name__ == '__main__':
    main()
