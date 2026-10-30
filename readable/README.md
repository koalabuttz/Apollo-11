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
in the real x86_64 flight program in place of the faithful code and leaves
memory exactly as the original does, word for word (see *Gates*).

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

Beside them:

- [`src/agc/`](src/agc/): the machine the routines run on: erasable memory,
  the central registers, one's complement words, fixed memory, and the
  listing's names (`names.h`; the addresses and constants are generated
  from the assembled listings by `tools/listing_names.py`).
- [`src/binding/`](src/binding/): connects the routines to the flight
  program (the keys and table names the faithful kernel uses). Plumbing
  only.
- [`tests/`](tests/): the side-by-side tests of the inter-bank routines
  and the phase table maintenance, and the build of the x86_64 flight program with the readable routines in
  it.

Both missions come from the same source, built with `LUMINARY099=1` or
`COMANCHE055=1`; where the listings differ the code shows both in place.

How much of the program is readable C: `make coverage` counts the
listing files in [`COVERAGE`](COVERAGE), whole files only, weighted by
their source lines over all 175 files of both missions.

Next, one subsystem at a time: the restart routines and tables, the
interpreter, then the rest of the flight program. A Rust edition with the same layout and the same gates
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
   missions, each checked against the original program's yaAGC digest and
   against the faithful routines' final memory word for word.
   `make interbank-cases` runs every inter-bank entry, readable and
   faithful side by side, on 100,000 random machine states each, and
   compares memory and channels word for word. `make phase-cases` runs
   the faithful port's phase-table test (every TC NEWMODEX, CHECKMM,
   PHASCHNG and 2PHSCHNG site of the original program, against its yaAGC
   digest) on the readable routines, then the readable and faithful
   routines side by side on 200,000 random states per entry.
   `make names-check` checks
   the generated names against the listings.
2. **x86 exact scenarios.** The x86_64 flight program, with the readable
   routines linked in place of the faithful ones, flies the flight
   scenarios (P63 descent, rendezvous, entry, restarts, ...) against the
   harness, which runs the faithful port. Each must end with the same
   erasable memory as the harness, word for word.

CI runs both on every PR that touches `readable/`
(`.github/workflows/readable-ci.yml`).

## License

MIT, as the port (`c64-port/LICENSE`). The AGC source is in the public
domain.
