#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/interp_routine_differential.py's edge_values shape (#148).

An edge vector written at an operand near the top of erasable stops at
03777, as the other shapes do: it never wraps into A, L and Q at 0-5.

  tools/interp_edges_test.py    (the CI ledgers job)
"""
import collections
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import interp_routine_differential as D  # noqa: E402

E = collections.OrderedDict()
D.put_words(E, 0o3774, list(range(1, 7)))
assert dict(E) == {0o3774: 1, 0o3775: 2, 0o3776: 3, 0o3777: 4}, dict(E)
E.clear()
D.put_words(E, 0o3700, list(range(1, 7)))
assert list(E) == list(range(0o3700, 0o3706)), list(E)
# An edge vector is three (hi, lo) pairs: six words from the operand.
assert len([w for pair in D.edge_vector(random.Random(1), None) for w in pair]) == 6

print('interp_edges_test: edge_values writes stop at 03777 (no wrap into A, L, Q)')
