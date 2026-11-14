#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The helpers that stand for one CCS at several places, and their masks.

Some helpers of readable/src are one CCS used at many places:
bit_set_in_A() (src/p20_p25/p20_p25.h), pack_flag() (src/p51_p53/p57.c),
ccs_positive() (src/q_r_axis/q_r_axis.c), masked_is_set()
(src/extended_verbs/radar.c and dap.c), ccs_A_positive()
(src/t4rupt/imu_monitor.c and optics.c) and positive()
(src/fresh_start/v37.c). Each follows the CCS's first two outcomes and
refuses the other two, negative and -0, with not_modelled(): what the
listing would run there differs with the place (STYLE.md, Fidelity). That
rests on one premise: the word the CCS looks at is A after a MASK with a
word whose bit 15, the sign, is clear, so it is never negative and never
-0. The helpers' names are tools/style_check.py's CCS_HELPERS: the style
check lets no other function refuse with those words, so a new helper
has to be named there, and is then held to the premise here.

This check holds every call of them to that premise:

  - It finds the mask of each call: the mask(X) before it, or the set_A(X)
    before a mask_at() or a rand_channel() (A is then X masked with an
    erasable word or a channel), with nothing between but steps that leave
    A as it is. A helper's first argument, for pack_flag(). Where a
    function begins before a mask is found, every caller of the function
    is followed the same way.
  - Each mission's text is searched apart (#if LUMINARY099 and the
    like), so a call whose mask differs between the two has both.
  - Each mask that is a constant's name goes into a C file as a static
    assert that its bit 15 is clear, under the #if of the mission that
    compiles the call, and the C compiler says whether it holds, with
    src/agc/names.h's own values.
  - A call whose mask is no constant is in NOT_CONSTANT below, with what
    bounds the word there instead. A call that is not listed, or a listed
    one that is gone, fails.

--list prints every call and its mask. --self-check runs the search on
made-up sources.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import style_check  # noqa: E402  (blanked, body_of, for_mission)

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / 'src'
HELPERS = style_check.CCS_HELPERS
CALL = re.compile(r'\b(' + '|'.join(HELPERS) + r')\(')
DEFINITION = re.compile(r'^(?!(?:if|for|while|switch|else|return)\b)[A-Za-z_][\w \t\*]*?\b(\w+)\([^;{}]*\)\s*\{', re.M)
OWN = re.compile(r'^\s*(?:static\s+)?(?:inline\s+)?(?:bool|void)\s+$', re.M)
CONSTANT = re.compile(r'[A-Z][A-Z0-9_]*')
KEEPS_A = ('ts', 'lxch', 'qxch', 'inhint', 'relint')        # steps that leave a masked A as it is
MASKS_A = ('mask_at', 'rand_channel')                       # A masked with a word that is no constant
HEADERS = ('agc/agc.h', 'agc/names.h')

MAJOR_MODE = ('a CCS of MMNUMBER, not of A: the major mode V37 was entered with (TS MMNUMBER at V37, and in '
              'the CM at R00TOP00 too), a program\'s number')
CHANNEL_33 = ('A is IMODES33 masked with the bit NXTIBT found (INDEX RUPTREG1; CAF BIT13): %s, since '
              'C33JMP\'s %s word alone comes here')

# The calls whose mask is no constant: (file, function, helper, which call of the helper in it) and what
# bounds the word instead, the missions that compile the call, and the constants the reason rests on:
# they are asserted too.
NOT_CONSTANT = {
    ('p20_p25/radar_read.c', 'scalechk_with_bit', 'bit_set_in_A', 1):
        ('A is the caller\'s scale bit masked with RADMODES, then RXOR with L, the same bit masked with '
         'channel 33: no bit but the caller\'s, and the two callers load BIT9 and BIT3', 'lm', ('BIT9', 'BIT3')),
    ('p20_p25/radar_read.c', 'bothlits', 'bit_set_in_A', 2):
        ('A is FLGWRD11 masked with a word of fixed memory: HFLSHBIT, the constant BIT1, or the word before '
         'it, BIT2 (INDEX ITEMP5, +0 or -1)', 'lm', ('BIT1', 'BIT2')),
    ('q_r_axis/q_r_axis.c', 'x_translation', 'ccs_positive', 2):
        ('a CCS of POLYTEMP, not of A: POLICY\'s word, a word of the table ALLJETS masked with one of '
         'TYPEPOLY, jet bits of channel 5 (bits 1 to 8)', 'lm', ()),
    ('q_r_axis/q_r_axis.c', 'selctsub', 'ccs_positive', 1):
        ('A is POLYTEMP masked with CH5MASK in POLICY: jet bits of channel 5 again', 'lm', ()),
    ('q_r_axis/q_r_axis.c', 'selctsub', 'ccs_positive', 2):
        ('a CCS of NUMBERT, not of A: the THREE loaded above the loop, then what this CCS left of it', 'lm', ()),
    ('fresh_start/v37.c', 'r00_after_downflag_xovinflg', 'positive', 1):
        (MAJOR_MODE, 'lm', ()),
    ('fresh_start/v37.c', 'roc_after_downflag_r21mark', 'positive', 1):
        (MAJOR_MODE, 'cm', ()),
    ('fresh_start/v37.c', 'seudop00_after_v37klean', 'positive', 1):
        (MAJOR_MODE + ' (the CM\'s function is groupkil_after_v37klean)', 'both', ()),
    ('fresh_start/v37.c', 'rake', 'positive', 1):
        ('a CCS of MPAC, not of A: the ELEVEN loaded above the loop, then what this CCS left of it', 'cm', ()),
    ('t4rupt/imu_monitor.c', 'tnontest_plus2', 'ccs_A_positive', 1):
        ('A is IMODES30\'s complement masked with BIT7 at TNONTEST +1, or the +0 that NXTIFAIL\'s CCS leaves '
         'of a -0, which comes straight here', 'both', ('BIT7',)),
    ('t4rupt/imu_monitor.c', 'setglock', 'ccs_A_positive', 1):
        ('A is the word SETGLOCK +1 masked with BIT6, masked again with DSPTAB +11D', 'both', ('BIT6',)),
    ('t4rupt/imu_monitor.c', 'imucage', 'ccs_A_positive', 1):
        ('A is IMODES30 masked with the bit NXTIFBIT found (INDEX RUPTREG1; CAF BIT14): BIT11, since '
         'IFAILJMP\'s fourth word alone comes here', 'both', ('BIT11',)),
    ('t4rupt/imu_monitor.c', 'pipfail', 'ccs_A_positive', 1):
        (CHANNEL_33 % ('BIT13', 'first'), 'both', ('BIT13',)),
    ('t4rupt/imu_monitor.c', 'dntmfast', 'ccs_A_positive', 1):
        (CHANNEL_33 % ('BIT12', 'second'), 'both', ('BIT12',)),
    ('t4rupt/imu_monitor.c', 'uptmfast', 'ccs_A_positive', 1):
        (CHANNEL_33 % ('BIT11', 'third'), 'both', ('BIT11',)),
}


def functions(code):
    """The function definitions in code (comments blanked): [(name, the body's first brace, its last)]."""
    out = []
    for found in DEFINITION.finditer(code):
        span = style_check.body_of(code, found.end() - 1)
        if span:
            out.append((found.group(1), span[0] - 1, span[1]))
    return out


def within(defined, at):
    for name, first, last in defined:
        if first < at <= last:
            return name, first
    return None, None


def argument(code, close):
    """The call that ends at code[close], a ')': (its name, its arguments' text, where the name begins)."""
    depth = 0
    for at in range(close, -1, -1):
        if code[at] == ')':
            depth += 1
        elif code[at] == '(':
            depth -= 1
            if not depth:
                name = re.search(r'(\w+)\s*$', code[:at])
                return (name.group(1), code[at + 1:close], name.start()) if name else (None, None, at)
    return None, None, 0


def mask_before(code, defined, at, wanted='mask', depth=0):
    """The masks of the A that a CCS at code[at] looks at: ([constants' names], None), or (None, why not).
    wanted: 'mask' (a mask is still to be found) or 'set' (found: mask_at or rand_channel; now its A)."""
    function, first = within(defined, at)
    if function is None:
        return None, 'not inside a function'
    at = max(code.rfind(mark, first, at) for mark in ';{}')     # the end of the step before this one
    while True:
        while code[at].isspace():
            at -= 1
        if at == first:                                         # the function begins: its callers, then
            if depth > 3:
                return None, f'{function}() begins before a mask, and so on through its callers'
            names = []
            calls = [found.start() for found in re.finditer(r'\b' + function + r'\(', code)
                     if within(defined, found.start())[0] is not None]
            if not calls:
                return None, f'{function}() begins before a mask, and no caller of it is in the file'
            for call in calls:
                got, why = mask_before(code, defined, call, wanted, depth + 1)
                if got is None:
                    return None, f'through {function}(): {why}'
                names += got
            return names, None
        if code[at] != ';' or code[at - 1] != ')':
            return None, 'a step that is no plain call comes before the CCS' if code[at] == ';' else \
                'the mask is not in the CCS\'s own block'
        name, arguments, begins = argument(code, at - 1)
        before = code[:begins].rstrip()
        if name is None or before[-1] not in ';{}':
            return None, 'a step that is no plain call comes before the CCS'
        arguments = arguments.strip()
        if wanted == 'mask' and name == 'mask' or wanted == 'set' and name == 'set_A':
            if CONSTANT.fullmatch(arguments):
                return [arguments], None
            return None, f'the mask is {arguments}, no constant\'s name'
        if wanted == 'mask' and name in MASKS_A:
            wanted = 'set'
        elif not (wanted == 'mask' and name in KEEPS_A):
            return None, f'{name}() comes before the CCS'
        at = len(before) - 1


def as_compiled(code, lm):
    """code (comments blanked) as one mission compiles it, place for place: the other mission's lines
    and the lines of each #if turned to spaces."""
    kept = style_check.for_mission(code, lm).split('\n')
    return '\n'.join(line if keep == line else ' ' * len(line) for line, keep in zip(code.split('\n'), kept))


def sites(file, text):
    """Each call of a helper in one file: (function, which call of that helper in it, line, helper,
    [(mission, constant)] or None, why not). mission: 'lm', 'cm' or 'both', the programs that compile
    the call with that mask."""
    code = style_check.blanked(text)
    missions = {'lm': as_compiled(code, True), 'cm': as_compiled(code, False)}
    defined = {mission: functions(compiled_code) for mission, compiled_code in missions.items()}
    out, count = [], {}
    for found in CALL.finditer(code):
        if OWN.search(code, code.rfind('\n', 0, found.start()) + 1, found.start()):
            continue                                            # the helper's own definition, or a prototype
        helper = found.group(1)
        function = next((within(defined[mission], found.start())[0] for mission in missions
                         if missions[mission][found.start():found.end()].strip()), None)
        if function is None:
            line = code.count('\n', 0, found.start()) + 1
            out.append((f'line {line}', 1, line, helper, None, 'the call is in no function that was found'))
            continue
        count[function, helper] = which = count.get((function, helper), 0) + 1
        line = code.count('\n', 0, found.start()) + 1
        _, arguments, _ = argument(code, style_check_close(code, found.end() - 1))
        first = arguments.split(',')[0].strip()
        of = re.fullmatch(r'ccs(?:_at|_register)?\((.*)\)', first)       # positive(ccs_at(X)): a CCS of X
        got, why = {}, None
        for mission, compiled_code in missions.items():
            if not compiled_code[found.start():found.end()].strip():
                continue                                        # the other mission's line
            if helper == 'pack_flag':
                masks, reason = ([first], None) if CONSTANT.fullmatch(first) else \
                    (None, f'the flag\'s bit is {first}')
            elif helper == 'ccs_positive' and first != 'A_ADDRESS' or of and of.group(1) != 'A_ADDRESS':
                masks, reason = None, f"a CCS of {of.group(1) if of else first}, not of A"
            else:
                masks, reason = mask_before(compiled_code, defined[mission], found.start())
            if masks is None:
                got, why = None, reason
                break
            got[mission] = masks
        if got is not None:
            same = got.get('lm') == got.get('cm')
            got = [('both' if same else mission, name) for mission, masks in got.items() for name in masks
                   if not (same and mission == 'cm')]
        out.append((function, which, line, helper, got, why))
    return out


def style_check_close(code, opened):
    """Where the parenthesis opened at code[opened] closes."""
    depth = 0
    for at in range(opened, len(code)):
        if code[at] == '(':
            depth += 1
        elif code[at] == ')':
            depth -= 1
            if not depth:
                return at
    return len(code) - 1


def asserts(found):
    """The C file that holds each constant mask to a clear bit 15."""
    lines = ['/* made by tools/ccs_helpers.py: each mask of a CCS helper has no sign bit */']
    lines += [f'#include "{header}"' for header in HEADERS]
    for mission, name, where in found:
        line = f'_Static_assert(((unsigned)({name}) & 040000u) == 0u, "{where}: {name} has bit 15");'
        lines.append(line if mission == 'both' else
                     f"#if {'LUMINARY099' if mission == 'lm' else 'COMANCHE055'}\n{line}\n#endif")
    return '\n'.join(lines) + '\n'


def compiled(source):
    """What the C compiler says of the asserts, for each mission: '' when they hold."""
    said = ''
    with tempfile.TemporaryDirectory() as scratch:
        path = Path(scratch) / 'ccs_helper_masks.c'
        path.write_text(source)
        for mission in ('-DLUMINARY099=1', '-DCOMANCHE055=1'):
            run = subprocess.run(['cc', '-std=c11', '-fsyntax-only', mission, f'-I{SRC}', str(path)],
                                 capture_output=True, text=True)
            if run.returncode:
                said += f'{mission}:\n{run.stderr}'
    return said


def self_check():
    made = '''static void lamp(void)
{
    mask_at(RADMODES);
    if (bit_set_in_A()) on();
}
void first(void)
{
    ca(STATE);
    mask(RNDVZBIT);
    ts(Q_ADDRESS);
    if (bit_set_in_A()) return;
    set_A(BIT4);
    rand_channel(CHAN33);
    if (!bit_set_in_A()) {
        set_A(BIT8);
        lamp();
    }
    ca(FLGWRD11);
    mask_at(indexed(HFLSHBIT, fetch(ITEMP5)));
    if (bit_set_in_A()) return;
    policy();
    if (ccs_positive(A_ADDRESS)) return;
    if (ccs_positive(ADDRESS_OF(NUMBERT))) return;
    pack_flag(REFSMBIT, ADDRESS_OF(FLAGWRD3), BIT7);
}
void second(void)
{
#if LUMINARY099
    set_A(IMUSEBIT);
    mask_at(ADDRESS_OF(FLAGWRD0));
#else
    set_A(IMUSEFLG);
    mask_at(ADDRESS_OF(STATE));
#endif
    if (ccs_A_positive()) return;
#if COMANCHE055
    mask(BIT7);
    if (ccs_A_positive()) return;
#endif
    if (positive(ccs_at(ADDRESS_OF(MMNUMBER)))) return;
}
'''
    expect = [('lamp', [('both', 'BIT8')]), ('first', [('both', 'RNDVZBIT')]), ('first', [('both', 'BIT4')]),
              ('first', None), ('first', None), ('first', None), ('first', [('both', 'REFSMBIT')]),
              ('second', [('lm', 'IMUSEBIT'), ('cm', 'IMUSEFLG')]), ('second', [('cm', 'BIT7')]),
              ('second', None)]
    got = [(function, masks) for function, _, _, _, masks, _ in sites('made/up.c', made)]
    wrong = 0 if got == expect else 1
    if wrong:
        print(f'FAIL the made-up source: {got}, not {expect}')
    source = asserts([('both', 'ONE', 'here'), ('lm', 'BIT14', 'there'), ('cm', 'BIT13', 'there')])
    if compiled(source):
        wrong += 1
        print('FAIL two masks with a clear bit 15 do not compile:\n' + compiled(source))
    for mission in ('both', 'lm', 'cm'):
        if not compiled(asserts([(mission, 'BIT15', 'here')])):
            wrong += 1
            print(f'FAIL a mask with bit 15, BIT15, compiles ({mission})')
    print(f"{'FAIL' if wrong else 'ok  '} ccs helpers self-check: a made-up source and three made-up masks, "
          f'{wrong} judged wrongly')
    return 1 if wrong else 0


def main():
    if '--self-check' in sys.argv:
        return self_check()
    found, loose, listing, calls = [], {}, [], 0
    for path in sorted(SRC.glob('*/*.[ch]')):
        file = str(path.relative_to(SRC))
        for function, which, line, helper, masks, why in sites(file, path.read_text()):
            calls += 1
            if masks is None:
                loose[file, function, helper, which] = (line, why)
                listing.append(f'src/{file}:{line}: {helper}() in {function}(): no constant ({why})')
            else:
                found += [(mission, name, f'src/{file}:{line}') for mission, name in masks]
                listing.append(f'src/{file}:{line}: {helper}() in {function}(): ' + ', '.join(
                    name if mission == 'both' else f'{name} ({mission.upper()})' for mission, name in masks))
    bad = []
    for key in sorted(set(loose) - set(NOT_CONSTANT)):
        line, why = loose[key]
        bad.append(f'src/{key[0]}:{line}: {key[2]}() in {key[1]}() has no constant mask ({why}) and is not listed')
    for key in sorted(set(NOT_CONSTANT) - set(loose)):
        bad.append(f'src/{key[0]}: the listed call {key[3]} of {key[2]}() in {key[1]}() is not there, or has a '
                   'constant mask')
    for key, (reason, mission, names) in sorted(NOT_CONSTANT.items()):
        found += [(mission, name, f'src/{key[0]} {key[1]}()') for name in names]
    said = compiled(asserts(found))
    if said:
        bad.append('the masks\' static asserts do not compile:\n' + said)
    if '--list' in sys.argv:
        print('\n'.join(listing))
        for key, (reason, _, _) in sorted(NOT_CONSTANT.items()):
            print(f'not a constant: src/{key[0]}, {key[1]}(), call {key[3]} of {key[2]}(): {reason}')
    for line in bad:
        print('FAIL ' + line)
    print(f"{'FAIL' if bad else 'ok  '} ccs helpers: {calls} calls of {', '.join(HELPERS)}; "
          f'{calls - len(loose)} with constant masks, {len(found)} static asserts of a clear bit 15 compiled; '
          f'{len(loose)} with no constant mask, {len(set(loose) & set(NOT_CONSTANT))} of them as listed')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
