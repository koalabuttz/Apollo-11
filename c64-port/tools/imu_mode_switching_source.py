#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-to-native map for imu-mode-switching.

Reads the fresh host assembly listings of both unmodified programs
(build/fixed-data/<mission>/assembly.lst), lists every word
IMU_MODE_SWITCHING_ROUTINES emits, assigns each to the native function in
src/imu_mode_switching.c that compiles it (the owner whose start is the
nearest at or below the word, Luminary addresses; Comanche starts follow
from ims_comanche_address()), and fails if any word is unowned or an owner
covers no word. The fixed constants the native code uses as literals (K_* in
src/imu_mode_switching.c, GYROFRAC, the -GYROMIN MASK operand and KALEBCON)
are checked against the assembled ropes. Writes
build/imu-mode-switching-source-map.json.
"""
import json
import pathlib
import re
import sys

R = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(R / 'tools'))
from fixed_data_generate import rope_word  # noqa: E402

LINE = re.compile(r'^\d{6},(\d{6}): (.*)$')
WORD = re.compile(r'^(?:(\d\d),)?(\d{4})\s+(\d{5})(?: (\d{5}))?\s+(.*)$')
FILE = 'IMU_MODE_SWITCHING_ROUTINES.agc'
FF = 0o2  # fixed-fixed words are listed without a bank
# (Luminary bank, start, native owner); an owner runs to the next start.
OWNERS = [
    (FF, 0o5457, 'zeroicdu (ZEROICDU)'),
    (7, 0o2714, 'r_imuzero (IMUZERO, IMUZEROA)'),
    (7, 0o2723, 'r_c2723 (after ALARM 206)'),
    (7, 0o2724, 'r_imuzero (IMUZEROA: CAGETSTJ, ZEROICDU, WAITLIST)'),
    (7, 0o2747, 'r_c2747 (after WAITLIST)'),
    (7, 0o2755, 'modeexit (MODEEXIT)'),
    (7, 0o2757, 'r_imuzero2 (IMUZERO2)'),
    (7, 0o2766, 'r_imuzero3 (IMUZERO3)'),
    (7, 0o2777, 'r_c2777 (after SETISSW)'),
    (7, 0o3000, 'r_imucoars (IMUCOARS)'),
    (7, 0o3007, 'r_c3007 (after WAITLIST)'),
    (7, 0o3010, 'r_coars (COARS, COARS1)'),
    (7, 0o3040, 'r_coars2 (COARS2, COMPOS, NEXTCDU)'),
    (7, 0o3072, 'r_c3072 (CHKCORS, CORSCHK2, COARSERR)'),
    (7, 0o3115, 'r_c3115 (after ALARM 211)'),
    (7, 0o3116, 'K_COARSTOL (COARSTOL)'),
    (7, 0o3117, 'r_coars2 (COMNEG, COMZERO, SENDPULS)'),
    (7, 0o3140, 'r_ca_ece (CA+ECE)'),
    (7, 0o3144, 'setcoars (SETCOARS)'),
    (7, 0o3175, 'rndrefdr (RNDREFDR)'),
    (7, 0o3207, 'K_OCT40010 (OCT40010)'),
    (7, 0o3210, 'r_imufine (IMUFINE)'),
    (7, 0o3225, 'r_c3225 (WAITLIST IMUFINED)'),
    (7, 0o3231, 'r_c3231 (after WAITLIST)'),
    (7, 0o3232, 'imufined (IMUFINED)'),
    (7, 0o3234, 'r_ifailok (IFAILOK)'),
    (7, 0o3247, 'pfailok2 (PFAILOK2)'),
    (7, 0o3253, 'r_c3253 (after SETISSW)'),
    (7, 0o3254, 'r_pfailok (PFAILOK)'),
    (7, 0o3266, 'noattoff (NOATTOFF)'),
    (7, 0o3273, 'r_pipuse (PIPUSE)'),
    (7, 0o3277, 'pipuse1 (PIPUSE1)'),
    (7, 0o3305, 'pipfree2 (PIPFREE2; CADR is the interbank site operand)'),
    (7, 0o3307, 'r_c3307 (after IBNKCALL)'),
    (7, 0o3310, 'r_pipfree (PIPFREE)'),
    (7, 0o3321, 'r_c3321 (after ALARM 212)'),
    (7, 0o3323, 'r_imupulse (IMUPULSE)'),
    (7, 0o3334, 'gwake2 (GWAKE2)'),
    (7, 0o3337, 'r_c3337 (after WAITLIST)'),
    (7, 0o3346, 'gyroagre (GYROAGRE)'),
    (7, 0o3357, 'r_c3357 (after TPAGREE)'),
    (7, 0o3367, 'r_imupulse (GYROBUSY)'),
    (7, 0o3372, 'regsleep (REGSLEEP)'),
    (7, 0o3374, 'r_gwake (GWAKE)'),
    (7, 0o3404, 'lgwake (LGWAKE: CADR GWAKE)'),
    (7, 0o3405, 'r_strtgyro (STRTGYRO, STRTGYR2; the OCT words are the GSELECT operands)'),
    (7, 0o3430, 'r_noreset (NORESET)'),
    (7, 0o3431, 'r_strtgyro (GSELECT -2)'),
    (7, 0o3433, 'gselect (GSELECT, MIN+, MAJ+, GMERGE; -GYROMIN runs as MASK gyromin_word)'),
    (7, 0o3530, 'lastseg1 (LASTSEG; 2CADR STRTGYRO is WAITLIST\'s operand)'),
    (7, 0o3537, 'r_gyroexit (GYROEXIT)'),
    (7, 0o3543, 'gselect (LONGGYRO)'),
    (7, 0o3550, 'aug3 (AUG3)'),
    (7, 0o3556, 'r_c3556 (after WAITLIST)'),
    (7, 0o3557, 'r_8192aug (8192AUG, AUG2)'),
    (7, 0o3604, 'gselect (MIN-, MAJ-)'),
    (7, 0o3626, 'K_GDESELCT, K_GYROFRAC (GDESELCT, GYROFRAC)'),
    (7, 0o3631, 'r_endimu (ENDIMU, IMUGOOD)'),
    (7, 0o3637, 'imubad (IMUBAD)'),
    (7, 0o3641, 'cagetest (CAGETEST)'),
    (7, 0o3646, 'cagetstq (CAGETSTQ)'),
    (7, 0o3653, 'cagetstj (CAGETSTJ)'),
    (7, 0o3662, 'badend (BADEND)'),
    (7, 0o3665, 'goodend, endmode1 (GOODEND)'),
    (7, 0o3702, 'r_c3702 (after JOBWAKE)'),
    (7, 0o3706, 'endmode, endtail (ENDMODE)'),
    (7, 0o3712, 'r_aotstall (AOTSTALL)'),
    (7, 0o3714, 'r_radstall (RADSTALL)'),
    (7, 0o3716, 'r_imustall (IMUSTALL)'),
    (7, 0o3717, 'stall (STALL)'),
    (7, 0o3726, 'mg2 (MG2, MODEGOOD, MODESLP)'),
    (7, 0o3736, 'r_c3736 (after MAKECADR)'),
    (7, 0o3741, 'modabort (MODABORT)'),
    (7, 0o3744, 'K_BITS4_5, K_MCOMMAX, K_MCOMMAX_, K_600MS'),
    (7, 0o3750, 'r_gomanur (GOMANUR)'),
    (7, 0o3772, 'kalebcon (KALEBCON: ECADR BCDU)'),
    (4, 0o3253, 'r_r02both (DEC51 is its VARALARM operand)'),
    (4, 0o3254, 'r_r02both (R02BOTH)'),
    (4, 0o3267, 'r_c3267 (after VARALARM)'),
    (4, 0o3270, 'r_r02zero (R02ZERO)'),
    (4, 0o3272, 'r_c3272 (after UPFLAG)'),
    (4, 0o3273, 'K_OCT220 (OCT220)'),
    (0o37, 0o3655, 'r_p06 (P06)'),
    (0o37, 0o3657, 'r_prestand (PRESTAND)'),
    (0o37, 0o3664, 'r_c3664 (SCALPREP retry return)'),
    (0o37, 0o3665, 'r_c3665'),
    (0o37, 0o3672, 'r_c3672 (after BANKCALL)'),
    (0o37, 0o3674, 'r_c3674 (after DOWNFLAG)'),
    (0o37, 0o3676, 'r_c3676 (after DOWNFLAG)'),
    (0o37, 0o3706, 'r_c3706 (after PHASCHNG)'),
    (0o37, 0o3711, 'r_c3711 (GOPERF1 return)'),
    (0o37, 0o3712, 'r_c3712 (GOPERF1 return)'),
    (0o37, 0o3713, 'r_c3713 (GOPERF1 return)'),
    (0o37, 0o3714, 'scalprep (SCALPREP)'),
    (0o37, 0o3717, 'r_c3717 (after FINETIME)'),
    (0o37, 0o3734, 'r_postand (POSTAND)'),
    (0o37, 0o3744, 'r_c3744 (SCALPREP retry return)'),
    (0o37, 0o3745, 'r_c3745'),
    (0o37, 0o3752, 'r_c3752 (after SHORTMP)'),
    (0o37, 0o3755, 'r_c3755 (after TPAGREE, POSTCOM)'),
    (0o37, 0o3767, 'r_c3767 (after TPAGREE)'),
    (0o37, 0o3773, 'r_c3773 (after DOWNFLAG)'),
]
# Fixed constants used as native literals: symbol -> K_ name.
CONSTANTS = {'BIT1': 'K_BIT1', 'BIT4': 'K_BIT4', 'BIT5': 'K_BIT5', 'BIT6': 'K_BIT6', 'BIT8': 'K_BIT8',
             'BIT9': 'K_BIT9', 'BIT10': 'K_BIT10', 'BIT11': 'K_BIT11', 'BIT13': 'K_BIT13', 'BIT14': 'K_BIT14',
             'BIT15': 'K_BIT15', 'BITS3&4': 'K_BITS3_4', 'BITS4-5': 'K_BITS4_5', 'BITS4&6': 'K_BITS4_6',
             'SUPER011': 'K_SUPER011', 'OCT40010': 'K_OCT40010', '13,14,15': 'K_13_14_15', '600MS': 'K_600MS',
             '2SECS': 'K_2SECS', 'OCT62': 'K_OCT62', 'OCT220': 'K_OCT220', '-COMMAX': 'K_MCOMMAX',
             '-COMMAX-': 'K_MCOMMAX_', 'COARSTOL': 'K_COARSTOL', 'GDESELCT': 'K_GDESELCT',
             '-GYROMIN': 'K_MGYROMIN', 'LOW5': 'K_LOW5', 'LOW7': 'K_LOW7', 'LOW8': 'K_LOW8', 'SEVEN': 'K_SEVEN',
             'PRIO37': 'K_PRIO37', 'NEG0': 'K_NEG0', 'MINUS1': 'K_MINUS1', 'NEG3': 'K_NEG3'}
MISSIONS = {'Luminary099': (0, 'LM'), 'Comanche055': (1, 'CM')}


def cm7(a):
    return a - 0o115 if a <= 0o3673 else a - 0o117 if a <= 0o3741 else 0o3622 if a == 0o3742 \
        else 0o3623 if a == 0o3743 else a - 0o121


def place(cm, bank, a):
    """Comanche bank and address of a Luminary (bank, address)."""
    if not cm:
        return bank, a
    if bank == 7:
        return 7, cm7(a)
    if bank == 4:
        return 7, a + 0o377
    if bank == 0o37:
        return 0o26, a + 3
    return FF, a + 0o42


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
        bank = int(w.group(1), 8) if w.group(1) else FF
        addr = int(w.group(2), 8)
        out.append((bank, addr, int(w.group(3), 8), ' '.join(w.group(5).split())))
        if w.group(4):
            out.append((bank, addr + 1, int(w.group(4), 8), ' '.join(w.group(5).split()) + ' (second word)'))
    return out


def physical(bank, addr):
    return 2048 + addr - 0o4000 if bank == FF else bank * 1024 + addr - 0o2000


def symbol(listing, name):
    found = re.findall(r'(?:^|\s)' + re.escape(name) + r'\s+(?:(\d\d),)?([0-7]{4})\b', listing)
    bank, addr = found[-1]
    addr = int(addr, 8)
    return physical(int(bank, 8), addr) if bank else physical(FF, addr) if addr >= 0o4000 else None


def main():
    src = (R / 'src/imu_mode_switching.c').read_text()
    literals = {k: int(v, 8) for k, v in re.findall(r'#define (K_\w+) 0?([0-7]+)u', src)}
    frac = [int(v, 8) for v in re.search(r'K_GYROFRAC\[2\]=\{0?([0-7]+)u,0?([0-7]+)u\}', src).groups()]
    mission_words = {m.group(1): (int(m.group(2), 8), int(m.group(3), 8)) for m in re.finditer(
        r'static const ims_mission (LM|CM)=\{.*?0([0-7]+)u,0([0-7]+)u\};', src, re.S)}
    result = {'file': FILE, 'missions': {}}
    for mission, (cm, key) in MISSIONS.items():
        listing = (R / f'build/fixed-data/{mission}/assembly.lst').read_text(errors='replace')
        rope = (R / f'build/fixed-data/{mission}/MAIN.agc.bin').read_bytes()
        emitted = words(listing)
        owners = sorted((place(cm, b, a) + (name,) for b, a, name in OWNERS))
        rows, used = [], set()
        for bank, addr, word, text in emitted:
            assert rope_word(rope, physical(bank, addr)) == word, (mission, oct(addr), text)
            owner = [o for o in owners if o[0] == bank and o[1] <= addr]
            assert owner, (mission, oct(bank), oct(addr), text)
            used.add(owner[-1])
            rows.append({'address': f'{bank:02o},{addr:04o}', 'word': f'{word:05o}', 'source': text,
                         'native': owner[-1][2]})
        unused = set(owners) - used
        assert not unused, (mission, sorted((oct(b), oct(a), n) for b, a, n in unused))
        consts = {}
        for sym, k in CONSTANTS.items():
            value = rope_word(rope, symbol(listing, sym))
            assert value == literals[k], (mission, sym, oct(value), k)
            consts[sym] = {'literal': k, 'value': f'{value:05o}'}
        g = symbol(listing, 'GYROFRAC')
        assert [rope_word(rope, g), rope_word(rope, g + 1)] == frac, mission
        consts['GYROFRAC'] = {'literal': 'K_GYROFRAC', 'value': f'{frac[0]:05o} {frac[1]:05o}'}
        gyromin, kalebcon = mission_words[key]
        # -GYROMIN (OCT 77601) executes as MASK 7601: fixed-fixed 07601.
        assert rope_word(rope, physical(FF, 0o7601)) == gyromin, (mission, 'gyromin_word')
        assert rope_word(rope, symbol(listing, 'KALEBCON')) == kalebcon, (mission, 'kalebcon')
        consts['MASK 7601'] = {'literal': f'{key}.gyromin_word', 'value': f'{gyromin:05o}'}
        consts['KALEBCON'] = {'literal': f'{key}.kalebcon', 'value': f'{kalebcon:05o}'}
        result['missions'][mission] = {'words': len(rows), 'map': rows, 'constants': consts}
        print(f'{mission}: {len(rows)} words of {FILE} owned by {len(owners)} native ranges; '
              f'{len(consts)} literals equal the rope')
    (R / 'build/imu-mode-switching-source-map.json').write_text(json.dumps(result, indent=1) + '\n')


if __name__ == '__main__':
    main()
