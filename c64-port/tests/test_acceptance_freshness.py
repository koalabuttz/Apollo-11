#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/acceptance_freshness.py and the bootable doc's hash table
(tools/boot_acceptance.py image_hashes) on made-up docs in a scratch git
repository: the six docs must name one commit that is an ancestor of HEAD
(#942 B7), run logs must name that commit and match the sha256 their records and
docs pin (#979 B14, review-1028 R1), and the LM doc lists the check disk so a moved one is STALE."""
import hashlib
import pathlib
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import acceptance_freshness as af  # noqa: E402
import boot_acceptance as ba  # noqa: E402


def git(repo, *args):
    return subprocess.run(['git', *args], cwd=repo, check=True, capture_output=True, text=True,
                          env={'GIT_AUTHOR_NAME': 't', 'GIT_AUTHOR_EMAIL': 't@t', 'GIT_COMMITTER_NAME': 't',
                               'GIT_COMMITTER_EMAIL': 't@t', 'PATH': '/usr/bin:/bin'}).stdout.strip()


class Freshness(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.repo = pathlib.Path(self.tmp.name)
        git(self.repo, 'init', '-q', '-b', 'main')
        (self.repo / 'a').write_text('a\n')
        git(self.repo, 'add', 'a')
        git(self.repo, 'commit', '-q', '-m', 'one')
        self.first = git(self.repo, 'rev-parse', 'HEAD')
        # A commit on another branch: never an ancestor of main.
        git(self.repo, 'checkout', '-q', '-b', 'side')
        (self.repo / 'b').write_text('b\n')
        git(self.repo, 'add', 'b')
        git(self.repo, 'commit', '-q', '-m', 'side')
        self.side = git(self.repo, 'rev-parse', 'HEAD')
        git(self.repo, 'checkout', '-q', 'main')
        (self.repo / 'c').write_text('c\n')
        git(self.repo, 'add', 'c')
        git(self.repo, 'commit', '-q', '-m', 'two')
        (self.repo / 'docs/acceptance').mkdir(parents=True)
        self.patch = mock.patch.object(af, 'ROOT', self.repo)
        self.patch.start()

    def tearDown(self):
        self.patch.stop()
        self.tmp.cleanup()

    def docs(self, commit, result='PASS', skip=()):
        for c in af.CATEGORIES:
            if c in skip:
                continue
            (self.repo / 'docs/acceptance' / f'Luminary099-{c}.md').write_text(
                f'# x\n\nGenerated at commit `{commit}`. Result: **{result}**\n')

    def test_ancestor_commit_agrees(self):
        self.docs(self.first[:12])
        agree, seen = af.docs_agree('Luminary099')
        self.assertTrue(agree, seen)

    def test_commit_on_another_branch_fails(self):
        self.docs(self.side[:12])
        agree, seen = af.docs_agree('Luminary099')
        self.assertFalse(agree)
        self.assertTrue(all(v.endswith('(not an ancestor of HEAD)') for v in seen.values()), seen)

    def test_unknown_commit_fails_closed(self):
        self.docs('deadbeef1234')
        agree, seen = af.docs_agree('Luminary099')
        self.assertFalse(agree)
        self.assertIn('not an ancestor', next(iter(seen.values())))

    def test_mixed_and_not_pass_still_fail(self):
        self.docs(self.first[:12], result='FAIL')
        self.assertFalse(af.docs_agree('Luminary099')[0])
        self.docs(self.first[:12], skip=('native_output_tests',))
        self.assertFalse(af.docs_agree('Luminary099')[0])

    def test_run_logs_must_name_the_docs_commit(self):
        """#979 B14: a restart log left over from an earlier window fails."""
        ev = self.repo / 'evidence/cm-acceptance-restart'
        ev.mkdir(parents=True)
        (ev / 'host.log').write_text('no commit named here\n')
        (ev / 'cm-p20-restart-c64.log').write_text(
            f'cm-p20-restart: commit {self.first} with local changes; store cm.reu sha256 ab\n')
        self.assertEqual(af.logs_agree('cm', self.first[:12]), [])
        (ev / 'cm-p22-restart-c64.log').write_text(f'cm-p22-restart: commit {self.side} with local changes\n')
        stale = af.logs_agree('cm', self.first[:12])
        self.assertEqual(len(stale), 1)
        self.assertIn('cm-p22-restart-c64.log', stale[0])
        self.assertIn(self.side[:12], stale[0])
        self.assertEqual(af.logs_agree('lm', self.first[:12]), [])

    def evidence(self):
        """A restart record pinning its log, a boot record pinning its vice and
        monitor logs, and a device log pinned only by the doc's citation."""
        import json
        rs = self.repo / 'evidence/cm-acceptance-restart'
        bt = self.repo / 'evidence/cm-acceptance-boot'
        dv = self.repo / 'evidence/cm-acceptance-devices'
        for d in (rs, bt, dv):
            d.mkdir(parents=True, exist_ok=True)
        files = {rs / 'cm-p20-restart-c64.log': f'cm-p20-restart: commit {self.first} with local changes\nbody\n',
                 bt / 'cm-1581-vice.log': 'vice\n', bt / 'cm-1581-monitor.log': 'monitor\n',
                 dv / 'cm-uplink-reject.log': 'device\n'}
        for f, text in files.items():
            f.write_text(text)
        sha = {f: hashlib.sha256(f.read_bytes()).hexdigest() for f in files}
        (rs / 'cm-p20-restart.json').write_text(json.dumps(
            {'name': 'cm-p20-restart', 'c64': {'log': 'cm-p20-restart-c64.log', 'log_sha256': sha[rs / 'cm-p20-restart-c64.log']}}))
        (bt / 'cm-1581-boot.json').write_text(json.dumps(
            {'vice_log_sha256': sha[bt / 'cm-1581-vice.log'], 'monitor_log_sha256': sha[bt / 'cm-1581-monitor.log']}))
        (self.repo / 'docs/acceptance/Comanche055-device_adapter_tests.md').write_text(
            f'Generated at commit `{self.first[:12]}`. Result: **PASS**\n\n- (`evidence/cm-acceptance-devices/cm-uplink-reject.log` '
            f'(sha256 `{sha[dv / "cm-uplink-reject.log"][:16]}...`)): yes\n')
        return rs, bt, dv

    def test_review_1028_r1_mutations_fail(self):
        """Each mutation in review-1028's table fails; the untouched run passes."""
        rs, bt, dv = self.evidence()
        self.assertEqual(af.logs_agree('cm', self.first[:12]), [])
        log = rs / 'cm-p20-restart-c64.log'
        good = log.read_text()
        for mutated in (good.replace(self.first, self.side),           # another commit
                        good.replace(self.first, self.side[:12]),      # short other commit
                        good.replace(f'commit {self.first}', 'built'),  # commit line gone
                        good.replace('commit ', 'commit: ').replace(self.first, self.side),
                        good.replace('commit ', 'Commit ').replace(self.first, self.side),
                        good.replace(self.first, self.first + 'a'),     # 41 hex digits
                        good.replace('body', 'edited')):                # body edited, same commit
            log.write_text(mutated)
            self.assertNotEqual(af.logs_agree('cm', self.first[:12]), [], mutated)
        log.write_text(good)
        for f in (bt / 'cm-1581-vice.log', bt / 'cm-1581-monitor.log', dv / 'cm-uplink-reject.log'):
            keep = f.read_text()
            f.write_text(keep + 'edited\n')
            stale = af.logs_agree('cm', self.first[:12])
            self.assertEqual(len(stale), 1, stale)
            self.assertIn(f.name, stale[0])
            f.write_text(keep)
        (bt / 'cm-1581-monitor.log').unlink()
        self.assertIn('missing', af.logs_agree('cm', self.first[:12])[0])

    def test_check_disk_row_is_compared(self):
        build = self.repo / 'build'
        (build / 'flight-store').mkdir(parents=True)
        (build / 'flight-disk').mkdir(parents=True)
        files = {'flight-store/lm.reu': b'reu', 'flight-disk/lm-resident.prg': b'res',
                 'flight-disk/lm-loader.prg': b'ldr', 'flight-disk/test-1.d81': b'disk'}
        for n, b in files.items():
            (build / n).write_bytes(b)
        rows = '\n'.join(f'| `{n}` | `{hashlib.sha256(b).hexdigest()}` |' for n, b in files.items())
        (self.repo / 'docs/acceptance/Luminary099-bootable_c64_application.md').write_text(
            f'Generated at commit `{self.first[:12]}`. Result: **PASS**\n\n| File | sha256 |\n|---|---|\n{rows}\n')
        self.assertEqual(af.check('Luminary099', 'lm', build)[1], [])
        (build / 'flight-disk/test-1.d81').write_bytes(b'moved')
        self.assertEqual(af.check('Luminary099', 'lm', build)[1], ['flight-disk/test-1.d81'])
        (build / 'flight-disk/test-1.d81').unlink()
        self.assertEqual(af.check('Luminary099', 'lm', build)[3], ['flight-disk/test-1.d81'])


class ImageHashes(unittest.TestCase):
    boots = {'1581': {'sha256': {'flight-disk/lm-1.d81': 'a' * 64}},
             '1541': {'sha256': {'flight-disk/lm-1.d81': 'b' * 64, 'flight-disk/lm-1.d64': 'c' * 64}}}
    check = {'sha256': {'flight-disk/test-1.d81': 'd' * 64, 'flight-disk/test-2.d81': 'e' * 64,
                        'flight-disk/test-loader.prg': 'f' * 64}}

    def test_lm_lists_the_check_disk_images(self):
        rows = ba.image_hashes('lm', self.boots, self.check)
        self.assertEqual(list(rows), ['flight-disk/lm-1.d81', 'flight-disk/lm-1.d64',
                                      'flight-disk/test-1.d81', 'flight-disk/test-2.d81'])
        self.assertEqual(rows['flight-disk/lm-1.d81'], 'a' * 64)  # the 1581 run's record first

    def test_cm_and_no_check_disk(self):
        self.assertNotIn('flight-disk/test-1.d81', ba.image_hashes('cm', self.boots, self.check))
        self.assertEqual(len(ba.image_hashes('lm', self.boots, None)), 2)



class Rescore(unittest.TestCase):
    """#969 B8: the restart doc is re-judged from its evidence, and a pending
    "criteria" reason may name the categories it covers."""

    def test_criteria(self):
        self.assertEqual(af.criteria({'criteria': 'r'}, 'recovery_and_restart_tests'), 'r')
        entry = {'criteria': {'recovery_and_restart_tests': 'r'}}
        self.assertEqual(af.criteria(entry, 'recovery_and_restart_tests'), 'r')
        self.assertIsNone(af.criteria(entry, 'integrated_program_tests'))
        self.assertIsNone(af.criteria({}, 'integrated_program_tests'))

    def test_restart_rescore(self):
        import json
        import restart_acceptance as ra
        with tempfile.TemporaryDirectory() as tmp:
            root = pathlib.Path(tmp)
            (root / 'docs/acceptance').mkdir(parents=True)
            (root / 'evidence').mkdir()
            host = dict(commit='c0ffee', ok=True, routine_mismatches=[], new_mismatches=[], new_holes=[],
                        modreg=['77777', '77777'], redoctr=[1, 2], scenario='lm-p00-restart',
                        samples=10, samples_equal=10, base_samples_equal=10)
            ev = root / 'evidence/r.json'

            def doc(row, record):
                ev.write_text(json.dumps(dict(name='lm-p00-restart', gojam=5, asked=5, host=record, c64=None,
                                              gojam_on_c64=None)))
                (root / 'docs/acceptance/Luminary099-recovery_and_restart_tests.md').write_text(
                    'Generated by `x` at commit `c0ffee`. Result: **PASS**\n\n'
                    f'Flight store image: `flight-store/lm.reu` sha256 `{"a" * 64}`\n\n'
                    f'| lm-p00-restart | 5 (5) | h | c | g | {row} | `evidence/r.json` '
                    f'(sha256 `{ra.sha(ev)[:16]}...`) |\n')
            with mock.patch.object(ra, 'ROOT', root), mock.patch.object(ra, 'DOCS', root / 'docs/acceptance'), \
                    mock.patch.object(ra, 'GROUPS', {'lm': []}), mock.patch.object(ra, 'EXTRA', {'lm': ['lm-p00-restart']}):
                doc('pass', dict(host, sample_problems=[]))
                self.assertTrue(ra.rescore('lm')[0])
                doc('pass', host)   # no sample_problems record: the tip's verdict fails it
                same, text = ra.rescore('lm')
                self.assertFalse(same)
                self.assertIn('lm-p00-restart pass -> FAIL', text)


if __name__ == '__main__':
    unittest.main()
