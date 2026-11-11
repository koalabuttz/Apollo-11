# SPDX-License-Identifier: MIT
"""Which build a test's script works in.

Ordinarily readable/build. With LINE_COVERAGE set in the environment (make
line-coverage, tests/line_coverage.py) the Makefile builds every target
again under build/linecov with clang's source coverage, and a script that
links readable objects itself must link them with clang's profile runtime,
or the program's counts are never written, and through the measure's own
compiler (tests/coverage_cc.py), which keeps two builds of one source in
one program apart.
"""
import os
import sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
COVERAGE = bool(os.environ.get('LINE_COVERAGE'))
BUILD = READABLE / ('build/linecov' if COVERAGE else 'build')
LINK = ['-fprofile-instr-generate'] if COVERAGE else []


def linker(cc):
    """The compiler to link readable objects with: clang when they carry its counters."""
    return 'clang' if COVERAGE else cc


def link(cc):
    """The start of the command that links them: linker(cc), through
    tests/coverage_cc.py when they carry counters."""
    return [sys.executable, str(READABLE / 'tests/coverage_cc.py')] if COVERAGE else [cc]


def counts_object(program):
    """What that link adds to a program of its own accord: the object that
    names the program's count files. Nothing in the ordinary build."""
    return [f'{program}.counts.o'] if COVERAGE else []
