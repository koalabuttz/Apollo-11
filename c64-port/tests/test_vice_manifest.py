#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""cert_precheck's vice manifest check (#127) and the manifest's log reading."""
import datetime
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import cert_precheck  # noqa: E402
import vice_manifest  # noqa: E402

NOW = datetime.datetime(2026, 10, 13, 12, tzinfo=datetime.timezone.utc)
PATH = 'Luminary099/TEST.agc'


def manifest(date='2026-10-13T05:30:00Z', certified=True, **targets):
    rows = {'vice-a': {'status': 'pass', 'checks': 100, 'failures': 0},
            'vice-b': {'status': 'fail', 'checks': 0, 'failures': 3}}
    rows.update(targets)
    return {'schema': 1, 'run_id': '1', 'commit': 'f' * 40, 'date': date, 'targets': rows,
            'certificates': {PATH: {'complete_native_coverage': {'vice-a': 100}}} if certified else {}}


def check(named, data, **extra):
    certificate = {'evidence': {'c64_execution_results': {'path': 'x', 'vice_targets': named}}, **extra}
    if named is None:
        del certificate['evidence']['c64_execution_results']['vice_targets']
    report = cert_precheck.Report()
    cert_precheck.check_manifest(report, PATH, PATH, certificate, data, NOW)
    return report


class ViceManifest(unittest.TestCase):
    def test_pass(self):
        report = check({'vice-a': 100}, manifest())
        self.assertEqual((report.fails, report.warns), (0, 0), report.lines)

    def test_list_form_certifies_a_pass(self):
        self.assertEqual(check(['vice-a'], manifest(certified=False)).lines, [])

    def test_missing_target(self):
        report = check({'vice-a': 100, 'vice-c': 1}, manifest() | {
            'certificates': {PATH: {'complete_native_coverage': {'vice-a': 100, 'vice-c': 1}}}})
        self.assertEqual(report.fails, 1)
        self.assertIn('vice-c is not in', report.lines[0])

    def test_failed_target(self):
        report = check({'vice-b': 1}, manifest() | {'certificates': {PATH: {'x': {'vice-b': 1}}}})
        self.assertEqual(report.fails, 1)
        self.assertIn('FAILED', report.lines[0])

    def test_fewer_checks(self):
        report = check({'vice-a': 100}, manifest(**{'vice-a': {'status': 'pass', 'checks': 99}}))
        self.assertEqual(report.fails, 1)
        self.assertIn('fewer than the 100 certified', report.lines[0])

    def test_stale(self):
        report = check({'vice-a': 100}, manifest(date='2026-10-09T05:30:00Z'))
        self.assertEqual(report.fails, 1)
        self.assertIn('over 3 days old', report.lines[0])

    def test_newer_certificate_warns(self):
        report = check({'vice-a': 200, 'vice-new': 5}, manifest())
        self.assertEqual((report.fails, report.warns), (0, 2), report.lines)
        self.assertTrue(all('next nightly decides' in line for line in report.lines))

    def test_migration_and_exempt(self):
        report = check(None, manifest())
        self.assertEqual((report.fails, report.warns), (0, 1))
        self.assertIn('migrate', report.lines[0])
        self.assertEqual(check(None, manifest(), c64_exempt='documentation only').lines, [])

    def test_malformed(self):
        self.assertEqual(check({'a': 0}, manifest()).fails, 1)

    def test_no_manifest(self):
        self.assertEqual(check({'vice-a': 1}, None).lines, [])

    def test_log_counts(self):
        log = ('PASS: VICE PAL x.prg: 1,024 checks, 0 failures\n'
               'PASS: VICE PAL C64, 32 complete lm_single_trig group original-assembly states; ok\n'
               'PASS: stock PAL C64, slot swaps\n'
               "PASS complete states; {'hardware': 22, 'states': 64}\n")
        self.assertEqual(vice_manifest.run_result(log, True)[:2], (1024 + 32 + 1 + 64, 0))
        self.assertEqual(vice_manifest.run_result('FAIL: x.prg: mailbox (2 failures, 9 checks)', False)[:2], (0, 2))
        self.assertEqual(vice_manifest.run_result('PRG sha256: ' + 'a' * 64 + ' x.prg', True)[2], {'x.prg': 'a' * 64})


if __name__ == '__main__':
    unittest.main()
