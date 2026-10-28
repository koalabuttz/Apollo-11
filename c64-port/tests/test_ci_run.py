#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/ci_run.py: a stuck target is stopped and keeps the output it printed."""
import pathlib
import sys
import time
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / 'tools'))
import ci_run  # noqa: E402


class Make(unittest.TestCase):
    def test_stuck_command_is_stopped_with_its_output(self):
        start = time.monotonic()
        # The child sleep is in the same process group, so it goes too.
        proc = ci_run.make(['sh', '-c', 'echo before; sleep 30 & wait; echo after'], 1)
        self.assertEqual(proc.returncode, ci_run.TIMED_OUT)
        self.assertEqual(proc.stdout, 'before\n')
        self.assertLess(time.monotonic() - start, 15)

    def test_no_limit(self):
        proc = ci_run.make(['sh', '-c', 'echo ok; exit 3'], None)
        self.assertEqual((proc.returncode, proc.stdout), (3, 'ok\n'))


class Timeouts(unittest.TestCase):
    def test_only_measured_targets_get_a_limit(self):
        weights = {'a': 100.0, 'b': 1000.0, 'c': 4000.0}
        got = ci_run.target_timeouts(['a', 'b', 'c-1', 'new'], weights, 2)
        self.assertEqual(got, {'a': ci_run.TIMEOUT_FLOOR, 'b': 2000.0, 'c-1': 8000.0})


if __name__ == '__main__':
    unittest.main()
