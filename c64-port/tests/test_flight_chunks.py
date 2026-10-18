#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/flight_store_build.py pack_chunks: chunks of consecutive routines,
each as large as fits, the same on every run, against a brute-force greedy
packing of made-up routine sizes (a chunk's size is a shared closure plus
its routines'). Routines over the slot alone are reported, and a link
error stops the split."""
from pathlib import Path
import random
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from flight_store_build import pack_chunks  # noqa: E402

SLOT, BASE = 16384, 3000


def greedy(sizes):
    kept, over, start = [], [], 0
    while start < len(sizes):
        end = start
        while end < len(sizes) and BASE + sum(sizes[start:end + 1]) <= SLOT:
            end += 1
        if end == start:
            over.append(start)
            start += 1
        else:
            kept.append((start, end))
            start = end
    return kept, over


def main():
    for seed in range(400):
        rnd = random.Random(seed)
        sizes = [rnd.randint(100, 9000) for _ in range(rnd.randint(1, 300))]
        for _ in range(seed % 3):
            sizes[rnd.randrange(len(sizes))] = 20000
        log = []

        def fits(start, ends):
            log.append((start, tuple(ends)))
            return [True if BASE + sum(sizes[start:e]) <= SLOT else BASE + sum(sizes[start:e]) - SLOT for e in ends]
        probes = 1 + seed % 8
        first = pack_chunks(len(sizes), fits, probes)
        trials = list(log)
        log.clear()
        assert pack_chunks(len(sizes), fits, probes) == first and log == trials, f'seed {seed}: not deterministic'
        kept, over = greedy(sizes)
        assert first[0] == kept, f'seed {seed}: chunks {first[0]} != {kept}'
        assert [s for s, _ in first[1]] == over, f'seed {seed}: over {first[1]} != {over}'
        assert all(o == BASE + sizes[s] - SLOT for s, o in first[1]), f'seed {seed}: overflow'
    assert pack_chunks(5, lambda start, ends: [None] * len(ends), 4) is None
    print('PASS: pack_chunks matches greedy packing on 400 routine lists, the same on every run')


if __name__ == '__main__':
    main()
