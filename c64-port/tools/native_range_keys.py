#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Check that every native_ranges return point has a C64 key.

A compiled interpretive spec's 'native_ranges' (tools/interp_specs.py) are
basic routines the host runs inside the compiled routine, so the host run
never reaches them as a basic point. On the C64 the interpreter module runs
the interpretive code instead, and its RTB lands on that basic address: with
no key for it in that mission's store, the C64 stops there and nothing on the
host shows it. This reads the built stores (build/flight-store/<m>-resolver.json,
per mission: an address keyed only by the other mission's tables is still a
hole) and fails on any native_ranges label with no key that is not listed
below. Run after `make flight-store`.
"""
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / 'tools'))
import interp_compile  # noqa: E402
import interp_specs  # noqa: E402
from resident_flight_build import canonical_record  # noqa: E402

# Labels nothing RTBs to: reached only by TC from inside the range, or a
# constant. (spec, label): reason.
NOT_RTB_TARGETS = {
    ('lm-integration-init', 'SETBANK'): 'TC from inside MOVE*',
    ('lm-integration-init', 'FORTYONE'): 'constant (CAF FORTYONE)',
}
# Known holes with a fix on the way. (spec, mission, label): owner. Remove an
# entry when its key lands; the check then fails if the hole comes back.
KNOWN_HOLES = {
}


def keyed(mission):
    path = ROOT / 'build' / 'flight-store' / f'{mission}-resolver.json'
    if not path.exists():
        sys.exit(f'{path.relative_to(ROOT)} missing: run make flight-store first')
    return {(r['bank'], r['loc']) for r in json.loads(path.read_text())}


def main():
    keys = {m: keyed(m) for m in ('lm', 'cm')}
    listings = {}
    new, fixed, known = [], [], []
    for name, spec in interp_specs.SPECS.items():
        for label in spec.get('native_ranges', {}):
            if (name, label) in NOT_RTB_TARGETS:
                continue
            for m in spec['missions']:
                if m not in listings:
                    listings[m] = interp_compile.IL.load(m)
                phys = listings[m].labels.get(label)
                if phys is None:
                    continue
                rec = canonical_record(phys)
                hole = (rec[0], rec[1]) not in keys[m]
                entry = (name, m, label)
                where = f'{name} {m} {label} ({phys // 1024:02o},{0o2000 + phys % 1024:04o})'
                if hole and entry in KNOWN_HOLES:
                    known.append(f'{where}: no key yet ({KNOWN_HOLES[entry]})')
                elif hole:
                    new.append(where)
                elif entry in KNOWN_HOLES:
                    fixed.append(where)
    for line in known:
        print(f'native_range_keys: known hole {line}')
    for line in fixed:
        print(f'native_range_keys: {line} is keyed now: drop it from KNOWN_HOLES')
    for line in new:
        print(f'native_range_keys: FAIL {line}: an RTB lands here on the C64 and no key covers it')
    if new or fixed:
        sys.exit(1)
    print(f'native_range_keys: OK ({len(known)} known holes)')


if __name__ == '__main__':
    main()
