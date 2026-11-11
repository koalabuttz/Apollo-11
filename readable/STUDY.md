# A study path through a mission

[README.md](README.md) lists every file. This page gives an order to read
them in, following a mission: the computer starts, the crew talks to it,
it lines up its platform, the LM lands, the two craft meet again, and the
CM comes home. Each step says what that part of the computer does, the
files to read in order (under `readable/src/`), the listing pages to
open beside them (from each file's header comment; LM pages are in
`Luminary099/`, CM pages in `Comanche055/`, each marked `# Page N`), and
a flight to watch.

Read one routine at a time, the listing first and then the readable C, as
*How to read it* in the README shows. For every instruction, open the
faithful C each file's header names: it matches the faithful port.

## Before you start: build and fly

```sh
cd readable && make x86-flight     # build/x86-flight/x86-flight-lm and -cm
cd ../c64-port                     # the scenario loads are named from here
R=../readable/build/x86-flight
LM=evidence/fixed-data-lm-original.rope.bin
CM=evidence/fixed-data-cm-original.rope.bin
X86_DSKY=1 $R/x86-flight-lm $LM    # the LM, with a DSKY in the terminal
```

The program starts from GOJAM, the hardware restart. Its arguments are
the rope, how long to fly in centiseconds of AGC time, and the keys to
press (`@2200V37E63E` presses V37E63E at 22 s). On the live DSKY, `v`,
`n`, `e` are VERB, NOUN, ENTER; `c`, `r`, `k` are CLR, RSET, KEY REL; `p`
is PRO; `q` quits. `X86_SPEED=10` flies ten times faster than real time.
`c64-port/docs/X86-FLIGHT.md` lists every setting.

Each row named below is a flight scenario. Its settings and keys are in
the `FLIGHT` list of `c64-port/tools/flight_host_differential.py` (the
check rows of steps 1 and 2: `SCENARIOS` in
`c64-port/tools/x86_flight_check.py`). To fly a row as the gates do,
compared word for word with the faithful port, run from `readable/`
(after `c64-port/tools/setup_cloud.sh` once):

```sh
python3 tests/x86_readable.py scenarios lm-p63-descent-thrust
```

## Step 1. Boot and the Executive

**What it does.** The AGC has a small operating system of its own. The
Executive runs jobs by priority, and WAITLIST runs short tasks at a set
time on the TIME3 counter. Interrupts come in through the lead-ins;
T4RUPT, on TIME4, drives the display relays and watches the IMU. After a
fault the computer restarts and picks each job up again from its phase.

**Read:** `fresh_start/fresh_start.c` (the fresh start and GOPROG, the
restart); `executive/executive.c` (NOVAC, FINDVAC, the job switch);
`waitlist/waitlist.c`; `lead_ins/lead_ins.c`; `t4rupt/t4rupt.c`;
`phase_table/phase_table.c` (how a job records its restart phase);
`restart/restarts.c`.

| Beside it | LM pages | CM pages |
|---|---|---|
| `FRESH_START_AND_RESTART.agc` | 211-224 | 183-194 |
| `EXECUTIVE.agc` | 1103-1116 | 1208-1220 |
| `WAITLIST.agc` | 1117-1132 | 1221-1235 |
| `INTERRUPT_LEAD_INS.agc` | 153-154 | 131-132 |
| `T4RUPT_PROGRAM.agc` | 155-160 | 133-138 |
| `PHASE_TABLE_MAINTENANCE.agc` | 1294-1302 | 1404-1413 |
| `RESTARTS_ROUTINE.agc` | 1303-1308 | 1414-1419 |

**Watch.** Row `lm-v69-rset`: RSET puts the RESTART lamp out at 10 s, V69
forces a restart (a TC trap) at 25 s and the lamp comes on, and RSET puts
it out at 30 s. The program prints each change.

```sh
$R/x86-flight-lm $LM 4000 '@1000R@2350V69E@3000R'
```

## Step 2. The DSKY

**What it does.** The DSKY is the crew's keyboard and display. Each key
is an interrupt, KEYRUPT, which starts PINBALL as a job. PINBALL builds
the verb, noun and data a digit at a time, runs the verb and puts words
on the display. Programs ask for their own displays through the display
interface. Verb 37 changes the program, and ALARM lights PROG.

**Read:** `keyrupt/keyrupt.c`; in `pinball/`, `keys.c` (CHARIN and the
keys), `enter.c` (ENTER and the verb fan), `display_verbs.c`, `monitor.c`
and `nvsub.c` (a display a program asks for);
`pinball_noun_tables/tables.c`; in `display_interface/`, `starts.c`,
`makeplay.c` and `nvdsp.c`; `extended_verbs/front.c` (GOEXTVB, the verbs
from 40 up); `fresh_start/v37.c` (V37 and P00); `alarm/alarm.c`.

| Beside it | LM pages | CM pages |
|---|---|---|
| `KEYRUPT_UPRUPT.agc` | 1338-1340 | 1449-1451 |
| `PINBALL_GAME_BUTTONS_AND_LIGHTS.agc`: keys, ENTER; display verbs | 398-412; 414-417, 424 | 315-329; 331-334, 341-342 |
| same: monitor and alarms; NVSUB | 437-440, 446; 451-459, 464 | 355-358, 364; 369-377, 382 |
| `PINBALL_NOUN_TABLES.agc` | 305-319 | 271-284 |
| `DISPLAY_INTERFACE_ROUTINES.agc` | 1356-1360, 1364-1367 | 1467-1471, 1475-1478 |
| `EXTENDED_VERBS.agc` | 262-264 | 236-238 |
| `FRESH_START_AND_RESTART.agc`: V37, P00 | 224-236 | 194-209 |
| `ALARM_AND_ABORT.agc` | 1381-1385 | 1493-1496 |

**Watch.** Open the live DSKY and type: `v35e` is the lamp test (row
`lm-typed-v35e`), `v37e00ep` asks for P00 and proceeds (row
`lm-typed-v37e00e-pro`), and `v16n36e` shows the clock counting.
Row `cm-v37e06e` changes the CM's program: `$R/x86-flight-cm $CM 3000
V37E06E` ends in major mode 06.

## Step 3. The IMU and alignment

**What it does.** The IMU is a platform held fixed in space by gyros. The
computer reads its gimbal angles through the CDUs and its velocity
change through the PIPAs. The platform drifts, so the crew sights two
stars (the LM's AOT, the CM's sextant), and P52 or P57 finds how far off
it is and torques the gyros back.

**Read:** in `imu_mode_switching/`, `mode_switching.c` (CDU zero, coarse
and fine align), `gyro_torquing.c` and `mode_ends.c`;
`t4rupt/imu_monitor.c`; `inflight_align/inflight_align.c`; in
`p51_p53/`, `p52.c`, `s50_r56.c` (the stars), `r52_r59.c` (the optics),
then `aotmark/aotmark.c`, then `p51.c` and `p57.c`. For the CM:
`p51_p53_cm/p52.c`, `p51_p53_cm/r51_r55.c`, `sxtmark/sxtmark.c`.

| Beside it | LM pages | CM pages |
|---|---|---|
| `IMU_MODE_SWITCHING_ROUTINES.agc` | 1309-1331 | 1420-1442 |
| `T4RUPT_PROGRAM.agc`: IMU monitor | 161-181 | 139-159 |
| `INFLIGHT_ALIGNMENT_ROUTINES.agc` | 1249-1257 | 1355-1363 |
| `P51-P53.agc`: P52 | 926-933 | 737-743 |
| same: S50 and R56; P51; R52 and R59; P57 | 934-940; 949-954; 955-960; 975-983 | |
| same: R51 and R55 | | 756-759 |
| `AOTMARK.agc` / `SXTMARK.agc` | 244-261 | 222-235 |

**Watch.** Row `cm-p52`, the CM's first realignment in earth orbit: the
optics drive to each star, MARK (`M`) is pressed twice, the gyros are
torqued, and it ends in P00.

```sh
X86_DSKY=1 X86_SPEED=10 FLIGHT_CH30=037377 FLIGHT_CH33=077767 \
  FLIGHT_CHAN=4000:033=077757,9000:033=077777,11600:033=077757,13000:033=077777 \
  FLIGHT_LOAD=2000:tools/flight_scenarios/cm-p52.load,9500:tools/flight_scenarios/cm-p52-star1.load,13500:tools/flight_scenarios/cm-p52-star2.load \
  $R/x86-flight-cm $CM 20000 '@4500V37E52E@5500V22E3E@6500V33E@7500V33E@8000V33E@10000M@10500V33E@11000V33E@12000V33E@14000M@14500V33E@15000V33E@16000V33E@17000V33E@18000E@18500V37E00E'
```

The LM's rows `lm-p52-realign`, `lm-p51` and `lm-p57` fly in the suite.

## Step 4. The LM's descent: P63, P64, P66

**What it does.** Every two seconds READACCS reads the PIPAs and SERVICER
moves the state vector on by average G. Each pass the landing guidance
works out the thrust it wants and THROTTLE sets the descent engine. P63
brakes; in P64 the crew can move the landing site with the hand
controller; in P66 the crew sets the rate of descent a click at a time.
The landing radar corrects the altitude and velocity on the way down.

**Read:** `lunar_landing/lunar_landing.c` (P63's ignition algorithm);
`burn_baby/burn_baby.c` (the countdown); in `servicer/`, `readaccs.c`,
`servicer.c`, `average_g.c`; in `landing_guidance/`, `entries.c`,
`pass.c`, `state.c`, `quadguid.c`, `exits.c`, `displays.c`;
`throttle/throttle.c`; `landing_guidance/redesignator.c` (P64);
`landing_guidance/vertguid.c` and `p66.c` (P65 to P67); `servicer/lr_jobs.c`
and `lr_update.c` (the landing radar);
`landing_displays/landing_displays.c` (the meters).

| Beside it | LM pages |
|---|---|
| `THE_LUNAR_LANDING.agc` | 785-792 |
| `BURN_BABY_BURN--MASTER_IGNITION_ROUTINE.agc` | 731-743 |
| `SERVICER.agc`: READACCS, SERVICER; average G; landing radar | 857-863; 880-883; 884-897 |
| `LUNAR_LANDING_GUIDANCE_EQUATIONS.agc` | 798-822 |
| `THROTTLE_CONTROL_ROUTINES.agc` | 793-797 |
| `LANDING_ANALOG_DISPLAYS.agc` | 898-907 |

**Watch.** Row `lm-p63-descent-thrust`: P63 from a loaded state, with a
descent engine model. The crew proceeds on each display; ignition is at
146 s, and the guidance runs every 2 s from 172 s.

```sh
X86_DSKY=1 X86_SPEED=10 FLIGHT_ENGINE=15100 FLIGHT_CH30=036357 FLIGHT_CH31=057777 FLIGHT_CH33=077737 \
  FLIGHT_LOAD=2000:tools/flight_scenarios/lm-p63.load \
  $R/x86-flight-lm $LM 30000 '@2200V37E63E@3400V33E@3800E@4400V33E@9600E@14300V33E'
```

The landing rows start from Apollo 11's own P63 state, so they fly only
through the scenario suite: `lm-p64-redesignation` (one hand controller
pulse in P64), `lm-p63-landing-displays` (the radar and the meters) and
`lm-landing-ascent-nassp` (P64, P66 with the crew's clicks, touchdown,
and P12 back to orbit).

## Step 5. Rendezvous

**What it does.** After the ascent the LM must meet the CM. In P20 the
LM's rendezvous radar tracks the CSM (R21 locks on, R22 takes the
marks), and measurement incorporation works each mark into a state
vector. The CM does the same with its sextant and VHF range. P32 (CSI),
P33 (CDH) and P34 (TPI) work out the burns with the conic routines, P30
takes a burn from the ground, and the P40s fly it.

**Read:** in `p20_p25/`, `programs.c`, `r22_r21.c`, `lrs22.c`,
`lps20.c`, `rr_routines.c`; `measurement_inc/measurement_inc.c` and
`incorp2.c`; `conics/kepler.c`, `newstate.c`, `lambert.c`;
`p32_p35/programs.c`, `csi.c`, `cdh.c`; `p34_p35/p34.c`;
`lambert_aimpoint/lambert_aimpoint.c` (P31); `p30/p30.c`; in
`p40_p47/`, `programs.c`, `s40.c`, `steering.c`, `engine_off.c`. For the
CM: `p20_p25_cm/p20.c`, `r22.c`, `p22.c`.
(`kalman_filter/kalman_filter.c` is not the navigation filter: it times
the LM DAP's jets.)

| Beside it | LM pages | CM pages |
|---|---|---|
| `P20-P25.agc`: the programs; R22 and R21 | 492-503; 504-512 | 562-569 (P20, P22); 579-582 (R22) |
| same: radar routines; LPS20 and LRS22 | 521-537; 564-573 | |
| `MEASUREMENT_INCORPORATION.agc` | 1149-1158 | 1252-1261 |
| `CONIC_SUBROUTINES.agc`: KEPLER; NEWSTATE; LAMBERT | 1174-1179; 1184; 1193-1197 | 1277-1282; 1287; 1296-1300 |
| `P32-P35_P72-P75.agc` | 618-650 | |
| `P34-35_P74-75.agc`: P34 | 658-663 | 461-465 |
| `GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc` | 651-653 | |
| `P30_P37.agc`: P30 | 614-617 | |
| `P40-P47.agc` | 752-772 | |

**Watch.** Row `lm-rendezvous`: P20 locks on, R22 marks the CSM every
61 s, then P34 solves TPI. It ends in P34 (major mode 42, in octal).

```sh
X86_DSKY=1 X86_SPEED=10 FLIGHT_CH30=036377 FLIGHT_CH31=057777 FLIGHT_CH33=077775 \
  FLIGHT_LOAD=2000:tools/flight_scenarios/lm-rendezvous.load FLIGHT_BUSY_SAMPLE=50 \
  FLIGHT_RR=2000:-260373.984,1763762.175,372327.510,1589.402,294.044,-281.432:-198527.341,1801605.032,366129.729,1584.888,233.850,-291.320 \
  $R/x86-flight-lm $LM 40000 '@2200V37E20E@3000V33E@4700E@6100V33E@29000V37E34E@29400V33E@29700V33E@34000V33E@36700V33E@37500V33E@37800V33E'
```

Rows `lm-csi`, `lm-cdh` and `cm-p20` fly CSI, CDH and the CM's side.

## Step 6. The CM's entry

**What it does.** At the end of the mission the CM enters the earth's
atmosphere. P61 and P62 prepare: the displays, the separation from the
service module, the entry attitude. P63 waits for 0.05 g. Then the entry
guidance steers to the landing point by rolling the lift vector: P64
starts it, P65 flies up and out if it must, P66 is the ballistic phase
and P67 the final one. The entry DAP fires the jets to hold the roll.

**Read:** in `p61_p67/`, `p61_p62.c`, `s61_1.c`, `s61_2.c`, `s62_3.c`
(P61 and P62), then `p63_p67.c`; `servicer_cm/servicer.c` (the CM's
two-second pass); in `reentry/`, `startent.c`, `targetng.c`, then the
phases in order, `initroll.c`, `ranger.c`, `upcontrl.c`, `kep.c`,
`glimiter.c`, and `overnout.c` (the end of a pass); in `entry_dap/`,
`dapon.c`, `attrates.c`, `atm_dap.c`, `exdap.c`, `cm_rcs.c`,
`roll_jets.c`.

| Beside it | CM pages |
|---|---|
| `P61-P67.agc` | 789-818 |
| `SERVICER207.agc` | 828-831 |
| `REENTRY_CONTROL.agc` | 844-875 |
| `CM_ENTRY_DIGITAL_AUTOPILOT.agc` | 1066-1089 |

**Watch.** Row `cm-entry`: V37E61E, the crew proceeding on each display,
and an atmosphere model for the drag. It reaches P67 (major mode 103, in
octal) by the end.

```sh
X86_DSKY=1 X86_SPEED=10 FLIGHT_CH30=037377 FLIGHT_AERO=355 \
  FLIGHT_LOAD=2000:tools/flight_scenarios/cm-entry.load \
  $R/x86-flight-cm $CM 32000 '@2200V37E61E@4800V33E@5400V33E@6000V33E@6800V33E@7400V33E'
```

Row `cm-entry-restart` restarts the computer in P64; entry carries on.
