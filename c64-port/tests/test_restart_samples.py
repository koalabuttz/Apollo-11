#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/flight_restart_check.py's sample judge (sample_problems) on the
128 samples lm-p00-restart's host run printed at 11cab8d3 (FLIGHT_SHOW),
tests/restart_lm_p00_samples.txt (#942 B4)."""
import pathlib
import sys
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_restart_check as frc  # noqa: E402

# Luminary099's addresses (build/fixed-data/Luminary099/assembly.lst).
SYM = dict(DSPCNT=0o776, NOUT=0o1016, DSPTAB=0o1023, DSRUPTSW=0o1313, ITEMP1=0o61)
AT = 2500
TEXT = (ROOT / 'tests/restart_lm_p00_samples.txt').read_text()


def line(cs, words, n=None):
    body = ''.join(f' {a:04o} {x:05o}/{y:05o}' for a, (x, y) in words.items())
    return f'{cs} cs (native routine 1):{body} ({n or len(words)} words, native/yaAGC)'


class SampleJudge(unittest.TestCase):
    def setUp(self):
        self.got = frc.sample_diffs(TEXT)

    def problems(self, got, base=None, name='lm-p00-restart'):
        return frc.sample_problems(name, got, base or {}, AT, SYM)

    def test_the_observed_run_is_the_artifact(self):
        self.assertEqual(len(self.got), 128)
        self.assertEqual(sorted(self.got)[:6], [2507, 2509, 2511, 2513, 2515, 2527])
        self.assertEqual(self.problems(self.got), [])

    def test_samples_the_run_without_the_restart_shares_are_not_added(self):
        self.assertEqual(self.problems(self.got, self.got, 'cm-p00-restart'), [])
        changed = {**self.got, 2539: {0o1313: (0o4, 0o3)}}
        self.assertEqual(self.problems(changed, self.got, 'cm-p00-restart'), ['2539 cs differs'])

    def test_other_scenarios_have_no_artifact(self):
        self.assertEqual(len(self.problems(self.got, {}, 'cm-p00-restart')), 128)

    def test_anything_else_fails(self):
        cases = {
            'another word': (2539, {0o1313: (0o5, 0o4), 0o100: (1, 2)}, 'not the previous DSRUPTSW phase'),
            'two phases behind': (2539, {0o1313: (0o6, 0o4)}, 'not the previous DSRUPTSW phase'),
            'ITEMP1 alone': (2539, {0o61: (0, 0o37777)}, 'not the previous DSRUPTSW phase'),
            'a non-DSKY word at the start': (2509, {0o1313: (0o56406, 0o77006), 0o200: (0, 1)}, 'not DSKY words'),
        }
        for why, (cs, words, want) in cases.items():
            got = frc.sample_diffs(TEXT.replace(next(x for x in TEXT.splitlines() if x.startswith(f'{cs} cs')), line(cs, words)))
            p = self.problems(got)
            self.assertEqual(len(p), 1, why)
            self.assertIn(want, p[0], why)

    def test_off_the_t4rupt_grid_fails(self):
        got = dict(self.got)
        got[2552] = got.pop(2551)
        p = self.problems(got)
        self.assertEqual(p, ['2552 cs: off the 12 cs T4RUPT grid from 2527 cs'])
        got = {**self.got, 2545: {0o1313: (0o5, 0o4)}}
        self.assertEqual(self.problems(got), ['2545 cs: off the 12 cs T4RUPT grid from 2527 cs'])

    def test_a_sample_with_more_than_8_words_fails(self):
        got = frc.sample_diffs(TEXT + line(3999, {0o1313: (0o5, 0o4)}, n=9).replace(' (9', ' ... (9') + '\n')
        self.assertIsNone(got[3999])
        self.assertEqual(self.problems(got), ['3999 cs: more than 8 words differ'])


if __name__ == '__main__':
    unittest.main()
