# SPDX-License-Identifier: MIT
"""Which build a test's script works in.

Ordinarily readable/build. With LINE_COVERAGE set in the environment (make
line-coverage, tests/line_coverage.py) the Makefile builds every target
again under build/linecov with clang's source coverage, and a script that
links readable objects itself must link them with clang's profile runtime,
or the program's counts are never written.
"""
import os
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
COVERAGE = bool(os.environ.get('LINE_COVERAGE'))
BUILD = READABLE / ('build/linecov' if COVERAGE else 'build')
LINK = ['-fprofile-instr-generate'] if COVERAGE else []


def linker(cc):
    """The compiler to link readable objects with: clang when they carry its counters."""
    return 'clang' if COVERAGE else cc
