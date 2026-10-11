#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Cross-language exact roundtrip for all uint16_t words and real AGC fixtures.

Run from any directory. Requires the host C compiler; also runs its ASan/UBSan
build. Generated verification artifacts remain in build/frozen-verification.
"""
import os
from pathlib import Path
import random
import re
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / "tools"))
from frozen_words import c_header, decode_row, encode_rows  # noqa: E402


def main():
    target = root / "build/frozen-verification"
    target.mkdir(parents=True, exist_ok=True)
    source = (root / "tests/divide_vectors.h").read_text()
    original = [list(map(int, row.split(',')))
                for row in re.findall(r'\{([\d,]+)\}, /\*', source)]
    rng = random.Random(2867)
    varied = [[rng.randrange(65536) for _ in range(99)] for _ in range(100)]
    runs = [[value] * 768 for value in (0, 32767, 32768, 65535)]
    fixtures = [('packed_divide', original), ('varied', varied), ('runs', runs)]
    for block in range(4):
        values = list(range(block * 16384, (block + 1) * 16384))
        fixtures.append((f'domain{block}', [values[i:i+256] for i in range(0, len(values), 256)]))
    headers = []
    body = []
    for prefix, rows in fixtures:
        packed = encode_rows(rows)
        assert [decode_row(packed, i) for i in range(len(rows))] == rows
        # Determinism matters because generated headers are committed evidence.
        assert encode_rows(rows) == packed
        header = c_header(packed, prefix)
        if prefix == 'packed_divide':
            expected = 'divide_reference_words'
            print(f'156 complete 93-word states: {len(rows)*len(rows[0])*2} -> '
                  f'{packed.storage_bytes} bytes including dictionary and row offsets')
        else:
            expected = prefix + '_expected'
            header += f'static const uint16_t {expected}[{len(rows)}][{len(rows[0])}]={{\n'
            header += ''.join('{' + ','.join(map(str, row)) + '},\n' for row in rows)
            header += '};\n'
        (target / (prefix + '.h')).write_text(header)
        if prefix != 'packed_divide':
            headers.append(f'#define {prefix.upper()}_FROZEN_IMPLEMENTATION\n')
        headers.append(f'#include "{prefix}.h"\n')
        body.append(f'''    for(row=0;row<{len(rows)}u;++row) {{
        frozen_words r; uint16_t word;
        assert({prefix}_reference_begin(&r,row));
        for(i=0;i<{len(rows[0])}u;++i) {{
            assert(frozen_words_next(&r,&word));
            assert(word=={expected}[row][i]);
        }}
        assert(frozen_words_finished(&r));
        assert(!frozen_words_next(&r,&word));
    }}
''')
    program = '''/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include "frozen_words.h"
#include "divide_vectors.h"
''' + ''.join(headers) + '''
int shared_consumer(void);
static void reject(const uint8_t *bytes, uint16_t length, uint8_t dictionary_count) {
    static const uint16_t dictionary[1]={65535};
    frozen_words r; uint16_t word=31415;
    frozen_words_init(&r,bytes,length,dictionary,dictionary_count);
    while(frozen_words_next(&r,&word)) { }
    assert(r.failed);
    assert(!frozen_words_finished(&r));
    assert(!frozen_words_next(&r,&word));
}
int main(void) {
    unsigned row,i;
    assert(shared_consumer());
''' + ''.join(body) + '''    {
        static const uint8_t invalid[]={127};
        static const uint8_t dictionary_oob[]={1};
        static const uint8_t truncated_15[]={128};
        static const uint8_t truncated_16[]={125,255};
        static const uint8_t no_previous[]={126,1};
        static const uint8_t zero_repeat[]={0,126,0};
        static const uint8_t truncated_repeat[]={0,126};
        static const uint8_t trailing_run[]={0,126,2};
        frozen_words r; uint16_t word;
        reject(invalid,sizeof invalid,1);
        reject(dictionary_oob,sizeof dictionary_oob,1);
        reject(truncated_15,sizeof truncated_15,1);
        reject(truncated_16,sizeof truncated_16,1);
        reject(no_previous,sizeof no_previous,1);
        reject(zero_repeat,sizeof zero_repeat,1);
        reject(truncated_repeat,sizeof truncated_repeat,1);
        reject(invalid,sizeof invalid,126);
        frozen_words_init(&r,trailing_run,sizeof trailing_run,divide_reference_words[0],1);
        assert(frozen_words_next(&r,&word));
        assert(frozen_words_next(&r,&word));
        assert(!frozen_words_finished(&r));
        assert(frozen_words_next(&r,&word));
        assert(frozen_words_finished(&r));
    }
    puts("PASS: every uint16_t value; exact real/random fixtures; 768-word runs; malformed/truncated bounds");
    return 0;
}
'''
    (target / 'verify.c').write_text(program)
    # Separate owner and consumer TUs exercise the shared extern storage contract
    # used when the C64 check suite and timing diagnostic both take addresses.
    (target / 'shared_fixture.c').write_text(
        '#define PACKED_DIVIDE_FROZEN_IMPLEMENTATION\n#include "packed_divide.h"\n')
    (target / 'shared_consumer.c').write_text('''#include "packed_divide.h"
int shared_consumer(void) {
    frozen_words r; uint16_t word;
    if(!packed_divide_reference_begin(&r,0)) return 0;
    return frozen_words_next(&r,&word) && word==12664;
}
''')
    compiler = os.environ.get('CC', 'cc')
    common = [compiler, '-std=c99', '-g', '-Wall', '-Wextra', '-Werror',
              '-Wconversion', '-pedantic', '-I' + str(root / 'tests'),
              '-I' + str(target), str(root / 'tests/frozen_words.c'),
              str(target / 'shared_fixture.c'), str(target / 'shared_consumer.c'),
              str(target / 'verify.c')]
    for suffix, options in [('host', ['-O2']),
                            ('sanitize', ['-O1', '-fsanitize=address,undefined'])]:
        executable = target / ('verify-' + suffix)
        subprocess.run(common + options + ['-o', str(executable)], check=True)
        environment = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
        subprocess.run([str(executable)], check=True, env=environment)


if __name__ == '__main__':
    main()
