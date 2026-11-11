#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The link check of the readable x86 program, tried on links it must fail.

tests/x86_readable.py fails the build of the readable flight program if the
link loaded any file but the readable objects, the step and main, and the
seven machine objects it names (link_problems). A check that never fails
proves nothing, so this gives each part of it something wrong, and requires
that it say so:

  x86_link_check.py          no build needed (in `make all`):
                             made-up link maps: a faithful object by its
                             full path, by a relative path, under a path
                             with a space, with another suffix, in an
                             archive, copied in among the readable objects,
                             outside the repository, and among the C
                             library's files; a machine object left out;
                             and the right map, which must pass.
                             Then the readable sources themselves: an
                             #include can bring in what no link shows, so
                             none may name a C source, and of the files of
                             c64-port/src only the tables of rope words in
                             PORT_DATA. That check too is first tried on
                             made-up lines it must fail (a C source in
                             quotes, in angle brackets, by a path, through
                             a macro, after a comment, after a digraph, on
                             a joined line, by #include_next) and lines it
                             must pass.
  x86_link_check.py --link   after make x86-flight:
                             the real link of the LM program with one
                             faithful object added: the build's own check
                             (stands_alone) must stop it and name that
                             object. An object copied in among the readable
                             ones must not be taken for one of them. And
                             the machine objects are held to a list with
                             one function left out, which must fail, and to
                             the right one, which must pass.

Run by `make x86-link-check` (in `make all`) and by `make x86-flight`, which
the scenario targets run first (CI's gate 2, on every PR).
"""
import os, re, shutil, subprocess, sys
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
# A source as the preprocessor reads its directives: trigraph and digraph
# number signs as #, a line ending in a backslash joined to the next, and
# comments gone (a comment may stand before a directive's #).
COMMENT = re.compile(r'''"(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])*'|/\*.*?\*/|//[^\n]*''', re.S)
INCLUDE = re.compile(r'^[ \t]*#[ \t]*((?:include|import|embed)\w*)[ \t]*(.*)$', re.M)
NAMED = re.compile(r'"([^"]+)"|<([^>]+)>')


def directives(text):
    """text with its lines as the preprocessor sees them."""
    text = text.replace('??=', '#').replace('??/', '\\').replace('\\\n', '')
    text = COMMENT.sub(lambda m: m[0] if m[0][0] in '"\'' else ' ', text)
    return re.sub(r'^([ \t]*)%:', r'\1#', text, flags=re.M)


def port_sources():
    """The names of the port's own files: its sources, tests, tools and
    platforms (its headers in include/ are not among them)."""
    return {f.name for d in ('src', 'tests', 'tools', 'platform') for f in (PORT / d).rglob('*') if f.is_file()}


def include_problems(directory, text, port):
    """The #include lines of one readable source (text, in directory):
    (those that are wrong, the PORT_DATA files it includes). A line may
    name its file in quotes or in angle brackets, or by a macro the same
    file defines as such a name; any other form is wrong, and so is any
    directive that includes but #include. A name is wrong if it is a C
    source, or a file of the port's other than PORT_DATA (unless it is a
    readable file of that name, beside the source)."""
    wrong, seen = [], set()
    text = directives(text)
    for directive, rest in INCLUDE.findall(text):
        rest = rest.strip()
        if directive != 'include':
            wrong.append(f'#{directive} {rest}')
            continue
        named = NAMED.match(rest)
        names = [named[1] or named[2]] if named else []
        quoted = bool(named and named[1])
        if not named and re.fullmatch(r'\w+', rest):   # #include MACRO
            defined = re.findall(rf'^[ \t]*#[ \t]*define[ \t]+{rest}[ \t]+(.*)$', text, re.M)
            literal = [NAMED.fullmatch(d.strip()) for d in defined]
            if defined and all(literal):
                names = [m[1] or m[2] for m in literal]
                quoted = all(m[1] for m in literal)
        if not names:
            wrong.append(f'#include {rest}')
            continue
        for name in names:
            base = Path(name).name
            local = (Path(directory) / name).resolve()
            ours = quoted and local.is_file() and str(local).startswith(str(READABLE) + os.sep)
            if name.endswith('.c') or (not ours and base in port and base not in PORT_DATA):
                wrong.append(name)
            elif not ours and base in PORT_DATA:
                seen.add(base)
    return wrong, seen


def made_up_includes(port):
    """include_problems on lines it must fail, and on lines it must pass."""
    directory = READABLE / 'src/binding'
    other = sorted(n for n in port if n.endswith('.inc') and n not in PORT_DATA)[0]
    cases = (
        ('a C source in quotes', '#include "lm_single_trig.c"\n', 'lm_single_trig.c'),
        ('a C source in angle brackets', '#include <lm_single_trig.c>\n', 'lm_single_trig.c'),
        ('a C source by its path', '#include "../../../c64-port/src/lm_single_trig.c"\n', 'lm_single_trig.c'),
        ('a C source after spaces', '  #  include   <agc_transfer.c>\n', 'agc_transfer.c'),
        ('a C source through a macro', '#define ROUTINES "lm_single_trig.c"\n#include ROUTINES\n',
         'lm_single_trig.c'),
        ('a macro another file defines', '#include ROUTINES\n', 'ROUTINES'),
        ('a macro that is not a file name', '#define ROUTINES lm_single ## _trig.c\n#include ROUTINES\n', 'ROUTINES'),
        ('a C source after a comment', '/* SPSIN */ #include "lm_single_trig.c"\n', 'lm_single_trig.c'),
        ('a C source after a digraph', '%:include "lm_single_trig.c"\n', 'lm_single_trig.c'),
        ('a C source on a joined line', '#include \\\n"lm_single_trig.c"\n', 'lm_single_trig.c'),
        ('a C source by #include_next', '#include_next <lm_single_trig.c>\n', 'lm_single_trig.c'),
        ('another table of c64-port/src', f'#include "{other}"\n', other),
        ('the C library, the port\'s headers, a table of rope words, the file\'s own table',
         '#include <stddef.h>\n#include "agc_transfer.h"\n#include "lm_phase_sites.inc"\n'
         '#define KEYS "p34_keys.inc"\n#include KEYS\n#include "binding.h"\n'
         '/* not this: #include "lm_single_trig.c"\n */\nstatic const char *line = "\\n#include <x.c>";\n', None),
    )
    failures = 0
    for what, text, word in cases:
        wrong, _ = include_problems(directory, text, port)
        ok = not wrong if word is None else any(word in w for w in wrong)
        failures += not ok
        print(f'{"ok  " if ok else "FAIL"} {what}: ' + (f'named: {"; ".join(wrong)}' if wrong else 'passes'))
    print(f'x86-link-check: {len(cases)} made-up sources, {failures} failures')
    return failures


def sources(port):
    """No readable source includes a C source or, of c64-port/src, any file
    but PORT_DATA."""
    files = [f for f in sorted((READABLE / 'src').rglob('*')) if f.suffix in ('.c', '.h', '.inc')]
    wrong, seen = [], set()
    for f in files:
        bad, data = include_problems(f.parent, f.read_text(), port)
        wrong += [f'{f.relative_to(READABLE)} includes {name}' for name in bad]
        seen |= data
    ok = not wrong and seen == PORT_DATA
    print(f'{"ok  " if ok else "FAIL"} the readable sources ({len(files)} files) include no C source and, of '
          f'c64-port/src, only the {len(PORT_DATA)} tables of rope words named'
          + ('' if ok else ': ' + '; '.join(wrong[:5] or [f'not all of them: {sorted(PORT_DATA - seen)}'])))
    return 0 if ok else 1


def made_up():
    toolchain = ['/usr/lib/x86_64-linux-gnu', '/usr/lib/gcc/x86_64-linux-gnu/13']
    readable = [str(xr.cb.BUILD / 'x86-lm' / name) for name in ('agc/agc.o', 'pinball/nvsub.o')]
    step = [str(xr.cb.BUILD / 'x86-flight' / name) for name in ('flight_dispatch_lm.o', 'flight_main_lm.o')]
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
         right + [str(xr.cb.BUILD / 'x86-lm/binding' / FAITHFUL)], FAITHFUL),
        ('an object outside the repository', right + ['/tmp/elsewhere/routines.o'], 'routines.o'),
        ('an object among the C library\'s files', right + [f'/usr/lib/x86_64-linux-gnu/{FAITHFUL}'], FAITHFUL),
        ('an archive among the C library\'s files', right + ['/usr/lib/x86_64-linux-gnu/libfaithful.a'],
         'libfaithful.a'),
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
    """Link the LM program as x86_readable.build does, with FAITHFUL added:
    the build's own check, stands_alone, must stop it and name FAITHFUL."""
    out = xr.xb.OUT
    objects = [str(out / f'{source}_lm.o') for source in ('flight_dispatch', 'flight_main')]
    objects += [str(HOST / name) for name in xr.MACHINE] + xr.readable_objects('lm')
    link_map = out / 'x86-link-check.map'
    exe = out / 'x86-link-check'
    link = subprocess.run([cc, *objects, str(HOST / FAITHFUL), *xr.xb.LINK, '-Wl,--gc-sections',
                           f'-Wl,-Map={link_map}', '-lm', '-o', str(exe)], cwd=PORT,
                          capture_output=True, text=True)
    if link.returncode:
        print(f'FAIL x86-link-check: the link with {FAITHFUL} added did not link:\n{link.stderr[-800:]}')
        return 1
    try:
        xr.stands_alone(exe, link_map, objects, cc)
        said = ''
    except SystemExit as stop:
        said = str(stop.code)
    ok = said.count('loads ') == 1 and FAITHFUL in said and 'does not load' not in said
    print(f'{"ok  " if ok else "FAIL"} x86-link-check: the LM program linked with {FAITHFUL} added: '
          + (' '.join(said.split()) if said else 'the check passed it'))
    return 0 if ok else 1


def stray_object():
    """An object that lies among the readable ones is not one of them: the
    readable objects are those the Makefile names."""
    stray = xr.cb.BUILD / 'x86-lm/binding' / FAITHFUL
    shutil.copyfile(HOST / FAITHFUL, stray)
    try:
        listed = xr.readable_objects('lm')
    finally:
        stray.unlink()
    ok = str(stray) not in listed and len(listed) > 100
    print(f'{"ok  " if ok else "FAIL"} x86-link-check: {FAITHFUL} copied in among the readable objects: '
          + ('not taken for one of them' if ok else 'taken for a readable object'))
    return 0 if ok else 1


def machine():
    """machine_problems passes the machine objects as listed, and fails
    them held to a list with one function left out, naming the object."""
    short = dict(xr.MACHINE)
    name = 'agc_word.o'
    short[name] = tuple(f for f in short[name] if f != 'agc_negate')
    right, wrong = xr.machine_problems(), xr.machine_problems(short)
    ok = not right and len(wrong) == 1 and name in wrong[0] and 'agc_negate' in wrong[0]
    print(f'{"ok  " if ok else "FAIL"} x86-link-check: the machine objects held to a list without agc_negate: '
          + ('; '.join(wrong) if wrong else 'passed') + ('' if not right else f'; as listed: {"; ".join(right)}'))
    return 0 if ok else 1


def main():
    if '--link' in sys.argv[1:]:
        return 1 if real_link(os.environ.get('CC', 'cc')) + stray_object() + machine() else 0
    port = port_sources()
    return 1 if made_up() + made_up_includes(port) + sources(port) else 0


if __name__ == '__main__':
    sys.exit(main())
