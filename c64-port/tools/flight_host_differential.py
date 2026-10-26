#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Whole-program host differential (docs/FLIGHT-STORE.md): build
tools/flight_host_run.c for both missions over every lane's routine tables
(tools/flight_store_build.py --host-only), run each scenario against yaAGC
and write the logs, and the unregistered or refused points by file and lane
(tools/flight_holes.py), to build/flight-store/differential/ (not
milestone evidence: phase 1 adds no certificate).

Scaled time (docs/RESIDENT-FLIGHT.md): each SCALED scenario runs with
FLIGHT_SCALED, yaAGC's clock frozen and the AGC clock driven by a modeled
C64 tick schedule (x1, x0.5, forced overrun); every routine must match
yaAGC under that schedule.

  flight_host_differential.py [--host-cc CC] [--skip-index]
"""
import argparse, json, os, re, resource, subprocess, sys, threading
from pathlib import Path

# lm-p63-nassp's host run overflows the default 8 MB stack: raise the soft
# limit to the hard one, which every host flight run (a child) inherits.
_soft, _hard = resource.getrlimit(resource.RLIMIT_STACK)
if _soft != resource.RLIM_INFINITY and (_hard == resource.RLIM_INFINITY or _hard > _soft):
    resource.setrlimit(resource.RLIMIT_STACK, (_hard, _hard))

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_holes  # noqa: E402

YAAGC = ROOT / 'deps/virtualagc/yaAGC'
OUT = ROOT / 'build/flight-store'
EVIDENCE = OUT / 'differential'
ROPE = {'lm': 'evidence/fixed-data-lm-original.rope.bin', 'cm': 'evidence/fixed-data-cm-original.rope.bin'}
# (mission, centiseconds, DSKY keys)
SCENARIOS = [('lm', 4000, ''), ('cm', 4000, ''),
             ('lm', 3000, 'V37E00E'), ('cm', 3000, 'V37E00E'),
             ('lm', 3000, 'V37E12E'), ('cm', 3000, 'V37E06E'), ('lm', 3000, 'V37E63E')]
# Flight scenarios (tools/flight_scenarios/): (name, mission, centiseconds,
# keys, inputs), each run under scaled time x1 with no swap cost
# (FLIGHT_SWAP_MCT=0, the DAP pinned): unscaled, with the IMU operating,
# yaAGC's TIME3 ticks between WAITLIST's two reads of it, which no native
# routine can match; with 20 ms swaps the DAP starves the CPU and no key is
# taken. lm-p63-descent: the IMU operating, PGNCS, AUTO and auto throttle,
# the LR antenna in position 1, the P63 load at 20 s; V37E63E, proceed on
# the TIG display (V06N61), ENTER on fine align (R51), proceed and then
# ENTER on the attitude maneuver (R60), proceed on V99N62; ignition at
# 146 s, the LLGE every 2 s from 172 s. No engine model: the PIPAs stay
# still, so DVMON flashes V97 (thrust fail) while guidance runs.
# lm-p63-descent-thrust: the same with FLIGHT_ENGINE (a 15,100 kg LM, the
# descent engine thrusting along the DAP's desired attitude), so DVMON
# passes and the LLGE throttles and steers the braking phase (on yaAGC it
# reaches P64 at 787 s and 4,000 ft at 900 s).
P63 = {'FLIGHT_CH30': '036357', 'FLIGHT_CH31': '057777', 'FLIGHT_CH33': '077737',
       'FLIGHT_LOAD': '2000:tools/flight_scenarios/lm-p63.load', 'FLIGHT_SWAP_MCT': '0'}
P63_KEYS = '@2200V37E63E@3400V33E@3800E@4400V33E@9600E@14300V33E'
# cm-entry: the IMU operating, the entry load at 20 s (EI at 170 s);
# V37E61E, proceed on P61's N61, N60 and N63, on P62's separation request
# (V50N25 00041) and its N61; FLIGHT_AERO (a 355 kg/m^2 capsule) gives the
# drag, so P63 senses 0.05 g and P64 and P67 guide the roll to the splash
# point (on yaAGC: P64 at 204 s, P67 at 306 s, peak 6.7 g, drogue
# conditions at 470 s about 1 nm from the target).
# lm-p63-nassp: Apollo 11's own P63 state (FLIGHT_CORE, the NASSP core of
# tools/scenarios/luminary099-p63-ignition.core, 102:25 GET) with AUTO, the
# LR antenna in position 1 (moving to position 2 5 s after LRPOS2 commands
# it, FLIGHT_LR_POS2) and the descent engine model: V37E63E, proceed on the
# TIG display, ENTER through R51 and R60, proceed on V99N62; the LLGE flies
# the braking phase to P64 at about 1,118 s (yaAGC alone the same).
NASSP = {'FLIGHT_CORE': 'tools/scenarios/luminary099-p63-ignition.core', 'FLIGHT_CH31': '057777',
         'FLIGHT_CH33': '036733', 'FLIGHT_LR_POS2': '500', 'FLIGHT_SWAP_MCT': '0', 'FLIGHT_ENGINE': '15100'}
NASSP_KEYS = '@200V37E63E@6000V33E@6400E@6800E@7200E@7600E@64500V33E'
# lm-p52-realign: P52 option 3 (REFSMMAT) fine align from the same
# Apollo 11 core (NASSP, 102:25 GET), on stars 18 (Regulus) and 20
# (Gienah) in the forward AOT detent, the pair the core's own last P52
# sighted (BESTI 108, BESTJ 120). V37E52E, option 3 (V22E3E) and proceed
# on V04N06, ENTER on V50N25 00015 (no star pick), proceed on V01N70; for
# each star ENTER on V50N18 (no auto maneuver) with the attitude that puts
# it on the detent's optic axis held (FLIGHT_POKE of CDUX-CDUZ,
# tools/flight_scenarios/lm_p52.py), proceed on V01N71, a Y and an X mark
# (channel 16, KEYRUPT2) and proceed on V54N71; proceed on the star angle
# difference (V06N05, 0.00) and the torquing angles (V06N93, a few
# thousandths of a degree), ENTER on V50N25 00014; P52 ends in V37.
LM_P52 = {'FLIGHT_CORE': 'tools/scenarios/luminary099-p63-ignition.core', 'FLIGHT_CH31': '057777',
          'FLIGHT_CH33': '036673', 'FLIGHT_SWAP_MCT': '0',
          'FLIGHT_POKE': '3300:032=01062,3300:033=33476,3300:034=01144,5800:032=03521,5800:033=42475,5800:034=77717'}
LM_P52_KEYS = ('@200V37E52E@1000V22E3E@1500V33E@2200E@2600V33E@3400E@3800V33E@4300y@4400x@4800V33E'
               '@5300V33E@5900E@6300V33E@6800y@6900x@7300V33E@7900V33E@8500V33E@9100E')
# lm-landing-ascent-nassp: lm-p63-nassp on to the surface and back to orbit.
# FLIGHT_ENGINE_LAG: the throttle pulses at 3,200 a second and the thrust
# lags them 0.2 s (P66's loop needs both). P64 at about 1,118 s; at 1,225 s
# (about 140 m, 4.5 m/s down) the crew goes to attitude hold (channel 31
# bit 13 cleared) and clicks the ROD switch (M: channel 16 bit 6, +1 ft/s a
# click), so P66 starts and holds the sink rate; six clicks at 1,230 s and
# four at 1,250 s slow it to 0.8 m/s, and the crew tilts the LM to null the
# 34 m/s over the surface (FLIGHT_PILOT, 8 s) by 1,280 s. Touchdown at
# 1,344 s (FLIGHT_TOUCHDOWN: contact stops the LM, the surface holds it),
# AUTO again at 1,349 s. V37E68E at 1,360 s: P68 stores RLS and flashes
# V06N43; PRO (channel 32 bit 14) at 1,410 s, GOTOPOOH; V37E00E at
# 1,420 s: P00 on the surface. V37E12E at 1,480 s: P12 flashes V06N33 and
# the crew loads TIG 102:53:00 on the AGC clock (V25E; the flight's was
# 124:22) and proceeds, proceeds on V06N76, and on V99 at TIG - 5 s; the
# ascent engine (FLIGHT_ASCENT, 4,888 kg at liftoff) burns about 7 min
# 15 s to orbit insertion: 1,679 m/s, about 18 km up, climbing 6 m/s.
LANDING = {**NASSP, 'FLIGHT_ENGINE_LAG': '0.2', 'FLIGHT_PILOT': '8', 'FLIGHT_TOUCHDOWN': '134400',
           'FLIGHT_ASCENT': '4888',
           'FLIGHT_CHAN': '122500:031=067777,134900:031=057777,141000:032=057777,141100:032=077777'}
LANDING_KEYS = (NASSP_KEYS + '@122700M@123000MMMMMM@125000MMMM@136000V37E68E@142000V37E00E'
                '@148000V37E12E@150000V25E+00102E+00053E+00000E@153000V33E@155000V33E@162500V33E')
# lm-p63-lr-update: lm-p63-nassp with the landing radar from 600 s, the
# surface 150 m above the LGC's landing site (FLIGHT_LR=60000:150), and
# V57E at 660 s (LR updates permitted). From 816 s R12's altitude updates
# take HCALC from about 186 m above the true height to about 77 m by
# 955 s (HCALC lags the truth by 40-60 m here, a sample apart, with no
# bias). From 1,005 s SERVICER also reads the LR velocity (READV, LRVJOB)
# after each altitude read; it runs to 1,100 s, 50 velocity reads, and
# HMEAS is never zeroed (the interpretive words run with per-word
# interrupt and NEWJOB points, #739).
LR_UPDATE = {**NASSP, 'FLIGHT_LR': '60000:150'}
LR_UPDATE_KEYS = NASSP_KEYS + '@66000V57E'
# lm-p65-autoland-nassp: lm-p63-nassp on to the surface with no crew
# takeover. P64 at about 1,118 s; at 1,278 s P64's guidance reaches its
# low gate and starts P65, which holds the LM vertical and descends at
# about 1 m/s. Touchdown at 1,324.8 s (FLIGHT_TOUCHDOWN: the engine
# model's true height reaches the surface then); V37E68E at 1,340 s: P68
# flashes V06N43, PRO at 1,378 s, V37E00E at 1,390 s: P00 on the surface.
# The landing radar (FLIGHT_LR, no V57, so no updates) answers R12's reads
# down to 10 ft, on low scale (below 2,481.7 ft) from 1,171 s.
P65 = {**NASSP, 'FLIGHT_ENGINE_LAG': '0.2', 'FLIGHT_TOUCHDOWN': '132480', 'FLIGHT_LR': '60000',
       'FLIGHT_CHAN': '137800:032=057777,137900:032=077777'}
P65_KEYS = NASSP_KEYS + '@134000V37E68E@139000V37E00E'
# lm-p67-nassp: lm-landing-ascent-nassp to P00 on the surface, with the
# crew on manual throttle (channel 30 bit 5) from 1,240 s to 1,244 s in
# P66: GUILDENSTERN starts P67 (no throttle commands, the thrust holds),
# and when the discrete goes again it starts P66 over at the current sink
# rate; the ROD clicks at 1,250 s and the landing go on as before. The
# 4 s of held thrust bring touchdown forward to 1,334 s (where the engine
# model's true height reaches the surface; lm-landing-ascent-nassp
# touches down at 1,344 s).
P67 = {**LANDING, 'FLIGHT_TOUCHDOWN': '133400', 'FLIGHT_CHAN': '122500:031=067777,124000:030=036371,124400:030=036351,'
                                 '134900:031=057777,141000:032=057777,141100:032=077777'}
P67_KEYS = NASSP_KEYS + '@122700M@123000MMMMMM@125000MMMM@136000V37E68E@142000V37E00E'
# lm-p63-lr-v58: lm-p63-lr-update with V58E at 880 s (LR updates
# inhibited, FLGWRD11 LRINH cleared): the updates take HCALC from about
# 187 m above the true height to about 96 m by 873 s, then stop; HCALC
# drifts with the navigation after that (about 107 m by 953 s).
LR_V58_KEYS = LR_UPDATE_KEYS + '@88000V58E'
# lm-p63-landing-displays: lm-p63-lr-update with the mode select switch in
# PGNCS (channel 30 bit 6 clear; the NASSP core has it set, so R10 sends
# nothing). From P63 ignition (SWANDISP) at about 649 s R10 drives the
# landing analog displays: ALTM and channel 14 bits 2-3, 4 times a second.
LR_DISPLAYS = {**LR_UPDATE, 'FLIGHT_CH30': '036311'}
# lm-p63-v60-nassp: lm-p63-nassp with the landing radar (no V57) and V60E
# at 1,000 s: LRPOS2K commands the antenna to position 2 (RADMODES bit 6),
# the antenna gets there 5 s later (FLIGHT_LR_POS2) with no alarm 523;
# P64 at about 1,120 s. Three alarms fire, as Luminary 099 does in this
# state (yaAGC records the same): 1210 at 1,002.99 s, a BAILOUT1 software
# restart, because V60's RADSTALL and R12's next radar read wait on the
# same device (RDRUSECK should refuse V60 while R12 is on, but LRBYBIT is
# bit 15, so CS/MASK leave a negative value and CCS skips past the check);
# 522 at 1,003.87 s, R12's LRPOSCHK while the antenna is between positions;
# 511 at 1,007.88 s, SERVICER's POS1CHK: P63 wants position 1, so the
# update is bypassed.
V60 = {**NASSP, 'FLIGHT_LR': '60000'}
# lm-abort-p70-nassp and lm-abort-p71-nassp: lm-p63-nassp with the crew
# aborting in P64 at 1,150 s (about 1,500 m up). P70: ABORT (channel 30
# bit 1, active 0; the core's channel 30 is 036351); R10,R11 sees it within 0.25 s and starts P70, which
# throttles the descent engine to full and flies the abort guidance to
# insertion. P71: ABORT STAGE (channel 30 bit 4); the descent stage
# separates (FLIGHT_STAGE) and P71 flies the ascent engine (FLIGHT_ASCENT:
# the 4,888 kg ascent stage) to insertion.
ABORT = {**NASSP, 'FLIGHT_ENGINE_LAG': '0.2'}
ABORT_P70 = {**ABORT, 'FLIGHT_CHAN': '115000:030=036350'}
ABORT_P71 = {**ABORT, 'FLIGHT_CHAN': '115000:030=036341', 'FLIGHT_STAGE': '115000', 'FLIGHT_ASCENT': '4888'}
ENTRY = {'FLIGHT_CH30': '037377', 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-entry.load',
         'FLIGHT_SWAP_MCT': '0', 'FLIGHT_AERO': '355'}
ENTRY_KEYS = '@2200V37E61E@4800V33E@5400V33E@6000V33E@6800V33E@7400V33E'
# cm-p52: Apollo 11's first realignment in earth orbit (00:44 GET), the
# load at 20 s (tools/flight_scenarios/cm_p52.py: the launch REFSMMAT, the
# platform drifted from it, the CSM held between stars 30 and 37). The IMU
# operating, the optics switch at ZERO OPTICS, then CMC at 40 s
# (FLIGHT_CHAN: channel 33 bits 5 and 4); V37E52E, option 3 (V22E3E on
# V04N06), proceed; proceed on V50N25 00015 (PICAPAR picks 30 and 37);
# proceed on V01N70 (R52 drives the optics); for each star the crew turns
# the switch to MANUAL, puts the star on the reticle (the star loads: the
# CDUS and CDUT the drifted platform gives) and presses MARK (M: channel
# 16 bit 6, KEYRUPT2), proceeds on V50N25 00016 and V01N71 and turns the
# switch back to CMC for the next star; proceed on N05 (star angle
# difference 0.00) and on N93 (torquing angles -0.012, +0.022, -0.010
# degrees: the drift taken out), ENTER on V50N25 00014 and P00.
P52 = {'FLIGHT_CH30': '037377', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_CHAN': '4000:033=077757,9000:033=077777,11600:033=077757,13000:033=077777',
       'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p52.load,9500:tools/flight_scenarios/cm-p52-star1.load,'
                      '13500:tools/flight_scenarios/cm-p52-star2.load'}
P52_KEYS = ('@4500V37E52E@5500V22E3E@6500V33E@7500V33E@8000V33E@10000M@10500V33E@11000V33E'
            '@12000V33E@14000M@14500V33E@15000V33E@16000V33E@17000V33E@18000E@18500V37E00E')
# cm-p23: cislunar midcourse navigation in translunar coast (24:30 GET),
# the load at 20 s (tools/flight_scenarios/cm_p23.py: the CSM 180,000 km
# out on a two-body TLI ellipse, the CMC's state vector 76 km off it, the
# shaft axis on the earth's near horizon toward star 2, the W matrix and
# horizon altitude pads). Optics to CMC at 40 s; V37E23E; V25 on V05N70
# loads star 02, no landmark, earth near horizon (00110), proceed; ENTER
# on V50N25 00202 (no auto maneuver: the attitude is held); R57's V59:
# the switch to MANUAL, the star line on the landmark line (the cal load:
# trunnion 0.011 degrees, the bias), MARK, proceed on V06N87; R52 with
# the switch on CMC; MANUAL again, the star on the horizon (the mark
# load: the trunnion the true trajectory gives), MARK; proceed on V50N25
# 00016 and V05N71; INCORP1 shows the update on V06N49 (about 20 km in
# position); proceed incorporates it, then P00.
P23 = {'FLIGHT_CH30': '037377', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_CHAN': '4000:033=077757,10000:033=077777,12000:033=077757,16000:033=077777',
       'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p23.load,10500:tools/flight_scenarios/cm-p23-cal.load,'
                      '16500:tools/flight_scenarios/cm-p23-mark.load'}
P23_KEYS = ('@4500V37E23E@5500V25E00002E00000E00110E@7000V33E@9000E@11000M@13000V33E@17000M@19000V33E'
            '@20500V33E@25000V33E@27000V37E00E')
# cm-p40: Apollo 11's LOI-1 (75:49:50 GET), the load at 20 s
# (tools/flight_scenarios/cm_p40.py: the docked CSM+LM on its approach
# hyperbola 2,060 km from the moon, TIG at 300 s, the external DV 889 m/s
# retrograde, the SPS pads, the platform with X along the DV and the CDUs
# at the ignition attitude S40.1 gives). G&N AUTO (channel 31); V37E30E,
# proceed on V06N33 (TIG), V06N81 (DV), V06N42 (apocynthion and
# pericynthion) and V16N45; V37E40E, proceed on V50N18 (R60's maneuver is
# nil), ENTER on V50N18 (end R60), proceed on V50N25 00204 (the gimbal
# test); average G at TIG-30, proceed on V99N40 at TIG-5; FLIGHT_SPS (the
# 43,900 kg stack, its centre of mass 0.5 degrees in pitch and -0.3 in yaw
# off the pad-loaded trim, so the TVC DAP has to find it) burns about
# 372 s to cutoff (VG under 1 m/s); proceed
# on V16N40 and V16N85, then P00.
P40 = {'FLIGHT_CH30': '037377', 'FLIGHT_CH31': '037777', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_SPS': '43900,0.5,-0.3', 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p40.load'}
P40_KEYS = ('@4500V37E30E@6000V33E@7000V33E@8500V33E@10000V33E@11500V37E40E@14000V33E@15000E@16000V33E'
            '@29600V33E@69000V33E@70500V33E@72000V37E00E')
# lm-p57: Apollo 11's two-star lunar surface alignment (103:45 GET), the
# load at 20 s (tools/flight_scenarios/lm_p57.py: the LM at Tranquility
# Base facing west 13 degrees left, the landing REFSMMAT, the platform
# drifted from it) and the CDUs as the moon turns the LM at each mark.
# V37E57E, proceed on V04N06 (option 3, REFSMMAT), technique 2 (two
# stars: V22E2E on V05N06), proceed; the stored-attitude initial pass;
# for each star R59's V01N70 (star code 12 Rigel, then 03 Navi: V21E),
# proceed on V06N79 (R59 finds detents 6 and 4), on V01N71 and on V54,
# X MARK (x: channel 16 bit 3, KEYRUPT2), and the reticle's cursor and
# spiral angles loaded on the flashing V06N79 (V21E, V22E), proceed;
# proceed on N05 (0.00) and on N93 (-0.021, +0.012, -0.019 degrees: the
# drift taken out); ENTER on V50N25 00014, V34 on N89 (the crew rejected
# the landing site update) and P00.
P57 = {'FLIGHT_CH30': '036357', 'FLIGHT_CH31': '057777', 'FLIGHT_CH33': '077737', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_LOAD': '2000:tools/flight_scenarios/lm-p57.load,15900:tools/flight_scenarios/lm-p57-star1.load,'
                      '29900:tools/flight_scenarios/lm-p57-star2.load'}
P57_KEYS = ('@3000V37E57E@4000V33E@5000V22E2E@6000V33E@12000V21E00012E@13000V33E@14000V33E@15000V33E@16000x@17000'
            'V21E+08608E@18000V22E+12128E@19000V33E@26000V21E00003E@27000V33E@28000V33E@29000V33E@30000x@31000V21'
            'E+01538E@32000V22E+13554E@33000V33E@40000V33E@42000V33E@48000E@50000V34E@52000V37E00E')
# lm-doi-nassp: Apollo 11's descent orbit insertion from the same core
# (NASSP, 102:25:51 GET), with DOI backed out (tools/flight_scenarios/
# lm_doi.py: the LM on the 113.8 x 117.5 km pre-DOI orbit, DOI at TIG
# 102:34:00) and the descent engine model. V37E30E: P30 with TIG and
# 76.4 ft/s retrograde (N81) loaded, PRO on N42 (61.4 x 8.1 nmi); V37E40E:
# the crew's maneuver to the burn attitude (FLIGHT_POKE: CDUX-Z and
# CDUXD-ZD at R60's N18 angles), ENTER on V50N18, PRO on V99N40; the DPS
# at 10% (no throttle-up) burns 75 s to cutoff (VG 1.1 ft/s); PRO on
# V16N40 and N85, V37E00E; V82 (this vehicle): 61.4 x 8.8 nmi, V34E.
DOI = {**NASSP, 'FLIGHT_CH33': '036673', 'FLIGHT_ENGINE_LAG': '0.2',
       'FLIGHT_LOAD': '100:tools/flight_scenarios/lm-doi.load',
       'FLIGHT_POKE': '7000:032=77776,7000:033=60216,7000:034=74211,7000:3235=77776,7000:3236=60216,7000:3237=74211'}
del DOI['FLIGHT_LR_POS2']
DOI_KEYS = ('@200V37E30E@1000V25E+00102E+00034E+00000E@2000V33E@2600V25E-00764E+00000E+00000E@3600V33E@4400V33E'
            '@6000V37E40E@7200E@48500V33E@57000V33E@58000V33E@59000V37E00E@60000V82E@61000V33E@63000V34E')
# lm-p47-nassp: the lm-doi-nassp start in P00 with the RCS translation
# model (FLIGHT_RCS: 15,100 kg). V37E47E: P47 shows V16N83 from 25 s; the
# crew's +X translation (channel 31 bit 7) from 30 s to 40 s, two jets,
# reads 1.9 ft/s on N83 X; PRO at 50 s and V37E00E: P00.
P47 = {**{k: v for k, v in DOI.items() if k != 'FLIGHT_POKE'}, 'FLIGHT_RCS': '15100',
       'FLIGHT_CHAN': '3000:031=057677,4000:031=057777'}
P47_KEYS = '@200V37E47E@5000V33E@5500V37E00E'
# lm-p41-nassp: lm-doi-nassp's P30 with a 5.0 ft/s retrograde trim in
# place of DOI, done by RCS: V37E41E, the crew's maneuver to P41's N18
# angles (FLIGHT_POKE), ENTER, V16N85 (body VG 4.6, 1.8, -0.8 ft/s); from
# TIG the crew translates +X 23.5 s, +Y 9.3 s and -Z 4.1 s (FLIGHT_RCS,
# FLIGHT_CHAN), nulling N85 to 0.1, 0, 0; PRO and V37E00E: P00.
P41 = {**DOI, 'FLIGHT_RCS': '15100',
       'FLIGHT_POKE': '7000:032=77776,7000:033=56420,7000:034=74217,7000:3235=77776,7000:3236=56420,7000:3237=74217',
       'FLIGHT_CHAN': '49000:031=057677,51350:031=057777,51400:031=057377,52330:031=057777,52400:031=053777,52810:031=057777'}
P41_KEYS = DOI_KEYS.split('@6000')[0].replace('-00764E', '-00050E') + '@6000V37E41E@7200E@53500V33E@54500V37E00E'
# lm-p42-nassp: lm-doi-nassp's DOI burned on the ascent engine. The
# descent stage goes at 1.5 s (FLIGHT_STAGE, FLIGHT_ASCENT 4,888 kg); V48:
# DAPDATR1 11002 (ascent stage, so APSFLAG) and 10,776 lb on N47; P30 as
# in lm-doi-nassp; V37E42E, the R60 maneuver as there, PRO on V99N40; the
# APS burns about 6.6 s. With no attitude dynamics the DAP's steering does
# not turn the LM, so cutoff leaves N85 at 5.3, 14.3, -0.1 ft/s. PRO on
# V16N40 and N85, V37E00E: P00.
P42 = {**DOI, 'FLIGHT_STAGE': '150', 'FLIGHT_ASCENT': '4888',
       'FLIGHT_POKE': '10000:032=77776,10000:033=60216,10000:034=74211,10000:3235=77776,10000:3236=60216,10000:3237=74211'}
P42_KEYS = ('@200V48E@600V21E11002E@1400V33E@2000V21E+10776E@2800V33E@3200V37E30E@4000V25E+00102E+00034E+00000E'
            '@5000V33E@5600V25E-00764E+00000E+00000E@6600V33E@7400V33E@9000V37E42E@10200E@48500V33E@52000V33E'
            '@53000V33E@54000V37E00E')
# cm-p11: Apollo 11's launch (16 July 1969 13:32 UT), the pad load at 1 s
# (tools/flight_scenarios/cm_p11.py: launch complex 39A, the 72.058
# degree flight azimuth, TEPHEM for a 100 s liftoff). The IMU model
# (FLIGHT_IMU) zeros and coarse aligns the gimbals; V37E01E: P01 turns
# the IMU on and aligns it, then the gyrocompass loop P02; liftoff at
# 100 s (FLIGHT_SATURN: channel 30 bit 5) and P11, which zeros the clock
# and shows V06N62 (inertial speed, climb rate, altitude) every 2 s
# through the S-IC, S-II and S-IVB burns (the PIPAs the boost model
# gives) to orbit insertion, 25,555 ft/s and 102 nmi; V82 in P11:
# V16N44, 115.8 x 81.5 nmi; proceed, then P00. Pinball RELDSP1 (02,2516,
# Lane C) is the one point the host bridges. Key times after liftoff
# count from the zeroed clock, and so does the run's length.
P11 = {'FLIGHT_CH30': '037377', 'FLIGHT_SWAP_MCT': '0', 'FLIGHT_IMU': '1', 'FLIGHT_SATURN': '10000',
       'FLIGHT_LOAD': '100:tools/flight_scenarios/cm-p11.load'}
P11_KEYS = '@2000V37E01E@84000V82E@86000V33E@88000V37E00E'
# cm-p20: the CSM's side of the LM rendezvous (tools/flight_scenarios/
# cm_p20.py: the CSM in a 60 nmi lunar orbit, the LM 15 nmi below and 2.1
# degrees behind, the CMC's LM state 1.8 km too far along the line of
# sight; the CSM at R61's tracking attitude). FLIGHT_RR's CM side
# (tools/flight_radar.h) holds the attitude on CDUXD-ZD, points the SXT
# at the LM and answers VHF range reads. The IMU operating, CMC mode
# (channels 31 and 33, FLIGHT_CHAN at 40 s); V37E20E (P20: R61's tracking
# attitude, R22 running), V87E (VHF ranging allowed), V57E (R21's V51)
# and MARK, proceed on V51 (the SXT mark goes into the LM state: TRKMKCNT
# 1); R22's first VHF range (about 125 s) shows V06N49 (the 1.8 km), proceed:
# VHFCNT 1, and the second (about 185 s, a 4 ft/s velocity change) again,
# proceed. R22 waits on N49 in a loop (RENDISP7), so the AGC does not idle:
# FLIGHT_BUSY_SAMPLE gives the keys their sample points.
CM_P20 = {'FLIGHT_CH30': '037377', 'FLIGHT_CH31': '037777', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
          'FLIGHT_CHAN': '4000:033=077757', 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p20.load',
          'FLIGHT_RR': '2000:-260373.984,1763762.175,372327.510,1589.402,294.044,-281.432:-198527.341,1801605.032,366129.729,1584.888,233.850,-291.320',
          'FLIGHT_BUSY_SAMPLE': '100'}
CM_P20_KEYS = '@4500V37E20E@6000V87E@9000V57E@10000M@10500V33E@14000V33E@20000V33E'
# lm-rendezvous: the LM's side (tools/flight_scenarios/lm_rendezvous.py:
# the CSM in a 60 nmi lunar orbit, the LM 15 nmi below and 2.1 degrees
# behind, both retrograde about reference Z as Apollo's were, the AGC's
# state vectors on the true orbits). FLIGHT_RR's LM side holds the
# attitude, points the RR at the CSM and answers range and range-rate
# reads. V37E20E (P20: R61, the V50N18 maneuver: PRO, then ENTER on the
# final V50N18), PRO on R21's V50N72 lock-on; R22 marks every 61 s (6 by
# 340 s, no V06N49). V37E34E at 290 s (P34): PRO on N37 (TTPI), N55 and
# N45; P34 solves TPI for 26.6 degrees elevation (N58: 7.4 m/s), PRO on
# N37, N58 and N81, ending on N59.
LM_RDZ = {'FLIGHT_CH30': '036377', 'FLIGHT_CH31': '057777', 'FLIGHT_CH33': '077775', 'FLIGHT_SWAP_MCT': '0',
          'FLIGHT_LOAD': '2000:tools/flight_scenarios/lm-rendezvous.load',
          'FLIGHT_RR': CM_P20['FLIGHT_RR'],   # the same two orbits (lm_rendezvous.py prints it)
          'FLIGHT_BUSY_SAMPLE': '50'}
LM_RDZ_KEYS = ('@2200V37E20E@3000V33E@4700E@6100V33E@29000V37E34E@29400V33E@29700V33E@34000V33E@36700V33E'
               '@37500V33E@37800V33E')
# The LM's rendezvous targeting before TPI (tools/flight_scenarios/
# lm_coelliptic.py: the CSM in a 60 nmi circular orbit 9.5 degrees ahead,
# the LM in a 9 x 45 nmi orbit ten minutes before apolune, where CSI
# comes; TPI 97 minutes after apolune, as Apollo 11). No radar marks: the
# programs work from the state vectors.
# lm-csi: V37E32E (P32, CSI): PRO on V06N11 (TCSI 0: P32 takes the
# apolune, 10:20.10); N55 NN 1, ELEV 26.6 (P32 zeroes NN: with 0 it finds
# no CDH, alarm 604); PRO on N37 (TTPI) and V16N45; the CSI solution on
# V06N75 (DH 16.5 nmi, CSI to CDH 58:03, CDH to TPI 38:56), V06N81 (48.4
# ft/s forward) and V06N82, PRO on each, back to N45. Then V37E72E (P72,
# the CSM-active CSI, the same pads with ELEV 206.6: the CSM looks down at
# the LM): N75 DH -14.3 nmi, 57:50, 39:10; N81 49.9 ft/s back (the CSM's
# burn); N82.
LM_CSI = {'FLIGHT_CH30': '036377', 'FLIGHT_CH31': '057777', 'FLIGHT_CH33': '077775', 'FLIGHT_SWAP_MCT': '0',
          'FLIGHT_LOAD': '2000:tools/flight_scenarios/lm-csi.load',
          'FLIGHT_RR': '2000:-259681.119,1759068.734,371336.732,1576.461,307.349,-276.315:'
                       '34932.422,1820802.682,320471.671,1594.204,27.571,-330.419',
          'FLIGHT_BUSY_SAMPLE': '50'}
LM_CSI_KEYS = ('@2200V37E32E@3000V33E@4000V33E@4400V25E+00001E+02660E+00000E@5400V33E@6000V33E@6600V33E'
               '@9800V33E@10500V33E@11300V33E@12000V37E72E@12800V25E+00000E+00010E+02010E@14000V33E'
               '@14600V25E+00001E+20660E+00000E@15600V33E@16200V33E@16800V33E@20300V33E@21100V33E@21900V33E')
# lm-cdh: the LM after CSI (lm_coelliptic.py BURN=62010:48.4,0,0: P32's
# N81 burned at TCSI). V37E33E (P33, CDH): V25 on V06N13, TCDH 1:08:23.10
# (the next apse after CSI), PRO; PRO on V16N45; the CDH solution on V06N75
# (DH 16.5 nmi, CDH to TPI 38:34, TPI 23 s before TTPI) and V06N81 (2.0
# ft/s back, 0.3 down), PRO on each, back to N45.
LM_CDH = {**LM_CSI, 'FLIGHT_LOAD': '2000:tools/flight_scenarios/lm-cdh.load',
          'FLIGHT_RR': '2000:-267291.806,1762276.412,373515.357,1587.947,300.827,-279.905:'
                       '34932.422,1820802.682,320471.671,1594.204,27.571,-330.419'}
LM_CDH_KEYS = '@2200V37E33E@3000V25E+00001E+00008E+02310E@4200V33E@5000V33E@21000V33E@21800V33E@22600V33E'
# lm-cdh-csm: the CSM after its CSI (lm_coelliptic.py CSMBURN=62010:
# -49.9,0,0 ELEV=206.6: P72's N81 burned by the CSM). V37E73E (P73, the
# CSM-active CDH), keyed as lm-cdh: N75 DH -14.3 nmi, 38:35, -0:22; N81
# 1.3 ft/s back, 15.9 up.
LM_CDH_CSM = {**LM_CSI, 'FLIGHT_LOAD': '2000:tools/flight_scenarios/lm-cdh-csm.load',
              'FLIGHT_RR': '2000:-259681.119,1759068.734,371336.732,1576.461,307.349,-276.315:'
                           '42138.296,1816290.127,318143.276,1583.547,36.018,-326.656'}
LM_CDH_CSM_KEYS = LM_CDH_KEYS.replace('V37E33E', 'V37E73E')
# lm-rendezvous-tpm: lm-rendezvous's P20 and P34, with the rendezvous
# displays and the programs after TPI. V85 at 120 s (V16N56: the RR's
# line of sight, within 0.3 degrees of the CSM), V34; V83 at 200 s
# (R31: V16N54, 35.4 nmi, -110 ft/s, 241.9 degrees), V34; P34 as
# lm-rendezvous, PRO on N59; V37E35E (P35, TPM, TPI not burned: the whole
# TPI again, N81 21.5 ft/s, N59), PRO to N45; V37E74E (P74, the CSM-active
# TPI): TTPI 0:12:00, ELEV 206.6 (N58, N81 22.7 ft/s back, N59);
# V37E75E (P75, the CSM-active TPM): N81, N59, back to N45; V37E76E (P76,
# target delta V): V06N84 loaded 10 ft/s forward, PRO on N33: the CSM's
# state takes the burn; V56 (end tracking: P20 stops, V37 flashes).
LM_TPM_KEYS = ('@2200V37E20E@3000V33E@4700E@6100V33E@12000V85E@14000V34E@20000V83E@25000V34E'
               '@29000V37E34E@29400V33E@29700V33E@34000V33E@36700V33E@37500V33E@37800V33E@39000V33E'
               '@40000V37E35E@41000V33E@42000V33E@43000V33E@44000V33E@45000V37E74E@46000V25E+00000E+00012E+00000E'
               '@47500V33E@50000V25E+00000E+20660E+13000E@51200V33E@52500V33E@56000V33E@57000V33E@58000V33E'
               '@59000V33E@60000V37E75E@61000V33E@62000V33E@63000V33E@64000V33E@65000V33E@66000V37E76E'
               '@67000V25E+00010E+00000E+00000E@68000V33E@69000V33E@70000V56E')
# lm-stable-orbit: lm-rendezvous's P20, then V37E78E (P78, the CSM-active
# stable orbit rendezvous) at 80 s: V25 on V06N33, TIG 0:10:00; PRO on
# N55 (CENTANG 130), V04N06 (option 1: SOI), N57 (DR 0), N34 (time of
# intercept 0:51:58.33) and V16N45; P78's Lambert solution after 200 s
# (N58, then N81 19.8 ft/s back, 22.3 down), PRO on each; V37E79E (P79,
# the stable orbit midcourse) at 480 s, PRO on N45: N81. lm-stable-orbit-
# lm: the same with the LM active, P38 and P39.
LM_SO_KEYS = ('@2200V37E20E@3000V33E@4700E@6100V33E@8000V37E78E@9500V25E+00000E+00010E+00000E@11000V33E'
              '@14500V33E@16500V33E@19500V33E@21500V33E@23000V33E@46000V33E@47000V33E@48000V37E79E@50000V33E')
# The CSM's side of the same programs (Comanche: P32-P35 and P38/P39
# with the CSM active, P72-P79 with the LM active, the CMC solving for
# it). cm-csi, cm-cdh and cm-cdh-csm: lm_coelliptic.py CM, the moments of
# lm-csi, lm-cdh and lm-cdh-csm, keyed as those with the program numbers
# swapped (P72 for P32 and back), so each solution is the LM side's: CM
# P72 is LM P32 (DH 16.5 nmi, 48.4 ft/s), CM P32 is LM P72 (49.9 ft/s
# back, ELEV 206.6), CM P73 after the LM's CSI, CM P33 after the CSM's.
CM_CSI = {**{k: v for k, v in CM_P20.items() if k != 'FLIGHT_CHAN'},
          'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-csi.load', 'FLIGHT_RR': LM_CSI['FLIGHT_RR']}
CM_CSI_KEYS = ('@2200V37E72E@3000V25E+00000E+00010E+02010E@4200V33E@4800V25E+00001E+02660E+00000E@5800V33E'
               '@6400V33E@7000V33E@10500V33E@11300V33E@12100V33E@13000V37E32E@13800V25E+00000E+00010E+02010E'
               '@15000V33E@15600V25E+00001E+20660E+00000E@16600V33E@17200V33E@17800V33E@21300V33E@22100V33E@22900V33E')
CM_CDH = {**CM_CSI, 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-cdh.load', 'FLIGHT_RR': LM_CDH['FLIGHT_RR']}
CM_CDH_KEYS = '@2200V37E73E@3000V25E+00001E+00008E+02310E@4200V33E@5000V33E@18000V33E@18800V33E@19600V33E'
CM_CDH_CSM = {**CM_CSI, 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-cdh-csm.load', 'FLIGHT_RR': LM_CDH_CSM['FLIGHT_RR']}
CM_CDH_CSM_KEYS = CM_CDH_KEYS.replace('V37E73E', 'V37E33E')
# cm-rendezvous-tpm: cm-p20, then V83 at 210 s (V16N54: 35.7 nmi, -107.9
# ft/s), V34; V85 (V16N53, R32's range, rate and angle), V34; V37E34E
# (P34, the CSM active): TTPI 0:06:31, ELEV left 0, so P34 solves it
# (208.3), N58, N81, N59; V37E35E (P35): N81, N59, N45; V37E74E (P74,
# the LM active): TTPI 0:12:00 with ELEV 26.6, so P74 solves the TIG
# (0:06:26.03, N81 21.9 ft/s, as the LM's own P34); V37E75E (P75): N81,
# N59, N45; V37E76E (P76): 10 ft/s on the LM's state; V56. (V83's R31
# display loop, 31,2000-2007, is still refused by cm_r31_basic: Lane E's.)
CM_TPM_KEYS = CM_P20_KEYS + ('@21000V83E@23000V34E@23500V85E@25000V34E@26000V37E34E@26700V25E+00000E+00006E+03100E'
                             '@28000V33E@29000V33E@32000V33E@36000V33E@38000V33E@40000V33E@41000V33E@42000V37E35E'
                             '@43000V33E@44000V33E@45000V33E@46000V33E@47000V33E@48000V33E@50000V37E74E'
                             '@51000V25E+00000E+00012E+00000E@52500V33E@52800V25E+00000E+02660E+13000E@54200V33E'
                             '@55000V33E@56000V33E@57000V33E@58000V33E@59000V33E@60000V33E@61000V33E@62000V33E'
                             '@64000V37E75E@65000V33E@66000V33E@67000V33E@68000V33E@69000V33E@70000V33E@72000V37E76E'
                             '@73000V25E+00010E+00000E+00000E@74000V33E@75000V33E@76000V33E@77000V56E')
# cm-stable-orbit: cm-p20, then V37E38E (P38, the CSM active) at 210 s:
# TIG 0:12:00 on V06N33, CENTANG 130 (V23 on N55: the CMC keeps 0),
# option 1, DR 0, PRO on N34 and N45; the Lambert solution after 160 s
# (N58, N81 20.3 ft/s back), then V37E39E (P39): N81 (P39's 04,3204 is
# native from #711). cm-stable-orbit-lm:
# P78 and P79, the LM active. cm-tpi-search: P17 (the CSM active) and P77
# (the LM), each TTPI 0:10:00: N72 (phase angle, height difference), N58,
# N55.
CM_SO_KEYS = CM_P20_KEYS + ('@21000V37E38E@22500V25E+00000E+00012E+00000E@24000V33E@24300V23E+13000E@25300V33E'
                            '@27000V33E@30000V33E@31000V33E@33000V33E@55000V33E@56000V33E@57000V37E39E@59000V33E')
CM_TPI_SEARCH_KEYS = CM_P20_KEYS + ('@21000V37E17E@22000V25E+00000E+00010E+00000E@23400V33E@26000V33E@29000V33E'
                                    '@31000V33E@32000V37E77E@33000V25E+00000E+00010E+00000E@34400V33E@37000V33E'
                                    '@40000V33E@42000V33E')
# cm-tli: Apollo 11's translunar injection (02:44:16 GET), the load at
# 20 s (tools/flight_scenarios/cm_tli.py: the CSM in the 100 nmi parking
# orbit, the launch REFSMMAT, body X along the burn, TEPHEM, the sun and
# moon). V37E47E: P47 counts the S-IVB's velocity on V16N83; the S-IVB
# (FLIGHT_SIVB) burns 347 s from 300 s, inertially fixed, to 10,861 m/s
# (C3 -1.4 km2/s2; N83 X passes 9999.9 ft/s and wraps); proceed (P47's
# GOTOPOOH flashes V37), V37E00E, then V82: V16N44 apogee 9999.9 nmi
# (the display's limit), perigee 101.6 nmi; proceed.
TLI = {'FLIGHT_CH30': '037377', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_SIVB': '30000,0.423909,-0.000000,-0.905705',
       'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-tli.load'}
TLI_KEYS = '@3000V37E47E@66000V33E@66500V37E00E@67000V82E@69000V33E'
# cm-p22: Apollo 11's lunar-orbit landmark tracking (about 98 h GET),
# the load at 20 s (tools/flight_scenarios/cm_p22.py: the CSM in a 60 nmi
# retrograde lunar orbit passing over landmark 130's site, 1.20 N 23.70 E,
# the CMC's state 1.4 km off the true orbit, REFSMMAT and platform local
# vertical, the shaft axis on the landmark at mid-pass, the W matrix
# pads). Optics to CMC at 40 s; V37E22E; proceed on V06N45; V22E10000E on
# V05N70 (landmark code 1: known, so N89), proceed; proceed on V06N89
# (1.200, half longitude 11.850, -0.81 nmi); R52 drives the optics on
# V06N92; the switch to MANUAL, MARK at 132 s and 152 s (the mark loads:
# the CDUS and CDUT the true orbit gives), proceed on V51; proceed on
# V05N71 and V06N89; INCORP shows V06N49 (0.6 nmi), proceed: the
# landmark solution on V06N89, proceed; V05N70, then V37E00E.
P22 = {'FLIGHT_CH30': '037377', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_CHAN': '4000:033=077757,13000:033=077777',
       'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p22.load,13100:tools/flight_scenarios/cm-p22-mark1.load,'
                      '15100:tools/flight_scenarios/cm-p22-mark2.load'}
P22_KEYS = ('@4500V37E22E@6000V33E@7000V22E10000E@7500V33E@8500V33E@13200M@15200M@15600V33E@16200V33E'
            '@16800V33E@18500V33E@20300V33E@21500V37E00E')
# cm-tei: Apollo 11's transearth injection (TIG 135:23:42 GET), the load
# at 20 s (tools/flight_scenarios/cm_tei.py, set up as cm_p40.py: the CSM
# alone in a circular 60 nmi lunar orbit on the far side, the SPS pads,
# the platform with X along the DV, the CDUs at the ignition attitude,
# RLS for the altitude displays). G&N AUTO; V37E30E, the TEI-1 pad keyed:
# V25 on V06N33 (TIG 300 s on the CMC clock), V25 on V06N81 (+3283.6
# ft/s), proceed on N81, N42 (9999.9 x 60.0 nmi) and V16N45; V37E40E,
# proceed and ENTER on V50N18, proceed on V50N25 00204; proceed on V99N40
# at TIG-5; FLIGHT_SPS (16,400 kg, its centre of mass -0.3 degrees in
# pitch and +0.2 in yaw off the trim) burns about 154 s to cutoff (N40 VG 6.1
# ft/s); proceed on V16N40 and V16N85, V37E00E; V82: V16N44 9999.9 x 60.0
# nmi, then V34E.
TEI = {'FLIGHT_CH30': '037377', 'FLIGHT_CH31': '037777', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_SPS': '16400,-0.3,0.2', 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-tei.load'}
TEI_KEYS = ('@4500V37E30E@5000V25E+00000E+00005E+00000E@6400V33E@6800V25E+32836E+00000E+00000E@8100V33E'
            '@9500V33E@10500V33E@11500V37E40E@14000V33E@15000E@16000V33E@29600V33E@47000V33E@48500V33E'
            '@50000V37E00E@51000V82E@53000V33E@55000V34E')
# cm-p37: return to earth targeting on the transearth coast (about 150 h
# GET), the load at 20 s (tools/flight_scenarios/cm_p37.py: the CSM inbound
# 240,000 km from the earth on an ellipse from the moon's distance with a
# natural -6.5 degree entry, outside the moon's sphere, the platform in
# local vertical axes). V37E37E: V25 on V06N33 (TIG 1 h on the CMC clock),
# V22 and V23 on V06N60 (100 ft/s, -6.50 degrees), proceed; the conic
# solution (V06N61, N39, N60, N81), proceed through it; the precision
# solution (about 170 s), proceed through the same displays, V04N06 (SPS),
# the biased V06N33 and V16N45; V37E00E; V82 (V16N44), then V34E.
P37 = {'FLIGHT_CH30': '037377', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p37.load'}
P37_KEYS = ('@4500V37E37E@5000V25E+00001E+00000E+00000E@6400V33E@7000V22E+00100E@7600V23E-00650E@8200V33E'
            '@11000V33E@12000V33E@13000V33E@14000V33E@38000V33E@39000V33E@40000V33E@41000V33E@42000V33E'
            '@43000V33E@44000V33E@45000V37E00E@46000V82E@47500V33E@49500V34E')

# Off-nominal radar and uplink inputs (docs/FLIGHT-STORE.md, Flight
# scenarios), each a nominal scenario with one input gone wrong.
# lm-rendezvous-rr-dropout: lm-rendezvous with the RR data good lost from
# 180 s to 210 s (FLIGHT_RR_DROP), over R22's third mark on every clock
# (the scaled run reaches it at 200 s, the reference clock (W) at
# 186 s): the read fails (alarm 521, RADMODES data fail), R22 drops the
# lock and R21 flashes V50N72 again at 211 s; PRO at 215 s, the RR locks
# on and the marks go on (one fewer); P34 solves TPI and ends on N59 as
# before. P34's two PROs come at 304 s and 307 s, not 294 s and 297 s:
# on the reference clock P34 comes up 5 s late after the dropout.
LM_RDZ_RR_DROP = {**LM_RDZ, 'FLIGHT_RR_DROP': '18000-21000'}
LM_RDZ_RR_DROP_KEYS = (LM_RDZ_KEYS.replace('@29000', '@21500V33E@29000')
                       .replace('@29400V33E@29700V33E', '@30400V33E@30700V33E'))
assert LM_RDZ_RR_DROP_KEYS.count('@30400V33E@30700V33E') == 1
# cm-p20-vhf-dropout: cm-p20 with the VHF data good lost from 180 s to
# 190 s, over R22's second range: VHFREAD takes BADEND, so no N49 and no
# mark (VHFCNT 3 by 320 s, one fewer); the next ranges incorporate on
# their own.
CM_P20_VHF_DROP = {**CM_P20, 'FLIGHT_RR_DROP': '18000-19000'}
CM_P20_VHF_DROP_KEYS = CM_P20_KEYS.replace('@20000V33E', '')
# Off-nominal IMU inputs, each a P52 scenario with one IMU input gone
# wrong (channel 30, inverted: FLIGHT_CHAN; the gimbal angle: FLIGHT_POKE
# of CDUZ). docs/FLIGHT-STORE.md, Flight scenarios.
# lm-p52-imu-fail: lm-p52-realign with IMU FAIL (channel 30 bit 13) from
# 30 s to 45 s, over the first star's marks: IMUMON records it in
# IMODES30, stores alarm 07777 and lights ISS WARNING (channel 11 bit 1);
# P52 goes on as before and ends in V37.
LM_P52_IMU_FAIL = {**LM_P52, 'FLIGHT_CHAN': '3000:030=026351,4500:030=036351'}
# lm-p52-imu-cage: lm-p52-realign with the IMU caged (channel 30 bit 11)
# from 30 s to 35 s: IMUCAGE zeroes the CDUs and lights NO ATT
# (DSPTAB+11D bit 4), and the first star's marks are made against the
# caged platform, so N05 shows 114.83 degrees and N93 large torquing
# angles; P52 still ends in V37.
LM_P52_IMU_CAGE = {**LM_P52, 'FLIGHT_CHAN': '3000:030=034351,3500:030=036351'}
# cm-p52-imu-fail: cm-p52 with IMU FAIL from 60 s to 70 s, on V04N06
# and V50N25:
# IMUMON records it in IMODES30 and stores alarm 07777 (FAILREG +1); P52
# goes on as before (N93 -0.012, +0.022, -0.010) and ends in P00.
CM_P52_IMU_FAIL = {**P52, 'FLIGHT_CHAN': '4000:033=077757,6000:030=027377,7000:030=037377,9000:033=077777,'
                                         '11600:033=077757,13000:033=077777'}
# cm-p52-imu-cage: cm-p52 with the IMU caged from 60 s to 65 s, on
# V04N06: NO ATT lights (DSPTAB+11D bit 4), and the marks are made
# against the caged platform: N05 shows 5.44 degrees, N93 large torquing
# angles, and P52 ends in P00.
CM_P52_IMU_CAGE = {**P52, 'FLIGHT_CHAN': '4000:033=077757,6000:030=035377,6500:030=037377,9000:033=077777,'
                                         '11600:033=077757,13000:033=077777'}
# cm-p52-gimbal-lock: cm-p52 with the middle gimbal (CDUZ) at 72 degrees
# from 30 s to 36 s: GLOCKMON lights GIMBAL LOCK (DSPTAB+11D bit 6) and
# turns it off when the angle comes back; P52 runs as before to P00.
# At 190 s CDUZ goes to 86 degrees: past 85, GLOCKMON coarse aligns the
# platform (channel 12 bit 6 with the CDU drives), and NO ATT lights with
# GIMBAL LOCK.
CM_P52_GIMBAL_LOCK = {**P52, 'FLIGHT_POKE': '3000:034=014600,3600:034=77773,19000:034=017200'}
# The output models (flight_host_run.c gyro_due, optics_due), each on in
# its own scenario so the others keep their inputs:
# lm-p52-cage-torque: lm-p52-imu-cage with the platform model
# (FLIGHT_GYRO): the large torquing angles after the cage (V50N25 00014)
# turn the stable member by the pulses the gyro drive sends.
LM_P52_TORQUE = {**LM_P52_IMU_CAGE, 'FLIGHT_GYRO': '1'}
# lm-p52-coarse: P52 option 2 (nominal) with the ISS model (FLIGHT_ISS: in
# coarse align the gimbals follow the CDU drive pulses, 4 counts a pulse).
# lm-p52-realign's start (no CDU pokes); V37E52E, V22E2E on V04N06 (option
# 2), proceed; proceed on V06N34 with the time 0 (now): V06N22 020.57,
# 145.04, 359.49 degrees; proceed; proceed on V50N25 00013 (coarse align):
# from 46.9 s COARS drives CDUY from 194.74 to 145.04 degrees, 3.5 s of
# pulses (CDUX and CDUZ move a count or two), and P52 goes on to V50N25
# 00015 (the star acquisition) at 54.2 s.
LM_P52_COARSE = {**{k: v for k, v in LM_P52.items() if k != 'FLIGHT_POKE'}, 'FLIGHT_ISS': '1'}
LM_P52_COARSE_KEYS = '@200V37E52E@1000V22E2E@1500V33E@2500V33E@3500V33E@4500V33E'
# cm-p52-optics: cm-p52 with the platform and optics models (FLIGHT_GYRO,
# FLIGHT_OPTICS): R52 drives the sextant to each star (CDUT/CDUS follow
# the 053/054 pulses), and the torquing at about 171.5 s turns the stable
# member. The star loads still set the mark angles.
CM_P52_OPTICS = {**P52, 'FLIGHT_GYRO': '1', 'FLIGHT_OPTICS': '1'}
# cm-p52-coarse: P52 option 2 (nominal), the coarse align R50 runs (the
# Comanche055 coverage map). cm-p52's load, the IMU model on (FLIGHT_IMU:
# with channel 12 bits 4 and 6 set the CDUs go to THETAD); V37E52E,
# proceed on V04N06 (option 00002, the default with PFRATFLG clear);
# proceed on V06N34 with the time 0 (now): S52.3 computes the nominal
# orientation and S52.2 the gimbal angles, V06N22 353.72, 110.96, 359.93
# degrees; proceed; proceed on V50N25 00013 (coarse align, not gyro
# torque): CAL53A coarse aligns (CDUY 314.7 to 111.0 degrees) and P52
# goes on to V50N25 00015 (the star acquisition); terminate (V34), P00.
CM_P52_COARSE = {'FLIGHT_CH30': '037377', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0', 'FLIGHT_IMU': '1',
                 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p52.load'}
CM_P52_COARSE_KEYS = '@4500V37E52E@6500V33E@7500V33E@8500V33E@9500V33E@12000V34E@13000V37E00E'
# cm-p41: an RCS midcourse correction in translunar coast (the Comanche055
# coverage map; inferred: a correction too small for the SPS). cm-p23's
# load (the CSM at 24:30 GET), the vehicle held at the burn attitude
# (FLIGHT_POKE puts the RCS DAP's CDUXD-ZD at the gimbal angles P41's
# S40.1 asks for, 0.05, 266.07, 0.04 degrees, and FLIGHT_SPS's attitude
# hold puts the CDUs there; the SPS never lights). V37E30E: V25 on V06N33
# loads TIG at 8 minutes on the CMC clock, V25 on V06N81 a DV of 5.0 ft/s
# along local horizontal X; proceed on N81, N42 and N45. V37E41E: R60's
# maneuver is nil (proceed, then ENTER on V50N18); V06N85 shows VG 5.0
# ft/s, average G starts at TIG-35 s, V16N85 at TIG. No CM RCS
# translation model: the crew's +X translation gives no thrust, so VG
# stays 5.0; proceed at 500 s, then P00.
CM_P41 = {'FLIGHT_CH30': '037377', 'FLIGHT_CH31': '037777', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
          'FLIGHT_SPS': '28800', 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p23.load',
          'FLIGHT_POKE': '2100:3246=00005,2100:3250=57232,2100:3252=00004'}
CM_P41_KEYS = ('@4500V37E30E@5500V25E+00000E+00008E+00000E@6800V33E@7200V25E+00050E+00000E+00000E'
               '@8500V33E@9500V33E@10500V33E@11500V37E41E@12500V33E@13500E@50000V33E@51000V37E00E')
# cm-v48-dap: the DAP data load (V48, R03) and V46 (the Comanche055
# coverage map). cm-p40's load (DAPDATR1 21102: CSM+LM, quads AC and
# BD, narrow deadband, 0.5 degree/s), before P30; V48: V04N46 shows
# 21102 01111, the crew loads R1 21112 (the wide 5 degree deadband) and
# proceeds; proceed on V06N47 (CSM 63,493 and LM 33,290 lb) and V06N48
# (trims 0); S41.2 sets ADB 455 (wide) and RATEINDX 4 (0.5 degree/s);
# V46 (STABLISH) establishes the RCS DAP with the new data.
CM_V48_KEYS = '@2500V48E@3500V21E21112E@4500V33E@5500V33E@6500V33E@7500V46E'
# cm-p40-sps-fail: cm-p40 with the SPS failing to light (the Comanche055
# coverage map, off-nominal; no FLIGHT_SPS: no thrust, and with the
# R60 maneuver nil no attitude model is needed). The same keys to the
# V99N40 at TIG-5; at TIG the engine is commanded on but the PIPAs see
# nothing, so R40 (LOTHRUST) finds the thrust low and FAILDSP flashes
# V97N40 (SPS thrust fail) at TIG+3 s; the crew terminates (V34):
# V97T turns the engine off and hands back to the RCS DAP; then P00.
P40_SPS_FAIL = {k: v for k, v in P40.items() if k != 'FLIGHT_SPS'}
P40_SPS_FAIL_KEYS = ('@4500V37E30E@6000V33E@7000V33E@8500V33E@10000V33E@11500V37E40E@14000V33E@15000E@16000V33E'
                     '@29600V33E@32000V34E@34000V37E00E')
# Crew verbs the other CM scenarios do not key (the Comanche055 coverage
# map, /mnt/project-files/acceptance/comanche055-coverage-map.md).
# cm-p20-crew-verbs: cm-p20 to 200 s, then V90 (R36, out-of-plane): V04N12
# (option 2), proceed, V06N16 (event time 0: now), proceed, V06N90 (Y and
# Y-dot 0: the two orbits are coplanar, PSI 180 degrees), proceed. V44
# (surface flag set): R61 recomputes the tracking attitude for an LM on
# the surface, stores alarm 00407 through R52's path and asks for a
# maneuver (V50N18); V45 (reset), V88 (VHF ranging off) and V58 (auto
# maneuver enabled) go in under the flashing V50N18, then V93 (RENDWFLG cleared: the W matrix is no longer valid).
CM_P20_CREW_KEYS = CM_P20_KEYS + '@21000V90E@22000V33E@23000V33E@24000V33E@25000V33E@26000V44E@27000V45E@28000V88E@29000V58E@30000V93E'
# cm-p23-attached: cm-p23 to P00, then V66 (vehicles attached): ATTACHIT
# copies the CSM's state vector into the LM's, as after docking.
P23_ATTACHED_KEYS = P23_KEYS + '@28500V66E'
# cm-v49-maneuver: cm-p40's start (the docked stack before LOI), V76
# (preferred attitude flag set), then V49 (R62): V06N22 loaded with V25 to roll 180 degrees, proceed, R60's
# V50N18, proceed: KALCMANU steers toward the new attitude and R60 shows
# V06N18. The harness has no CM attitude dynamics, so the CDUs stay put
# and the steering runs to the end of the run.
V49_KEYS = '@2000V76E@3000V49E@4000V25E+18000E+00000E+00000E@5500V33E@6500V33E'
# cm-v49-att: cm-p40's start with the CSM turning under its jets
# (FLIGHT_ATT, tools/flight_attitude.h). V48 proceeds through N46-N48, so
# MASSPROP sets IXX and IAVG for the docked stack; V46 starts the RCS DAP;
# V76, then V49 to gimbal angles 30, 0, 0 (proceed, then R60's V50N18).
# KALCMANU steers CDUXD toward 30 degrees at 0.5 degree/s, the DAP fires
# the roll, pitch and yaw jets, and CDUX-Z follow it there.
V49_ATT_KEYS = ('@2500V48E@3500V33E@4500V33E@5500V33E@6500V46E@7500V76E@8500V49E@9500V25E+03000E+00000E+00000E'
                '@11000V33E@12000V33E')
# cm-p21: cm-p52 to P00, then V37E21E (P21, ground track): V04N06 option 2
# (this vehicle), proceed; V06N34 (the GET of interest: now, 00:33:00.81),
# proceed; V06N43 shows the latitude, longitude and altitude there
# (-7.40, +111.70 degrees, 97.1 nmi).
P21_KEYS = P52_KEYS + '@19500V37E21E@20500V33E@21500V33E@22500V33E@23500V33E'
# lm-p63-lr-dropout: lm-p63-nassp with the landing radar from 600 s
# (FLIGHT_LR, tools/flight_lr.h: range and velocity beams from the true
# trajectory, data good below 50,000 ft of slant range) and its data good
# lost from 880 s to 950 s (FLIGHT_LR_DROP). R12's range reads fail
# (RADMODES LR range fail) until the slant range comes inside 50,000 ft
# at 816 s (about 38,000 ft up), pass, fail again in the dropout, and
# pass again from 952 s. No V57, so nothing is updated.
LR_DROP = {**NASSP, 'FLIGHT_LR': '60000', 'FLIGHT_LR_DROP': '88000-95000'}
# lm-uplink-reject: the ground's uplink (FLIGHT_UPLINK) in P00, rejected
# by the V71 checks: V71 (P27 flashes V21N01), 25 components (octal: 21,
# over OHWELL1's limit of 20, so V21N01 again), V34 (P27 ends); a garbled
# word at 50 s (UPRUPT's triple-redundancy test fails: the uplink locks
# out, FLAGWRD7 bit 4), so V37E at 55 s is ignored; ERROR RESET at 65 s
# unlocks it and V37E00E takes P00.
UPLINK = {'FLIGHT_SWAP_MCT': '0', 'FLIGHT_UPLINK': '2000:V71E25E;3500:V34E;5000:#00001;5500:V37E;6500:R;7000:V37E00E'}
# lm-uplink-accept / cm-uplink-accept: accepted P27 loads in P00, one per
# uplink verb. V71 (contiguous block): 4 components (II, the ECADR of
# TTPI, its two words), each a flashing V21N01; V21N02 then asks for the
# verify and V33 stores them (UPEND71). V72 (scatter): II 3, then one
# ECADR and its word: TTPI's high word 1. V73 (clock increment) and V70
# (liftoff time increment): 0 and 144 octal, V33 (100 cs each). V06N37
# shows TTPI, 1 x 2^14 + 5120 cs: 00 03 35.04. V74 at 120 s starts the
# erasable dump on the downlink (DNEDUMP; list 01776 LM, 01777 CM), so
# DOWNRUPT runs and both sides' downlink words are compared.
# cm-uplink-reject: lm-uplink-reject's keys on the CMC.
def uplink_accept(ecadr):
    return {'FLIGHT_SWAP_MCT': '0', 'FLIGHT_DOWNRUPT': '1', 'FLIGHT_UPLINK': '2000:V71E;2400:4E;2800:%sE;3200:0E;3600:12000E;4200:V33E;'
            '5000:V72E;5400:3E;5800:%sE;6200:1E;6600:V33E;7200:V73E;7600:0E;8000:144E;8400:V33E;'
            '9200:V70E;9600:0E;10000:144E;10400:V33E;11200:V06N37E;12000:V74E' % (ecadr, ecadr)}
UPLINK_ACCEPT_LM = uplink_accept('3635')   # TTPI E7,1635
UPLINK_ACCEPT_CM = uplink_accept('3662')   # TTPI E7,1662
# Executive overload (the 1201/1202 alarms). Apollo 11's RR CDU counts
# stole counter cycles from the LGC in P63; FLIGHT_STEAL spends N MCT a
# centisecond that no routine runs in, so the jobs get less time.
# FLIGHT_BUSY_SAMPLE gives the keys and the DSKY trace their sample points.
# lm-p63-overload: lm-p63-descent-thrust with V16N68 at 180 s and 420 MCT a
# centisecond stolen from 185 s to 240 s. This synthetic P63 runs lighter
# than the flight's (no landing radar reads), so the steal that overloads it
# is larger than the flight's 13% or so. SERVICER falls behind: alarm 1201
# (no VAC areas, FINDVAC) at 196 s, then 1202 (no core sets, NOVAC) at
# 210 s, each a BAILOUT that restarts through the phase tables (6
# restarts). V06N63 comes back after each one; the V16N68 monitor does not,
# as in the flight. With the steal gone, P63 updates N63 every 2 s again and
# flies on.
# cm-entry-overload: cm-entry with 600 MCT a centisecond stolen from 210 s
# to 270 s, over P64 and P67: alarm 1201 three times, 6 restarts, and the
# entry still reaches P67.
OVERLOAD_LM = {**P63, 'FLIGHT_ENGINE': '15100', 'FLIGHT_STEAL': '18500-24000:420', 'FLIGHT_BUSY_SAMPLE': '50'}
OVERLOAD_CM = {**ENTRY, 'FLIGHT_STEAL': '21000-27000:600', 'FLIGHT_BUSY_SAMPLE': '50'}
# Coverage scenarios for the Luminary099 program map (docs/acceptance/
# Luminary099-program-map.json): the extended verbs and V37 programs no
# flight scenario reached. Each starts from lm-rendezvous's load (the LM
# and the CSM in lunar orbit, tools/flight_scenarios/lm-rendezvous.load)
# or, for the AOT and surface rows, from lm-p52-realign's core and
# lm-p57's load.
# lm-coast-verbs: the coast verbs one after another, each ended by the
# crew: V64E (V06N51 S-band angles, V34E), V66E (vehicles attached), V61E
# and V62E (the attitude error needles' source), V75E and V65E (U,V jets),
# V76E and V77E (minimum impulse, rate command), V93E (RENDWFLG cleared),
# V95E (no state vector update), V67E (V06N99, the W-matrix errors, PRO),
# V90E (V06N16 time, PRO; V06N90 out-of-plane, PRO), V47E (R47: V06N16,
# PRO; the AGS downlist for 20 s and the IMU zero, then V50N16, PRO),
# V44E (RR continuous designate ended: RADMODES bit 15 cleared), V80E and
# V81E (LM and CSM state vector updates), V43E (V21N22 needle angles
# +1.00, +2.00, -3.00 degrees loaded; V43 again and the CDU error
# commands), V49E (R62: V06N22, PRO; R60's V50N18, PRO; V34E), V89E
# (R63: V04N12 option 3, PRO; V06N18, V34E), V74E (erasable dump on the downlink), V42E (V21N93 fine
# align torquing, +0.010, 0, -0.010 degrees), V41N20E (V21N22 coarse
# align to +1.00, 0, 0 degrees), V40N20E (CDUs zeroed) and V96E
# (integration stopped, P00).
COAST_VERBS_KEYS = ('@2600V64E@3500V34E@4000V66E@4500V61E@5000V34E@5500V62E@6000V34E@6500V75E@7000V65E@7500V76E'
                    '@8000V77E@8500V93E@9000V95E@9500V67E@10500V33E@11000V90E@12000V33E@12800V33E@13500V34E@14000V47E'
                    '@15000V33E@18500V33E@19000V44E@19500V80E@20000V81E@20500V43E@20900+00100E+00200E-00300E@22000V49E@23000V33E'
                    '@24000V33E@25000V34E@25500V89E@26500V33E@27500V34E@28000V74E@29000V42E'
                    '@29500+00010E+00000E-00010E@31000V41N20E@31500+00100E+00000E+00000E@34000V40N20E@35500V96E')
# V44's body runs only in continuous designate (RADMODES, FLGWRD12 at
# 0110, negative): FLIGHT_POKE sets bit 15 at 188 s, as a designate the crew
# ends would. V43's needs the PGNS mode switch off (channel 31 bits 13 and
# 14 both 1) from 204 s to 220 s; LM_RDZ's 057777 is auto.
COAST_VERBS = {**LM_RDZ, 'FLIGHT_POKE': '18800:0110=040100', 'FLIGHT_CHAN': '20400:031=077777,22000:031=057777'}
# lm-coast-v55: V55E (V21N24, the AGC clock's correction, loaded as 0 h,
# 0 min, 0.00 s). The N24 load goes through Pinball's HMSIN.
COAST_V55_KEYS = '@2500V55E@3000+00000E+00000E+00000E'
# lm-coast-r04: V63E (R04, the radar self test): V04N12 option 4 (the
# RR), PRO; V16N72 shows the RR angles while the test drives them; V34E.
COAST_R04_KEYS = '@2500V63E@4000V33E@6000V34E'
# lm-coast-programs: V37E21E (P21: V04N06 option, PRO; V06N34 time 0,
# PRO; V06N43 the LM's latitude, longitude and altitude, PRO); V37E31E
# (P31, the Lambert aimpoint: V06N33 TIG, PRO; V06N81, V06N42 and V06N45,
# PRO each); V37E25E (P25: R60's V50N18 maneuver to the tracking
# attitude, PRO); V37E06E (P06: V50N25 00062, PGNCS power down, PRO).
COAST_PROGRAMS_KEYS = ('@2500V37E21E@4000V33E@5500V33E@7000V33E@8500V37E31E@10000V33E@11500V33E@13000V33E'
                       '@14500V33E@16000V33E@17000V37E25E@18500V33E@20000V37E06E@21500V33E')
# lm-p51: P51 in flight from lm-p52-realign's core (its FLIGHT_POKE puts
# each star on the forward detent's axis): V37E51E, proceed on V50N25
# 00015; for each star (Regulus 22, then Gienah 24) its code loaded on
# V01N71, PRO, an X then a Y mark (V53 after the X mark; channel 16,
# KEYRUPT2), PRO on V54N71; PRO on V06N05 (star angle difference 0.00);
# P51 ends in V37.
LM_P51_KEYS = ('@200V37E51E@1000V33E@1300V21E00222E@2000V33E@3500x@3600y@4000V33E@4300V21E00224E@5000V33E'
               '@6000x@6100y@6500V33E@7100V33E')
# lm-p22-surface: P22 on the lunar surface from lm-p57's load: V37E22E,
# PRO on V04N06; the CSM is beyond the RR's 400 nm, so R21 flashes alarm
# 526 on V05N09 until V34E. (There is no surface RR model, so tracking is
# not reached.)
LM_P22_KEYS = '@3000V37E22E@4000V33E@5000V33E@6000V33E@7000V34E'
# lm-rhc-att-hold: lm-dap's start (DAP, below; no V37), then ATTITUDE HOLD
# at 20 s (channel 31 bit 13 on, AUTO off) and the hand controller
# (FLIGHT_RHC, tools/flight_rhc.h) out of detent twice: pitch +20 counts
# for 6 s, then yaw -12 and roll +15 for 6 s. The DAP's manual rate command
# reads the counters each pass (P-AXIS_RCS_AUTOPILOT RATERROR, Q,R alike).
RHC = {'FLIGHT_CH30': '036377', 'FLIGHT_CH31': '057777', 'FLIGHT_POKE': '1500:035=02000', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_CHAN': '2000:031=067777', 'FLIGHT_RHC': '2500-3100:20,0,0;3600-4200:0,-12,15'}
# lm-rhc-att-turn: lm-rhc-att-hold with the LM turning under its jets
# (FLIGHT_ATT, tools/flight_attitude.h): each stick deflection starts a turn
# the DAP's rate command holds, and the DAP stops it at release; CDUX-Z
# follow the body.
RHC_ATT = {**RHC, 'FLIGHT_ATT': '1'}
FLIGHT = [('lm-p63-descent', 'lm', 25000, P63_KEYS, P63),
          ('lm-p63-descent-thrust', 'lm', 30000, P63_KEYS, {**P63, 'FLIGHT_ENGINE': '15100'}),
          ('cm-entry', 'cm', 32000, ENTRY_KEYS, ENTRY),
          ('cm-p52', 'cm', 20000, P52_KEYS, P52),
          ('cm-p23', 'cm', 30000, P23_KEYS, P23),
          ('cm-p40', 'cm', 74000, P40_KEYS, P40),
          ('cm-p11', 'cm', 90000, P11_KEYS, P11),
          ('cm-tli', 'cm', 70000, TLI_KEYS, TLI),
          ('cm-p22', 'cm', 23000, P22_KEYS, P22),
          ('cm-tei', 'cm', 56000, TEI_KEYS, TEI),
          ('cm-p37', 'cm', 51000, P37_KEYS, P37),
          ('lm-p57', 'lm', 56000, P57_KEYS, P57),
          ('cm-p20', 'cm', 25000, CM_P20_KEYS, CM_P20),
          ('lm-rendezvous', 'lm', 40000, LM_RDZ_KEYS, LM_RDZ),
          ('lm-rendezvous-rr-dropout', 'lm', 40000, LM_RDZ_RR_DROP_KEYS, LM_RDZ_RR_DROP),
          ('lm-csi', 'lm', 23000, LM_CSI_KEYS, LM_CSI),
          ('lm-cdh', 'lm', 23500, LM_CDH_KEYS, LM_CDH),
          ('lm-cdh-csm', 'lm', 23500, LM_CDH_CSM_KEYS, LM_CDH_CSM),
          ('lm-rendezvous-tpm', 'lm', 71000, LM_TPM_KEYS, LM_RDZ),
          ('lm-stable-orbit', 'lm', 67000, LM_SO_KEYS, LM_RDZ),
          ('lm-stable-orbit-lm', 'lm', 67000, LM_SO_KEYS.replace('V37E78E', 'V37E38E').replace('V37E79E', 'V37E39E'), LM_RDZ),
          ('cm-csi', 'cm', 24000, CM_CSI_KEYS, CM_CSI),
          ('cm-cdh', 'cm', 20500, CM_CDH_KEYS, CM_CDH),
          ('cm-cdh-csm', 'cm', 20500, CM_CDH_CSM_KEYS, CM_CDH_CSM),
          ('cm-rendezvous-tpm', 'cm', 79000, CM_TPM_KEYS, CM_P20),
          ('cm-stable-orbit', 'cm', 74000, CM_SO_KEYS, CM_P20),
          ('cm-stable-orbit-lm', 'cm', 74000, CM_SO_KEYS.replace('V37E38E', 'V37E78E').replace('V37E39E', 'V37E79E'), CM_P20),
          ('cm-tpi-search', 'cm', 44000, CM_TPI_SEARCH_KEYS, CM_P20),
          ('cm-p20-vhf-dropout', 'cm', 32000, CM_P20_VHF_DROP_KEYS, CM_P20_VHF_DROP),
          ('lm-p63-lr-dropout', 'lm', 115000, NASSP_KEYS, LR_DROP),
          ('lm-p63-lr-update', 'lm', 110000, LR_UPDATE_KEYS, LR_UPDATE),
          ('lm-p63-lr-v58', 'lm', 96000, LR_V58_KEYS, LR_UPDATE),
          ('lm-p63-landing-displays', 'lm', 70000, LR_UPDATE_KEYS, LR_DISPLAYS),
          ('lm-p63-v60-nassp', 'lm', 115000, NASSP_KEYS + '@100000V60E', V60),
          ('lm-uplink-reject', 'lm', 9000, '', UPLINK),
          ('lm-uplink-accept', 'lm', 14000, '', UPLINK_ACCEPT_LM),
          ('cm-uplink-accept', 'cm', 14000, '', UPLINK_ACCEPT_CM),
          ('cm-uplink-reject', 'cm', 9000, '', UPLINK),
          ('lm-p63-nassp', 'lm', 115000, NASSP_KEYS, NASSP),
          ('lm-p52-realign', 'lm', 10000, LM_P52_KEYS, LM_P52),
          ('lm-p52-imu-fail', 'lm', 10000, LM_P52_KEYS, LM_P52_IMU_FAIL),
          ('lm-p52-imu-cage', 'lm', 10000, LM_P52_KEYS, LM_P52_IMU_CAGE),
          ('cm-p52-imu-fail', 'cm', 20000, P52_KEYS, CM_P52_IMU_FAIL),
          ('cm-p52-imu-cage', 'cm', 20000, P52_KEYS, CM_P52_IMU_CAGE),
          ('cm-p52-gimbal-lock', 'cm', 21000, P52_KEYS, CM_P52_GIMBAL_LOCK),
          ('cm-p20-crew-verbs', 'cm', 32000, CM_P20_CREW_KEYS, CM_P20),
          ('cm-p23-attached', 'cm', 30000, P23_ATTACHED_KEYS, P23),
          ('cm-v49-maneuver', 'cm', 15000, V49_KEYS, P40),
          ('cm-v49-att', 'cm', 22000, V49_ATT_KEYS, {**P40, 'FLIGHT_ATT': '1'}),
          ('cm-p21', 'cm', 25000, P21_KEYS, P52),
          ('cm-p52-coarse', 'cm', 14000, CM_P52_COARSE_KEYS, CM_P52_COARSE),
          ('lm-p52-cage-torque', 'lm', 10000, LM_P52_KEYS, LM_P52_TORQUE),
          ('lm-p52-coarse', 'lm', 6000, LM_P52_COARSE_KEYS, LM_P52_COARSE),
          ('cm-p52-optics', 'cm', 20000, P52_KEYS, CM_P52_OPTICS),
          ('cm-p41', 'cm', 53000, CM_P41_KEYS, CM_P41),
          ('cm-v48-dap', 'cm', 12000, CM_V48_KEYS, P40),
          ('cm-p40-sps-fail', 'cm', 37000, P40_SPS_FAIL_KEYS, P40_SPS_FAIL),
          ('lm-landing-ascent-nassp', 'lm', 215000, LANDING_KEYS, LANDING),
          ('lm-p65-autoland-nassp', 'lm', 140000, P65_KEYS, P65),
          ('lm-p67-nassp', 'lm', 143000, P67_KEYS, P67),
          ('lm-doi-nassp', 'lm', 65000, DOI_KEYS, DOI),
          ('lm-p47-nassp', 'lm', 7500, P47_KEYS, P47),
          ('lm-rhc-att-hold', 'lm', 5500, '', RHC),
          ('lm-rhc-att-turn', 'lm', 5500, '', RHC_ATT),
          ('lm-p41-nassp', 'lm', 57500, P41_KEYS, P41),
          ('lm-p42-nassp', 'lm', 57000, P42_KEYS, P42),
          ('lm-abort-p70-nassp', 'lm', 180000, NASSP_KEYS, ABORT_P70),
          ('lm-abort-p71-nassp', 'lm', 180000, NASSP_KEYS, ABORT_P71),
          # lm-p63-descent-restart: lm-p63-descent-thrust with a hardware
          # restart at 200 s (FLIGHT_GOJAM; tools/flight_restart_check.py).
          ('lm-p63-descent-restart', 'lm', 30000, P63_KEYS, {**P63, 'FLIGHT_ENGINE': '15100', 'FLIGHT_GOJAM': '20000'}),
          # lm-p63-trim: lm-p63-descent-thrust with the thrust along the DPS
          # trim gimbal (FLIGHT_TRIM; tools/flight_dps_gimbal.h).
          ('lm-p63-trim', 'lm', 30000, P63_KEYS, {**P63, 'FLIGHT_ENGINE': '15100', 'FLIGHT_TRIM': '1'}),
          # lm-p63-att: lm-p63-trim with the LM turning under its jets and
          # the trim gimbal's torque (FLIGHT_ATT; tools/flight_attitude.h).
          ('lm-p63-att', 'lm', 30000, P63_KEYS, {**P63, 'FLIGHT_ENGINE': '15100', 'FLIGHT_TRIM': '1', 'FLIGHT_ATT': '1'}),
          ('lm-p63-overload', 'lm', 27000, P63_KEYS + '@18000V16N68E', OVERLOAD_LM),
          ('cm-entry-overload', 'cm', 32000, ENTRY_KEYS, OVERLOAD_CM),
          ('lm-coast-verbs', 'lm', 36500, COAST_VERBS_KEYS, COAST_VERBS),
          ('lm-coast-v55', 'lm', 5000, COAST_V55_KEYS, LM_RDZ),
          ('lm-coast-r04', 'lm', 7000, COAST_R04_KEYS, LM_RDZ),
          ('lm-coast-programs', 'lm', 23000, COAST_PROGRAMS_KEYS, LM_RDZ),
          ('lm-p51', 'lm', 8000, LM_P51_KEYS, LM_P52),
          ('lm-p22-surface', 'lm', 8000, LM_P22_KEYS, P57),
          ('lm-p63-overload-nomon', 'lm', 27000, P63_KEYS, OVERLOAD_LM)]
# Restarts for the whole-program acceptance (recovery_and_restart_tests,
# one per phase group): a FLIGHT scenario with a hardware restart
# (FLIGHT_GOJAM) at a centisecond in that phase, run to an end centisecond.
# tools/flight_restart_check.py checks each on the host and the C64;
# tools/flight_scenario_vice.py --replay --snap-cs <GOJAM cs> replays the
# recovery on the C64 (the GOJAM comes before the snapshot at the same
# sample point). The host differential does not run these.
RESTART_POINTS = [  # (name, scenario, GOJAM cs, end cs, phase)
    ('lm-p00-doi-restart', 'lm-doi-nassp', 64000, 65000, 'P00 after the DOI burn and V82 (V34E at 630 s)'),
    ('lm-doi-burn-restart', 'lm-doi-nassp', 52000, 57000, 'DOI P40 burn, about 30 s after ignition'),
    ('lm-p12-restart', 'lm-landing-ascent-nassp', 185000, 190000, 'P12 ascent burn, mid-burn'),
    ('lm-rendezvous-restart', 'lm-rendezvous', 20000, 25000, 'P20 radar tracking'),
    ('cm-p00-p40-restart', 'cm-p40', 73000, 74000, 'P00 after the LOI-1 burn (V37E00E at 720 s)'),
    ('cm-p40-burn-restart', 'cm-p40', 50000, 55000, 'P40 SPS burn, TVC DAP running, mid-burn'),
    ('cm-entry-restart', 'cm-entry', 25000, 32000, 'P64 entry, groups 1, 5 and 6 active'),
    ('cm-p20-restart', 'cm-p20', 22000, 25000, 'P20 rendezvous navigation, R22 running'),
    # Ends at 21000, before the V37E00E at 21500, so the major-mode check
    # sees P22 resumed, not the P00 both runs reach (#762).
    ('cm-p22-restart', 'cm-p22', 14000, 21000, 'P22 orbital navigation, between the marks'),
]
RESTART_FLIGHT = [(name, base[1], end, base[3], {**base[4], 'FLIGHT_GOJAM': str(at)})
                  for name, scen, at, end, _ in RESTART_POINTS
                  for base in [next(x for x in FLIGHT if x[0] == scen)]]


# TC trap restarts: P00 idle, then V69E at 23.5 s. VERB69 is a TC to
# itself, so the TC trap restarts the computer (yaAGC's own trap unscaled,
# yaagc_trap in tools/flight_host_run.c; the spin at VERB69 under the work
# clock there and in the C64's rk_resident_step; docs/FLIGHT-RESTART.md):
# GOPROG with no group active, back to P00. tools/flight_restart_check.py
# checks them on the host (lm-v69-restart, cm-v69-restart);
# tools/flight_scenario_vice.py --scenario lm-v69 --snap-cs 2300 --replay on
# the C64. Like RESTART_FLIGHT, the host differential does not run them;
# tools/x86_flight_scenarios.py flies them on the x86 program.
V69_FLIGHT = [('lm-v69', 'lm', 4000, '@2350V69E', {'FLIGHT_SWAP_MCT': '0'}),
              ('cm-v69', 'cm', 4000, '@2350V69E', {'FLIGHT_SWAP_MCT': '0'})]

# The night watchman (docs/FLIGHT-RESTART.md, The night watchman):
# lm-p52-watchman is lm-p52-realign with SAVQR52 (QTEMP1, 03665) poked at
# 24 s, during R52's V01N70, to point to itself. R52's GOTO SAVQR52 after
# the R60 maneuver then loops in GOTOERS (an erasable CADR cycle) until
# the night watchman restarts the AGC (35.84 s on the harness's work
# clock); R52 restarts at V01N70 and P52 goes on. Like V69_FLIGHT, the host
# differential does not run it; tools/x86_flight_scenarios.py flies it on
# the x86 program.
WATCHMAN_FLIGHT = [('lm-p52-watchman', 'lm', 10000, LM_P52_KEYS,
                    {**LM_P52, 'FLIGHT_POKE': '2400:03665=03665,' + LM_P52['FLIGHT_POKE']})]


def reach_points():
    """{FLIGHT scenario: [point, ...]}: the bodies the program maps' rows
    name (a row's `reach`, [[point, label], ...]) for the host run to
    watch for (FLIGHT_REACH, tools/flight_host_run.c)."""
    out = {}
    for path in sorted((ROOT / 'docs/acceptance').glob('*-program-map.json')):
        for row in json.loads(path.read_text()).get('rows', []):
            for point, *_ in row.get('reach', []):
                for s in row.get('scenarios', []):
                    if point not in out.setdefault(s, []):
                        out[s].append(point)
    return out


def scenario(name):
    """A FLIGHT, RESTART_FLIGHT, V69_FLIGHT or WATCHMAN_FLIGHT row by name."""
    return {s[0]: s for s in FLIGHT + RESTART_FLIGHT + V69_FLIGHT + WATCHMAN_FLIGHT}[name]


# Scaled time: (mission, centiseconds, keys, FLIGHT_SCALED=K,D,LAG[,SEED],
# name). x1: the C64 as fast as the AGC; x0.5: the half-speed build
# (RK_TIME_DIVISOR=2) on a C64 1.7 times slower than the AGC (the measured
# LM DAP load); overrun: four times slower with each span's cost jittered
# from 1/4 to 4 times, so most boundaries drop ticks; work: the work clock
# (FLIGHT_SCALED=W), each routine's calibrated weight, no tick dropped.
# lm-dap: the DAP running (IMU operate, PGNCS control, AUTO; CDUY moved at
# 15 s to drive its jets), T5RUPT and T6RUPT every period; no swap cost
# (FLIGHT_SWAP_MCT=0: with 20 ms swaps P and QR swaps starve the DAP into
# restarts and P12 is lost). V37 goes in at 16 s, after the IMU turn-on
# (ISSUP): before it, V37 is rejected (CANTROD) and MODREG stays 77777.
DAP = {'FLIGHT_CH30': '036377', 'FLIGHT_CH31': '057777', 'FLIGHT_POKE': '1500:035=02000', 'FLIGHT_SWAP_MCT': '0'}
DAP_KEYS = '@1600V37E12E'
SCALED = [(m, cs, keys, env, sched, tag)
          for m, cs, keys, env in (('lm', 3000, 'V37E12E', {}), ('lm', 3000, DAP_KEYS, DAP), ('cm', 3000, 'V37E06E', {}))
          for sched, tag in (('1,1,2', 'x1'), ('1.7,2,2', 'x0.5'), ('4,1,2,1', 'overrun'), ('W', 'work'))]


def _tmp(path):
    """A per-process name next to path. Several c64 jobs in one shard build
    these files in parallel; each writes its own temp file and renames it in
    place, so a link never reads another job's half-written object."""
    return path.with_name(f'{path.name}.{os.getpid()}.{threading.get_ident()}.tmp')


def scaled_watchman_bad(watchman, sched):
    """A scaled row's night watchman fails: it sounded, the native rule
    missed a NEWJOB access yaAGC made (any schedule), or on the work clock
    the two disagree at all. No summary line fails too."""
    if not watchman:
        return True
    return bool(watchman['alarms'] or watchman['native_armed'] or (sched == 'W' and watchman['disagree']))


def frozen_engine(cc):
    """build/agc_engine_hardware_dv.c with AgcFrozenClock: when set, the
    scaler (and with it every counter timer and the cycles they take) and
    yaAGC's output drives (IMU CDU, optics, gyro) stop. Unset, the engine is
    unchanged."""
    src = (ROOT / 'build/agc_engine_hardware_dv.c').read_text()
    loop = '  while (State->ScalerCounter >= SCALER_OVERFLOW)\n'
    assert src.count(loop) == 1
    src = src.replace(loop, '  if (AgcFrozenClock)\n    State->ScalerCounter = 0;\n'
                      '  while (!AgcFrozenClock && State->ScalerCounter >= SCALER_OVERFLOW)\n')
    # The IMU CDU drive bursts on yaAGC's cycle counter, a hardware clock
    # too: under the frozen clock it stops with the scaler.
    burst = '  if (i != 0 && (State->CycleCounter - ImuCduCount) >= IMUCDU_BURST_CYCLES)'
    assert src.count(burst) == 1
    src = src.replace(burst, burst.replace('if (', 'if (!AgcFrozenClock && ', 1))
    # yaAGC's optics drive dumps RegOPTX and RegOPTY (036 and 035) while
    # channel 14 bit 11 or 12 is set and zeroes them: those are the
    # sextant's CDU counters CDUS and CDUT, not the drive counters CDUSCMD
    # and CDUTCMD (054, 053), so after a coarse optics drive yaAGC loses
    # the angles the crew marks on (CM P52). Under the frozen clock it
    # stops with the IMU drive; the C64 resident's drive and the host's
    # optics model (flight_host_run.c drives, optics_due) run instead.
    optics = '  if (State->Erasable[0][RegOPT'
    assert src.count(optics) == 2
    src = src.replace(optics, '  if (!AgcFrozenClock && State->Erasable[0][RegOPT')
    # yaAGC's gyro drive takes GYROCMD (047) whole and zeroes it the
    # instruction after channel 14 bit 10 comes on, where the hardware
    # sends 3,200 pulses a second (P52's torquing times itself on WAITLIST
    # by that rate): under the frozen clock it stops too, and the C64
    # resident's drive (32 pulses a period, resident_cdu.h) runs instead.
    gyro = '  if (0 != (State->InputChannel[014] & 01000))\n    if (0 != State->Erasable[0][RegGYROCTR])'
    assert src.count(gyro) == 1
    src = src.replace(gyro, gyro.replace('if (0 != (', 'if (!AgcFrozenClock && 0 != (', 1))
    head = '#include "agc_engine.h"\n'
    assert src.count(head) == 1
    src = src.replace(head, head + 'int AgcFrozenClock;\n')
    OUT.mkdir(parents=True, exist_ok=True)
    c, o = OUT / 'agc_engine_scaled.c', OUT / 'agc_engine_scaled.o'
    tc = _tmp(c)
    tc.write_text(src)
    os.replace(tc, c)
    to = _tmp(o)
    subprocess.run([cc, '-O2', f'-I{YAAGC}', '-c', str(c), '-o', str(to)], cwd=ROOT, check=True)
    os.replace(to, o)
    return o


def build(cc, m, engine, store=False, chunked=False):
    """The host run for mission m. store=True: it is compared with the C64
    store, so it must carry the store's chunk list (<m>-host-chunks.inc,
    written by the full store build). A --host-only build that ran after the
    store build deletes that list, and the host then runs without chunk
    tracking: its work clock no longer matches the C64's (10-25: the LM
    check disk's expected CRC came from such a host). chunked=True builds
    with the list (as store) under a name of its own, -chunked."""
    chunks = ROOT / f'build/flight-store/{m}-host-chunks.inc'
    store = store or chunked
    if store and not chunks.exists():
        raise SystemExit(f'{chunks.relative_to(ROOT)} is missing, so this host run would not match the C64 store: '
                         f'run tools/flight_store_build.py --missions {m} (not --host-only) first')
    tag = '-chunked' if chunked else ''
    exe = OUT / f'flight-host-run-{m}{tag}'
    obj = OUT / f'flight_host_run_{m}{tag}.o'
    subprocess.run([cc, '-std=c99', '-O1', '-g', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', f'-I{YAAGC}',
                    f'-DRK_{m.upper()}=1', f'-DFLIGHT_HOST_TABLES="../build/flight-store/{m}-host.inc"',
                    f'-DFLIGHT_HOST_SITES="../build/flight-store/{m}-sites.inc"',
                    *([f'-DFLIGHT_HOST_CHUNKS="../build/flight-store/{m}-host-chunks.inc"']
                      if chunks.exists() else []),
                    '-c', 'tools/flight_host_run.c', '-o', str(_tmp(obj))], cwd=ROOT, check=True)
    os.replace(_tmp(obj), obj)
    # Every host/*.o is linked: refuse objects left from another checkout
    # or source (10-19: a pre-#734 flight_io.o failed the CM channel compare).
    import flight_store_build
    stale = flight_store_build.stale_host_objects(cc)
    if stale:
        raise SystemExit(f'{len(stale)} host objects are stale ({", ".join(stale[:5])}...): '
                         'run tools/flight_store_build.py --host-only first')
    objs = sorted(str(p) for p in (OUT / 'host').glob('*.o'))
    subprocess.run([cc, str(obj), *objs, str(engine), str(YAAGC / 'agc_engine_init.o'),
                    str(YAAGC / 'rfopen.o'), '-Wl,--gc-sections', '-lm', '-o', str(_tmp(exe))], cwd=ROOT, check=True)
    os.replace(_tmp(exe), exe)
    return exe


# The interpretive words' charge is a fitted per-order table
# (tools/interp_word_mct.py, include/interp_word_mct_table.h), the same on
# the host and the C64. Each scaled run reports it against yaAGC's own MCT
# over the same spans (flight_host_run.c "interpreter charge", issue #765).
# Two bounds, both needed for a run of at least CHARGE_MIN_ROUTINES
# routines:
# - the net error (fitted total against yaAGC's total) within
#   CHARGE_BOUND percent;
# - the absolute error routine by routine (the sum of |fitted - yaAGC|
#   over yaAGC's total) within CHARGE_ABS_BOUND percent, so routines
#   charged too much cannot hide routines charged too little.
# The per-order table ignores data-dependent timing, so the absolute
# error is large even when the net is small: measured on da027bee, 58
# scaled runs of 1000+ routines, net -0.7..+17.8% (cm-p11) and absolute
# 14.4..39.3% (cm-p11), 33.4% next (cm-p41). The absolute bound is a
# regression guard on that measurement, not an accuracy claim; a
# per-span refit would tighten it. yaAGC counts whole spans, so orders
# inside one routine can still cancel. A run beyond either bound is
# interpretive_charge.state 'over', which tools/program_acceptance.py and
# tools/device_acceptance.py fail; a run under CHARGE_MIN_ROUTINES is
# 'few', reported in both documents but not bounded.
CHARGE_BOUND = 20.0
CHARGE_ABS_BOUND = 45.0
CHARGE_MIN_ROUTINES = 1000


def charge_verdict(charge):
    """The interpretive_charge row of one scaled run, from its report line
    (fitted, yaagc, percent, routines, mean_abs_error), with abs_percent,
    bound, state ('ok', 'over' or 'few') and text."""
    c = dict(charge)
    c['abs_percent'] = round(100.0 * c['mean_abs_error'] * c['routines'] / c['yaagc'], 1) if c['yaagc'] else 0.0
    c['bound'] = dict(net=CHARGE_BOUND, routine=CHARGE_ABS_BOUND)
    if c['routines'] < CHARGE_MIN_ROUTINES:
        c['state'] = 'few'
    else:
        c['state'] = 'over' if abs(c['percent']) > CHARGE_BOUND or c['abs_percent'] > CHARGE_ABS_BOUND else 'ok'
    c['text'] = (f"interpretive charge {c['percent']:+.1f}% net, {c['abs_percent']:.1f}% routine by routine, "
                 f"from yaAGC's MCT over {c['routines']} routines; "
                 + {'ok': f'within {CHARGE_BOUND:g}% net and {CHARGE_ABS_BOUND:g}% routine by routine',
                    'over': f'OVER the bound ({CHARGE_BOUND:g}% net, {CHARGE_ABS_BOUND:g}% routine by routine)',
                    'few': f'fewer than {CHARGE_MIN_ROUTINES} routines, not bounded'}[c['state']])
    return c


def charge_state(row):
    """(state, text) of a summary row's interpretive charge: None, '' for a
    row without one (an older summary, or a run with no interpretive
    routine)."""
    c = row.get('interpretive_charge')
    if not c:
        return None, ''
    if 'state' not in c:
        c = charge_verdict(c)
    return c['state'], c['text']


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-index', action='store_true', help='reuse build/flight-store/<m>-host.inc and host objects')
    p.add_argument('--chunked', action='store_true',
                   help='also the chunked rows: builds the C64 store first (llvm-mos) for its chunk lists')
    p.add_argument('--chunked-only', action='store_true',
                   help='only the chunked rows (make flight-chunked-lockstep), summary in summary-chunked.json')
    a = p.parse_args()
    engine = frozen_engine(a.host_cc)
    # The chunked rows (#926 F3): the host the C64 is compared with (replays,
    # check disk) tracks which chunk of a split table the job slot holds, and
    # its work clock with it, a schedule of its own. These rows run that host
    # on the work clock in lockstep with yaAGC, as the -scaled-work rows run
    # the host-only build's, and fail on any routine yaAGC does not match.
    # They take the chunk lists of a current full store build, so they are
    # built before the --host-only build below can delete the lists.
    chunked = {}
    if a.chunked or a.chunked_only:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-cc', a.host_cc], cwd=ROOT, check=True)
        chunked = {m: build(a.host_cc, m, engine, chunked=True) for m in ('lm', 'cm')}
    if not a.skip_index and not a.chunked_only:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
    # The other rows' host takes a chunk list only when one is left (a
    # current full store build; CI's host-only build leaves none).
    exe = {m: build(a.host_cc, m, engine) for m in ('lm', 'cm')} if not a.chunked_only else {}
    listed = {m: (OUT / f'{m}-host-chunks.inc').exists() for m in exe}
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    words = {m: flight_holes.listing(flight_holes.MISSION[m]) for m in ('lm', 'cm')}
    lanes = flight_holes.lanes()
    summary, failed = [], 0
    commit = flight_holes.source_commit()
    charge_over, watchman_bad = [], []
    for m, cs, keys in SCENARIOS if exe else ():
        name, env = f'{m}-{keys or "p00-idle"}', {}
        r = subprocess.run([str(exe[m]), ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                           capture_output=True, text=True)
        (EVIDENCE / f'{name}.log').write_text(r.stdout)
        if r.returncode:
            failed += 1
        text = r.stdout
        s = re.search(r'(\d+) samples: (\d+) equal, (\d+) differ', text)
        mode = re.search(r'native (\d+), yaAGC (\d+)', text)
        misses = re.findall(r'^  (?:basic |interp) (\d\d,\d{4}) (\S+): (\d+) times; (.*)$', text, re.M)
        holes = [flight_holes.describe(h, words[m], lanes, m) for h in flight_holes.HOLE.finditer(text)]
        swaps = re.search(r'^module loads \((\d+) in (\d+) cs\)', text, re.M)
        loads = sorted((dict(module=x[0], slot=int(x[1]), kind=x[2].strip(), point=x[3],
                             label=flight_holes.label(words[m], x[3]), count=int(x[4]))
                        for x in re.findall(r'^  load (\S+) slot (\d) (basic |interp) (\d\d,\d{4}): (\d+)$', text, re.M)),
                       key=lambda d: -d['count'])
        row = dict(scenario=name, mission=m, centiseconds=cs, keys=keys, inputs=env, exit=r.returncode, commit=commit,
                   samples=int(s[1]) if s else 0, equal=int(s[2]) if s else 0, differ=int(s[3]) if s else None,
                   major_mode=dict(native=mode[1], yaagc=mode[2]) if mode else None,
                   routine_mismatches=[dict(point=x[0], table=x[1], times=int(x[2]), detail=x[3]) for x in misses],
                   unregistered=holes,
                   module_loads=int(swaps[1]) if swaps else None,
                   loads_per_second=round(int(swaps[1]) * 100 / max(int(swaps[2]), 1), 1) if swaps else None,
                   loads_by_point=loads)
        summary.append(row)
        print(f"{name}: {row['equal']}/{row['samples']} samples equal, {len(row['routine_mismatches'])} routines "
              f"yaAGC did not match, {len(holes)} unregistered or refused points"
              + (f" ({sum(h['core_start'] for h in holes)} core-start artifacts)" if any(h['core_start'] for h in holes) else '')
              + ", "
              f"{row['loads_per_second']} module loads per second"
              + (f", major mode native {mode[1]} yaAGC {mode[2]}" if mode else '') + ('' if not r.returncode else ' FAILED'))
    reach = reach_points()
    scaled = [(f'{m}-{re.sub(r"@[0-9]+", "", keys) or "p00-idle"}{"-dap" if env else ""}-scaled-{tag}', m, cs, keys, env, sched)
              for m, cs, keys, env, sched, tag in SCALED]
    rows = [] if a.chunked_only else scaled
    rows += [(f'{name}-chunked', m, cs, keys, env, sched) for name, m, cs, keys, env, sched in scaled
             if sched == 'W' and m in chunked]
    rows += [] if a.chunked_only else [(f'{name}-scaled-x1', m, cs, keys, env, '1,1,2')
                                       for name, m, cs, keys, env in FLIGHT]
    chunk_misses = []
    for name, m, cs, keys, env, sched in rows:
        points = reach.get(name[:-len('-scaled-x1')], []) if name.endswith('-scaled-x1') else []
        run = chunked[m] if name.endswith('-chunked') else exe[m]
        r = subprocess.run([str(run), ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                           capture_output=True, text=True, env={**os.environ, **env, 'FLIGHT_SCALED': sched,
                                                                **({'FLIGHT_REACH': ','.join(points)} if points else {})})
        (EVIDENCE / f'{name}.log').write_text(r.stdout)
        failed += bool(r.returncode)
        chunk_row = name.endswith('-chunked')
        text = r.stdout
        t = re.search(r'(\d+) CIA interrupts, (\d+) clock periods taken, (\d+) dropped \(rk_dilation\); AGC time (\d+) cs', text)
        n = re.search(r'(\d+) native routines \((\d+) yaAGC instructions of time\), (\d+) bridged', text)
        misses = re.findall(r'^  (?:basic |interp) (\d\d,\d{4}) (\S+): (\d+) times; (.*)$', text, re.M)
        # reach points: every centisecond yaAGC ran one (FLIGHT_REACH); a list
        # the host could not keep whole says how many it dropped before the last
        arrivals = re.findall(r'^reach (\d\d,\d{4}): (?:at ([\d ]+) cs(?: \((\d+) more before the last\))?|not reached)$',
                              text, re.M)
        holes = [flight_holes.describe(h, words[m], lanes, m) for h in flight_holes.HOLE.finditer(text)]
        c = re.search(r'^interpreter charge: fitted (\d+) MCT, yaAGC (\d+) MCT \(([-+][0-9.]+)%\) over (\d+) routines; '
                      r'mean \|error\| ([0-9.]+) MCT', text, re.M)
        charge = charge_verdict(dict(fitted=int(c[1]), yaagc=int(c[2]), percent=float(c[3]), routines=int(c[4]),
                                     mean_abs_error=float(c[5]))) if c else None
        if charge and charge['state'] == 'over':
            charge_over.append(name)
        # The night watchman (docs/FLIGHT-RESTART.md): no row here may sound
        # it, and on the work clock the native rule must agree with yaAGC's
        # NEWJOB accesses at every armed check. The other schedules miss some
        # of yaAGC's NEWJOB reads (#886), which leaves yaAGC's watchman armed
        # where the native rule is not; that direction is only reported. The
        # other (native armed: yaAGC read NEWJOB, the native rule saw no
        # access) fails on every schedule.
        w = re.search(r'^night watchman: (\d+) alarms, (\d+) stuck steps; (\d+) checks while armed, (\d+) where'
                      r'.*?\((\d+) with the native rule armed\)', text, re.M)
        watchman = dict(alarms=int(w[1]), stuck_steps=int(w[2]), checks=int(w[3]), disagree=int(w[4]),
                        native_armed=int(w[5])) if w else None
        if scaled_watchman_bad(watchman, sched):
            watchman_bad.append(name)
        row = dict(scenario=name, mission=m, centiseconds=cs, keys=keys, inputs=env, schedule=sched, chunked=chunk_row or listed[m], exit=r.returncode,
                   commit=commit, cia_interrupts=int(t[1]) if t else None, periods_taken=int(t[2]) if t else None,
                   periods_dropped=int(t[3]) if t else None, agc_centiseconds=int(t[4]) if t else None,
                   native_routines=int(n[1]) if n else None, bridged_instructions=int(n[3]) if n else None,
                   routine_mismatches=[dict(point=x[0], table=x[1], times=int(x[2]), detail=x[3]) for x in misses],
                   unregistered=holes, interpretive_charge=charge, night_watchman=watchman,
                   reached={x[0]: int(x[1].split()[0]) if x[1] else None for x in arrivals},
                   arrivals={x[0]: [int(c) for c in x[1].split()] for x in arrivals},
                   arrivals_dropped={x[0]: int(x[2]) for x in arrivals if x[2]})
        summary.append(row)
        if chunk_row and misses:
            chunk_misses.append(name)
        print(f"{name}: {row['native_routines']} routines, {len(misses)} yaAGC did not match under the schedule; "
              f"{row['periods_taken']} periods taken, {row['periods_dropped']} dropped of {row['cia_interrupts']} "
              f"interrupts" + (f"; {charge['text']}" if charge else '')
              + ('' if not r.returncode else ' FAILED'))
    (EVIDENCE / ('summary-chunked.json' if a.chunked_only else 'summary.json')).write_text(
        json.dumps(summary, indent=1) + '\n')
    if chunk_misses:
        print(f"FAIL: the chunked host ran a routine yaAGC did not match on the work clock, in: {', '.join(chunk_misses)}")
    if charge_over:
        print(f"FAIL: the fitted interpretive charge is more than {CHARGE_BOUND:g}% net or {CHARGE_ABS_BOUND:g}% "
              f"routine by routine from yaAGC's MCT in: {', '.join(charge_over)}")
    if watchman_bad:
        print(f"FAIL: the night watchman sounded, missed a NEWJOB access yaAGC made, or on the work clock "
              f"disagreed with yaAGC's NEWJOB accesses, in: "
              f"{', '.join(watchman_bad)}")
    return 1 if failed or charge_over or watchman_bad or chunk_misses else 0


if __name__ == '__main__':
    sys.exit(main())
