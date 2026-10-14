#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fail when two tools/mk/*.mk files assign the same make variable.

Every .mk is included into one make namespace (Makefile: include
$(sort $(wildcard tools/mk/*.mk))), so a later file's `X = ...` silently
replaces an earlier file's X. Each recipe expands X late and gets the later
value: lm-kalman.mk's KAL_SRC made test-kalcmanu-steering build the KALMAN
sources and fail all 840 frozen cases. `+=` appends are allowed.

  python3 tools/mk_vars_check.py
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
ASSIGN = re.compile(r'^(?:override\s+|export\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*(?::::?|\?|!)?=')


def main():
    owners = {}
    for path in sorted((ROOT / 'tools/mk').glob('*.mk')):
        for line in path.read_text().splitlines():
            match = ASSIGN.match(line)
            if match and not line.lstrip().startswith('#'):
                owners.setdefault(match.group(1), set()).add(path.name)
    clashes = {name: files for name, files in owners.items() if len(files) > 1}
    for name, files in sorted(clashes.items()):
        print(f'FAIL: {name} is assigned in {", ".join(sorted(files))}; give each file its own prefix')
    if not clashes:
        print(f'mk variables: {len(owners)} names, each assigned in one tools/mk file')
    return 1 if clashes else 0


if __name__ == '__main__':
    sys.exit(main())
