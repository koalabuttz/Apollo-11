#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/replay_batch.py: a window's status comes from its exit code and
every log line (a FAIL before the pass line, seam and chain FAILs, a crash
after the pass line, masks, rc 0 with no pass line); logs from another tree
or command are stale, in the run and in the summary; crashes retry up to 3
times; budget, timeout, SIGTERM and the lock. A stand-in runner prints
flight_scenario_vice.py's line shapes in place of VICE."""
import fcntl
import os
import pathlib
import signal
import subprocess
import sys
import tempfile
import time
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
TOOL = ROOT / 'tools/replay_batch.py'
FAKE = r'''
import os, pathlib, sys, time
a = sys.argv[1:]
s = a[a.index('--scenario') + 1]
snap = a[a.index('--snap-cs') + 1]
c = pathlib.Path(sys.argv[0]).with_name('runs.txt')
c.write_text(c.read_text() + s + '\n' if c.exists() else s + '\n')
pathlib.Path(sys.argv[0]).with_name('pid-' + s).write_text(str(os.getpid()))
print(f'{s}: commit 0123abcd; store lm.reu sha256 ' + ('ff' if s.startswith('other') else 'ee') * 32)
ok = (f"{s}: the C64 matches the host at 17600 cs: it ends the window (17800 cs, all 12 input rows)"
      " in the host's state")
if s.startswith('dma'):        # fsv:745, printed before the pass line
    print(f'FAIL: {s}: 3 DMAs outside the lm REU layout'); print(ok); sys.exit(1)
if s.startswith('seam'):       # fsv:645-646 and :875 do not start with FAIL
    print(ok); print(f'{s}: seam 1: FAIL: the C64 end of window 0 is not the start of window 1: 2 differences, first TIME3')
    print(f'{s}: chain of 3 windows from {snap} cs: FAIL at window 1'); sys.exit(1)
if s.startswith('chainok'):
    print(ok); print(f'{s}: seam 1: the C64 end of window 0 is the start of window 1')
    if not s.endswith('short'):
        print(f'{s}: chain of 2 windows from {snap} cs: every window matches the host and every seam holds')
    sys.exit(0)
if s.startswith('crash'):      # dies after the pass line
    print(ok); print('Traceback (most recent call last):'); print('OSError: [Errno 28] No space left on device'); sys.exit(1)
if s.startswith('mask'):
    print(f'{s}: masked_dead_superbank: 1 range'); print(ok); sys.exit(0)
if s.startswith('unpaired'):
    print(f'{s}: states with no partner, not compared: c64 17650 cs'); print(ok); sys.exit(0)
if s.startswith('big'):        # fsv:630
    sys.exit(f'FAIL: {s}: the host ran a window 20000 cs long from {snap} cs; one window is at most 16383 cs')
if s.startswith('nopoint'):     # fsv:781-782 with nothing paired, then :818
    print(f"{s}: the C64 matches the host at no point: it ends the window (17800 cs, all 12 input rows)"
          " in the host's state"); sys.exit(0)
if s.startswith('nopass'):
    sys.exit(0)
if s.startswith('sleep'):
    time.sleep(60)
if s.startswith('flip') and pathlib.Path(sys.argv[0]).with_name('flip-fails').exists():
    print(f'FAIL: {s}: 3 DMAs outside the lm REU layout'); sys.exit(1)
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
        self.assertIn('DMAs outside the lm REU layout', row[5])

    def test_seam_and_chain_fail(self):
        self.list('seam-a 100 200 --chain 3\n')
        self.assertEqual(self.run_batch().returncode, 0)
        row = self.rows()[1]['seam-a-100+200']
        self.assertEqual(row[1], 'FAIL')
        self.assertIn('seam 1: FAIL', row[5])

    def test_crash_after_pass_is_rerun_up_to_3_tries(self):
        self.list('crash-a 1 2\nlm-b 3 4\n')
        p = self.run_batch()
        self.assertEqual(p.returncode, 3, p.stdout)       # a crash is not finished
        head, rows = self.rows()
        self.assertEqual(rows['crash-a-1+2'][1:3], ['crash', '1'])
        self.assertIn('INCOMPLETE', head)
        self.assertEqual(self.run_batch().returncode, 3)
        self.assertEqual(self.run_batch().returncode, 0)  # third try: counts as finished
        self.assertEqual(self.run_batch().returncode, 0)
        self.assertEqual(self.runs(), ['crash-a', 'lm-b', 'crash-a', 'crash-a'])
        self.assertEqual(self.rows()[0], '# 2 of 2 windows finished: 1 match, 1 crash')

    def test_rc_0_without_pass_line_is_no_verdict(self):
        self.list('nopass-a 100 200\n')
        self.assertEqual(self.run_batch().returncode, 3)
        head, rows = self.rows()
        self.assertEqual(rows['nopass-a-100+200'][1], 'no verdict')
        self.assertIn('INCOMPLETE', head)

    def test_no_compared_point_is_no_verdict(self):
        self.list('nopoint-a 1 2\n')
        self.assertEqual(self.run_batch().returncode, 3)
        self.assertEqual(self.rows()[1]['nopoint-a-1+2'][1], 'no verdict')

    def test_sigkill_of_batch_keeps_out_locked_while_runner_runs(self):
        self.list('sleep-a 1 2\n')
        p = subprocess.Popen([sys.executable, str(TOOL), '--windows', str(self.d / 'w.txt'), '--out', str(self.out),
                              '--runner', f'{sys.executable} {self.d / "fake.py"}', '--head', 'c1'],
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        pidf = self.d / 'pid-sleep-a'
        for _ in range(100):
            if pidf.exists() and pidf.read_text():
                break
            time.sleep(0.05)
        p.kill()
        p.wait(10)
        pid = int(pidf.read_text())
        try:
            q = self.run_batch()
            self.assertEqual(q.returncode, 1)
            self.assertIn('held by', q.stderr)
        finally:
            os.kill(pid, signal.SIGKILL)

    def test_chain_needs_the_chain_line(self):
        self.list('chainok-a 1 2 --chain 2\nchainok-short 3 4 --chain 2\n')
        self.run_batch()
        rows = self.rows()[1]
        self.assertEqual(rows['chainok-a-1+2'][1], 'match')
        self.assertIn('every seam holds', rows['chainok-a-1+2'][5])
        self.assertEqual(rows['chainok-short-3+4'][1], 'no verdict')

    def test_masked_is_not_a_plain_match(self):
        self.list('mask-a 1 2\n')
        self.run_batch()
        head, rows = self.rows()
        self.assertEqual(rows['mask-a-1+2'][1], 'match-masked')
        self.assertIn('masked_dead_superbank', rows['mask-a-1+2'][5])
        self.assertIn('1 match-masked', head)

    def test_unpaired_states_are_masked(self):
        self.list('unpaired-a 1 2\n')
        self.run_batch()
        self.assertEqual(self.rows()[1]['unpaired-a-1+2'][1], 'match-masked')

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

    def test_stale_logs_are_not_finished_in_the_summary(self):
        # Same commit and store, new uncommitted change: flip-b now FAILs.
        self.list('lm-a 1 2\nflip-b 3 4\n')
        self.assertEqual(self.run_batch(head='c1+aaaa').returncode, 0)
        (self.d / 'flip-fails').write_text('')
        p = self.run_batch('--budget-min', '0', head='c1+bbbb')
        self.assertEqual(p.returncode, 3)
        head, rows = self.rows()
        self.assertEqual(head, '# 1 of 2 windows finished INCOMPLETE: 1 match, 1 stale')
        self.assertEqual(rows['flip-b-3+4'][1], 'stale')
        self.assertIn('c1+aaaa', rows['flip-b-3+4'][5])
        self.assertEqual(self.run_batch(head='c1+bbbb').returncode, 0)
        self.assertEqual(self.rows()[1]['flip-b-3+4'][1], 'FAIL')

    def test_window_timeout(self):
        self.list('sleep-a 1 2\n')
        p = self.run_batch('--window-timeout-min', '0.02')
        self.assertEqual(p.returncode, 3)
        self.assertEqual(self.rows()[1]['sleep-a-1+2'][1:3], ['timeout', 'timeout'])

    def test_sigterm_stops_the_runner_too(self):
        self.list('sleep-a 1 2\n')
        p = subprocess.Popen([sys.executable, str(TOOL), '--windows', str(self.d / 'w.txt'), '--out', str(self.out),
                              '--runner', f'{sys.executable} {self.d / "fake.py"}', '--head', 'c1'],
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        pidf = self.d / 'pid-sleep-a'
        for _ in range(100):
            if pidf.exists() and pidf.read_text():
                break
            time.sleep(0.05)
        p.send_signal(signal.SIGTERM)
        self.assertEqual(p.wait(10), 128 + signal.SIGTERM)
        pid = int(pidf.read_text())
        time.sleep(0.2)
        with self.assertRaises(ProcessLookupError):
            os.kill(pid, 0)
        self.assertFalse((self.out / 'sleep-a-1+2.log').exists())

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
        fd = os.open(self.out / '.lock', os.O_RDWR | os.O_CREAT)
        os.write(fd, b'4242 elsewhere')
        fcntl.flock(fd, fcntl.LOCK_EX)
        p = self.run_batch()
        self.assertEqual(p.returncode, 1)
        self.assertIn('held by 4242 elsewhere', p.stderr)
        self.assertEqual(self.runs(), [])
        os.close(fd)                                    # holder gone: the lock is free
        self.assertEqual(self.run_batch().returncode, 0)

    def test_bad_list(self):
        self.list('lm-a 100\n')
        p = self.run_batch()
        self.assertEqual(p.returncode, 1)
        self.assertIn('want SCENARIO SNAP_CS CS', p.stderr)


if __name__ == '__main__':
    unittest.main()
