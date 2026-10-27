#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The interrupt hold gate (tools/flight_scenario_vice.py host_hold): an
overload window (FLIGHT_STEAL above the flight's share) gates the hold
without its stolen MCT; a window at the flight's share (fhd.STEAL_FLIGHT,
Apollo 11's 13%) gates the elapsed hold, stolen MCT and all; a window with
no steal gates the elapsed hold (docs/acceptance/interrupt-hold.md, issue
#912 item 3).
"""
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_host_differential as fhd  # noqa: E402
import flight_scenario_vice as fsv  # noqa: E402

LOG = ('interrupt hold: max 4149 MCT at TIME1 11450 (1149 without 3000 stolen)\n'
       'interrupt hold work: max 1241 MCT at TIME1 11021 (2700 stolen)\n')


class HoldGate(unittest.TestCase):
    def test_flight_share_is_13_percent_of_a_period(self):
        # 13% of a period's 2560 / 3 MCT.
        self.assertEqual(fhd.FLIGHT_STEAL_MCT, round(0.13 * 2560 / 3))

    def test_flight_rows_steal_the_flight_share_over_the_overload_windows(self):
        for row, of in (('cm-entry-steal13', 'cm-entry-overload'), ('lm-p63-steal13-nomon', 'lm-p63-overload-nomon')):
            env, over = fhd.scenario(row)[4], fhd.scenario(of)[4]
            self.assertEqual(env['FLIGHT_STEAL'].split(':')[0], over['FLIGHT_STEAL'].split(':')[0])
            self.assertEqual(int(env['FLIGHT_STEAL'].split(':')[1]), fhd.FLIGHT_STEAL_MCT)
            self.assertEqual({k: v for k, v in env.items() if k != 'FLIGHT_STEAL'},
                             {k: v for k, v in over.items() if k != 'FLIGHT_STEAL'})
            self.assertEqual(fhd.scenario(row)[1:4], fhd.scenario(of)[1:4])
            self.assertNotIn(row, {s[0] for s in fhd.FLIGHT})

    def test_overload_window_gates_work(self):
        h = fsv.host_hold('cm-entry-overload', LOG)
        self.assertEqual((h['mct'], h['gated'], h['gated_time1']), (4149, 1241, '11021'))
        self.assertNotIn('flight_level', h)

    def test_flight_level_window_gates_elapsed(self):
        h = fsv.host_hold('cm-entry-steal13', LOG)
        self.assertEqual((h['mct'], h['gated'], h['gated_time1'], h['work_mct']), (4149, 4149, '11450', 1241))
        self.assertTrue(h['flight_level'])

    def test_no_steal_gates_elapsed(self):
        h = fsv.host_hold('lm-p63-descent-thrust', LOG.splitlines()[0])
        self.assertEqual((h['mct'], h['gated']), (4149, 4149))
        self.assertFalse(fsv.flight_level_steal('lm-p63-descent-thrust'))


if __name__ == '__main__':
    unittest.main()
