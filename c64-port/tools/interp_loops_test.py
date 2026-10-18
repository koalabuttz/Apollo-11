#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/interp_routine_differential.py's step_cap_loops (#664): a case both
sides loop on past their own caps (the oracle's cycle limit, kind 9; the
native runner's step limit, kind 2) is a loop only when the native row is
the same again at 4x the steps. Rows are as the runners print them: case,
kind, Z, image digest."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import interp_routine_differential as D  # noqa: E402

# cm-conics specs1017 case 27577 at 12,2074 (#664): R1 0340 with C2 +21.4,
# ALPHA's DDV overflows, the period is +0 and PERIODCH never ends. yaAGC
# stops at its 4M-cycle cap in PERIODCH (Z 06475); the port at its step cap
# with the same image at 2,000 and 65,000 steps.
PERIODCH_R = ['0', '9', '3389', '0']
PERIODCH_N = ['0', '2', '3103', '1554354102']


def classify(rows_r, rows_n, rows_x4):
    asked = []

    def rerun(idx):
        asked.append(list(idx))
        # (The rerun's own case numbers start at 0, as on its sub file.)
        return [[str(k)] + rows_x4[i][1:] for k, i in enumerate(idx)]
    return D.step_cap_loops(rows_r, rows_n, rerun), asked


# The PERIODCH case: a loop, and only it is rerun.
got, asked = classify([PERIODCH_R], [PERIODCH_N], [PERIODCH_N])
assert got == {0} and asked == [[0]], (got, asked)

# Near-miss: the 4x rerun's image differs (the native run was still making
# progress at its cap): a mismatch.
got, _ = classify([PERIODCH_R], [PERIODCH_N], [['0', '2', '3103', '1554354103']])
assert got == set(), got
# ... or it stopped further on, or ended by itself.
got, _ = classify([PERIODCH_R], [PERIODCH_N], [['0', '2', '3104', '1554354102']])
assert got == set(), got
got, _ = classify([PERIODCH_R], [PERIODCH_N], [['0', '0', '3103', '1554354102']])
assert got == set(), got

# Not candidates at all (no rerun): the oracle ended, or the native runner
# ended or stopped on its own.
for r, n in ((['0', '0', '3103', '1'], PERIODCH_N), (PERIODCH_R, ['0', '0', '3103', '1']),
             (PERIODCH_R, ['0', '1', '3103', '1'])):
    got, asked = classify([r], [n], [n])
    assert got == set() and asked == [], (r, n, got, asked)

# Mixed rows: only the step-cap pair whose rerun repeats is a loop.
rows_r = [PERIODCH_R, ['1', '0', '5', '7'], ['2', '9', '3389', '0'], ['3', '9', '3389', '0']]
rows_n = [PERIODCH_N, ['1', '0', '5', '7'], ['2', '2', '3103', '9'], ['3', '9', '100', '0']]
x4 = [PERIODCH_N, None, ['2', '2', '3103', '8'], None]
got, asked = classify(rows_r, rows_n, x4)
assert got == {0} and asked == [[0, 2]], (got, asked)

print('interp_loops_test: step-cap loops OK (PERIODCH +0 period a loop; 4 near-misses still mismatches)')
