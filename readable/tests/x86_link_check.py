#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The link check of the readable x86 program, tried on links it must fail.

tests/x86_readable.py fails the build of the readable flight program if the
link loaded any file but the readable objects, the step and main, and the
seven machine objects it names (link_problems). A check that never fails
proves nothing, so this gives it link maps that are wrong, each in one
way, and requires that it say so:

  x86_link_check.py          made-up link maps (no build needed): a faithful
                             object by its full path, by a relative path,
                             under a path with a space, with another
                             suffix, in an archive, copied in among the
                             readable objects, and outside the repository;
                             a machine object left out; and the right map,
                             which must pass.
                             Then the readable sources themselves: an
                             #include can bring in what no link shows,
                             so none may name a C source, and of the
                             files of c64-port/src only the tables of
                             rope words in PORT_DATA.
  x86_link_check.py --link   the real link of the LM program with one
                             faithful object added (after make x86-flight):
                             the check must name that object.

Run by `make x86-link-check` (in `make all`) and by `make x86-flight`.
"""
import os, re, subprocess, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import x86_readable as xr  # noqa: E402

PORT, READABLE = xr.PORT, xr.READABLE
HOST = PORT / 'build/flight-store/host'
# A faithful routine's object, to stand for any: SPSIN and SPCOS.
FAITHFUL = 'lm_single_trig.o'


# The files of c64-port/src a readable source may include: words of the
# rope the port's generators wrote out as tables (from the assembled rope,
# not from the faithful routines), which three bindings read in place of
# fixed memory: the downlists and DOWN-TELEMETRY's constants, the CM's
# restart tables, and the words after the phase table routines' callers.
PORT_DATA = {'lm_downtelemetry_data.inc', 'cm_downtelemetry_data.inc', 'cm_restart_table_data.inc',
             'lm_phase_sites.inc', 'cm_phase_sites.inc'}


def sources():
    """No readable source includes a C source or, of c64-port/src, any file
    but PORT_DATA."""
    include = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)
    port_sources = {f.name for d in ('src', 'tests', 'tools', 'platform') for f in (PORT / d).rglob('*') if f.is_file()}
    files = [f for f in sorted((READABLE / 'src').rglob('*')) if f.suffix in ('.c', '.h', '.inc')]
    wrong, seen = [], set()
    for f in files:
        for name in include.findall(f.read_text()):
            base = Path(name).name
            local = (f.parent / name).resolve()
            ours = local.is_file() and str(local).startswith(str(READABLE) + os.sep)
            if name.endswith('.c') or (not ours and base in port_sources and base not in PORT_DATA):
                wrong.append(f'{f.relative_to(READABLE)} includes {name}')
            elif not ours and base in PORT_DATA:
                seen.add(base)
    ok = not wrong and seen == PORT_DATA
    print(f'{"ok  " if ok else "FAIL"} the readable sources ({len(files)} files) include no C source and, of '
          f'c64-port/src, only the {len(PORT_DATA)} tables of rope words named'
          + ('' if ok else ': ' + '; '.join(wrong[:5] or [f'not all of them: {sorted(PORT_DATA - seen)}'])))
    return 0 if ok else 1


def made_up():
    toolchain = ['/usr/lib/x86_64-linux-gnu', '/usr/lib/gcc/x86_64-linux-gnu/13']
    readable = [str(READABLE / 'build/x86-lm' / name) for name in ('agc/agc.o', 'pinball/nvsub.o')]
    step = [str(READABLE / 'build/x86-flight' / name) for name in ('flight_dispatch_lm.o', 'flight_main_lm.o')]
    machine = [str(HOST / name) for name in xr.MACHINE]
    expected = step + machine + readable
    library = ['/usr/lib/x86_64-linux-gnu/Scrt1.o', '/usr/lib/gcc/x86_64-linux-gnu/13/libgcc.a',
               '/usr/lib/x86_64-linux-gnu/libm.so']

    def text(paths):
        return ''.join(f'LOAD {p}\n' for p in paths)

    right = library[:1] + expected + library[1:]
    # (what the map is, its LOAD lines, a word the check's answer must hold)
    cases = (
        ('the right map', right, None),
        ('a faithful object by its full path', right + [str(HOST / FAITHFUL)], FAITHFUL),
        ('a faithful object by a relative path', right + [f'build/flight-store/host/{FAITHFUL}'], FAITHFUL),
        ('a faithful object under a path with a space', right + [str(PORT / 'build/a b' / FAITHFUL)], FAITHFUL),
        ('a faithful object with another suffix', right + [str(HOST / 'lm_single_trig.obj')], '.obj'),
        ('an archive of faithful objects', right + [str(PORT / 'build/libfaithful.a')], 'libfaithful.a'),
        ('an object copied in among the readable ones',
         right + [str(READABLE / 'build/x86-lm/binding' / FAITHFUL)], FAITHFUL),
        ('an object outside the repository', right + ['/tmp/elsewhere/routines.o'], 'routines.o'),
        ('a machine object left out', [p for p in right if not p.endswith('agc_word.o')], 'agc_word.o'),
        ('a readable object left out', [p for p in right if not p.endswith('nvsub.o')], 'nvsub.o'),
        ('a map without the linker\'s LOAD lines', [], 'does not load'),
    )
    failures = 0
    for what, paths, word in cases:
        problems = xr.link_problems(text(paths) if paths else 'no such lines\n', PORT, expected, toolchain)
        ok = not problems if word is None else any(word in p for p in problems)
        failures += not ok
        print(f'{"ok  " if ok else "FAIL"} {what}: '
              + ('passes' if not problems else f'{len(problems)} named, the first: {problems[0]}'))
    print(f'x86-link-check: {len(cases)} made-up link maps, {failures} failures')
    return failures


def real_link(cc):
    """Link the LM program as x86_readable.build does, with FAITHFUL added."""
    out = xr.xb.OUT
    objects = [str(out / f'{source}_lm.o') for source in ('flight_dispatch', 'flight_main')]
    objects += [str(HOST / name) for name in xr.MACHINE] + xr.readable_objects('lm')
    link_map = out / 'x86-link-check.map'
    link = subprocess.run([cc, *objects, str(HOST / FAITHFUL), *xr.xb.LINK, '-Wl,--gc-sections',
                           f'-Wl,-Map={link_map}', '-lm', '-o', str(out / 'x86-link-check')], cwd=PORT,
                          capture_output=True, text=True)
    if link.returncode:
        print(f'FAIL x86-link-check: the link with {FAITHFUL} added did not link:\n{link.stderr[-800:]}')
        return 1
    problems = xr.link_problems(link_map.read_text(), PORT, objects, xr.toolchain_directories(cc))
    ok = len(problems) == 1 and FAITHFUL in problems[0]
    print(f'{"ok  " if ok else "FAIL"} x86-link-check: the LM program linked with {FAITHFUL} added: '
          + ('; '.join(problems) if problems else 'the check passed it'))
    return 0 if ok else 1


def main():
    if '--link' in sys.argv[1:]:
        return real_link(os.environ.get('CC', 'cc'))
    return 1 if made_up() + sources() else 0


if __name__ == '__main__':
    sys.exit(main())
