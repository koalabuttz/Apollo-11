# Actual Comanche clock display lifecycle composition

`cm_clock_display_mission` extends the resident clock mission with explicit
source groups for MAKEPLAY, mixed PRIOCHNG, JOBWAKE and GOSLEEPS/JOBSLEEP.
Every group returns before the next disk slot is loaded. The canonical
2,048-word image remains authoritative; only native scheduler metadata and
external I/O latches live outside it. The checked loader, caller-owned
producer exclusion and physical bank/source continuation contract remain
unchanged.

The source corpus executes real TIME3 IRQ/CLOKTASK/WAITLIST/admission,
RESUME/DANZIG selection of CLOCKJOB, actual QUICTRIG and display control,
then MAKEPLAY priority change. Normal contexts wake through source JOBWAKE
and reach genuine GOPLAY (`10,3312`). PMFLAG blocking contexts execute actual
GOSLEEPS/JOBWAKE/XCHTOEND/JOBSLEEP and restore the suspended interpreter at
NEWOPS (`06037`), retaining the dormant display context. Every group compares
all physical words, raw CPU registers, SUPERBNK, IRQ, channel11 and native
transfer/control metadata against uninterrupted original assembly execution.

The first corpus has 64 sequences and 1,344 complete checkpoints, lossless SHA256
`295f2ea42a6563b411a00f9c2254c2b8b5d9c284b7309e1edc10340200f2d4c2`.
Original-layout rope `6e3150ae...` is independently assembled and authenticated.
Strict and full ASAN/UBSAN checks pass. The DSPLOCK1/TEMPFLSH0 cohort follows
normal GOPLAY because TEMPFLSH differs from saved CLOCKJOB caller CADR051366;
a separate matching-TEMPFLSH051366 cohort takes actual busy END (`05217`)
and restores the suspended interpreter in all 64 additional sequences. Its
1,216 complete checkpoints have SHA256
`ce2a6fe603dea893e6e1ba202e267b581e37e0e7949444c7da22fe606b79f5fe`. The freezer asserts
these actual outcomes instead of inferring them from input names.

The target disk contains seven actual native service modules and two
proof-only input/checker modules. Three representative source sequences run
with both incoming C64 I states, yielding 122 checked source boundaries.
Three complete terminal image/I/O sample contexts cover normal GOPLAY, actual
sleep restoration and matching busy END; each runs with both I states. The final whole canonical image
is compared directly. The earlier eight-context/336-checkpoint target trial
exceeded its fixed 600-second budget before a completion mailbox; its exact
inputs/artifact bytes are retained with no correctness verdict. All payloads fit `$8000..$bfff`; the retained target report
supplies the actual verdict and stack extent.

GOPLAY is an explicit next-consumer boundary. This packet does not translate
V06N40 scaling/display conversion, resume the dormant display job, execute
arbitrary selected interpreter bodies or complete every mission program.
The existing V06N85 consumer has a distinct source domain. These boundaries
remain visible for later source-qualified composition.

Run `make -f Makefile -f tools/cm_clock_display_mission.mk test-cm-clock-display-mission`
and `sanitize-cm-clock-display-mission`; the separate overlay build/VICE scripts
reproduce native disk execution. `tools/cm_freeze_clock_display.py` regenerates
source outcomes and target vectors without changing program semantics.
