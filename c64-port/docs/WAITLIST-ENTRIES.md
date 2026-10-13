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

`tools/waitlist_entries_freeze.py` runs the 1,266 cases of
`tests/waitlist_entries_cases.c` through `agc_transfer_run` and in yaAGC from
the original ropes:

| kind | cases | reach the continuation |
|---|---|---|
| LM T3RUPT 01,3407 | 512 | 512 |
| CM T3RUPT 01,3416 | 512 | 512 |
| LM DLY2 -1, from DELAYJOB | 57 | 49 |
| CM DLY2 -1, from DELAYJOB | 57 | 43 |
| LM NOQBRSM +1 | 128 | 128 |

The T3RUPT cases are the lm-exec-jobs and cm-exec-services dispatch-vector
inputs entered at the body. The DLY2 -1 cases are the service-transfer DELAYJOB
inputs, run with the service and Executive tables together, so the run goes
DELAYJOB, DLY2 -1, WAIT2, TCGETCAD to JOBSLEEP or BAILOUT. The remaining DLY2 -1
cases are inputs the native DELAYJOB refuses; they are compared as refusals.
yaAGC stops at the native Z (and FB when banked) or when it leaves the
interrupt. All 2048 erasable words, the superbank and the inhibit and
interrupt flags are identical. The proof
(`evidence/waitlist-entries/waitlist-entries-proof.json`) lists the hashes of
the native sources it depends on.

`tests/test_waitlist_entries.c` checks the frozen digests and one refusal per
kind: 1,271 checks. The C64 programs (`waitlist-entries-0` .. `-4`, one per
kind) run every fourth case and each kind's first. The CM DLY2 -1 program is
built with `CM_EXEC_DLY2_ONLY` and `CM_SERVICE_DELAYJOB_ONLY`, which add the
one-entry tables `cm_exec_dly2_routines` and `cm_service_delayjob_routines`
(the same routines) so the rest of both tables is not linked and the program
fits below the $C000 mailbox; a case that left them would stop early and fail
its digest. The check's state
lives at $E000 under the banked-out ROMs.
