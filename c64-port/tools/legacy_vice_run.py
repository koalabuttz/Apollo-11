#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Run one legacy tools/*_vice_test.py script with a longer wall-clock guard.

The legacy scripts bound each VICE run twice: by emulated cycles
(-limitcycles), which is the test's real limit, and by a subprocess timeout
picked on a fast workstation. On CI, four warp-mode VICE processes share a
runner, so a run that stays inside its cycle limit can still hit the
wall-clock guard. This wrapper scales every subprocess timeout the script
sets by LEGACY_VICE_TIMEOUT_SCALE (default 3) and then runs it unchanged.

  python3 tools/legacy_vice_run.py tools/<name>_vice_test.py [args...]
"""
import os
import runpy
import subprocess
import sys

SCALE = float(os.environ.get('LEGACY_VICE_TIMEOUT_SCALE', '3'))


def scaled(function):
    def call(*args, **kwargs):
        if kwargs.get('timeout'):
            kwargs['timeout'] *= SCALE
        return function(*args, **kwargs)
    return call


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    for name in ('run', 'call', 'check_call', 'check_output'):
        setattr(subprocess, name, scaled(getattr(subprocess, name)))
    script = sys.argv[1]
    sys.argv = sys.argv[1:]
    sys.path.insert(0, os.path.dirname(os.path.abspath(script)))
    runpy.run_path(script, run_name='__main__')


if __name__ == '__main__':
    main()
