#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The x86_64 flight program with the readable Executive and WAITLIST.

The faithful port's x86 program (c64-port/docs/X86-FLIGHT.md) links every
routine table of c64-port/src; this builds it with the readable objects
(readable/build/x86-<m>/) in place of the faithful kernel's
(src/lm_kernel.c, src/cm_kernel.c), into readable/build/x86-flight/. The
scenario suite (c64-port/tools/x86_flight_scenarios.py) then flies that
program against the harness, which still runs the faithful kernel: every
row must end with the harness's erasable memory, word for word.

  x86_readable.py build [--host-cc CC]
  x86_readable.py scenarios [--quick] [--jobs N] [NAME ...]

Run by `make x86-flight`, `make x86-scenarios-quick` and `make
x86-scenarios` from readable/. Needs the port's toolchain
(c64-port/tools/setup_cloud.sh) for the harness's yaAGC.
"""
import os, subprocess, sys
from pathlib import Path

READABLE = Path(__file__).resolve().parents[1]
PORT = READABLE.parent / 'c64-port'
sys.path.insert(0, str(PORT / 'tools'))
import x86_flight_build as xb  # noqa: E402

FAITHFUL_KERNEL = ('lm_kernel.o', 'cm_kernel.o')
xb.OUT = READABLE / 'build/x86-flight'


def readable_objects(m):
    objects = sorted((READABLE / f'build/x86-{m}').rglob('*.o'))
    if not objects:
        sys.exit(f'x86_readable: no readable objects in build/x86-{m} (run make from readable/)')
    return [str(o) for o in objects]


def build(cc, m, mode, certified_only=False):
    """x86_flight_build.build, linking the readable kernel for mission m."""
    define, suffix = xb.MODES[mode]
    suffix += '-certified' if certified_only else ''
    flags = xb.flags(m, define, certified_only)
    xb.OUT.mkdir(parents=True, exist_ok=True)
    objects = []
    for source in ('flight_dispatch', 'flight_main'):
        o = xb.OUT / f'{source}_{m}{suffix}.o'
        subprocess.run([cc, *flags, '-c', f'platform/x86/{source}.c', '-o', str(o)], cwd=PORT, check=True)
        objects.append(str(o))
    host = [str(p) for p in sorted((xb.STORE / 'host').glob('*.o')) if p.name not in FAITHFUL_KERNEL]
    exe = xb.OUT / f'x86-flight-{m}{suffix}'
    subprocess.run([cc, *objects, *host, *readable_objects(m), *xb.LINK, '-Wl,--gc-sections', '-lm',
                    '-o', str(exe)], cwd=PORT, check=True)
    # The readable kernel must be the one linked: it defines novac, and the
    # faithful kernel's instruction helpers (k_go and the rest) are absent.
    symbols = subprocess.run(['nm', str(exe)], capture_output=True, text=True, check=True).stdout.split('\n')
    names = {line.split()[-1] for line in symbols if line.strip()}
    if 'novac' not in names or 'k_go' in names:
        sys.exit(f'x86_readable: {exe.name} does not link the readable kernel alone')
    return exe


xb.build = build


def main():
    args = sys.argv[1:]
    if not args or args[0] not in ('build', 'scenarios'):
        sys.exit(__doc__)
    os.chdir(PORT)
    if args[0] == 'build':
        cc = args[args.index('--host-cc') + 1] if '--host-cc' in args else os.environ.get('CC', 'cc')
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', cc],
                       cwd=PORT, check=True)
        for m in ('lm', 'cm'):
            for mode in ('compiled', 'core'):
                print(f'built {build(cc, m, mode)}')
        return 0
    import x86_flight_scenarios as xs
    xs.OUT = xb.OUT / 'scenarios'
    rest = args[1:]
    if '--jobs' not in rest and '-j' not in rest:
        rest = ['--jobs', str(os.cpu_count() or 1)] + rest
    sys.argv = ['x86_flight_scenarios.py'] + rest
    return xs.main()


if __name__ == '__main__':
    sys.exit(main())
