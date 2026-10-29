#!/usr/bin/env python3
"""Check that every C64 diagnostic PRG ends below the $c000 mailbox.

tools/vice_run.py reads a PRG's result from $c000-$c004 and stops on the
first store there, so a PRG whose data or BSS reaches $c000 reports garbage
(mailbox 00ff..., "255 failures, 0 checks"). Shared-source changes can push
another lane's PRG over the line without that PRG's own files changing.

Checks each link map that uses the default C64 layout (a `__heap_start`
symbol and the soft stack at $D000): `__heap_start` must be at most $c000.
Maps from the overlay and resident linker scripts have their own ASSERTs and
are skipped.

  python3 tools/map_check.py                 # every build/**/*.map
  python3 tools/map_check.py build/r30-3.map
"""
import argparse
import glob
import re
import sys

LIMIT = 0xc000
HEAP = re.compile(r'^\s*([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+0\s+1\s+__heap_start = ', re.M)
STACK = re.compile(r'__stack = 0x[dD]000\b')


def check(path):
    """Return the map's __heap_start, or None for a map this check skips."""
    with open(path, errors='replace') as f:
        text = f.read()
    heap = HEAP.search(text)
    if not heap or not STACK.search(text):
        return None
    return int(heap.group(1), 16)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('maps', nargs='*')
    args = parser.parse_args()
    maps = args.maps or sorted(glob.glob('build/**/*.map', recursive=True))
    if not maps:
        # A CI shard whose share of the affected targets is empty builds
        # nothing: no map is not a changed map format.
        print('map-check: no link maps (nothing built)')
        return 0
    checked, over = 0, []
    for path in maps:
        heap = check(path)
        if heap is None:
            continue
        checked += 1
        if heap > LIMIT:
            over.append((path, heap))
    for path, heap in over:
        print(f'FAIL: {path}: __heap_start ${heap:04x} is past the ${LIMIT:04x} mailbox '
              f'by {heap - LIMIT} bytes')
    print(f'map-check: {checked} PRG maps checked, {len(over)} past ${LIMIT:04x}')
    if not checked:
        print('FAIL: no C64 link map matched (no PRGs built, or the map format changed)')
    return 1 if over or not checked else 0


if __name__ == '__main__':
    sys.exit(main())
