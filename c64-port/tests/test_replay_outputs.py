#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/flight_scenario_vice.py --replay-outputs: the output list compare
on made-up lists."""
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_scenario_vice as fsv  # noqa: E402

HOST = [(10, 0o5, 0o210), (10, 0o6, 0o4), (40, 0o5, 0), (90, 0o11, 0o20400)]


class Compare(unittest.TestCase):
    def test_same(self):
        self.assertIsNone(fsv.outputs_compare(HOST, list(HOST), 100))

    def test_late_change(self):
        c64 = [*HOST[:2], (42, 0o5, 0), HOST[3]]
        first = fsv.outputs_compare(HOST, c64, 100)
        self.assertEqual(first['index'], 2)
        self.assertEqual(first['host'], dict(cs=40, channel='5', value='00000'))
        self.assertEqual(first['c64']['cs'], 42)
        self.assertIsNone(fsv.outputs_compare(HOST, c64, 100, slack=2))
        self.assertIsNotNone(fsv.outputs_compare(HOST, c64, 100, slack=1))

    def test_other_value_or_order(self):
        self.assertEqual(fsv.outputs_compare(HOST, [HOST[1], HOST[0], *HOST[2:]], 100)['index'], 0)
        self.assertEqual(fsv.outputs_compare(HOST, [*HOST[:3], (90, 0o11, 0o10400)], 100)['index'], 3)

    def test_missing_or_extra(self):
        self.assertEqual(fsv.outputs_compare(HOST, HOST[:3], 100)['c64'], None)
        self.assertEqual(fsv.outputs_compare(HOST, [*HOST, (95, 0o12, 1)], 100)['host'], None)

    def test_only_to_the_end(self):
        # Changes after the window's end on either side are not compared.
        self.assertIsNone(fsv.outputs_compare(HOST, HOST[:3], 80))
        self.assertIsNone(fsv.outputs_compare(HOST[:3], HOST, 80))

    def test_cap(self):
        self.assertGreaterEqual(fsv.outputs_cap(HOST), 2 * len(HOST))
        self.assertGreater(fsv.outputs_cap([]), 0)
        self.assertEqual(fsv.OUTPUT.size, 6)


if __name__ == '__main__':
    unittest.main()
