#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/cert_precheck.py's DSKY sweep summary checks (#843)."""
import json
import pathlib
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import cert_precheck as cp  # noqa: E402


def summary(lm=(1830, 1830), cm=(1830, 1830), failed=()):
    """A summary with (passed, cases) per mission and the failed cases."""
    return dict(mode='all', missions=['lm', 'cm'], passed=lm[0] + cm[0], cases=lm[1] + cm[1],
                by_mission=dict(lm=dict(passed=lm[0], cases=lm[1]), cm=dict(passed=cm[0], cases=cm[1])),
                failed=list(failed))


class SummaryFaults(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.saved = cp.DSKY_SWEEP_SUMMARY
        cp.DSKY_SWEEP_SUMMARY = pathlib.Path(self.dir.name) / 'dsky-sweep-summary.json'

    def tearDown(self):
        cp.DSKY_SWEEP_SUMMARY = self.saved
        self.dir.cleanup()

    def test_clean_summary_has_no_fault(self):
        self.assertEqual(cp.dsky_sweep_summary_faults(summary(), 'lm'), [])

    def test_failed_case_listed_adds_up(self):
        s = summary(cm=(1829, 1830), failed=[dict(m='cm', ok=False, keys='V83E')])
        self.assertEqual(cp.dsky_sweep_summary_faults(s, 'cm'), [])

    def test_mission_case_dropped_unseen(self):
        s = summary()
        s['by_mission']['lm']['passed'] = 1829
        faults = cp.dsky_sweep_summary_faults(s, 'lm')
        self.assertTrue(any('lm counts do not add up' in f for f in faults), faults)

    def test_overall_case_dropped_unseen(self):
        s = summary()
        s['passed'] -= 1
        self.assertTrue(any(f.startswith('counts do not add up') for f in cp.dsky_sweep_summary_faults(s, 'cm')))

    def test_missions_must_sum_to_cases(self):
        s = summary()
        s['cases'] += 1
        s['passed'] += 1
        self.assertTrue(any('do not sum' in f for f in cp.dsky_sweep_summary_faults(s, 'lm')))

    def test_bytes_must_match_manifest(self):
        cp.DSKY_SWEEP_SUMMARY.write_text(json.dumps(summary()))
        (cp.DSKY_SWEEP_SUMMARY.parent / 'SHA256.json').write_text(
            json.dumps(dict(files={'dsky-sweep-summary.json': dict(sha256='0' * 64)})))
        self.assertTrue(any('SHA256.json' in f for f in cp.dsky_sweep_summary_faults(summary(), 'lm')))


class SweepFlags(unittest.TestCase):
    """check_dsky_sweep FAILs a draft on a sweep that is not a full match
    (#862 item 2): no --replay-exact, words left out, or no item6 flag."""
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        d = pathlib.Path(self.dir.name)
        self.saved = cp.DSKY_SWEEP_SUMMARY, cp.DSKY_SWEEP_EVIDENCE, cp.DSKY_SWEEP_KNOWN, cp.ROOT
        cp.DSKY_SWEEP_SUMMARY, cp.DSKY_SWEEP_EVIDENCE = d / 'dsky-sweep-summary.json', d / 'none.json'
        cp.DSKY_SWEEP_KNOWN, cp.ROOT = d / 'known.json', d
        cp.DSKY_SWEEP_KNOWN.write_text('{}')

    def tearDown(self):
        cp.DSKY_SWEEP_SUMMARY, cp.DSKY_SWEEP_EVIDENCE, cp.DSKY_SWEEP_KNOWN, cp.ROOT = self.saved
        self.dir.cleanup()

    def run_check(self, accepted=False, made=None, **flags):
        s = dict(made or summary(), replay_exact=True, skipped_cases=0, item6=True)
        s.update(flags)
        cp.DSKY_SWEEP_SUMMARY.write_text(json.dumps(s))
        report = cp.Report()
        cp.check_dsky_sweep(report, 'x', 'Luminary099', cp.DSKY_SWEEP_FILES[0], accepted)
        return report

    def test_full_match_is_clean(self):
        self.assertEqual(self.run_check().lines, [])

    def test_not_replay_exact_fails_a_draft(self):
        r = self.run_check(replay_exact=False)
        self.assertEqual(r.fails, 1, r.lines)
        self.assertEqual(self.run_check(accepted=True, replay_exact=False).warns, 1)

    def test_skipped_words_fail(self):
        self.assertEqual(self.run_check(skipped_cases=3).fails, 1)

    def test_item6_false_or_missing_fails(self):
        self.assertEqual(self.run_check(item6=False).fails, 1)
        self.assertEqual(self.run_check(item6=None).fails, 1)

    def test_clock_case_explains_item6(self):
        made = summary(lm=(1829, 1830), failed=[dict(m='lm', ok=False, keys='V16N36E', **{'class': 'clock'})])
        r = self.run_check(item6=False, made=made)
        self.assertEqual(r.fails, 0, r.lines)
        self.assertEqual(r.warns, 1, r.lines)


if __name__ == '__main__':
    unittest.main()
