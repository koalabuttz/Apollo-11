#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Generate make targets for the legacy tools/*_vice_test.py scripts.

The serial loop ran these VICE checks by hand, so CI never did. This writes
tools/mk/legacy-vice.mk with one `vice-legacy-<name>` target per script, which
tools/ci_run.py can schedule like any other target: when a script stops on a
missing build/ input, ci_run finds the c64 rule or differential that writes it.

Scripts with required selectors (--phase, --mission, --stage, ...) run once
per sampled value: the first choice (both missions where there are two) or 0.

  python3 tools/legacy_vice.py            # rewrite tools/mk/legacy-vice.mk
  python3 tools/legacy_vice.py --check    # fail if the file is stale
  python3 tools/legacy_vice.py --shard 2/8   # print shard 2 of 8 of the targets
"""
import argparse
import pathlib
import re
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUTPUT = ROOT / 'tools/mk/legacy-vice.mk'
# Arguments every script gets from the environment CI sets up (deps/env.sh).
FIXED = {'--vice': '"$(VICE)"', '--rom-dir': '"$(VICE_ROM_DIR)"', '--data-dir': '"$(VICE_DATA_DIR)"',
         '--cc': '"$(MOS_CC)"', '--mos-cc': '"$(MOS_CC)"', '--c1541': '"$(C1541)"'}
# Scripts their milestone records as unqualified development trials, not
# tests: they never passed, and a staged script carries the qualified C64
# evidence instead. Listed in the generated file, not run.
TRIALS = {
    'cm_ephemeris_solar_vice_test.py':
        'monolithic trial that never fit in RAM; docs/NATIVE-CM-EPHEMERIS-SOLAR.md and '
        'evidence/cm-ephemeris-solar-prequalification/; qualified by cm_ephemeris_solar_phases',
}
USAGE_ARG = re.compile(r'(\[)?(--[a-z0-9-]+)(?: (\{[^}]*\}|[A-Z_][A-Z0-9_]*))?\]?')


def scripts():
    return sorted(p for p in (ROOT / 'tools').glob('*_vice_test.py') if p.name not in TRIALS)


def usage(script):
    """The argparse usage block, which marks optional arguments with [...]."""
    proc = subprocess.run([sys.executable, str(script), '--help'], cwd=ROOT, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    text = proc.stdout
    block = text.split('\n\n', 1)[0] if text.startswith('usage:') else ''
    return ' '.join(block.split())


def variants(script, only=None):
    """Argument lists to run: fixed tool paths plus sampled required selectors.

    `only` maps selector names to the values to use (a build script run before
    its test takes the test's values for the selectors both accept); selectors
    it does not name are left out."""
    fixed, selectors = [], []
    for optional, name, value in USAGE_ARG.findall(usage(script)):
        if name == '--help':
            continue
        if name in FIXED:
            fixed += [name, FIXED[name]]
        elif not optional and (only is None or name in only):
            choices = value[1:-1].split(',') if value.startswith('{') else None
            if only is not None:
                picked = [only[name]]
            elif choices:
                picked = choices[:2] if name == '--mission' else choices[:1]
            else:
                picked = ['0', '1'] if name == '--mission' else ['0']
            selectors.append([[name, v] for v in picked])
    combos = [[]]
    for options in selectors:
        combos = [c + o for c in combos for o in options]
    return [fixed + c for c in combos]


def selectors(args):
    """The selector values in one argument list, by name."""
    return {name: value for name, value in zip(args, args[1:])
            if name.startswith('--') and name not in FIXED}


def producers():
    """Map each build/*.prg that a make rule writes to how to run that rule.

    Main-Makefile rules become prerequisites; rules in tools/*.mk files the
    Makefile does not include run as `$(MAKE) -f tools/<x>.mk <rule>`."""
    sys.path.insert(0, str(ROOT / 'tools'))
    import ci_affected
    found = {}
    for name, (_, recipe) in ci_affected.database().items():
        if re.fullmatch(r'build/[A-Za-z0-9_.-]+\.prg', name):
            found.setdefault(name, ('dep', name))
        for prg in re.findall(r'-o\s*"?(build/[A-Za-z0-9_.-]+\.prg)', recipe):
            found.setdefault(prg, ('dep', name))
    included = set(re.findall(r'^-?include\s+(tools/\S+\.mk)', (ROOT / 'Makefile').read_text(), re.M))
    for mk in sorted((ROOT / 'tools').glob('*.mk')):
        if mk.relative_to(ROOT).as_posix() in included:
            continue
        rule = None
        for line in mk.read_text(errors='replace').splitlines():
            head = re.match(r'([A-Za-z0-9_./-]+)\s*:(?!=)', line)
            if head:
                rule = head[1]
            elif rule and line.startswith('\t'):
                for prg in re.findall(r'-o\s*"?(build/[A-Za-z0-9_.-]+\.prg)', line):
                    found.setdefault(prg, ('submake', f'$(MAKE) -f {mk.relative_to(ROOT).as_posix()} {rule}'))
    return found


def prerequisites(script, known):
    """Make prerequisites and recipe lines that build the PRGs the script reads."""
    c64 = 'c64-' + script.name[:-len('_vice_test.py')].replace('_', '-')
    if ('dep', c64) in known.values():
        return [c64], []
    deps, lines = [], []
    for prg in sorted(set(re.findall(r'[A-Za-z0-9_.-]+\.prg', script.read_text(errors='replace')))):
        kind, how = known.get(f'build/{prg}', (None, None))
        if kind == 'dep' and how not in deps:
            deps.append(how)
        elif kind == 'submake' and how not in lines:
            lines.append(how)
    return deps, lines


def proof_builder(script):
    """tools/<name>_build.py, or the build script the milestone's proof records.

    Some PRGs are built by a shared script under another name
    (native_source_adoption_build.py writes native-source-dispatch.prg);
    the proof manifest's commands name it."""
    builder = script.with_name(script.name.replace('_vice_test.py', '_build.py'))
    if builder.is_file():
        return builder
    stem = script.name[:-len('_vice_test.py')].replace('_', '-')
    proof = ROOT / 'evidence' / f'{stem}-proof.json'
    if proof.is_file():
        found = re.findall(r'tools/([a-z0-9_]+_build\.py)', proof.read_text(errors='replace'))
        if found and (ROOT / 'tools' / found[0]).is_file():
            return ROOT / 'tools' / found[0]
    return builder


def target(script):
    return 'vice-legacy-' + script.name[:-len('_vice_test.py')].replace('_', '-')


def render():
    lines = ['# SPDX-License-Identifier: MIT',
             '# Generated by tools/legacy_vice.py; do not edit. One target per legacy',
             '# tools/*_vice_test.py script, run by the nightly c64-legacy job.']
    lines += [f'# not run: tools/{name}: {why}' for name, why in sorted(TRIALS.items())]
    names, known = [], producers()
    for script in scripts():
        name = target(script)
        names.append(name)
        needs, steps = prerequisites(script, known)
        builder = proof_builder(script)
        lines.append(f'{name}: ' + ' '.join(needs + ['|', 'build']))
        lines.append('\tmkdir -p build/vice-cache/vice build/vice-config/vice')
        lines += ['\t' + step for step in steps]
        for args in variants(script):
            if not needs and not steps and builder.is_file():
                build_args = variants(builder, selectors(args))[0]
                lines.append(f'\tpython3 tools/{builder.name} ' + ' '.join(build_args))
            lines.append(f'\tpython3 tools/{script.name} ' + ' '.join(args))
    lines.insert(3 + len(TRIALS), '.PHONY: ' + ' '.join(names))
    return '\n'.join(lines) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--shard', help='K/N: print every Nth target starting at K (0-based)')
    args = parser.parse_args()
    if args.shard:
        index, count = map(int, args.shard.split('/'))
        print(' '.join(target(s) for s in scripts()[index::count]))
        return 0
    text = render()
    if args.check:
        if not OUTPUT.is_file() or OUTPUT.read_text() != text:
            print('FAIL: tools/mk/legacy-vice.mk is stale; run python3 tools/legacy_vice.py', file=sys.stderr)
            return 1
        print('PASS: tools/mk/legacy-vice.mk matches', len(scripts()), 'scripts')
        return 0
    OUTPUT.write_text(text)
    print('wrote', OUTPUT.relative_to(ROOT), 'with', len(scripts()), 'targets')
    return 0


if __name__ == '__main__':
    sys.exit(main())
