#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compare real queued-mode preemption and resumed P66 against original AGC."""
import argparse
import hashlib
import itertools
import lzma
import os
from pathlib import Path
import subprocess
from p66_application_port_host import inventories

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--retained', action='store_true')
    parser.add_argument('--adoption', action='store_true')
    parser.add_argument('--without-preemption', action='store_true')
    args = parser.parse_args()
    assert not (args.adoption and args.without_preemption)
    _, sources, inputs, defines = inventories()[2]
    inputs = inputs + ['tests/p66_recurrence_live_cases.c']
    extra = ['src/p66_length_return_control.c', 'src/p66_application_recurrence.c',
             'src/lm_clock.c', 'src/lm_idle_start.c', 'src/lm_taskover_resume.c',
             'src/lm_waitlist_dispatch.c', 'src/lm_waitlist_irq_dispatch.c',
             'src/p66_rodtask_image.c', 'src/p66_application_split.c',
             'src/p66_application_pending.c', 'src/p66_elapsed_return_control.c',
             'src/lm_danzig_change.c', 'src/lm_interpreter_change.c']
    flags = ['cc', '-std=c99', '-O1' if args.sanitize else '-O2',
             '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests',
             '-ffunction-sections', '-fdata-sections', '-Wl,--gc-sections']
    if args.sanitize:
        flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
    if args.without_preemption:
        flags.append('-DP66_PENDING_NO_INJECTION')
    name = ('p66-pending-compatibility-' if args.without_preemption else
            'p66-pending-adoption-' if args.adoption else 'p66-pending-flight-') + ('sanitize' if args.sanitize else 'native')
    subprocess.run(flags + defines + list(dict.fromkeys(sources + extra + inputs)) +
                   ['tools/p66_pending_adoption_snapshot.c' if args.adoption else
                    'tools/p66_pending_flight_snapshot.c', '-o', 'build/' + name],
                   cwd=ROOT, check=True)
    archive = ROOT / ('evidence' if args.retained else 'build') / \
        'p66-pending-flight-reference.snapshots.xz'
    if args.without_preemption:
        archive = ROOT / 'evidence/p66-recurrence-live-source.snapshots.xz'
    if not args.sanitize and not args.retained and not args.adoption and not args.without_preemption:
        engine = ROOT / 'deps/virtualagc/yaAGC'
        subprocess.run(flags + ['-DP66_NATIVE_DISPATCH_ONLY', '-I' + str(engine),
            'tools/p66_pending_flight_reference.c', 'tests/scenarios.c',
            'src/agc_word.c', 'src/agc_multiword.c', 'src/executive.c',
            'src/p66_native.c'] + inputs + ['build/agc_engine_hardware_dv.o',
            str(engine / 'agc_engine_init.o'), str(engine / 'rfopen.o'),
            '-o', 'build/p66-pending-flight-reference'], cwd=ROOT, check=True)
        with lzma.open(archive, 'wb', preset=1) as output:
            process = subprocess.Popen(['build/p66-pending-flight-reference',
                                        'deps/fixture/MAIN.agc.bin'],
                                       cwd=ROOT, stdout=subprocess.PIPE)
            try:
                for line in process.stdout:
                    output.write(line)
                assert process.wait() == 0
            finally:
                if process.poll() is None:
                    process.kill()
                    process.wait()
                process.stdout.close()
    digest = hashlib.sha256()
    errors = []
    with lzma.open(archive, 'rb') as expected:
        process = subprocess.Popen(['build/' + name], cwd=ROOT,
            stdout=subprocess.PIPE, env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))
        try:
            selected = (line for line in expected if int(line.split(maxsplit=2)[1]) < 4) if args.adoption else expected
            for count, (left, right) in enumerate(itertools.zip_longest(selected,
                                                   process.stdout), 1):
                if left:
                    digest.update(left)
                if left != right and len(errors) < 12:
                    x = list(map(int, left.split())) if left else []
                    y = list(map(int, right.split())) if right else []
                    errors.append((count, x[:3], y[:3], [(oct(i - 3), oct(a), oct(b))
                        for i, (a, b) in enumerate(zip(x, y)) if a != b]))
            status = process.wait()
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
            process.stdout.close()
    if status or errors:
        raise SystemExit('native exit %s, source mismatches %r' % (status, errors))
    if args.adoption:
        assert count == 64
        print('PASS:16 first-legal CHANG2 adoptions;64 complete2053-word original '
              'states, enabled/masked guard restoration, stale callback lookup '
              'and arriving IRQ ticket retained; SHA256', digest.hexdigest(), flush=True)
        return
    if args.without_preemption:
        assert count == 2460 and digest.hexdigest() == 'd1aa5f4de9e02bf126731818227c4bc7592e62bddbbd66a425cc56346683f186'
        print('PASS:2460 unchanged complete original recurrence states with '
              'additive resolver, elapsed observer and returned view stages; '
              'SHA256', digest.hexdigest(), flush=True)
        return
    print('PASS:16 original queued-mode→first legal CHANG2→mode END→P66 '
          'resumption→display→idle→T3/RODTASK recurrence paths;', count,
          'complete2053-word source states; SHA256', digest.hexdigest(), flush=True)


if __name__ == '__main__':
    main()
