#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compile the inline operands of Luminary099 PHASE_TABLE_MAINTENANCE callers.

The same decoding as tools/cm_phase_sites_generate.py (the files share their
instruction sequence apart from Luminary's PHSCHNGA entry, which takes the
phase word in A and so has no inline operand). Each word comes from the
unmodified source listing and is checked against the rope.

  python3 tools/lm_phase_sites_generate.py           # write src/lm_phase_sites.inc
  python3 tools/lm_phase_sites_generate.py --check   # fail if stale
"""
import argparse
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from cm_phase_sites_generate import build, render  # noqa: E402

MISSION = 'Luminary099'
OUTPUT = 'src/lm_phase_sites.inc'
CALLEES = ('NEWMODEX', 'CHECKMM', 'PHASCHNG', '2PHSCHNG', 'PHSCHNGA')


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--json', help='also write the operands and sites as JSON (evidence)')
    args = parser.parse_args()
    operands, sites, constants, rope_sha = build(MISSION, CALLEES)
    text = render(operands, sites, constants, rope_sha, MISSION, CALLEES, 'LM_PHASE', 'lm_phase',
                  'tools/lm_phase_sites_generate.py')
    if args.json:
        pathlib.Path(args.json).write_text(json.dumps(
            {'mission': MISSION, 'rope_sha256': rope_sha, 'operands': operands, 'sites': sites,
             'constants': {n: {'physical': a, 'words': w} for n, w, a in constants}}, indent=1) + '\n')
    path = ROOT / OUTPUT
    if args.check:
        if not path.exists() or path.read_text() != text:
            sys.exit(f'{OUTPUT} is stale; run tools/lm_phase_sites_generate.py')
        print(f'PASS: {OUTPUT} matches {len(sites)} {MISSION} sites, {len(operands)} operand words')
        return
    path.write_text(text)
    print(f'wrote {OUTPUT}: {len(sites)} sites, {len(operands)} operand words')


if __name__ == '__main__':
    main()
