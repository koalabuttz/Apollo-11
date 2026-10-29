#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Prepare the independently assembled original CM host-oracle fixture."""
import hashlib
from pathlib import Path
from fixed_data_generate import assemble

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = '6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061'
original = (ROOT / 'evidence/fixed-data-cm-original.rope.bin').read_bytes()
assert hashlib.sha256(original).hexdigest() == EXPECTED
assemble('Comanche055', ROOT / 'build/fixed-data')
actual = (ROOT / 'build/fixed-data/Comanche055/MAIN.agc.bin').read_bytes()
assert actual == original
print('PASS: fresh independently assembled original Comanche fixture', EXPECTED)
