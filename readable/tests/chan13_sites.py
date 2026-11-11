#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Every instruction of the two listings that can touch channel 13.

The lockstep tool (c64-port/tools/x86_flight_lockstep.c, gate 3) compares
channel 13 without its bits 12 to 14 and compares the three HANDRUPT traps
in their place: on the AGC those bits arm the traps and never read back,
and the x86 step keeps a routine's write of them in the channel until its
next clock step. That is sound only while no code of the programs can
tell the difference, so this lists every instruction that reads or writes
the channel and holds the listings to the list:

  - no READ or ROR names channel 13;
  - each RAND that names it follows a CA or CAF of a constant, and the
    one RXOR is followed by a MASK with one: the bits the code keeps of
    the channel are below bit 12;
  - each WAND and WOR is a read-modify-write: A and the channel both get
    the result, so a channel bit one side holds and the other does not
    shows in A, which is compared (erasable word 0);
  - four instructions take their channel from a register (Pinball's
    channel display and load, one READ and one WRITE in each program):
    with 13 keyed in, the READ's word goes to A and from there to the
    display registers, which are compared. (Six more lines read READ
    under an INDEX and are no channel instruction: the interpreter's VAD
    and VSU add ADDRWD to a READ to make a DCA or DCS.)

A new instruction of any of these kinds, or one that has moved, fails
this check until SITES says what it is. Run by `make chan13-sites` (in
`make all`); needs nothing but the listings.
"""
import re, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
LISTINGS = {'LM': ROOT / 'Luminary099', 'CM': ROOT / 'Comanche055'}
CHANNEL = ('READ', 'RAND', 'ROR', 'RXOR', 'WRITE', 'WAND', 'WOR')
# A line's label (in column 1, or none), instruction and operand.
LINE = re.compile(r'^(\S*)[ \t]+(\S+)(?:[ \t]+([^\s#]+))?')
BITS_12_TO_14 = 0o34000

# (program, file, line, instruction): for a RAND, the constant in A, and
# for the RXOR, the constant of the MASK after it, with the bits of the
# channel the code so keeps; for the others, None.
SITES = {
    ('LM', 'DOWN_TELEMETRY_PROGRAM', 171, 'WOR'): None,
    ('LM', 'DOWN_TELEMETRY_PROGRAM', 237, 'WAND'): None,
    ('LM', 'DOWN_TELEMETRY_PROGRAM', 450, 'RAND'): ('BIT7', 0o100),
    ('LM', 'FRESH_START_AND_RESTART', 96, 'WRITE'): None,
    ('LM', 'FRESH_START_AND_RESTART', 415, 'WAND'): None,
    ('LM', 'FRESH_START_AND_RESTART', 461, 'WAND'): None,
    ('LM', 'FRESH_START_AND_RESTART', 465, 'WOR'): None,
    ('LM', 'IMU_MODE_SWITCHING_ROUTINES', 976, 'WOR'): None,
    ('LM', 'IMU_MODE_SWITCHING_ROUTINES', 1035, 'WAND'): None,
    ('LM', 'LUNAR_LANDING_GUIDANCE_EQUATIONS', 266, 'WOR'): None,
    ('LM', 'LUNAR_LANDING_GUIDANCE_EQUATIONS', 1171, 'WOR'): None,
    ('LM', 'P-AXIS_RCS_AUTOPILOT', 715, 'WOR'): None,
    ('LM', 'P20-P25', 2776, 'WAND'): None,
    ('LM', 'P20-P25', 2781, 'WOR'): None,
    ('LM', 'P20-P25', 2811, 'RAND'): ('SEVEN', 0o7),
    ('LM', 'P20-P25', 2843, 'RAND'): ('BIT3', 0o4),
    ('LM', 'P20-P25', 2862, 'RXOR'): ('BIN3', 0o3),
    ('LM', 'P20-P25', 2897, 'WOR'): None,
    ('LM', 'P20-P25', 2922, 'RAND'): ('BIT1', 0o1),
    ('LM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 3636, 'WOR'): None,
    ('LM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 3686, 'WAND'): None,
    ('LM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 3758, 'WAND'): None,
    ('LM', 'Q_R-AXIS_RCS_AUTOPILOT', 748, 'WOR'): None,
    ('LM', 'T6-RUPT_PROGRAMS', 94, 'WAND'): None,
    ('LM', 'T6-RUPT_PROGRAMS', 111, 'WOR'): None,
    ('CM', 'DOWN-TELEMETRY_PROGRAM', 175, 'WOR'): None,
    ('CM', 'DOWN-TELEMETRY_PROGRAM', 242, 'WAND'): None,
    ('CM', 'FRESH_START_AND_RESTART', 182, 'WRITE'): None,
    ('CM', 'FRESH_START_AND_RESTART', 521, 'WAND'): None,
    ('CM', 'FRESH_START_AND_RESTART', 930, 'WAND'): None,   # WAND 13, by number
    ('CM', 'IMU_MODE_SWITCHING_ROUTINES', 976, 'WOR'): None,
    ('CM', 'IMU_MODE_SWITCHING_ROUTINES', 1035, 'WAND'): None,
    ('CM', 'JET_SELECTION_LOGIC', 52, 'WOR'): None,
    ('CM', 'JET_SELECTION_LOGIC', 915, 'WOR'): None,
    ('CM', 'P20-P25', 1231, 'WAND'): None,
    ('CM', 'P20-P25', 1235, 'WOR'): None,
    ('CM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 3651, 'WOR'): None,
    ('CM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 3699, 'WAND'): None,
    ('CM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 3769, 'WAND'): None,
    ('CM', 'RCS-CSM_DIGITAL_AUTOPILOT', 657, 'WOR'): None,
    ('CM', 'TVCROLLDAP', 574, 'WOR'): None,
}
# Channel instructions under an INDEX, and what each is.
CHANNEL_KEYED = 'the channel is the one keyed in (noun 10)'
NO_CHANNEL = 'no channel instruction: READ plus ADDRWD is the DCA or DCS of VAD and VSU'
INDEXED = {
    ('LM', 'INTERPRETER', 781, 'READ'): NO_CHANNEL,
    ('LM', 'INTERPRETER', 789, 'READ'): NO_CHANNEL,
    ('LM', 'INTERPRETER', 797, 'READ'): NO_CHANNEL,
    ('LM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 1306, 'READ'): CHANNEL_KEYED,    # CHANDSP, verb 1
    ('LM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 2013, 'WRITE'): CHANNEL_KEYED,   # the load, verb 21
    ('CM', 'INTERPRETER', 788, 'READ'): NO_CHANNEL,
    ('CM', 'INTERPRETER', 796, 'READ'): NO_CHANNEL,
    ('CM', 'INTERPRETER', 804, 'READ'): NO_CHANNEL,
    ('CM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 1266, 'READ'): CHANNEL_KEYED,
    ('CM', 'PINBALL_GAME_BUTTONS_AND_LIGHTS', 2001, 'WRITE'): CHANNEL_KEYED,
}


def instructions(path):
    """The instruction lines of a listing file: (line number, label,
    instruction, operand)."""
    out = []
    for number, line in enumerate(path.read_text().splitlines(), 1):
        got = LINE.match(line)
        if got and not line.lstrip().startswith('#'):
            out.append((number, got[1], got[2], got[3] or ''))
    return out


def scan(listings):
    """What the listings hold: (the channel 13 instructions with the
    constant a RAND or RXOR follows, the indexed channel instructions, the
    names of channel 13)."""
    named, indexed, names = {}, set(), {}
    for program, directory in listings.items():
        files = {f.stem: instructions(f) for f in sorted(directory.glob('*.agc'))}
        # The names of the channel: 13 itself and any symbol equated to it.
        names[program] = {'13'} | {label for lines in files.values() for _, label, op, operand in lines
                                   if label and op in ('=', 'EQUALS') and operand == '13'
                                   and label.startswith('CHAN')}
        equal = {label: operand for lines in files.values() for _, label, op, operand in lines
                 if label and op in ('=', 'EQUALS')}
        octal = {label: int(operand, 8) for lines in files.values() for _, label, op, operand in lines
                 if label and op in ('OCT', 'OCTAL') and re.fullmatch(r'[0-7]+', operand)}
        for stem, lines in files.items():
            for i, (number, _, op, operand) in enumerate(lines):
                if op not in CHANNEL:
                    continue
                before = [l[2] for l in lines[max(0, i - 2):i]]
                if 'INDEX' in before or 'NDX' in before:
                    indexed.add((program, stem, number, op))
                elif operand in names[program]:
                    constant = None
                    if op in ('RAND', 'RXOR'):
                        # RAND keeps the bits of the constant in A; RXOR
                        # keeps every bit, until the MASK that follows it.
                        loads = ([l for l in lines[max(0, i - 2):i] if l[2] in ('CA', 'CAF')] if op == 'RAND'
                                 else [l for l in lines[i + 1:i + 2] if l[2] == 'MASK'])
                        name = loads[-1][3] if loads else ''
                        word, hops = name, 0
                        while word in equal and hops < 8:
                            word, hops = equal[word], hops + 1
                        constant = (name, octal.get(word))
                    named[program, stem, number, op] = constant
    return named, indexed, names


def problems(named, indexed, names):
    """What of the listings SITES and INDEXED do not say; nothing if they
    are the whole of it."""
    out = [f'{p} {f} line {n}: {op} of channel 13 is not in SITES' for p, f, n, op in sorted(set(named) - set(SITES))]
    out += [f'{p} {f} line {n}: SITES has a {op} the listing does not'
            for p, f, n, op in sorted(set(SITES) - set(named))]
    out += [f'{p} {f} line {n}: an indexed {op} is not in INDEXED' for p, f, n, op in sorted(indexed - set(INDEXED))]
    out += [f'{p} {f} line {n}: INDEXED has a {op} the listing does not'
            for p, f, n, op in sorted(set(INDEXED) - indexed)]
    out += [f'{p}: channel 13 is named {sorted(n)}, not 13 and CHAN13' for p, n in sorted(names.items())
            if n != {'13', 'CHAN13'}]
    for site, constant in sorted(named.items()):
        p, f, n, op = site
        if op in ('READ', 'ROR'):
            out.append(f'{p} {f} line {n}: a {op} reads every bit of channel 13')
        elif op in ('RAND', 'RXOR'):
            if site in SITES and constant != SITES[site]:
                out.append(f'{p} {f} line {n}: {op} with {constant}, where SITES has {SITES[site]}')
            if constant[1] is None or constant[1] & BITS_12_TO_14:
                out.append(f'{p} {f} line {n}: {op} with {constant[0]} can keep bits 12 to 14')
    return out


def made_up():
    """The check tried on listings that are wrong."""
    named, indexed, names = scan(LISTINGS)
    cases = (
        ('a READ of the channel', {**named, ('LM', 'T4RUPT_PROGRAM', 100, 'READ'): None}, indexed, 'reads every bit'),
        ('a WAND that is not listed', {**named, ('CM', 'T4RUPT_PROGRAM', 100, 'WAND'): None}, indexed, 'not in SITES'),
        ('a listed WOR gone', {k: v for k, v in named.items() if k != ('LM', 'T6-RUPT_PROGRAMS', 111, 'WOR')},
         indexed, 'the listing does not'),
        ('a RAND after a wider constant', {**named, ('LM', 'P20-P25', 2811, 'RAND'): ('POSMAX', 0o37777)}, indexed,
         'can keep bits 12 to 14'),
        ('a RAND after no constant', {**named, ('LM', 'P20-P25', 2811, 'RAND'): ('', None)}, indexed,
         'can keep bits 12 to 14'),
        ('another indexed READ', named, indexed | {('LM', 'T4RUPT_PROGRAM', 100, 'READ')}, 'not in INDEXED'),
    )
    failures = 0
    for what, n, x, word in cases:
        found = problems(n, x, names)
        ok = any(word in p for p in found)
        failures += not ok
        print(f'{"ok  " if ok else "FAIL"} {what}: ' + (found[0] if found else 'passes'))
    return failures


def main():
    failures = made_up()
    named, indexed, names = scan(LISTINGS)
    found = problems(named, indexed, names)
    kinds = {op: sum(1 for site in named if site[3] == op) for op in CHANNEL}
    print(f'{"FAIL" if found else "ok  "} channel 13 in the two listings: {len(named)} instructions name it ('
          + ', '.join(f'{kinds[op]} {op}' for op in CHANNEL if kinds[op]) + f') and {len(indexed)} stand under '
          f'an INDEX ({sum(1 for site in indexed if INDEXED.get(site) == CHANNEL_KEYED)} of them take the '
          'channel keyed in), each as tests/chan13_sites.py lists it; no READ or ROR names it, and each RAND '
          'and RXOR keeps bits below bit 12' + ''.join(f'\n     {p}' for p in found))
    return 1 if failures or found else 0


if __name__ == '__main__':
    sys.exit(main())
