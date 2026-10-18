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
# 43,900 kg stack) burns about 372 s to cutoff (VG under 1 m/s); proceed
# on V16N40 and V16N85, then P00.
P40 = {'FLIGHT_CH30': '037377', 'FLIGHT_CH31': '037777', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_SPS': '43900', 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-p40.load'}
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
# at TIG-5; FLIGHT_SPS (16,400 kg) burns about 154 s to cutoff (N40 VG 6.1
# ft/s); proceed on V16N40 and V16N85, V37E00E; V82: V16N44 9999.9 x 60.0
# nmi, then V34E.
TEI = {'FLIGHT_CH30': '037377', 'FLIGHT_CH31': '037777', 'FLIGHT_CH33': '077767', 'FLIGHT_SWAP_MCT': '0',
       'FLIGHT_SPS': '16400', 'FLIGHT_LOAD': '2000:tools/flight_scenarios/cm-tei.load'}
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
          ('lm-p63-nassp', 'lm', 115000, NASSP_KEYS, NASSP),
          ('lm-p52-realign', 'lm', 10000, LM_P52_KEYS, LM_P52),
          ('lm-landing-ascent-nassp', 'lm', 215000, LANDING_KEYS, LANDING),
          ('lm-doi-nassp', 'lm', 65000, DOI_KEYS, DOI),
          ('lm-abort-p70-nassp', 'lm', 180000, NASSP_KEYS, ABORT_P70),
          ('lm-abort-p71-nassp', 'lm', 180000, NASSP_KEYS, ABORT_P71),
          # lm-p63-descent-restart: lm-p63-descent-thrust with a hardware
          # restart at 200 s (FLIGHT_GOJAM; tools/flight_restart_check.py).
          ('lm-p63-descent-restart', 'lm', 30000, P63_KEYS, {**P63, 'FLIGHT_ENGINE': '15100', 'FLIGHT_GOJAM': '20000'})]
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


def frozen_engine(cc):
    """build/agc_engine_hardware_dv.c with AgcFrozenClock: when set, the
    scaler (and with it every counter timer and the cycles they take) and
    the IMU CDU drive stop. Unset, the engine is unchanged."""
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
    # the angles the crew marks on (CM P52). No C64 resident drives the
    # optics: under the frozen clock it stops with the IMU drive.
    optics = '  if (State->Erasable[0][RegOPT'
    assert src.count(optics) == 2
    src = src.replace(optics, '  if (!AgcFrozenClock && State->Erasable[0][RegOPT')
    # yaAGC's gyro drive takes GYROCMD (047) whole and zeroes it the
    # instruction after channel 14 bit 10 comes on; no C64 resident drives
    # the gyros (P52's torquing times itself on WAITLIST): under the frozen
    # clock the drive stops too.
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


def build(cc, m, engine):
    exe = OUT / f'flight-host-run-{m}'
    obj = OUT / f'flight_host_run_{m}.o'
    subprocess.run([cc, '-std=c99', '-O1', '-g', '-Wall', '-Wextra', '-Werror', '-Iinclude', '-Itests', f'-I{YAAGC}',
                    f'-DRK_{m.upper()}=1', f'-DFLIGHT_HOST_TABLES="../build/flight-store/{m}-host.inc"',
                    f'-DFLIGHT_HOST_SITES="../build/flight-store/{m}-sites.inc"',
                    '-c', 'tools/flight_host_run.c', '-o', str(_tmp(obj))], cwd=ROOT, check=True)
    os.replace(_tmp(obj), obj)
    objs = sorted(str(p) for p in (OUT / 'host').glob('*.o'))
    subprocess.run([cc, str(obj), *objs, str(engine), str(YAAGC / 'agc_engine_init.o'),
                    str(YAAGC / 'rfopen.o'), '-Wl,--gc-sections', '-lm', '-o', str(_tmp(exe))], cwd=ROOT, check=True)
    os.replace(_tmp(exe), exe)
    return exe


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--host-cc', default=os.environ.get('CC', 'cc'))
    p.add_argument('--skip-index', action='store_true', help='reuse build/flight-store/<m>-host.inc and host objects')
    a = p.parse_args()
    if not a.skip_index:
        subprocess.run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--host-cc', a.host_cc],
                       cwd=ROOT, check=True)
    engine = frozen_engine(a.host_cc)
    exe = {m: build(a.host_cc, m, engine) for m in ('lm', 'cm')}
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    words = {m: flight_holes.listing(flight_holes.MISSION[m]) for m in exe}
    lanes = flight_holes.lanes()
    summary, failed = [], 0
    for m, cs, keys in SCENARIOS:
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
        row = dict(scenario=name, mission=m, centiseconds=cs, keys=keys, inputs=env, exit=r.returncode,
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
    for name, m, cs, keys, env, sched in ([(f'{m}-{re.sub(r"@[0-9]+", "", keys) or "p00-idle"}{"-dap" if env else ""}-scaled-{tag}', m, cs, keys, env, sched)
                                           for m, cs, keys, env, sched, tag in SCALED]
                                          + [(f'{name}-scaled-x1', m, cs, keys, env, '1,1,2') for name, m, cs, keys, env in FLIGHT]):
        r = subprocess.run([str(exe[m]), ROPE[m], str(cs)] + ([keys] if keys else []), cwd=ROOT,
                           capture_output=True, text=True, env={**os.environ, **env, 'FLIGHT_SCALED': sched})
        (EVIDENCE / f'{name}.log').write_text(r.stdout)
        failed += bool(r.returncode)
        text = r.stdout
        t = re.search(r'(\d+) CIA interrupts, (\d+) clock periods taken, (\d+) dropped \(rk_dilation\); AGC time (\d+) cs', text)
        n = re.search(r'(\d+) native routines \((\d+) yaAGC instructions of time\), (\d+) bridged', text)
        misses = re.findall(r'^  (?:basic |interp) (\d\d,\d{4}) (\S+): (\d+) times; (.*)$', text, re.M)
        row = dict(scenario=name, mission=m, centiseconds=cs, keys=keys, inputs=env, schedule=sched, exit=r.returncode,
                   cia_interrupts=int(t[1]) if t else None, periods_taken=int(t[2]) if t else None,
                   periods_dropped=int(t[3]) if t else None, agc_centiseconds=int(t[4]) if t else None,
                   native_routines=int(n[1]) if n else None, bridged_instructions=int(n[3]) if n else None,
                   routine_mismatches=[dict(point=x[0], table=x[1], times=int(x[2]), detail=x[3]) for x in misses])
        summary.append(row)
        print(f"{name}: {row['native_routines']} routines, {len(misses)} yaAGC did not match under the schedule; "
              f"{row['periods_taken']} periods taken, {row['periods_dropped']} dropped of {row['cia_interrupts']} "
              f"interrupts" + ('' if not r.returncode else ' FAILED'))
    (EVIDENCE / 'summary.json').write_text(json.dumps(summary, indent=1) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
