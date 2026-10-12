#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Write a 2 MB REU image holding both missions' ropes (include/rope_image.h).

The Luminary099 rope goes at 0x1c0000 and the Comanche055 rope at 0x1e0000,
two bytes per physical word (bank * 1024 + offset), little-endian; the rest
is zero (the overlay ABI 2 module store sits below 0x1c0000).
  python3 tools/rope_image_pack.py build/self-check.reu
"""
import pathlib
import sys

R = pathlib.Path(__file__).resolve().parents[1]
BASES = {'Luminary099': 0x1c0000, 'Comanche055': 0x1e0000}
WORDS = 36 * 1024


def physical_words(path):
    """The rope's words by physical address from the yaYUL binary (banks 2, 3, 0, 1, 4, ...)."""
    data = pathlib.Path(path).read_bytes()
    assert len(data) == 2 * WORDS, path
    out = [0] * WORDS
    for i in range(WORDS):
        bank = i // 1024
        bank = bank + 2 if bank < 2 else bank - 2 if bank < 4 else bank
        out[bank * 1024 + i % 1024] = (data[2 * i] << 8 | data[2 * i + 1]) >> 1
    return out


def main():
    image = bytearray(2 * 1024 * 1024)
    for mission, base in BASES.items():
        for k, w in enumerate(physical_words(R / f'build/fixed-data/{mission}/MAIN.agc.bin')):
            image[base + 2 * k:base + 2 * k + 2] = w.to_bytes(2, 'little')
    pathlib.Path(sys.argv[1]).write_bytes(image)


if __name__ == '__main__':
    main()
