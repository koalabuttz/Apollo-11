# SPDX-License-Identifier: MIT
"""Lossless uint16_t fixture packing; paired with tests/frozen_words.c.

Only expected-state storage changes: every word still decodes exactly and is
compared individually. Rows reset run state and have offsets for random access.
Dictionary/literal tokens cost one/two bytes for ordinary 15-bit AGC words.
Full 16-bit words, including 65535, are also supported without normalization.
"""
from collections import Counter
from dataclasses import dataclass
import re


@dataclass(frozen=True)
class FrozenWords:
    dictionary: tuple[int, ...]
    offsets: tuple[int, ...]
    data: bytes
    row_width: int

    @property
    def storage_bytes(self):
        # Empty dictionaries still emit a one-word C array placeholder.
        return len(self.data) + 2 * (max(1, len(self.dictionary)) + len(self.offsets))


def encode_rows(rows):
    """Pack equally sized nonempty rows, deterministically, without lossy hashes."""
    rows = tuple(tuple(row) for row in rows)
    if not rows or not rows[0] or any(len(r) != len(rows[0]) for r in rows):
        raise ValueError("expected nonempty rows with a uniform word count")
    if any(type(v) is not int or not 0 <= v <= 65535 for r in rows for v in r):
        raise ValueError("fixture values must be uint16_t integers")
    counts = Counter(v for row in rows for v in row)
    # Save at least the two-byte dictionary entry. Numeric tie breaking makes
    # output independent of input traversal order and Python hash ordering.
    frequent = sorted(counts, key=lambda v: (-counts[v], v))
    dictionary = tuple(v for v in frequent if counts[v] > 2)[:125]
    codes = {v: i for i, v in enumerate(dictionary)}
    data = bytearray()
    offsets = [0]
    for row in rows:
        i = 0
        while i < len(row):
            word = row[i]
            if word in codes:
                token = bytes((codes[word],))
            elif word < 32768:
                token = bytes((128 | (word >> 8), word & 255))
            else:
                token = bytes((125, word & 255, word >> 8))
            run = 1
            while i + run < len(row) and row[i + run] == word and run < 256:
                run += 1
            data.extend(token)
            if (run - 1) * len(token) > 2:
                data.extend((126, run - 1))
            else:
                data.extend(token * (run - 1))
            i += run
        offsets.append(len(data))
    packed = FrozenWords(dictionary, tuple(offsets), bytes(data), len(rows[0]))
    if any(decode_row(packed, i) != list(row) for i, row in enumerate(rows)):
        raise ValueError("fixture compression failed exact roundtrip")
    return packed


def decode_row(packed, row):
    """Reference unpacker for generator checks; rejects malformed row streams."""
    if not 0 <= row < len(packed.offsets) - 1:
        raise IndexError(row)
    data = packed.data[packed.offsets[row]:packed.offsets[row + 1]]
    words = []
    i = 0
    while i < len(data):
        token = data[i]
        i += 1
        if token < 125:
            if token >= len(packed.dictionary):
                raise ValueError("dictionary index out of bounds")
            words.append(packed.dictionary[token])
        elif token == 127:
            raise ValueError("invalid token")
        elif token == 126:
            if not words or i == len(data) or not data[i]:
                raise ValueError("invalid or truncated repeat")
            words.extend([words[-1]] * data[i])
            i += 1
        else:
            needed = 2 if token == 125 else 1
            if i + needed > len(data):
                raise ValueError("truncated literal")
            if token == 125:
                words.append(data[i] | (data[i + 1] << 8))
            else:
                words.append(((token & 127) << 8) | data[i])
            i += needed
        if len(words) > packed.row_width:
            raise ValueError("row exceeds expected word count")
    if len(words) != packed.row_width:
        raise ValueError("row is missing words")
    return words


def c_header(packed, prefix):
    """Return arrays and PREFIX_reference_begin(reader, row) for a generated header.

    Caller adds the SPDX/provenance comment, include guard, path array, and final
    #endif. Define PREFIX_FROZEN_IMPLEMENTATION before including the generated
    header in exactly one translation unit. Other users see extern arrays, so
    taking fixture addresses in several translation units never duplicates the
    stream. Link tests/frozen_words.c into host and C64 diagnostic targets.
    """
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", prefix):
        raise ValueError("prefix must be a C identifier")
    if len(packed.data) > 65535 or len(packed.offsets) > 65536:
        raise ValueError("fixture exceeds 16-bit target addressing")
    # Generators must establish independent native/reference equality first;
    # this check additionally prevents compression from corrupting snapshots.
    for row in range(len(packed.offsets) - 1):
        decode_row(packed, row)
    upper = prefix.upper()
    text = '#include "frozen_words.h"\n'
    text += f'#define {upper}_FROZEN_ROWS {len(packed.offsets) - 1}u\n'
    text += f'#define {upper}_FROZEN_WORDS {packed.row_width}u\n'
    text += f'#define {upper}_FROZEN_STORAGE_BYTES {packed.storage_bytes}u\n'
    arrays = (("uint16_t", "dictionary", packed.dictionary or (0,)),
              ("uint16_t", "offsets", packed.offsets),
              ("uint8_t", "stream", packed.data))
    text += f'#ifdef {upper}_FROZEN_IMPLEMENTATION\n'
    for ctype, name, values in arrays:
        text += f'const {ctype} {prefix}_reference_{name}[{len(values)}]={{\n'
        for i in range(0, len(values), 16):
            text += '    ' + ','.join(map(str, values[i:i + 16])) + ',\n'
        text += '};\n'
    text += '#else\n'
    for ctype, name, values in arrays:
        text += f'extern const {ctype} {prefix}_reference_{name}[{len(values)}];\n'
    text += '#endif\n'
    text += (f'static inline uint8_t {prefix}_reference_begin(frozen_words *r, unsigned row) {{\n'
             f'    if(row>={upper}_FROZEN_ROWS) return 0;\n'
             f'    frozen_words_init(r,{prefix}_reference_stream+{prefix}_reference_offsets[row],\n'
             f'        (uint16_t)({prefix}_reference_offsets[row+1]-{prefix}_reference_offsets[row]),\n'
             f'        {prefix}_reference_dictionary,{len(packed.dictionary)});\n'
             f'    return 1;\n}}\n')
    return text
