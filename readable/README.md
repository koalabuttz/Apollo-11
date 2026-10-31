# Apollo 11 flight software: readable C

The Apollo Guidance Computer's programs for Apollo 11, Luminary099 (the
Lunar Module) and Comanche055 (the Command Module), rewritten as plain,
readable C for study, one subsystem at a time.

This is the third of three views of the same program:

| | Where | What it is |
|---|---|---|
| 1 | `Luminary099/`, `Comanche055/` | the original assembly listings |
| 2 | `c64-port/` | the faithful C port: one C call per AGC instruction, runs on a Commodore 64 and on x86_64 |
| 3 | `readable/` (here) | the same routines written to be read, x86_64 only |

The readable code is not a summary or a model of the original. It runs
in the real x86_64 flight program in place of the faithful code, and the
gates check it against the faithful port, word for word: the faithful
port is the reference, and it is checked against the original program
run in yaAGC on the host. So the readable code is as exact as the
faithful port, apart from the few places where one of them departs from
the AGC on purpose (see *Departures*), and as far as the gates look (see
*Gates*).

The rules it is written to are in [STYLE.md](STYLE.md). In short: every
AGC name keeps its listing spelling, every routine is a C function named
after its label, and every function names the listing pages it comes from.

## How to read it

Take one routine across the three views. NOVAC starts a basic job:

1. **The listing.** `Luminary099/EXECUTIVE.agc`, page 1103:

   ```
   NOVAC     INHINT
             AD        FAKEPRET   # LOC(MPAC +6) - LOC(QPRET)
             TS        NEWPRIO    # PRIORITY OF NEW JOB + NOVAC C(FIXLOC)
             EXTEND
             INDEX     Q          # Q WILL BE UNDISTURBED THROUGHOUT.
             DCA       0          # 2CADR OF JOB ENTERED.
             DXCH      NEWLOC
             ...
   ```

2. **The faithful C.** `c64-port/src/resident_kernel_body.inc`, label
   `NOVAC:`, the same instructions as calls:

   ```c
   NOVAC:
    INHINT();ADF(KW_FAKEPRET);TS(K_NEWPRIO);DCAF(op);DXCH(K_NEWLOC);
    CAF(KW_EXECBANK);XCH(FB_);TS(K_EXECTEM1);goto NOVAC2;
   ```

3. **The readable C.** [`src/executive/executive.c`](src/executive/executive.c),
   `novac()`, with what it is for:

   ```c
   /* NOVAC -- LM p. 1103, CM p. 1208.
    * Start a job that needs no VAC area (a basic job): A is its priority, the
    * 2CADR after the TC NOVAC is where it starts. ... */
   void novac(const word job[2])
   ```

   and the search for a free core set, the job switch and the rest follow
   the same way, each function headed by its label and pages.

## What is here

| Subsystem | Listing | Readable C | Routines |
|---|---|---|---|
| Executive | `EXECUTIVE.agc` (LM pp. 1103-1116, CM pp. 1208-1220) | [`src/executive/`](src/executive/) | NOVAC, FINDVAC, SPVAC, CHANG1/2, JOBSLEEP, JOBWAKE, PRIOCHNG, ENDOFJOB, the job switch (CHANJOB), the priority scan (EJSCAN) and the idle loop (DUMMYJOB, ADVAN) |
| WAITLIST | `WAITLIST.agc` (LM pp. 1117-1132, CM pp. 1221-1235) | [`src/waitlist/`](src/waitlist/) | WAITLIST, TWIDDLE, FIXDELAY, VARDELAY, T3RUPT, TASKOVER, SVCT3 and LONGCALL; RESUME, NOQRSM and NOQBRSM, the end of every interrupt |
| Inter-bank communication | `INTER-BANK_COMMUNICATION.agc` (LM pp. 998-1001, CM pp. 1103-1106) | [`src/interbank/`](src/interbank/) | BANKCALL, SWCALL, SWRETURN, POSTJUMP, BANKJUMP, MAKECADR, SUPDACAL, IBNKCALL, ISWCALL, ISWRETRN, USPRCADR and SUPERSW: calls and jumps between banks of fixed memory |
| Phase table maintenance | `PHASE_TABLE_MAINTENANCE.agc` (LM pp. 1294-1302, CM pp. 1404-1413) | [`src/phase_table/`](src/phase_table/) | NEWMODEX, NEWMODEA, MMDSPLAY, CHECKMM, SETUPDSP, PHASCHNG, 2PHSCHNG, PHSCHNGA (LM), PHSCHNG2, ONEORTWO and CON2: the major mode on the DSKY, and the phase changes that tell a restart what to start again |
| Restarts | `RESTARTS_ROUTINE.agc` (LM pp. 1303-1308, CM pp. 1414-1419), `RESTART_TABLES.agc` (LM pp. 238-243, CM pp. 211-221) | [`src/restart/`](src/restart/) | RESTARTS, GETPART2, PHSPART2 and the tables they read; NEWPHASE (CM): what the program starts again, group by group, after a restart |
| The interpreter | `INTERPRETER.agc` (LM pp. 1002-1094, CM pp. 1107-1199) | [`src/interpreter/`](src/interpreter/) | INTPRET, the dispatcher (NEWOPS, DANZIG, DIRADRES, INDEX, PUSHUP), loads and stores, vector and scalar arithmetic, DDV and the divides, SQRT, UNIT, SIN and COS, ARCSIN and ARCCOS, the shifts, the index orders, branching (GOTO, CALL, RVQ, BZE, BPL, BOV) and the switch orders: the interpretive language most of the flight program's guidance and navigation is written in |
| Alarms and aborts | `ALARM_AND_ABORT.agc` (LM pp. 1381-1385, CM pp. 1493-1496) | [`src/alarm/`](src/alarm/) | ALARM, ALARM2, BORTENT, PRIOENT, LARMENT, CHKFAIL1/2, FAIL3, PROGLARM, MULTEXIT, MULTFAIL, PRIOLARM, CURTAINS, VARALARM, BAILOUT, WHIMPER, POODOO, ABORT2, CCSHOLE; in the LM also GOPOODOO, STRTIDLE, BAILOUT1, POODOO1, ALARM1, ALMNCADR and FLAGS: the program alarm codes (FAILREG, the PROG light, V05N09) and the software restarts and aborts |
| Single precision sine and cosine | `SINGLE_PRECISION_SUBROUTINES.agc` (LM p. 1102, CM p. 1207) | [`src/single_precision/`](src/single_precision/) | SPCOS, SPSIN, SPT, POLLEY and ARG90: a quick sine or cosine of an angle in A, for the autopilots and displays, by a short polynomial after bringing the angle within 90 degrees |
| Service routines | `SERVICE_ROUTINES.agc` (LM pp. 1374-1380, CM pp. 1485-1492) | [`src/service/`](src/service/) | UPFLAG, DOWNFLAG, COMFLAG, DEBIT, UPENT2, DOWNENT2, DELAYJOB, TCGETCAD, WAKER, GENTRAN and B5OFF; in the CM also TRFAILOF and TRFAILON: setting and clearing flags, putting a job to sleep for a while, copying a block of words |
| Down-telemetry | `DOWN_TELEMETRY_PROGRAM.agc` (LM pp. 988-997), `DOWN-TELEMETRY_PROGRAM.agc` (CM pp. 1093-1102) | [`src/downtelemetry/`](src/downtelemetry/) | DODOWNTM, WOTEST (LM), DNPHASE1, DNPHASE2, NEWLIST, NEXTINCL, SETWO, DODNCHAN, WOZERO, DODNPTR, SNAPLOOP, FETCH2WD, NEXTINSL, DNTMEXIT, DNDUMPI, DNDUMP, DNDUMP1, DNDUMP2 and SENDID: the two words sent to the ground at each DOWNRUPT, from the current downlist or the erasable dump |
| Downlists | `DOWNLINK_LISTS.agc` (LM pp. 193-205, CM pp. 170-180) | [`src/downtelemetry/downlink_lists.c`](src/downtelemetry/downlink_lists.c) | LOWIDCOD, every downlist (control lists and sublists, word by word: DNADR, DNCHAN, DNPTR) and DNTABLE: what DOWNRUPT sends, written as the listing has it and laid out where the listing assembles it |
| Fresh start and restart | `FRESH_START_AND_RESTART.agc` (LM pp. 211-224, CM pp. 183-194) | [`src/fresh_start/`](src/fresh_start/) | SLAP1 (the fresh start: SKIPSIM, DOFSTART, STARTSUB, STARTSB2), MR.KLEAN, P00KLEAN, V37KLEAN, GOPROG (the hardware restart, with BUTTONS, LIGHTSET and the E memory repair), ENEMA and GOPROG2 (the software restart), GOPROG3 and PCLOOP (the phase table check), NXTRST, PINACT, ENDRSTRT, the CM's T5IDLOC, and GOTOPOOH |
| Verb 37 and P00 | `FRESH_START_AND_RESTART.agc` (LM pp. 224-235, CM pp. 194-209), V37 and P00 | [`src/fresh_start/v37.c`](src/fresh_start/v37.c) | GOP00FIX ("please choose a program"), V37 (the astronaut's change of major mode: CANV37, R00 or the CM's R00TOP00, POOH, GOMOD, NOUVEAU, V37XEQ, V37XEQC), the tables PREMM1, FCADRMM1 and DNLADMM1; in the LM also V37RET, RENDN00, KILL2 and SETUP70; in the CM INITSUB, RAKE and VAC5STOR |
| P00's integration | `FRESH_START_AND_RESTART.agc` (LM pp. 229, 236-237, CM pp. 199, 209-210), interpretive code | [`src/fresh_start/stateup.c`](src/fresh_start/stateup.c) | STATEUP (bring both state vectors up to the present), THISVINT, and R00's wait for the integration (CALL INTSTALL, EXIT): interpretive code written in the listing's notation ([`src/interpreter/notation.h`](src/interpreter/notation.h)) and run by the readable interpreter |
| Keyboard and uplink interrupts | `KEYRUPT_UPRUPT.agc` (LM pp. 1338-1340, CM pp. 1449-1451) | [`src/keyrupt/`](src/keyrupt/) | KEYRUPT1, KEYCOM, ACCEPTUP, UPRUPT, UPRPT1, UPTEST, UPOK, CLUPLOCK and TMFAIL2: a key pressed on the DSKY, or an uplink word with its code three times over, handed to a new CHARIN job; a bad uplink word locks the uplink out until ERROR RESET |
| Interrupt lead-ins | `INTERRUPT_LEAD_INS.agc` (LM pp. 153-154, CM pp. 131-132) | [`src/lead_ins/`](src/lead_ins/) | The four words at 04000 + 4n where the AGC goes on interrupt n: GO (the restart), T6RUPT, T5RUPT, T4RUPT, KEYRUPT1, KEYRUPT2, UPRUPT, DOWNRUPT, RADAR RUPT and the LM's RUPT10 or the CM's HAND CONTROL RUPT, each saving A and L, switching to its handler's banks and jumping there; the CM's T5RUPT, which ends the interrupt when TIME5 is within half a second of its next overflow |
| The LM's T6 interrupt | `T6-RUPT_PROGRAMS.agc` (LM pp. 1403-1405) | [`src/t6rupt/`](src/t6rupt/) | DOT6RUPT, T6JOBCHK, T6JOB, ENABLET6, GOCH56, WRITEP, WRITEU and WRITEV: the RCS jets on or off at the moments the autopilot queued on TIME6, the queue moved up and the clock started again |
| T4RUPT, part 1 | `T4RUPT_PROGRAM.agc` (LM pp. 155-160, CM pp. 133-138), its first part | [`src/t4rupt/`](src/t4rupt/) | T4RUPT, NORMT4, CDRVE, DSPOUT, DSPOUTSB, DSPSCAN, DSPLAY, HANG20, SETTIME4, PROCEEDE, T4JUMP, NODSPOUT, QUIKDSP, QUIKRUPT, NODSPY, SYNCT4, QUIKOFF and the CM's OPTTEST: every 120 ms one relay word from the display table to the DSKY, turned off again 20 ms later, the PRO key watched, and one of four checks in turn |
| T4RUPT, part 2 | `T4RUPT_PROGRAM.agc` (LM pp. 161-181, CM pp. 139-159), the IMU monitor | [`src/t4rupt/`](src/t4rupt/) | IMUMON, NXTIFBIT, NXTIFAIL, TNONTEST, PROCTNON, RETNON, ENDTNON, UNZ2, ISSUP, OPONLY, ISSZERO, C33TEST, NXTIBT, NXTFL33, GLOCKMON, GLOCKCHK, NOGIMRUN, SETGLOCK, GLINVERT, GLAMPTST, TLIM, TEMPOK, ITURNON, IMUCAGE, IMUOP, PIPFAIL, DNTMFAST, UPTMFAST, SETISSW, ISSWON, CAGESUB and LAMPTEST: the IMU's status bits (channel 30) and channel 33's flip-flops watched once a second, the IMU's turn-on and cage sequences, the TEMP, ISS WARNING, GIMBAL LOCK and NO ATT lamps, and the gimbal lock monitor |
| T4RUPT, part 3 | `T4RUPT_PROGRAM.agc` (LM pp. 182-189), the LM's radar checks | [`src/t4rupt/`](src/t4rupt/) | RRAUTCHK, RRCDUCHK, TRKFLCDU, RRGIMON, MONREPOS and GPMATRIX (DAPT4S, NORRGMON): the rendezvous radar's auto mode and turn-on, its CDU fail bit and the TRACKER lamp, its gimbal limits and reposition, and the DAP's gimbal-to-pilot matrix elements from the gimbal angles. With it the LM's file is all readable |
| T4RUPT, part 4 | `T4RUPT_PROGRAM.agc` (CM pp. 160-169), the CM's optics | [`src/t4rupt/`](src/t4rupt/) | OPTMON, 33OPTMON, PROCESSW, ZOPTDES, ZTOMAN, ZOPALARM, ZTOCSC, COARSLOK, ZOPFINI, CANZOPT, MANUDES, MANTOMAN, MANTOCSC, CSCDES, CSCTOMAN, CANCOARS, OPTZERO, SETDESMD, SETZOEND, ENDZOPT, ZEROPCDU, INITZOPT, OCDUFTST, OPFAILON, OPFAILOF, OPTDRIVE, CONTDRVE, GETOPCMD, STORCMD, CMDSETUP, TRUNCMD, POSOPCMD, NEXTOPT, NEGOPCMD, SENDOCMD and SETBIT: the optics CDU fail bit and the OCDU FAIL lamp, the optics mode switch, zeroing the optics CDUs, and the coarse optics drive with its shaft stop avoidance. With it the CM's file is all readable |
| RCS failure monitor | `RCS_FAILURE_MONITOR.agc` (LM pp. 190-192) | [`src/rcs_failure_monitor/`](src/rcs_failure_monitor/) | RCSMON (RCSMONIT), VOPENED and 1/ACCFIX, with the tables 5FAILTAB and 6FAILTAB: every 0.48 s, a jet isolation valve the crew have just closed or opened (channel 32) takes its two jets out of or back into the DAP's jet masks CH5MASK and CH6MASK, one change a pass, and starts 1/ACCJOB. With it the LM's file is all readable |
| AGS initialization | `AGS_INITIALIZATION.agc` (LM pp. 206-210) | [`src/ags_initialization/`](src/ags_initialization/) | R47: AGSINIT, NEWAGS, REDSPTEM, AGSDISPK, AGSVCALC, CKSTALL and AGSEND in basic code, and AGSVCALC's and SCALEVEC's interpretive code in the listing's notation: verb 47 sets the AGS clock's zero (AGSK) on an ENTER, sends the LM's and CSM's state vectors, scaled for the AGS, and the time since that zero down the downlink (LAGSLIST), then zeroes the IMU's CDUs with the AGS's. With it the LM's file is all readable |
| Sighting marks | `AOTMARK.agc` (LM pp. 244-261) | [`src/aotmark/`](src/aotmark/), with CDULOGIC of `RTB_OP_CODES.agc` (LM p. 1397, CM p. 1508) in [`src/rtb_op_codes/`](src/rtb_op_codes/) | AOTMARK, the LM's star sightings for P51-P53 and R59: the crew's detent and star code (V01N71, GETDAT, CODE7, CODE1TO6), the mark planes (OPTAXIS, OANB), the X and Y mark buttons (MARKRUPT, FINDKEY, VERIFYMK, VACSTOR, MKREJ, CHANGEVB's next mark verb) and the star direction averaged over up to five pairs (MARKCHEX, AVESTAR, AVEIT), or on the lunar surface the cursor and spiral readings (V06N79) and SURFSTAR; basic code, and its interpretive code in the listing's notation. With it the LM's file is all readable |
| Sextant marks | `SXTMARK.agc` (CM pp. 222-235) | [`src/sxtmark/`](src/sxtmark/) | SXTMARK, the CM's sextant marks for P51-P53 and for R21-R22 (P22, P23): a VAC area for the marks (MARKOK, MKVACFND), the mark verbs (MKVB51's V51, MKVB50's V50N25, MKVB5X), the MARK and MARK REJECT buttons and the navigation DSKY's keys (MARKRUPT, KEYCALL, MKREJECT, REJECT2, REJECT3), the check that the ship was not turning too fast at the mark (MARKIT, MARKDIF, DIFCHK), keeping the mark (MARKCONT, MARK2, VACSTOR, or R21's MRKBUF1), the end (ENTANSWR, JAMIT, ENDMARKS, TERMSXT) and the release of the mark system (MKRELEAS, MKRLEES). With it the CM's file is all readable |
| Extended verbs, part 1 | `EXTENDED_VERBS.agc` (LM pp. 262-264, CM pp. 236-238), the front end | [`src/extended_verbs/`](src/extended_verbs/) | GOEXTVB and its fan LST2FAN (each mission's 60 verbs, V40 to V99, to their routines), TESTXACT, SETXTACT and XACT0 (claiming the extended verb display system, EXTVBACT, unless another extended verb or a priority display has it), XACTALM, ALM/END and GOPIN (the operator error light and back to PINBALL), and the LM's CHKPOOH (P00 or P00- only). The rest of the file comes part by part |
| Extended verbs, part 2 | `EXTENDED_VERBS.agc` (LM pp. 266-299, CM pp. 243-267), the short verbs | [`src/extended_verbs/`](src/extended_verbs/) | The verbs that set or clear a flag and go back to PINBALL (the LM's 57, 58, 61, 62, 65, 75-77, 80, 81, 93, 95 and 96; the CM's 44, 45, 58, 61-63, 75-77, 80, 81, 93 and 96), those that claim the display system and start a job (the LM's 47, 49, 64, 66, 67, 83, 89, 90 and 92; the CM's 49, 64-68, 78, 83, 85, 89, 90 and 92), the hand-overs to other banks (verb 50 and the other please-perform verbs; the LM's 48, 55, 56, 82 and 85; the CM's 46, 54, 57, 82 and 94), verb 74 and the CM's 52, 60 and 86, CKMODCAD, the CM's CHKPOOH and EXDAPOFF, and SHOWSUM, verb 91's bank sums |
| Extended verbs, part 3 | `EXTENDED_VERBS.agc` (LM pp. 265-281, CM pp. 239-250), alignment and radar designate | [`src/extended_verbs/`](src/extended_verbs/) | V40, zero the IMU's CDUs (or the LM's RR's), V41, coarse align the IMU (or designate the LM's RR, with AURLOKON's lock-on question and the job RRDESK2, or drive the CM's optics), V42, fine align by gyro torquing, V43, the attitude error needles, the LM's V44 (end the continuous designate) and V60 (the landing radar to position 2), and the checks OP/INERT (which noun), the LM's RDRUSECK (is a radar in use?) and the CM's CKLFTBTS (has it lifted off?) |
| Extended verbs, part 4 | `EXTENDED_VERBS.agc` (LM pp. 276-279), the LM's radar test verbs | [`src/extended_verbs/`](src/extended_verbs/) | R04 (verb 63), the radar test: the rendezvous or landing radar sampled once a second and shown (the RR's angles, range and range rate; the LR's range, position and velocities), proceed going over to the other radar; R77 (verb 78), the LR's spurious-return test, sampling four times a second with no display; and R77END (verb 79), which stops it |
| Extended verbs, part 5 | `EXTENDED_VERBS.agc` (LM pp. 292-295, CM pp. 253-254), the DAP data load | [`src/extended_verbs/`](src/extended_verbs/) | Verb 48: the autopilot's configuration (noun 46), the masses (noun 47) and the trims (noun 48), shown flashing and taken back from the crew. The LM's R03 from DAPDATA1 on (APSFLAG, DAPBOOLS, MASS, the X-axis jets, the maneuver rate, the hand controller's sensitivity and deadband, the masses checked against the empty vehicle's, RESTORDB, and the trim task's TRIMDONE); the CM's DAPDISP and DAPDISP1 (refused under TVC, the maximum deadband flag, S41.2, MASSPROP and S40.14) |
| Extended verbs, part 6 | `EXTENDED_VERBS.agc` (LM pp. 275 and 282, CM p. 246), the clock update and ATTCK2 | [`src/extended_verbs/`](src/extended_verbs/) | Verb 55: flashing V25 N24 for a time to add to the computer's clock, taken only on verb 23's data in (the LM's R33 in bank 42, the CM's ALINTIME), and UPDATIME, which adds it with interrupts held; and the LM's ATTCK2, V43's task that puts the attitude errors out to the CDUs' needles and turns their drives on |
| Extended verbs, part 7 | `EXTENDED_VERBS.agc` (LM pp. 262, 287-288, 296-300, CM pp. 236, 259-265), the tracking and state vector verbs | [`src/extended_verbs/`](src/extended_verbs/) | The basic code of V69 (the forced hardware restart), V56 (terminate tracking: the LM's TRMTRAK1, the CM's TRACKTRM and CLEANOUT), V66 (ATTACHIT, which copies this vehicle's state vector into the other's) and the CM's V47 (LMTOCMSV and LMTOCM, the other way round, sharing TACHEXIT), the LM's V85 (DSPRRLOS and RRLOSDSP, the rendezvous radar's line of sight shown once a second) and the CM's V87 and V88 (the VHF range flag). The CM's interpretive words they hand over are shown in comments; the LM's are part 8 |
| Extended verbs, part 8 | `EXTENDED_VERBS.agc` (LM pp. 287-288, 296-300), the LM's interpretive words of the tracking verbs | [`src/extended_verbs/`](src/extended_verbs/) | The interpretive code V56, V66 and V85 hand to the interpreter, in the listing's notation, run by the readable interpreter: TRMTRAK1's and ATTACHIT's CALL INTSTALL (integration let finish), ATTACHIT's sphere of influence switches and its CALL PTOALEM and SVDWN1 (the other vehicle's state vector brought up to date), TCPIN's RTB back to PINBALL, and RRLOSDSP's vector arithmetic from the radar's line of sight to its elevation and azimuth (UNIT, DOT, ARCTRIG), shown 0-360 degrees. With it the LM's EXTENDED_VERBS.agc is all readable C |

Beside them:

- [`src/agc/`](src/agc/): the machine the routines run on: erasable memory,
  the central registers, one's complement words, fixed memory, and the
  listing's names (`names.h`; the addresses and constants are generated
  from the assembled listings by `tools/listing_names.py`).
- [`src/binding/`](src/binding/): connects the routines to the flight
  program (the keys and table names the faithful kernel uses). Plumbing
  only.
- [`tests/`](tests/): the side-by-side tests of the machine's
  instructions, the inter-bank routines, the phase table maintenance, the
  restarts, the interpreter, the alarms, the single precision sine and
  cosine, the service routines and the down-telemetry, and the build of the x86_64 flight program with the readable
  routines in it.

Both missions come from the same source, built with `LUMINARY099=1` or
`COMANCHE055=1`; where the listings differ the code shows both in place.

How much of the program is readable C: `make coverage` counts the
listing files in [`COVERAGE`](COVERAGE), whole files only, weighted by
their source lines over all 175 files of both missions.

Next, one subsystem at a time: the rest of the flight program, in study
order. A Rust edition with the same layout and the same gates
comes after the C.

## Building and the gates

From `readable/`, with a C compiler, Make and Python 3:

```sh
make                       # gate 1: names-check, coverage and the case tests (seconds)
make x86-scenarios-quick   # gate 2 on the PR set (26 rows)
make x86-scenarios         # gate 2 on every row (86)
```

Gate 2 needs the port's toolchain once, for the harness's yaAGC:
`c64-port/tools/setup_cloud.sh`.

1. **Case tests.** `make kernel-cases` runs the faithful port's
   resident-kernel corpus on the readable Executive and WAITLIST: every
   case of its certified milestones, about 42,000 runs over the two
   missions, each checked against the original program's yaAGC digest
   (frozen when the faithful port was certified) and against the faithful
   routines' final memory word for word.
   `make interbank-cases` runs every inter-bank entry, readable and
   faithful side by side, on 100,000 random machine states each, and
   compares memory and channels word for word. `make phase-cases` runs
   the faithful port's phase-table test (every TC NEWMODEX, CHECKMM,
   PHASCHNG and 2PHSCHNG site of the original program, against its yaAGC
   digest) on the readable routines, then the readable and faithful
   routines side by side on 200,000 random states per entry.
   `make restart-cases` does the same for the restarts: the faithful
   port's restart tests (yaAGC digests) on the readable RESTARTS,
   GETPART2, PHSPART2 and NEWPHASE, then readable and faithful side by
   side on 200,000 random states per entry. `make machine-cases` runs
   every instruction of the readable machine and of the faithful port's
   (checked against yaAGC) on 200,000 random states. `make
   interpreter-cases` runs the readable interpreter and the faithful
   port's interpreter core side by side on random interpretive programs
   (the port's own fuzz generator, 20,000 programs per seed, two seeds per
   mission), pass by pass, and compares memory and channels word for word;
   it leaves out the operands no assembled program has (the test's header
   lists them). `make alarm-cases` runs the faithful port's alarm tests
   (every alarm site and entry of each mission, 4,096 cases against the
   original program's yaAGC digests) on the readable alarms and aborts,
   then readable and faithful side by side on 40,000 random states per
   entry. `make sp-cases` runs the faithful port's SPSIN and SPCOS test
   (yaAGC return states) on the readable routines, then readable and
   faithful side by side on every value of A, four states each. `make
   service-cases` runs the faithful port's service routine test (every
   UPFLAG, DOWNFLAG and GENTRAN site of both missions, against yaAGC
   digests) on the readable service routines, then readable and faithful
   side by side on 40,000 random states per entry. `make
   downtelemetry-cases` runs the faithful port's down-telemetry tests
   (whole downlists and erasable dumps, DOWNRUPT by DOWNRUPT, against the
   original program's yaAGC digests) on the readable routines, then
   readable and faithful side by side on every step of 2,000 runs of
   1,200 DOWNRUPTs per mission, some steps with a control word put out of
   place; before that it checks the readable downlists word by word
   against the assembled listing. `make fresh-start-cases` runs the
   faithful port's fresh start tests (4,096 cases per mission: every
   entry, with restart groups active, inactive or corrupt, against the
   original program's yaAGC digests) on the readable fresh start and
   restart, then readable and faithful side by side on 65,536 states per
   mission. `make v37-cases` does the same for verb 37 and P00, after
   checking V37's tables word by word against the original program's
   rope. `make stateup-cases` does the same for STATEUP's order
   pairs, run by the readable interpreter, after checking the words the
   notation assembles against the rope. `make keyrupt-cases` runs the
   faithful port's keyboard and uplink test (1,536 states per mission over
   the six entries, every uplink path, against the original program's
   yaAGC digests) on the readable KEYRUPT and UPRUPT, then readable and
   faithful side by side on 65,536 states per entry. `make rupt-cases`
   runs the faithful port's lead-in test (128 states per entry, both
   missions, with the CM's RESUME entries) and the LM's T6 test (3,840
   states, every T6JOBCHK path and jet writer, with the LM's RESUME
   entries), against the original program's yaAGC digests, on the
   readable lead-ins, RESUME and T6 interrupt, then readable and faithful
   side by side on 65,536 states per entry. `make t4rupt-cases` does the
   same for T4RUPT's display and dispatch core with the faithful port's
   T4RUPT test (2,048 states at T4RUPT and 64 at each return per mission,
   every continuation and display path), and for its IMU monitor with the
   faithful port's tests of it (4,736 states per mission, every exit and
   SETISSW return), of the LM's radar checks (3,456 states, every exit)
   and of the CM's optics (274 frozen cases, each run to its stop). `make
   rcs-monitor-cases` runs the faithful port's test of the LM's jet failure
   monitor (72 frozen cases, each run to its stop) on the readable one,
   then the two side by side on 65,536 states per entry. `make ags-cases`
   runs the faithful port's test of the LM's AGS initialization (29,952
   frozen cases, each run until it leaves the file) on the readable one,
   then the two side by side on 65,536 states, as the host runs the file
   and as the flight program does. `make aotmark-cases` runs the faithful
   port's test of the LM's sighting marks (2,036 frozen cases, each run to
   its stop) on the readable one, then the two side by side, 4,096 states
   at each of its 122 keys. `make sxtmark-cases` does the same for the
   CM's sextant marks (679 frozen cases; 4,096 states at each of its 42
   keys). `make extended-verbs-cases` runs the faithful port's frozen
   oracle rows for the extended verbs' front end (23 cases of its suite,
   one for every branch, against the original program's Virtual AGC
   results) on the readable routines, then readable and faithful side by
   side on the port's whole suite (5,760 cases) and 4,096 states at each
   of the 21 keys of both missions; then the same for the short verbs
   (121 oracle rows, the port's 6,368 cases, and 4,096 states at each of
   the 199 entries of both missions) and for alignment and radar
   designate (97 oracle rows, the port's 5,056 cases, and 4,096 states at
   each of the 158 entries of both missions) and for the LM's radar test
   verbs (31 oracle rows, the port's 2,448 cases, and 4,096 states at each
   of the 51 entries) and for the DAP data load (46 oracle rows, the
   port's 2,832 cases, and 4,096 states at each of the 59 entries of both
   missions) and for the clock update and ATTCK2 (13 oracle rows, the
   port's 1,152 cases, and 4,096 states at each of the 18 entries of both
   missions) and for the tracking and state vector verbs (38 oracle rows,
   the port's 2,304 cases, and 4,096 states at each of the 48 entries of
   both missions) and for the LM's interpretive words of those verbs (its
   55 words checked against the rope, 41 oracle rows, the port's 1,856
   cases, and 4,096 states at each of the 29 entries). `make names-check` checks the
   generated names and the restart tables against the listings.
2. **x86 exact scenarios.** The x86_64 flight program, with the readable
   routines linked in place of the faithful ones, flies the flight
   scenarios (P63 descent, rendezvous, entry, restarts, ...) against the
   harness, which runs the faithful port. Each must end with the same
   erasable memory as the harness, word for word: words 061-03777 (every
   erasable bank, past the central and special registers and the
   counters), once, at the row's end. The registers (A, L, Q, the banks)
   and the I/O channels are not compared there; the case tests compare
   them, routine by routine. PRs fly the quick set (26 rows), the nightly
   run every row. The DOWNRUPT vector and its RESUME fly readable too
   (the flight program takes them from the down-telemetry tables). The
   CM's EXTENDED_VERBS does not fly here yet: the CM program keeps the
   faithful port's compiled copy of that whole file
   (`cm_extended_verbs_file.o`), which takes every key before the
   readable parts can, until the file is readable whole. Until then only
   the case tests run the CM's readable parts.

CI runs both on every PR that touches `readable/`, or what it is built
from and checked against: the faithful port's sources, tests and tools
and the listings (`.github/workflows/readable-ci.yml`).

## Departures

Where the readable code, the faithful port and the AGC do not all agree,
on purpose, in one list.

Where the readable code departs from the AGC as the faithful port does.
The gates cannot see these, since they compare the two ports:

- **An endless GOTO chain.** The AGC follows a chain of erasable CADRs for
  as long as it goes on; a cycle would run until the night watchman
  restarts the computer. The interpreter stops the job at GOTO after 63
  hops (`GOTO_HOPS`, `src/interpreter/sequence.c`), and the next pass goes
  on.
- **Q after a switch order.** SWITCHES leaves the switch word in Q with
  TS Q, which on the AGC copies all 16 bits of A. The faithful core keeps
  15, and the rest of the flight program takes Q so; after a pass that
  ran a switch order, the binding leaves Q as the faithful core does
  (`src/binding/interpreter_core.c`).
- **HI10 run as MASK.** On UPTEST's +0 path the AGC runs the constant
  HI10 (OCT 77740) as MASK 7740. It changes only A, which TMFAIL2
  overwrites at once, so neither port runs it (`src/keyrupt/keyrupt.c`).
- **OCT37737 run as CA 7737.** On SYNCT4's +0 path the AGC runs the
  constant OCT37737 as CA 7737. It changes only A, which SYNCT4's CAF
  overwrites at once, so neither port runs it (`src/t4rupt/t4rupt.c`).
- **RESUME reruns the word at ZRUPT -1.** The AGC's RESUME runs the
  instruction the interrupt saved in BRUPT; both ports go on at ZRUPT -1
  instead. The bindings take only states where BRUPT is that word under
  the banks being restored, and the port's tests check both outcomes
  against yaAGC (`src/waitlist/waitlist.c`, `src/binding/
  rupt_routines.c`).
- **VERB69 stops at its word.** Verb 69's fan word is TC VERB69, a TC
  to itself: the AGC runs it until the hardware's TC trap restarts the
  computer. Both ports stop the step at VERB69
  (`src/extended_verbs/front.c`).
- **Steps not modelled.** An INDEX that would make an instruction the
  listing does not intend, or a channel a routine does not use, calls
  `not_modelled()`: the binding puts the state back and refuses.

Where the faithful port departs from the AGC and the readable code does
what the AGC does, or refuses. All are states no flight program makes;
the side by side tests do not count them, and print how many they met:

- **DTCB's Z.** The readable DTCB keeps the 12-bit address, as the AGC
  does; the faithful one keeps 15 bits. The side by side tests keep DTCB
  returns below 010000.
- **TC Q with an overflow in Q** (the LM's FLAGS, MR.KLEAN, P00KLEAN,
  V37KLEAN): the faithful routine goes to the overflow-corrected address,
  where the AGC runs the 16-bit Q as another instruction. The readable
  binding refuses.
- **A negative ALMCADR** after the CM's VAC5STOR, in BAILOUT and POODOO:
  the faithful routine indexes with it as if positive. Refused.
- **UPFLAG or DOWNFLAG naming ITEMP3 as the flag word**: COMFLAG's LXCH
  then returns through the new flag word on the AGC, past the ADRES in
  the faithful routine. Refused.
- **A downlist word at the end of an E bank**: its second word is fixed
  address 02000, which the AGC reads from the rope and the faithful
  routine from erasable 0. The readable routine reads the rope (or
  refuses, in the LM, where it has no word for it).
- **A restart's delta time at a central register or GOLOC, or across
  01377-01400**: the faithful routine reads them as they were before
  RESTARTS changed them. Refused.
- **The interpreter's unassembled operands** (an erasable order given a
  fixed address, a store into fixed memory, an operand past the end of
  its bank, ...): the readable interpreter does what the AGC does, the
  faithful core something simpler; `make interpreter-cases` leaves them
  out (`tests/interpreter_fuzz.c` lists them).

And states the readable binding refuses where the faithful routine runs,
also not counted: STATEUP with NEWJOB negative, and the fresh start's
KLEANs returning to one of the file's own return points.

## License

MIT, as the port: [`LICENSE`](LICENSE), Copyright (c) 2026 David
Godlewski. The AGC source is in the public domain.
