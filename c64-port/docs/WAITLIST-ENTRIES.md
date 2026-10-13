# WAITLIST entries reached from other files (lane A)

Three WAITLIST points are entered from code outside WAITLIST.agc and were not
yet registered transfer routines. They are now, at their original addresses:

| entry | LM | CM | reached by |
|---|---|---|---|
| T3RUPT body | 01,3407 | 01,3416 | the T3RUPT vector in INTERRUPT_LEAD_INS (`DXCH ARUPT`, `CAF T3RPTBB`, `XCH BBANK`, `TCF T3RUPT`) |
| DLY2 -1 | 05211 | 05255 | SERVICE_ROUTINES DELAYJOB (`TC DLY2 -1` with the delay in A) |
| NOQBRSM +1 | 05275 | | the P-axis autopilot (`TCF NOQBRSM +1`), which resumes at SUPERJOB |

The T3RUPT body saves the superbank with the E and F banks in BANKRUPT, does
`QXCH QRUPT` and runs T3RUPT2's dispatch of the next task. DLY2 -1
stores WAITADR, takes the caller's BB and runs the certified internal Waitlist.
NOQBRSM +1 is `RELINT; RESUME` without NOQBRSM's `DXCH ARUPT`. Each refuses (and leaves the state as it was) where its body refuses.
They are in `lm_exec_task_routines` (`src/lm_exec_tasks.c`, 6 entries),
`cm_exec_routines` (`src/cm_exec_routines.c`, 16) and `lm_t6rupt_routines`
(`src/lm_t6rupt.c`, 13).

## Proof

`tools/waitlist_entries_freeze.py` runs the 1,380 cases of
`tests/waitlist_entries_cases.c` through `agc_transfer_run` and in yaAGC from
the original ropes:

| kind | cases | reach the continuation |
|---|---|---|
| LM T3RUPT 01,3407 | 512 | 512 |
| CM T3RUPT 01,3416 | 512 | 512 |
| LM DLY2 -1, from DELAYJOB | 114 | 95 |
| CM DLY2 -1, from DELAYJOB | 114 | 100 |
| LM NOQBRSM +1 | 128 | 128 |

The T3RUPT cases are the lm-exec-jobs and cm-exec-services dispatch-vector
inputs entered at the body. The DLY2 -1 cases are the service-transfer DELAYJOB
inputs, run with the service and Executive tables together, so the run goes
DELAYJOB, DLY2 -1, WAIT2, TCGETCAD to JOBSLEEP or BAILOUT. Each DELAYJOB input
runs twice: on its own random image, where WAIT2 almost always finds a broken
list and aborts, and (base `| WLE_SANE`) with a sane waitlist: a free DELAYLOC,
a positive delay, TIME3 on both sides of its overflow test, and 0-8 queued
tasks (one case in sixteen a full list of 9). Of those, 45 LM and 52 CM cases
insert the task and reach JOBSLEEP, 4 LM and 5 CM overflow the list and abort,
and one LM case ends at JOBWAKE. The remaining DLY2 -1 cases are inputs the
native DELAYJOB refuses (most with a delay near 1.0, whose WAIT2 sum would
overflow); they are compared as refusals.
yaAGC stops at the native Z (and FB when banked) or when it leaves the
interrupt. All 2048 erasable words, the superbank and the inhibit and
interrupt flags are identical. The proof
(`evidence/waitlist-entries/waitlist-entries-proof.json`) lists the hashes of
the native sources it depends on.

`tests/test_waitlist_entries.c` checks the frozen digests, one refusal per
kind, and TASKOVER's RESUME check in both missions under BANKRUPT's
superbank (BANKRUPT 060100 with channel 7 at 060: TASKOVER writes BANKRUPT
to SUPERBNK before RESUME): 1,389 checks. The C64 programs (`waitlist-entries-0` .. `-4`, one per
kind) run every fourth case, each kind's first two, and the cover rows of
`tests/waitlist_entries_c64_rows.h`: with them the C64 rows take every branch
arc (gcov -b, per object, all linked sources) the 1,380 host cases take
(`python3 tools/lane_a_c64_cover.py --check waitlist-entries`; record
`evidence/exec-closure/exec-closure-c64-cover.json`). The CM DLY2 -1 program is
built with `CM_EXEC_DLY2_ONLY` and `CM_SERVICE_DELAYJOB_ONLY`, which add the
one-entry tables `cm_exec_dly2_routines` and `cm_service_delayjob_routines`
(the same routines) so the rest of both tables is not linked and the program
fits below the $C000 mailbox; a case that left them would stop early and fail
its digest. The check's state
lives at $E000 under the banked-out ROMs.
