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

`tools/waitlist_entries_freeze.py` runs the 1,437 cases of
`tests/waitlist_entries_cases.c` through `agc_transfer_run` and in yaAGC from
the original ropes:

| kind | cases | reach the continuation |
|---|---|---|
| LM T3RUPT 01,3407 | 512 | 512 |
| CM T3RUPT 01,3416 | 512 | 512 |
| LM DLY2 -1, from DELAYJOB | 171 | 152 |
| CM DLY2 -1, from DELAYJOB | 114 | 100 |
| LM NOQBRSM +1 | 128 | 128 |

The T3RUPT cases are the lm-exec-jobs and cm-exec-services dispatch-vector
inputs entered at the body. The DLY2 -1 cases are the service-transfer DELAYJOB
inputs, run with the service and Executive tables together, so the run goes
DELAYJOB, DLY2 -1, WAIT2, TCGETCAD to JOBSLEEP or BAILOUT. Each DELAYJOB input
runs twice: on its own random image, where WAIT2 almost always finds a broken
list and aborts, and (base `| WLE_SANE`) with a sane waitlist: a free DELAYLOC,
a positive delay, TIME3 on both sides of its overflow test or exactly 0200
(where WAIT2's -TIME3 + 0200 is -0), and 0-8 queued tasks (one case in
sixteen a full list of 9). Of those, 43 LM and 53 CM cases insert the task
and reach JOBSLEEP, 6 LM and 4 CM overflow the list and abort, and one LM
case ends at JOBWAKE. The remaining DLY2 -1 cases are inputs the
native DELAYJOB refuses (most with a delay near 1.0, whose WAIT2 sum would
overflow); they are compared as refusals. Each LM input runs a third time
(base `| WLE_SANE | WLE_EDGE`): a sane list of 1-8 tasks with the new task
due exactly 1 cs before queued task j, so WAIT2's list search ends on
`CCS` -0 at j (`lm_delay_image.c` line 25, the -0 break) and the task is
inserted there. All 57 reach the continuation (56 JOBSLEEP, one JOBWAKE).
Two +0 outcomes in `lm_delay_image.c` are never taken, because they cannot
be: the `CCS` after -TIME3 + 0200 (line 22) and the one that ends the search
(line 27) would need a +0 sum, and a ones'-complement add gives +0 only from
+0 + +0. -TIME3 + 0200 is never +0 (TIME3 = 0200 gives -0), and the search
adds a queued task's negative delta T to a positive remainder.
yaAGC stops at the native Z (and FB when banked) or when it leaves the
interrupt. All 2048 erasable words, the superbank and the inhibit and
interrupt flags are identical. The proof
(`evidence/waitlist-entries/waitlist-entries-proof.json`) lists the hashes of
the native sources it depends on.

The same freeze also checks TASKOVER's RESUME check (#144) against the real
ropes (`tools/taskover_superbank_reference.c`, recorded under
`taskover_superbank` in the proof). In each case, yaAGC takes T3RUPT in a
program running in banks 030-033 under one superbank, so the hardware fills
ZRUPT and BRUPT from the rope. BANKRUPT keeps that superbank, as T3RUPT's
`ROR SUPERBNK` does, and channel 7 holds the task's superbank. The native
TASKOVER, given the rope as `fixed_data`, resumes in every case, and its
result equals yaAGC running TASKOVER until the interrupt ends: 488 Luminary099
and 493 Comanche055 cases. In 364 and 366 of them the word under channel 7's
superbank differs from BRUPT, so a check that read under channel 7 (the bug
#144 fixed) would refuse those cases; with that bug put back, the oracle
reports 364 Luminary099 failures.

`tests/test_waitlist_entries.c` checks the frozen digests, one refusal per
kind, and TASKOVER's RESUME check in both missions under BANKRUPT's
superbank (BANKRUPT 060100 with channel 7 at 060: TASKOVER writes BANKRUPT
to SUPERBNK before RESUME): 1,446 checks. The C64 programs (`waitlist-entries-0` .. `-4`, one per
kind) run every fourth case, cases 0 and 1, each kind's first two, and the cover rows of
`tests/waitlist_entries_c64_rows.h`: with them the C64 rows take every branch
arc (gcov -b, per object, all linked sources) the 1,437 host cases take
(`python3 tools/lane_a_c64_cover.py --check waitlist-entries`; record
`evidence/waitlist-entries/waitlist-closure-c64-cover.json`). The CM DLY2 -1 program is
built with `CM_EXEC_DLY2_ONLY` and `CM_SERVICE_DELAYJOB_ONLY`, which add the
small tables `cm_exec_dly2_routines` (DLY2 -1) and `cm_service_delayjob_routines`
(DELAYJOB and TCGETCAD; the same routines) so the rest of both tables is not linked and the program
fits below the $C000 mailbox; a case that left them would stop early and fail
its digest. The check's state
lives at $E000 under the banked-out ROMs.

The WAITLIST certificates (both missions) take their host, C64 and branch
evidence from `evidence/waitlist-entries/`: `waitlist-closure-host.log`,
`waitlist-closure-vice.log` and `waitlist-closure-c64-cover.json` are the
exec-closure records with this milestone's section rerun (379 VICE checks)
and its cover record added; the exec-closure files, which the EXECUTIVE
certificates pin, are unchanged.
