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


def check(named, data, accepted=True, **extra):
    certificate = {'evidence': {'c64_execution_results': {'path': 'x', 'vice_targets': named}}, **extra}
    if named is None:
        del certificate['evidence']['c64_execution_results']['vice_targets']
    report = cert_precheck.Report()
    cert_precheck.check_manifest(report, PATH, PATH, certificate, data, NOW, accepted=accepted)
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
        self.assertIn('next refreeze', report.lines[0])
        self.assertEqual(check(None, manifest(), c64_exempt='documentation only').lines, [])
        self.assertEqual(check(None, manifest(), accepted=False).fails, 1)

    def test_malformed(self):
        self.assertEqual(check({'a': 0}, manifest()).fails, 1)

    def test_cover(self):
        record = {'c64_branch_outcomes': {'path': 'evidence/m/m-cover.json'}}

        def cover(row, accepted):
            certificate = {'evidence': {'c64_execution_results': {'path': 'x', 'vice_targets': {'vice-a': 100}},
                                        **record}}
            report = cert_precheck.Report()
            cert_precheck.check_manifest(report, PATH, PATH, certificate, manifest() | {'c64_cover': {'m': row}},
                                         NOW, accepted=accepted)
            return (report.fails, report.warns)
        good = {'record': 'evidence/m/m-cover.json', 'verified': True}
        self.assertEqual(cover(good, False), (0, 0))
        for bad in ({**good, 'verified': False, 'reason': 'regenerated cases are not the frozen ones'},
                    {**good, 'missing_count': 2, 'missing': ['x']}):
            self.assertEqual(cover(bad, True), (0, 1))
            self.assertEqual(cover(bad, False), (1, 0))
        none = {'reason': 'no re-derivation for this record yet'}
        self.assertEqual(cover(none, True), (0, 0))
        self.assertEqual(cover(none, False), (1, 0))
        # another record re-derived: the next nightly decides, accepted or not
        self.assertEqual(cover({**good, 'record': 'evidence/m/old.json'}, False), (0, 1))

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
        line = 'PASS: stock-ROM PAL C64 proves 4232 native original bindings and alias RAM reads/writes\n'
        self.assertEqual(vice_manifest.run_result(line, True)[:2], (4232, 0))


class StaleEvidence(unittest.TestCase):
    """A release-stored file whose certificate and SHA256.json agree but whose
    local copy differs is reported as a stale download, not a bad hash."""

    def run_check(self, certified):
        import hashlib
        import json
        import tempfile
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / 'evidence' / 'm').mkdir(parents=True)
            good = hashlib.sha256(b'new log\n').hexdigest()
            (root / 'evidence' / 'm' / 'SHA256.json').write_text(json.dumps(
                {'files': {'m-host.log': {'sha256': good, 'store': 'release'}}}))
            (root / 'evidence' / 'm' / 'm-host.log').write_text('old log\n')
            record = {'path': 'evidence/m/m-host.log', 'sha256': good if certified else 'b' * 64}
            evidence = {c: dict(record) for c in cert_precheck.CATEGORIES}
            report = cert_precheck.Report()
            saved = cert_precheck.ROOT
            cert_precheck.ROOT = root
            try:
                cert_precheck.check_evidence(report, PATH, {'evidence': evidence}, False)
            finally:
                cert_precheck.ROOT = saved
            return '\n'.join(report.lines)

    def test_stale_copy(self):
        text = self.run_check(True)
        self.assertIn('stale local copy', text)
        self.assertNotIn('update the hash', text)

    def test_wrong_hash(self):
        text = self.run_check(False)
        self.assertIn('update the hash', text)
        self.assertNotIn('stale local copy', text)


if __name__ == '__main__':
    unittest.main()
