#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/replay_batch.py: a window's status comes from its exit code and
every log line (a FAIL before the pass line, seam and chain FAILs, a crash
after the pass line, masks); finished windows are skipped only on the same
tree and command; the budget stops the batch with exit 3; the summary lists
every window, run or not; a held lock stops a second batch. A stand-in
runner prints flight_scenario_vice.py's line shapes in place of VICE."""
import os
import pathlib
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOL = ROOT / 'tools/replay_batch.py'
FAKE = r'''
import pathlib, sys
a = sys.argv[1:]
s = a[a.index('--scenario') + 1]
c = pathlib.Path(sys.argv[0]).with_name('runs.txt')
c.write_text(c.read_text() + s + '\n' if c.exists() else s + '\n')
print(f'{s}: commit 0123abcd; store lm.reu sha256 ' + ('ff' if s.startswith('other') else 'ee') * 32)
ok = f"{s}: the C64 ends (100 cs) in the host's state"
if s.startswith('dma'):        # REU bounds FAIL printed before the pass line
    print('FAIL: lm: window 1: REU DMA out of bounds at $1C0000'); print(ok); sys.exit(1)
if s.startswith('seam'):       # chain seams do not start with FAIL
    print(ok); print(f'{s}: seam 2: TIME3 differs by 1: FAIL')
    print(f'{s}: chain of 3 windows: FAIL at window 2'); sys.exit(1)
if s.startswith('crash'):      # dies after the pass line
    print(ok); print('Traceback (most recent call last):'); print('OSError: VICE went away'); sys.exit(1)
if s.startswith('mask'):
    print(f'{s}: masked_dead_superbank 1 range'); print(ok); sys.exit(0)
if s.startswith('big'):
    print(f'FAIL: {s}: the window does not fit in memory'); sys.exit(1)
print(ok)
'''


class Batch(unittest.TestCase):
    def setUp(self):
        self.d = pathlib.Path(tempfile.mkdtemp())
        (self.d / 'fake.py').write_text(FAKE)
        self.list('# windows\nlm-a 100 200\ndma-b 300 400 --replay-outputs\n\ncm-c 5 6\n')
        self.out = self.d / 'out'

    def list(self, text):
        (self.d / 'w.txt').write_text(text)

    def run_batch(self, *more, head='c1'):
        return subprocess.run([sys.executable, str(TOOL), '--windows', str(self.d / 'w.txt'), '--out', str(self.out),
                               '--runner', f'{sys.executable} {self.d / "fake.py"}', '--head', head, *more],
                              capture_output=True, text=True)

    def runs(self):
        p = self.d / 'runs.txt'
        return p.read_text().split() if p.exists() else []

    def rows(self):
        lines = (self.out / '_summary.txt').read_text().splitlines()
        return lines[0], {r.split('|')[0]: r.split('|') for r in lines[1:]}

    def test_all_windows_and_summary(self):
        p = self.run_batch()
        self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
        self.assertEqual(self.runs(), ['lm-a', 'dma-b', 'cm-c'])
        head, rows = self.rows()
        self.assertEqual(list(rows), ['lm-a-100+200', 'dma-b-300+400', 'cm-c-5+6'])
        self.assertEqual(head, '# 3 of 3 windows finished: 2 match, 1 FAIL')
        self.assertEqual(rows['lm-a-100+200'][1:5], ['match', '0', '0123abcd', 'ee' * 32])
        self.assertRegex((self.out / 'dma-b-300+400.log').read_text().splitlines()[-1], r'^EXIT 1 wall \d+s$')

    def test_fail_before_pass_line_is_fail(self):
        self.run_batch()
        row = self.rows()[1]['dma-b-300+400']
        self.assertEqual(row[1:3], ['FAIL', '1'])
        self.assertIn('REU DMA out of bounds', row[5])

    def test_seam_and_chain_fail(self):
        self.list('seam-a 100 200 --chain 3\n')
        self.assertEqual(self.run_batch().returncode, 0)
        row = self.rows()[1]['seam-a-100+200']
        self.assertEqual(row[1], 'FAIL')
        self.assertIn('seam 2', row[5])

    def test_crash_after_pass_is_rerun(self):
        self.list('crash-a 1 2\nlm-b 3 4\n')
        p = self.run_batch()
        self.assertEqual(p.returncode, 3, p.stdout)       # a crash is not finished
        head, rows = self.rows()
        self.assertEqual(rows['crash-a-1+2'][1], 'crash')
        self.assertIn('INCOMPLETE', head)
        self.run_batch()
        self.assertEqual(self.runs(), ['crash-a', 'lm-b', 'crash-a'])

    def test_masked_is_not_a_plain_match(self):
        self.list('mask-a 1 2\n')
        self.run_batch()
        head, rows = self.rows()
        self.assertEqual(rows['mask-a-1+2'][1], 'match-masked')
        self.assertIn('masked_dead_superbank', rows['mask-a-1+2'][5])
        self.assertIn('1 match-masked', head)

    def test_too_big_window_is_fail(self):
        self.list('big-a 1 2\n')
        self.run_batch()
        self.assertEqual(self.rows()[1]['big-a-1+2'][1], 'FAIL')

    def test_resume_skips_finished_and_reruns_unfinished(self):
        self.run_batch()
        (self.out / 'cm-c-5+6.log').write_text('$ x\ncut off mid-run\n')   # no EXIT line
        (self.d / 'runs.txt').write_text('')
        p = self.run_batch()
        self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
        self.assertEqual(self.runs(), ['cm-c'])

    def test_changed_tree_or_command_reruns(self):
        self.run_batch()
        (self.d / 'runs.txt').write_text('')
        self.run_batch(head='c2')
        self.assertEqual(self.runs(), ['lm-a', 'dma-b', 'cm-c'])
        (self.d / 'runs.txt').write_text('')
        self.run_batch('--', '--replay-every', '50', head='c2')
        self.assertEqual(self.runs(), ['lm-a', 'dma-b', 'cm-c'])
        (self.d / 'runs.txt').write_text('')
        self.run_batch('--reuse-any', head='c3')
        self.assertEqual(self.runs(), [])

    def test_budget_stops_with_exit_3_and_lists_not_run(self):
        p = self.run_batch('--budget-min', '0')
        self.assertEqual(p.returncode, 3, p.stdout + p.stderr)
        self.assertEqual(self.runs(), ['lm-a'])
        self.assertIn('rerun the same command to resume', p.stdout)
        head, rows = self.rows()
        self.assertEqual(head, '# 1 of 3 windows finished INCOMPLETE: 1 match, 2 not run')
        self.assertEqual(rows['cm-c-5+6'][1], 'not run')
        self.assertEqual(self.run_batch().returncode, 0)
        self.assertEqual(self.runs(), ['lm-a', 'dma-b', 'cm-c'])

    def test_mixed_store_images_flagged(self):
        self.list('lm-a 1 2\nother-b 3 4\n')
        self.run_batch()
        self.assertIn('MIXED: 1 commits, 2 store images', self.rows()[0])

    def test_replay_always_added(self):
        self.run_batch('--', '--replay-every', '100')
        first = (self.out / 'lm-a-100+200.log').read_text().splitlines()[0]
        self.assertTrue(first.endswith('--replay --replay-every 100'), first)
        self.run_batch('--', '--replay', '--replay-every', '100', head='c9')
        first = (self.out / 'lm-a-100+200.log').read_text().splitlines()[0]
        self.assertEqual(first.count('--replay '), 1, first)

    def test_lock_held(self):
        self.out.mkdir()
        (self.out / '.lock').write_text(f'{os.getppid()} {os.uname().nodename}')
        p = self.run_batch()
        self.assertEqual(p.returncode, 1)
        self.assertIn('held by', p.stderr)
        self.assertEqual(self.runs(), [])
        (self.out / '.lock').write_text(f'999999999 {os.uname().nodename}')   # dead pid: taken over
        self.assertEqual(self.run_batch().returncode, 0)
        self.assertFalse((self.out / '.lock').exists())

    def test_bad_list(self):
        self.list('lm-a 100\n')
        p = self.run_batch()
        self.assertEqual(p.returncode, 1)
        self.assertIn('want SCENARIO SNAP_CS CS', p.stderr)


if __name__ == '__main__':
    unittest.main()
