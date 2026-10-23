#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Link the C64 flight-replay harness (platform/c64/dap_pass_check.c, as
tools/flight_scenario_vice.py --replay links it) for one mission and one
profile of replay options, so a change that grows the resident past the ABI 2
service table at $6F00 fails at link time on every push (the c64-maps job
links every build/*.prg) instead of in the next lane's replay.

Nothing runs: the snapshot is a fixed header (tests/dp_replay_link_snap.h,
flight_host_run.c's FLIGHT_SNAPSHOT format) and the REU addresses are
placeholders. Every profile has DP_REPLAY (with an output list, as
--replay-outputs) and RK_RUPT_TRACE (dap_pass_vice.py links it always); then
  busy      DP_BUSY, DP_KEYS (8 keys in the window) and RK_DOWNRUPT: the
            rendezvous, coast and P20 windows (FLIGHT_BUSY_SAMPLE, --keys,
            --downrupt)
  overload  DP_BUSY and RK_STEAL: lm-p63-overload, cm-entry-overload
            (FLIGHT_STEAL)
Every option at once has never fitted (RK_STEAL and DP_KEYS alone are about
330 bytes in the LM), and no scenario turns them all on.

  dp_replay_link.py --cc MOS_CC --mission lm|cm --profile busy|overload --out PRG
"""
import argparse
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from resident_flight_build import COMMON, MISSION, resident_sources  # noqa: E402
import dap_pass_vice as dp  # noqa: E402

SNAP = 'tests/dp_replay_link_snap.h'


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', required=True)
    p.add_argument('--mission', choices=('lm', 'cm'), required=True)
    p.add_argument('--profile', choices=('busy', 'overload'), required=True)
    p.add_argument('--out', type=pathlib.Path, required=True)
    a = p.parse_args()
    m = a.mission
    # (one directory per build: make links the four in parallel)
    work = ROOT / 'build/dp-replay-link' / f'{m}-{a.profile}'
    work.mkdir(parents=True, exist_ok=True)
    # The service table (tools/overlay_v2_services.py), as the flight store has it.
    subprocess.run([sys.executable, 'tools/overlay_v2_services.py', '--out-dir', str(work)], cwd=ROOT, check=True,
                   capture_output=True)
    n = 8
    keys = work / f'{m}-keys.h'
    keys.write_text(f'#define DP_NKEYS {n}u\n'
                    f"static const uint16_t dp_key_cs[{n}]={{{','.join(str(100 * (i + 1)) for i in range(n))}}};\n"
                    f"static const uint16_t dp_key_code[{n}]={{{','.join('021' for _ in range(n))}}};\n")
    defines = ['-DDP_REPLAY', '-DDP_REPLAY_REU=0x100000ul', '-DDP_REPLAY_ROWS=4000u', '-DDP_REPLAY_CS=2000u',
               '-DDP_STATE_FIRST=100u', '-DDP_STATE_TABLE=0x110000ul', '-DDP_END=0x108000ul', '-DDP_STATES_MAX=255u',
               '-DDP_OUTPUTS=0x112000ul', '-DDP_OUTPUTS_MAX=1000u', '-DDP_BUSY=10u']
    if a.profile == 'busy':
        defines += [f'-DDP_KEYS="{keys}"', '-DRK_DOWNRUPT=1']
    else:
        defines += ['-DRK_STEAL=4u', '-DRK_STEAL_FROM=1000ul', '-DRK_STEAL_TO=2000ul']
    out = a.out.resolve()
    subprocess.run([str(c) for c in [
        a.cc, *COMMON, *MISSION[m], '-DRK_RUPT_TRACE', f'-DDP_SNAP="{ROOT / SNAP}"', '-DDP_CS=2500u',
        '-DDP_SCRATCH=0x1f8000ul', f'-DDP_SCRATCH_BYTES={dp.SCRATCH_BYTES}ul', *defines,
        '-T', 'platform/c64/resident_flight.ld', f'-Wl,-Map,{out}.map',
        'platform/c64/dap_pass_check.c', *resident_sources(m), work / 'overlay_v2_services.S', '-o', out]],
        cwd=ROOT, check=True)
    print(f'{out.name}: the {m.upper()} replay harness links with the {a.profile} replay options')


if __name__ == '__main__':
    main()
