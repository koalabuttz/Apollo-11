#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Inventory original LM/CM BANK declarations for native module allocation.

This is conservative source-placement planning, not a decoder or a claim that
an AGC bank is a compiled native module. Symbol/SETLOC resolution still requires
the complete mission's assembler listing and reviewed native continuation maps.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def inventory(tree):
    directory = ROOT.parent / tree
    records = []
    for file in sorted(directory.glob('*.agc')):
        data = file.read_bytes()
        declarations = []
        for number, line in enumerate(data.decode().splitlines(), 1):
            code = line.split('#', 1)[0]
            match = re.search(r'(?<!\S)BANK\s+([0-7]+)(?:\s|$)', code)
            if match:
                declarations.append(dict(line=number, bank_octal=match[1].zfill(2)))
        records.append(dict(path=str(file.relative_to(ROOT.parent)),
            sha256=hashlib.sha256(data).hexdigest(), declared_banks=declarations))
    explicit = {d['bank_octal'] for file in records for d in file['declared_banks']}
    # Banks02/03 are the architectural fixed-fixed address ranges4000..7777
    # (octal on AGC), normally selected through SETLOC and symbol
    # assignments rather than BANK. Do not invent file membership for them.
    banks = sorted(explicit | {'02', '03'},
                   key=lambda value: int(value, 8))
    modules = [dict(allocation_key=tree + '-bank-' + bank, historical_bank_octal=bank,
        declares_placement=[r['path'] for r in records
            if any(d['bank_octal'] == bank for d in r['declared_banks'])],
        placement_provenance=('explicit BANK declarations' if bank in explicit else
            'fixed-fixed architectural address range; full listing required for source membership'),
        native_translation='unassigned', native_sources=[], native_continuations=[])
        for bank in banks]
    return dict(mission=tree, source_files=records, allocation_groups=modules,
        no_explicit_bank=[r['path'] for r in records if not r['declared_banks']])


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--output', type=Path, default=ROOT / 'build/overlay-source-manifest.json')
    args = p.parse_args()
    manifest = dict(schema_version=1, native_overlay_abi=1,
        placement_scope='Explicit original BANK declarations plus architectural fixed-fixed02/03; no inferred symbol addresses',
        resident_source_api='Canonical2048word erasable image; transient native views committed at ABI returns',
        module_slot=dict(address=32768, capacity=16384, entry=32784),
        proof_payloads_are_mission_translations=False,
        missions=[inventory(tree) for tree in ('Luminary099', 'Comanche055')])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(manifest, indent=2) + '\n')
    for mission in manifest['missions']:
        print(mission['mission'], len(mission['source_files']), 'source files;',
              len(mission['allocation_groups']), 'allocation groups; native mappings unassigned')
    print(args.output)


if __name__ == '__main__':
    main()
