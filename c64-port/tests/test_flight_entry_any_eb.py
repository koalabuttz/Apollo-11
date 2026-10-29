#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Flight parts (docs/FLIGHT-PARTS.md): a key whose per-key entry runs
under any EB (basic_sane_any_eb: spec 'any_eb_entries', or a routine that
sets EB first) must get the same test in the part's shared flight entry.
The per-key entries are left out of flight builds, so without it the C64
flight store refuses the key under a non-E5 EB (P06 calling FINETIME +1
stopped CM V37E06E). Reads the generated sources: no C64 tools needed.
tools/flight_parts_check.py runs those keys under another EB."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]


def any_eb_keys(src):
    """Physical addresses of the keys whose per-key entry is basic_sane_any_eb."""
    keys = set()
    for m in re.finditer(r'static enum exec_status r\w+_abi\(.*?\n(.*?)\n}', src, re.S):
        if 'basic_sane_any_eb' in m.group(1):
            k = re.search(r'basic_run\(s,\([^)]*\)v,(\d+)u\)', m.group(1))
            if k:
                keys.add(int(k.group(1)))
    return keys


def entry_cases(src):
    """The flight entry's guard line for each bank."""
    m = re.search(r'static enum exec_status \w+_flight_entry\(.*?\n}', src, re.S)
    if not m:
        return None
    return {int(b, 8): line for b, line in re.findall(r'\n case 0(\d+)u:(.*)', m.group(0))}


def main():
    files, failures = 0, []
    for path in sorted((ROOT / 'src').glob('*.c')):
        src = path.read_text()
        cases = entry_cases(src)
        if cases is None:
            continue
        files += 1
        for at in sorted(any_eb_keys(src)):
            line = cases.get(at // 1024)
            if line is None:
                continue  # (not a key of the flight parts)
            free = re.match(r'if\(!\(([^?]*)\?basic_sane_any_eb\(', line)
            if not free or 'at==%uu' % at not in free.group(1).split('||'):
                failures.append('%s: flight entry tests key %u with basic_sane, not basic_sane_any_eb'
                                % (path.name, at))
    if not files:
        sys.exit('FAIL: no flight entry found in src/')
    if failures:
        print('\n'.join(failures))
        sys.exit('FAIL: %d any-EB key(s) refused under a non-E5 EB in flight builds' % len(failures))
    print('PASS: any-EB keys keep basic_sane_any_eb in the flight entries of %d files' % files)


if __name__ == '__main__':
    main()
