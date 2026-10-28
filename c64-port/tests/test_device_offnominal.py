#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/device_offnominal.py: every off-nominal check that happens at a
time has a C64 window over that time (issue #925 A2), and the doc says
which window. A timed check that no window holds fails the section
unless its row gives a host_only reason (#942 B3). The section judges the
run's own times, so nothing here freezes host times.
"""
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import device_offnominal as off  # noqa: E402


def section(at, host_only=None):
    """The off-nominal section for one row whose second check happened at at cs."""
    row = dict(scenario='x-row', owner='D1', failure='f', response='r', windows=[(1900, 800)],
               checks=[off.quiet('W', 0, 10, 'quiet'), off.final('W', 1, 'final')])
    if host_only:
        row['host_only'] = host_only
    host = dict(ok=True, mismatches=0, bridged=0, evidence='e',
                checks=[dict(check='quiet', kind='quiet', ok=True, at_cs=None),
                        dict(check='final', kind='final', ok=True, at_cs=at)])
    runs = [dict(scenario='x-row', snap_cs=1900, snap_at=1907, agc_cs=808, _why=[], replay_matched_cs=[2715])]
    return off.section([dict(row=row, host=host)], runs, lambda w: '')


class Windows(unittest.TestCase):
    def test_a_covered_check_passes_and_names_its_window(self):
        lines, bad = section(2000)
        self.assertEqual(bad, [])
        self.assertIn('final: yes, at 2000 cs (C64 window 1900+800)', '\n'.join(lines))

    def test_an_uncovered_check_fails(self):
        lines, bad = section(3000)
        self.assertEqual(len(bad), 1)
        self.assertIn('final happened at 3000 cs, in no C64 window', bad[0])

    def test_a_host_only_reason_excuses_it_and_shows(self):
        lines, bad = section(3000, {1: 'too late for a window'})
        self.assertEqual(bad, [])
        self.assertIn('(host only: too late for a window)', '\n'.join(lines))

    def test_host_only_names_timed_checks(self):
        for m, rows in off.ROWS.items():
            for row in rows:
                for i, reason in row.get('host_only', {}).items():
                    with self.subTest(scenario=row['scenario'], check=i):
                        self.assertNotEqual(row['checks'][i][0], 'quiet')
                        self.assertTrue(reason)

    def test_windows_fit_the_replay_states(self):
        # 50 cs checkpoints over 800 cs is the most below the rope (tools/device_acceptance.py windows).
        for m in off.ROWS:
            for scenario, snap, cs, every in off.windows(m):
                self.assertLessEqual(cs, 800, scenario)
                self.assertEqual(every, 50)

    def test_nomon_window_ends_past_the_busy_jet_pass(self):
        # 19400+600 ends at 648 cs inside a DAP jet pass (host HEADTJET+5, C64
        # TJETLAW, every timer equal), a busy end the replay must compare;
        # 19400+750 matches there as a mid checkpoint and ends quiet at 759 cs.
        nomon = next(r for r in off.ROWS['lm'] if r['scenario'] == 'lm-p63-overload-nomon')
        self.assertEqual(nomon['windows'], [(19400, 750)])

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
