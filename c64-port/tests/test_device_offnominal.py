#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/device_offnominal.py: every off-nominal check that happens at a
time has a C64 window over that time (issue #925 A2), and the doc says
which window, or host only.

HOST_AT is when each check happened on the host (the device docs at
7dd3db43, Response checks). A row whose answer moves needs its windows
moved with it.
"""
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import device_offnominal as off  # noqa: E402

HOST_AT = {
    'lm-rendezvous-rr-dropout': [20030, 21156],
    'lm-p63-lr-dropout': [88206, 95205],
    'lm-uplink-reject': [2124, 5000, 6500, 7158],
    'lm-p52-imu-fail': [3014, 3014, 4503],
    'lm-p52-imu-cage': [3014, 3542],
    'lm-p63-overload-nomon': [19646],
    'cm-p20-vhf-dropout': [None, 31680],
    'cm-uplink-reject': [2124, 5000, 6500, 7157],
    'cm-p52-imu-fail': [6015, 6015, 7023],
    'cm-p52-imu-cage': [6015, 6543],
    'cm-p52-gimbal-lock': [3039, 3615, 19014],
    'cm-entry-overload': [22195],
}
# Checks no window covers, on purpose: VHFCNT's last mark comes 140 s after
# the dropout.
HOST_ONLY = {('cm-p20-vhf-dropout', 1)}
# A snapshot lands on the next native sample point, so it can start a few
# centiseconds late: an event this close to a window's start may fall
# before the run.
SLIDE = 40


class Windows(unittest.TestCase):
    def test_every_timed_check_has_a_window(self):
        for m, rows in off.ROWS.items():
            for row in rows:
                times = HOST_AT[row['scenario']]
                self.assertEqual(len(times), len(row['checks']), row['scenario'])
                for i, t in enumerate(times):
                    if t is None or (row['scenario'], i) in HOST_ONLY:
                        continue
                    with self.subTest(scenario=row['scenario'], check=i, at=t):
                        self.assertTrue(any(snap + SLIDE <= t <= snap + cs for snap, cs in row['windows']),
                                        f"{t} cs is in no window of {row['windows']}")

    def test_windows_fit_the_replay_states(self):
        # 50 cs checkpoints over 800 cs is the most below the rope (tools/device_acceptance.py windows).
        for m in off.ROWS:
            for scenario, snap, cs, every in off.windows(m):
                self.assertLessEqual(cs, 800, scenario)
                self.assertEqual(every, 50)

    def test_covered_names_the_window_that_ran(self):
        spans = [((1900, 800), (1907, 2715)), ((4900, 300), (4912, 5220))]
        self.assertEqual(off.covered(dict(kind='word', at_cs=5000), spans), '4900+300')
        self.assertIsNone(off.covered(dict(kind='word', at_cs=6500), spans))
        self.assertIsNone(off.covered(dict(kind='word', at_cs=4905), spans))   # before the slid start
        self.assertIsNone(off.covered(dict(kind='quiet', at_cs=None), spans))

    def test_span_uses_the_run(self):
        self.assertEqual(off.span(dict(snap_at=1907, agc_cs=808), 1900, 800), (1907, 2715))
        self.assertEqual(off.span(None, 1900, 800), (1900, 2700))


if __name__ == '__main__':
    unittest.main()
