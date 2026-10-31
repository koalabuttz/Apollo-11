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
| WAITLIST | `WAITLIST.agc` (LM pp. 1117-1132, CM pp. 1221-1235) | [`src/waitlist/`](src/waitlist/) | WAITLIST, TWIDDLE, FIXDELAY, VARDELAY, T3RUPT, TASKOVER, SVCT3 and LONGCALL |
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
   faithful side by side on 65,536 states per entry. `make names-check` checks the generated names and the
   restart tables against the listings.
2. **x86 exact scenarios.** The x86_64 flight program, with the readable
   routines linked in place of the faithful ones, flies the flight
   scenarios (P63 descent, rendezvous, entry, restarts, ...) against the
   harness, which runs the faithful port. Each must end with the same
   erasable memory as the harness, word for word: words 061-03777 (every
   erasable bank, past the central and special registers and the
   counters), once, at the row's end. The registers (A, L, Q, the banks)
   and the I/O channels are not compared there; the case tests compare
   them, routine by routine. PRs fly the quick set (26 rows), the nightly
   run every row.

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
