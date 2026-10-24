#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/dsky_sweep_vice.py's --keys parsing (key_cases)."""
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import dsky_sweep_vice as dsv  # noqa: E402

CHAN = 'V37E00E|1200:032=17777,1300:032=37777'


class KeyCases(unittest.TestCase):
    def test_comma_form_still_splits(self):
        self.assertEqual(dsv.key_cases(['V16N36E,V06N43E']), ['V16N36E', 'V06N43E'])

    def test_channel_part_keeps_its_commas(self):
        self.assertEqual(dsv.key_cases([CHAN]), [CHAN])
        self.assertEqual(dsv.case_chan(CHAN.partition('|')[2], 1100)[1][1], '032=37777')

    def test_semicolon_separates(self):
        self.assertEqual(dsv.key_cases([f'V16N36E;{CHAN};']), ['V16N36E', CHAN])

    def test_repeated_option(self):
        self.assertEqual(dsv.key_cases(['V16N36E', CHAN, 'V05N09E,V06N36E']),
                         ['V16N36E', CHAN, 'V05N09E', 'V06N36E'])

    def test_no_values(self):
        self.assertEqual(dsv.key_cases([]), [])


if __name__ == '__main__':
    unittest.main()
