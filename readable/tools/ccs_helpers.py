#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The CCS that is written as a test, and what holds it to two outcomes.

A CCS has four outcomes and the readable C names all four in a switch
(STYLE.md, Structure). One form is let off: ccs_positive() of src/agc/agc.h,
a CCS written as a test, `if (ccs_positive(A_ADDRESS))`. It follows the
CCS's first two outcomes and refuses the other two, negative and -0, with
not_modelled(): the one helper stands for that CCS at every place it is
used, and what the listing would run there differs with the place
(STYLE.md, Fidelity). pack_flag() of src/p51_p53/p57.c is the same for
two flags. That rests on one premise: the word the CCS looks at is never
negative and never -0. The helpers' names are tools/style_check.py's
CCS_HELPERS: the style check lets no other function refuse with those
words and lets no CCS be compared with an outcome outside them, so a new
helper has to be named there, and is then held to the premise here.

This check holds every call of them to that premise, one of two ways:

  - A CCS of A after a MASK with a constant. It finds the mask of the
    call: the mask(X) before it, or the set_A(X) before a mask_at(), a
    mask() of an erasable word or a rand_channel() (A is then X masked
    with an erasable word or a channel), with nothing between but steps
    that leave A as it is. A
    helper's first argument, for pack_flag(). Where a function begins
    before a mask is found, every caller of the function is followed the
    same way. Each mission's text is searched apart (#if LUMINARY099 and
    the like), so a call whose mask differs between the two has both.
    Each mask that is a constant's name goes into a C file as a static
    assert that its bit 15, the sign, is clear, under the #if of the
    mission that compiles the call, and the C compiler says whether it
    holds, with src/agc/names.h's own values.
  - Any other call: a CCS of another word (a count, a major mode), or of
    an A whose mask is no constant or was not found. A comment at the
    call, on its lines or right above them, names the word and says what
    keeps it from being negative, in one form:
        CCS SPNDX, never negative: the loop's index, counted down 2, 1, 0.
    The check holds the comment to that form, to the word of the call
    (SPNDX for ADDRESS_OF(SPNDX), A for A_ADDRESS, W.IND for
    ADDRESS_OF(W_IND)) and to a reason of three words at least. Whether
    the reason is right, a person reads: these calls rest on the reason,
    not on the compiler. One part of a reason the compiler does take: the
    constants after the words "masked with" (FLAGWRD8 masked with
    SURFFBIT; L masked with BIT1, BIT2, BIT5 or BIT6) get the same static
    assert as a mask that was found. A call with neither a constant mask
    nor such a comment fails.

--list prints every call with its mask, or with its reason and whether
any of it is asserted. --self-check runs the search on made-up sources.
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
ERASABLE = re.compile(r'^#define (\w+) +ERASABLE\(', re.M)        # src/agc/names.h: a word, no constant
VALUE = re.compile(r'^#define (\w+) +\(\(word\)KW_\w+\)', re.M)   # src/agc/names.h: a constant's value
ABOVE = re.compile(r'\s*(?:/\*|\*)')                         # a comment's line
REASON = re.compile(r'\bCCS ([^,]+), never negative: (.+)')   # the comment at a call with no constant mask
MASKED = re.compile(r'\bmasked with ([A-Z][A-Z0-9_]*(?:(?:,? (?:or |and )?)[A-Z][A-Z0-9_]*\b)*)')

def erasables(found=[]):
    """The names of src/agc/names.h that are erasable words (FLAGWRD7 is ERASABLE(K_FLAGWRD7)), not
    constants."""
    if not found:
        found.append(set(ERASABLE.findall((SRC / HEADERS[1]).read_text())))
    return found[0]


def values(found={}):
    """The names of src/agc/names.h that are constants with a value (BIT7 is (word)KW_BIT7), not words
    of the erasable memory and not places of the fixed: {'lm': those the LM's program has, 'cm': ...}."""
    if not found:
        names = (SRC / HEADERS[1]).read_text()
        found.update({mission: set(VALUE.findall(style_check.for_mission(names, mission == 'lm')))
                      for mission in ('lm', 'cm')})
    return found


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
        word = arguments in erasables()
        if wanted == 'mask' and name == 'mask' and not word or wanted == 'set' and name == 'set_A':
            if CONSTANT.fullmatch(arguments) and not word:
                return [arguments], None
            return None, f'the mask is {arguments}, no constant\'s name'
        if wanted == 'mask' and (name in MASKS_A or name == 'mask'):    # mask(FLAGWRD7): an erasable word
            wanted = 'set'
        elif not (wanted == 'mask' and name in KEEPS_A) or name in ('lxch', 'qxch') and \
                arguments == 'A_ADDRESS':                               # (an exchange with A itself changes it)
            return None, f'{name}() comes before the CCS'
        at = len(before) - 1


def as_compiled(code, lm):
    """code (comments blanked) as one mission compiles it, place for place: the other mission's lines
    and the lines of each #if turned to spaces."""
    kept = style_check.for_mission(code, lm).split('\n')
    return '\n'.join(line if keep == line else ' ' * len(line) for line, keep in zip(code.split('\n'), kept))


def said_at(text, first, last):
    """The comments at a call whose lines are first to last (counted from 1), each as one line: those
    on the call's lines and on the comment's lines right above them."""
    lines = text.split('\n')
    top = first
    while top > 1 and ABOVE.match(lines[top - 2]):
        top -= 1
    return [' '.join(re.sub(r'\n\s*\*(?=\s)', ' ', comment).split()) for comment in
            style_check.COMMENT.findall('\n'.join(lines[top - 1:last]))]


def plain(name):
    """A word's name with its letters and digits only: W.IND and W_IND are the same word."""
    return re.sub(r'[^A-Z0-9]', '', name.upper())


def word_of(first):
    """The word a call looks at, by its argument: A for A_ADDRESS, MPAC +1 for ADDRESS_OF(MPAC) + 1u."""
    name = re.sub(r'\bADDRESS_OF\(([^()]*)\)', r'\1', first)
    return plain(re.sub(r'(\d)u\b', r'\1', re.sub(r'_ADDRESS\b', '', name)))


def reasoned(said, first):
    """The reason a call's comments give that its word is never negative, or None: the comment names
    the word of the call, CCS X, never negative: and has three words more at least."""
    for comment in said:
        told = REASON.search(comment)
        if told and plain(told.group(1)) == word_of(first) and \
                len(style_check.WORD.findall(told.group(2))) >= style_check.REASON_WORDS:
            return told.group(2)
    return None


def masks_named(reason, here='both'):
    """The constants a reason says the word is masked with, each with the missions that have it,
    [(mission, constant)]: those the compiler can hold to a clear bit 15 (an erasable word or a place
    named there is passed over). here: the missions that compile the call."""
    out = []
    for name in (name for told in MASKED.finditer(reason) for name in CONSTANT.findall(told.group(1))):
        has = [mission for mission in ('lm', 'cm') if here in ('both', mission) and name in values()[mission]]
        if has:
            out.append(('both' if len(has) == 2 else has[0], name))
    return out


def sites(file, text):
    """Each call of a helper in one file: (function, line, helper, [(mission, constant)] or None, why
    not, the comments at the call, the call's first argument, the missions that compile the call).
    mission: 'lm', 'cm' or 'both', the programs that compile the call with that mask."""
    code = style_check.blanked(text)
    missions = {'lm': as_compiled(code, True), 'cm': as_compiled(code, False)}
    defined = {mission: functions(compiled_code) for mission, compiled_code in missions.items()}
    out = []
    for found in CALL.finditer(code):
        if OWN.search(code, code.rfind('\n', 0, found.start()) + 1, found.start()):
            continue                                            # the helper's own definition, or a prototype
        helper = found.group(1)
        function = next((within(defined[mission], found.start())[0] for mission in missions
                         if missions[mission][found.start():found.end()].strip()), None)
        line = code.count('\n', 0, found.start()) + 1
        close = style_check_close(code, found.end() - 1)
        said = said_at(text, line, code.count('\n', 0, close) + 1)
        if function is None:
            out.append((f'line {line}', line, helper, None, 'the call is in no function that was found', [],
                        '', 'both'))
            continue
        _, arguments, _ = argument(code, close)
        first = arguments.split(',')[0].strip()
        got, why = {}, None
        here = [mission for mission in missions if missions[mission][found.start():found.end()].strip()]
        for mission, compiled_code in missions.items():
            if not compiled_code[found.start():found.end()].strip():
                continue                                        # the other mission's line
            if helper == 'pack_flag':
                masks, reason = ([first], None) if CONSTANT.fullmatch(first) else \
                    (None, f'the flag\'s bit is {first}')
            elif first != 'A_ADDRESS':
                masks, reason = None, f'a CCS of {first}, not of A'
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
        out.append((function, line, helper, got, why, said, first, 'both' if len(here) == 2 else here[0]))
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
    made = """static void lamp(void)
{
    mask_at(RADMODES);
    if (ccs_positive(A_ADDRESS)) on();
}
void first(void)
{
    ca(STATE);
    mask(RNDVZBIT);
    ts(Q_ADDRESS);
    if (ccs_positive(A_ADDRESS)) return;
    set_A(BIT4);
    rand_channel(CHAN33);
    if (!ccs_positive(A_ADDRESS)) {
        set_A(BIT8);
        lamp();
    }
    ca(FLGWRD11);
    mask_at(indexed(HFLSHBIT, fetch(ITEMP5)));
    /* CCS A, never negative: FLGWRD11 masked with BIT1 or BIT2, HFLSHBIT or the
     * word before it, then masked with RADMODES. */
    if (ccs_positive(A_ADDRESS)) return;
    policy();
    if (ccs_positive(A_ADDRESS)) return; /* TCF +2 */
    do {
        step();
        /* CCS NUMBERT, never negative: the THREE loaded above, counted down. */
    } while (ccs_positive(ADDRESS_OF(NUMBERT))); /* TCF -3 */
    if (ccs_positive(ADDRESS_OF(NUMBERT))) return; /* CCS NUMBERT, never negative */
    /* CCS A, never negative: the THREE loaded above, counted down. */
    if (ccs_positive(ADDRESS_OF(W_IND) + 1u)) return;
    /* CCS W.IND +1, never negative: masked with BIT9, BIT3 and POODOO. */
    if (ccs_positive(ADDRESS_OF(W_IND) + 1u)) return;
    pack_flag(REFSMBIT, ADDRESS_OF(FLAGWRD3), BIT7);
}
void second(void)
{
#if LUMINARY099
    set_A(IMUSEBIT);
    mask_at(ADDRESS_OF(FLAGWRD0));
#else
    set_A(IMUSEFLG);
    mask(FLAGWRD7);
#endif
    if (ccs_positive(A_ADDRESS)) return;
#if COMANCHE055
    mask(BIT7);
    if (ccs_positive(A_ADDRESS)) return;
#endif
}
void third(void)
{
    mask(BIT5);
    lxch(A_ADDRESS);
    if (ccs_positive(A_ADDRESS)) return;
    mask(BIT6);
    lxch(Q_ADDRESS);
    if (ccs_positive(A_ADDRESS)) return;
}
"""
    expect = [('lamp', [('both', 'BIT8')], None), ('first', [('both', 'RNDVZBIT')], None),
              ('first', [('both', 'BIT4')], None), ('first', None, ['BIT1', 'BIT2']), ('first', None, None),
              ('first', None, []), ('first', None, None), ('first', None, None), ('first', None, ['BIT9', 'BIT3']),
              ('first', [('both', 'REFSMBIT')], None),
              ('second', [('lm', 'IMUSEBIT'), ('cm', 'IMUSEFLG')], None), ('second', [('cm', 'BIT7')], None),
              ('third', None, None), ('third', [('both', 'BIT6')], None)]
    got = [(function, masks, None if masks or reasoned(said, first) is None else
            [name for _, name in masks_named(reasoned(said, first), here)])
           for function, _, _, masks, _, said, first, here in sites('made/up.c', made)]
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
    found, listing, bad, calls, told, named = [], [], [], 0, 0, 0
    for path in sorted(path for path in SRC.rglob('*') if path.suffix in ('.c', '.h', '.inc')):
        file = str(path.relative_to(SRC))
        for function, line, helper, masks, why, said, first, here in sites(file, path.read_text()):
            calls += 1
            where = f'src/{file}:{line}: {helper}() in {function}()'
            reason = None if masks is not None else reasoned(said, first)
            if masks is not None:
                found += [(mission, name, f'src/{file}:{line}') for mission, name in masks]
                listing.append(f'{where}: ' + ', '.join(
                    name if mission == 'both' else f'{name} ({mission.upper()})' for mission, name in masks))
            elif reason is not None:
                told += 1
                constants = masks_named(reason, here)
                named += bool(constants)
                found += [(mission, name, f'src/{file}:{line}') for mission, name in constants]
                listing.append(f'{where}: no constant mask ({why}); CCS {first}, never negative: {reason} [' + (
                    'the reason\'s names asserted: ' + ', '.join(name for _, name in constants) if constants
                    else 'by the reason alone') + ']')
            else:
                bad.append(f'{where} has no constant mask ({why}), and no comment at the call says CCS '
                           f'{first}, never negative: and why')
    said = compiled(asserts(found))
    if said:
        bad.append('the masks\' static asserts do not compile:\n' + said)
    if '--list' in sys.argv:
        print('\n'.join(listing))
    for line in bad:
        print('FAIL ' + line)
    print(f"{'FAIL' if bad else 'ok  '} ccs helpers: {calls} calls of {', '.join(HELPERS)}; "
          f'{len(listing) - told} after constant masks; {told} more on a reason at the call, which a person '
          f'reads ({named} of the reasons name constants, asserted as named); {len(found)} '
          f'static asserts of a clear bit 15 compiled; {calls - len(listing)} with neither')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
