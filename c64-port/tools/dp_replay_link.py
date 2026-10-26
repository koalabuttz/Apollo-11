#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Link the C64 flight-replay harness (platform/c64/dap_pass_check.c, as
tools/flight_scenario_vice.py links it) for one mission and one profile of
options, so a change that grows the resident past the ABI 2 service table
at $6F00 fails at link time on every push (the c64-maps job links every
build/*.prg) instead of in the next lane's run.

Nothing runs: the snapshot is a fixed header (tests/dp_replay_link_snap.h,
flight_host_run.c's FLIGHT_SNAPSHOT format) and the REU addresses are
placeholders. The link is tools/dap_pass_vice.py's, with the replay options
from tools/flight_scenario_vice.py (replay_defines, steal_flags,
DOWNRUPT_DEFINES), so the two cannot drift apart. Profiles:
  plain     no replay: tools/flight_scenario_vice.py without --replay, and
            tools/dap_pass_vice.py
  busy      DP_REPLAY (with an output list, as --replay-outputs), DP_BUSY,
            DP_KEYS (8 keys in the window) and RK_DOWNRUPT: the rendezvous,
            coast and P20 windows (FLIGHT_BUSY_SAMPLE, --keys, --downrupt)
  overload  DP_REPLAY, DP_BUSY and RK_STEAL, without an output list:
            lm-p63-overload, lm-p63-overload-nomon, cm-entry-overload
            (FLIGHT_STEAL; no FLIGHT_STEAL run takes --replay-outputs, and
            the LM's output list does not fit beside RK_STEAL)
Every option at once has never fitted (RK_STEAL and DP_KEYS alone are about
330 bytes in the LM), and no scenario turns them all on.

  dp_replay_link.py --cc MOS_CC --mission lm|cm --profile plain|busy|overload --out PRG
"""
import argparse
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import dap_pass_vice as dp  # noqa: E402
import flight_scenario_vice as fsv  # noqa: E402

SNAP = 'tests/dp_replay_link_snap.h'


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', required=True)
    p.add_argument('--mission', choices=('lm', 'cm'), required=True)
    p.add_argument('--profile', choices=('plain', 'busy', 'overload'), required=True)
    p.add_argument('--out', type=pathlib.Path, required=True)
    a = p.parse_args()
    m = a.mission
    # (one directory per build: make links them in parallel)
    work = ROOT / 'build/dp-replay-link' / f'{m}-{a.profile}'
    work.mkdir(parents=True, exist_ok=True)
    # The service table (tools/overlay_v2_services.py), as the flight store has it.
    subprocess.run([sys.executable, 'tools/overlay_v2_services.py', '--out-dir', str(work)], cwd=ROOT, check=True,
                   capture_output=True)
    keys, defines = None, []
    if a.profile != 'plain':
        outputs = 1000 if a.profile == 'busy' else 0
        defines = fsv.replay_defines(0x100000, 4000, 2000, 100, 0x110000, 0x108000, 255, 10, 0x112000, outputs)
    if a.profile == 'busy':
        keys = fsv.keys_header(work / f'{m}-keys.h', [(100 * (i + 1), 0o21) for i in range(8)])
        defines += fsv.DOWNRUPT_DEFINES
    elif a.profile == 'overload':
        defines += fsv.steal_flags(4, 1000, 2000)
    out = a.out.resolve()
    dp.link(a.cc, out.stem, ROOT / SNAP, 2500, 0x1f8000, m=m, keys=keys, defines=defines,
            services=work / 'overlay_v2_services.S', out=out)
    print(f'{out.name}: the {m.upper()} replay harness links with the {a.profile} options')


if __name__ == '__main__':
    main()
