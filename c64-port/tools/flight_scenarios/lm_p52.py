#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""LM P52 IMU realign scenario (docs/FLIGHT-STORE.md, "Flight scenarios"):
the attitudes the crew holds for the two AOT sightings of a P52 option 3
(REFSMMAT) fine align, from Apollo 11's own LGC state.

The run starts from tools/scenarios/luminary099-p63-ignition.core (the
NASSP core about ten minutes before PDI, 102:25 GET; FLIGHT_CORE), whose
last P52 sighted stars 18 (Regulus) and 20 (Gienah) in the forward AOT
detent: BESTI 108 and BESTJ 120, STARCODE 0224. The crew keys V37E52E,
option 3 and ENTER on V50N25 00015 (no star pick: R52 offers BESTI and
BESTJ), and for each star skips the auto maneuver (ENTER on V50N18) and
holds the attitude that puts the star on the detent's optic axis. This
script finds those attitudes: the smallest rotation from the core's
attitude that turns the forward detent's optic axis (azimuth 0, elevation
45 degrees, as OANB computes it from AOTAZ and AOTEL) onto the star in
stable member coordinates (REFSMMAT times the catalog vector), as CDU
angles (the outer, inner and middle gimbals, CDUX, CDUY and CDUZ).

  lm_p52.py   prints the FLIGHT_POKE entries for the two holds
"""
import math
from pathlib import Path

from sighting import catalog, cdu_angle, cdu_word, dp, gimbals, mm, mv, optic_axis, rotation, smnb, unit

ROOT = Path(__file__).resolve().parents[2]
LST = ROOT / 'build/fixed-data/Luminary099/assembly.lst'
CORE = ROOT / 'tools/scenarios/luminary099-p63-ignition.core'
STARS = (18, 20)
REFSMMAT = 0o1733
AOTAZ, AOTEL = 0o3404, 0o3412
DETENT = 2


def main():
    words = open(CORE).read().split()
    E = [int(w, 8) for w in words[512:512 + 2048]]
    refs = [[dp(E[REFSMMAT + 6 * r + 2 * c], E[REFSMMAT + 6 * r + 2 * c + 1], 1) for c in range(3)] for r in range(3)]
    az, el = cdu_angle(E[AOTAZ + DETENT - 1]), cdu_angle(E[AOTEL + DETENT - 1])
    oa = optic_axis(az, el)[0]
    cat = catalog(LST)
    att = smnb(cdu_angle(E[0o32]), cdu_angle(E[0o33]), cdu_angle(E[0o34]))
    print('core CDUX %05o CDUY %05o CDUZ %05o; detent %d optic axis NB %s'
          % (E[0o32], E[0o33], E[0o34], DETENT, [round(x, 4) for x in oa]))
    for code in STARS:
        sm = unit(mv(refs, cat[code]))
        new = mm(rotation(unit(mv(att, sm)), oa), att)
        x, y, z = gimbals(new)
        check = mv(smnb(x, y, z), sm)
        err = math.degrees(math.acos(max(-1.0, min(1.0, sum(a * b for a, b in zip(check, oa))))))
        print('star %d: SM %s; hold CDUX %05o CDUY %05o CDUZ %05o (axis error %.4f deg)'
              % (code, [round(c, 5) for c in sm], cdu_word(x), cdu_word(y), cdu_word(z), err))


if __name__ == '__main__':
    main()
