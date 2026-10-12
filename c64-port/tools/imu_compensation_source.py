#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for imu-compensation.

Reads the fresh host assembly listings of both unmodified programs
(build/fixed-data/<mission>/assembly.lst), lists every word
IMU_COMPENSATION_PACKAGE emits, assigns each to the native function in
src/imu_compensation.c that compiles it (by source address range), and fails
if any word is unowned or an owner range covers no word. The fixed constants
the native code uses as literals (K_* in src/imu_compensation.c) are checked
against the assembled ropes. Writes build/imu-compensation-source-map.json.
"""
import json
import pathlib
import re
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import rope_word  # noqa: E402

LINE = re.compile(r'^\d{6},(\d{6}): (.*)$')
WORD = re.compile(r'^(\d\d),(\d{4})\s+(\d{5})(?: (\d{5}))?\s+(.*)$')
FILE = 'IMU_COMPENSATION_PACKAGE.agc'
# (first, last, native owner) in bank 06; every emitted word must fall in one.
OWNERS = {
    'Luminary099': [
        (0o3263, 0o3271, 'lm_1pipa (save_ebank, CCS GCOMPSW)'),
        (0o3272, 0o3327, 'pipa_loop (1/PIPA1; NOOP is the impossible negative CCS branch)'),
        (0o3330, 0o3342, 'lm_1pipa (IRIGCOMP)'),
        (0o3343, 0o3351, 'gyro_job (CCS GCOMPSW, CA PRIO21, TC NOVAC; 2CADR 1/GYRO is the lm-exec site operand)'),
        (0o3352, 0o3352, 'lm_novac_return (RELINT)'),
        (0o3353, 0o3355, 'irig1'),
        (0o3356, 0o3424, 'lm_irig/accel (IRIGX, IRIGY, IRIGZ)'),
        (0o3425, 0o3453, 'gcompsub'),
        (0o3454, 0o3473, 'driftsub'),
        (0o3474, 0o3506, 'drftsub2'),
        (0o3507, 0o3534, 'gyro (1/GYRO; LGCOMP is K_LGCOMP; CADR IMUPULSE is the lm-interbank site operand)'),
        (0o3535, 0o3536, 'lm_imupulse_return (CADR IMUSTALL is the lm-interbank site operand)'),
        (0o3537, 0o3537, 'lm_imustall_return'),
        (0o3540, 0o3554, 'gcomp1 (COMPCHK is K_NEG1, read by drftsub2 as MASK operand)'),
        (0o3555, 0o3574, 'lm_nbdonly (CADR PIPASR +3 is the lm-interbank site operand)'),
        (0o3575, 0o3601, 'lm_pipasr_return'),
        (0o3602, 0o3643, 'lm_nbd2 (NBD2, NBD3)'),
        (0o3644, 0o3662, 'fbiassub'),
        (0o3663, 0o3664, 'lm_lastbias (CADR PIPUSE1 is the lm-interbank site operand)'),
        (0o3665, 0o3700, 'lm_pipuse1_return'),
        (0o3701, 0o3714, 'lm_gcompzer'),
    ],
    'Comanche055': [
        (0o3262, 0o3271, 'cm_1pipa (save_ebank, CCS GCOMPSW, INHINT)'),
        (0o3272, 0o3327, 'pipa_loop (1/PIPA1; NOOP is the impossible negative CCS branch)'),
        (0o3330, 0o3332, 'cm_1pipa (RELINT, IRIGCOMP)'),
        (0o3333, 0o3376, 'cm_irig/accel (IRIGX, IRIGY, IRIGZ falling through)'),
        (0o3377, 0o3405, 'gyro_job (2CADR 1/GYRO: NOVAC has no native Comanche routine yet)'),
        (0o3406, 0o3406, 'cm_novac_return (RELINT)'),
        (0o3407, 0o3411, 'irig1'),
        (0o3412, 0o3440, 'gcompsub'),
        (0o3441, 0o3460, 'driftsub'),
        (0o3461, 0o3473, 'drftsub2'),
        (0o3474, 0o3521, 'gyro (1/GYRO; LGCOMP is K_LGCOMP; CADR IMUPULSE is the cm-interbank site operand)'),
        (0o3522, 0o3523, 'cm_imupulse_return'),
        (0o3524, 0o3524, 'cm_imustall_return'),
        (0o3525, 0o3541, 'gcomp1 (V06N30S, VN 0630, has no reader: it is the impossible negative CCS branch)'),
        (0o3542, 0o3557, 'cm_nbdonly'),
        (0o3560, 0o3616, 'cm_nbd2 (NBD2, NBD3)'),
        (0o3617, 0o3635, 'fbiassub'),
        (0o3636, 0o3637, 'cm_lastbias (CADR PIPUSE is the cm-interbank site operand)'),
        (0o3640, 0o3650, 'cm_pipuse_return'),
    ],
}
# Fixed constants used as native literals: symbol -> K_ name, per mission.
CONSTANTS = {
    'Luminary099': {'BIT4': 'K_BIT4', 'BIT6': 'K_BIT6', 'BIT8': 'K_BIT8', 'BIT9': 'K_BIT9', 'BIT10': 'K_BIT10',
                    'HALF': 'K_HALF', 'NEG1': 'K_NEG1', 'PRIO21': 'K_PRIO21', 'PRIO31': 'K_PRIO31',
                    'SURFFBIT': 'K_BIT8', 'COMPCHK': 'K_NEG1', 'LGCOMP': 'K_LGCOMP'},
    'Comanche055': {'BIT1': 'K_BIT1', 'BIT4': 'K_BIT4', 'BIT6': 'K_BIT6', 'BIT8': 'K_BIT8', 'BIT10': 'K_BIT10',
                    'BIT12': 'K_BIT12', 'ONE': 'K_BIT1', 'POSMAX': 'K_POSMAX', 'NEG1': 'K_NEG1',
                    'NEGONE': 'K_NEG1', 'PRIO21': 'K_PRIO21', 'PRIO31': 'K_PRIO31', 'LGCOMP': 'K_LGCOMP'},
}


def words(listing):
    """(bank, address, word, source text) of each word the file emits."""
    out, on = [], False
    for text in listing.splitlines():
        if 'Filename:' in text:
            on = FILE in text
        m = LINE.match(text)
        if not on or not m:
            continue
        body = m.group(2).split('\t#')[0].rstrip()
        w = WORD.match(body.strip())
        if not w or not w.group(3):
            continue
        bank, addr = int(w.group(1), 8), int(w.group(2), 8)
        out.append((bank, addr, int(w.group(3), 8), ' '.join(w.group(5).split())))
        if w.group(4):
            out.append((bank, addr + 1, int(w.group(4), 8), ' '.join(w.group(5).split()) + ' (second word)'))
    return out


def symbol(listing, name):
    found = re.findall(r'(?:^|\s)' + re.escape(name) + r'\s+(?:(\d\d),)?([0-7]{4})\b', listing)
    bank, addr = found[-1]
    addr = int(addr, 8)
    if bank:
        return int(bank, 8) * 1024 + addr - 0o2000
    return 2048 + addr - 0o4000


def main():
    src = (R / 'src/imu_compensation.c').read_text()
    literals = {k: int(v, 8) for k, v in re.findall(r'#define (K_\w+) 0?([0-7]+)u', src)}
    result = {'file': FILE, 'missions': {}}
    for mission, owners in OWNERS.items():
        listing = (R / f'build/fixed-data/{mission}/assembly.lst').read_text(errors='replace')
        rope = (R / f'build/fixed-data/{mission}/MAIN.agc.bin').read_bytes()
        emitted = words(listing)
        assert emitted and all(b == 6 for b, *_ in emitted), mission
        rows, used = [], set()
        for bank, addr, word, text in emitted:
            assert rope_word(rope, bank * 1024 + addr - 0o2000) == word, (mission, oct(addr), text)
            owner = [o for o in owners if o[0] <= addr <= o[1]]
            assert len(owner) == 1, (mission, oct(addr), text)
            used.add(owner[0])
            rows.append({'address': f'06,{addr:04o}', 'word': f'{word:05o}', 'source': text, 'native': owner[0][2]})
        assert used == set(owners), (mission, set(owners) - used)
        consts = {}
        for sym, k in CONSTANTS[mission].items():
            value = rope_word(rope, symbol(listing, sym))
            assert value == literals[k], (mission, sym, oct(value), k)
            consts[sym] = {'literal': k, 'value': f'{value:05o}'}
        result['missions'][mission] = {'words': len(rows), 'map': rows, 'constants': consts}
        print(f'{mission}: {len(rows)} words of {FILE} owned by {len(owners)} native ranges; '
              f'{len(consts)} literals equal the rope')
    (R / 'build/imu-compensation-source-map.json').write_text(json.dumps(result, indent=1) + '\n')


if __name__ == '__main__':
    main()
