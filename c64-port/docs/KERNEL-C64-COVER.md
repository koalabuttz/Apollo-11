# Kernel C64 rows by branch cover, flight edges, and the #143/#144 fixes

Lane A, milestone `kernel-c64-cover`. Covers the nine kernel milestones whose
certificates the Certificate reviewer holds: `lm-exec-jobs`,
`lm-exec-services`, `lm-exec-idle`, `cm-exec-services`, `cm-exec-jobs`,
`cm-exec-idle`, `lm-t6rupt`, `rupt-lead-ins` and `keyrupt-uprupt` (with
`exec-closure`, which certifies the Executive and Waitlist files from them).

## C64 rows

Each C64 diagnostic already ran every `<P>_C64_STRIDE`-th case. That can miss
a branch the host corpus takes only a few times. `tools/lane_a_c64_cover.py`
builds the C64 program's own linked sources on the host with the C64 defines
(`-DP66_NATIVE_DISPATCH_ONLY` and the rest, read from the milestone's make
variables), traces every host case with gcov `-b` (`tools/c64_row_cover.py
trace`), seeds the cover with the stride rows and adds the fewest cases
(`c64_row_cover.py cover`) until the rows take every branch arc the whole
corpus takes, in every linked source. It writes:

- `tests/<m>_c64_rows.h`: the extra cases and their native digests (the host
  test proves every native digest equal to yaAGC's frozen one);
- `build/<m>-c64-cover.json`, copied to `evidence/<m>/<m>-c64-cover.json`:
  linked sources, defines, host cases, host branch arcs, stride rows, cover
  rows, the arcs the C64 rows take and `host_branches_missing_on_c64` (empty).

The diagnostics run the stride rows and then the cover rows; `make vice-<m>`
checks all of them in VICE and prints the PRG's sha256 (#127). Recheck with
`python3 tools/lane_a_c64_cover.py --check <m>`, which retraces and requires
every arc. Traces are tens of MB, so they stay in `build/`; the certificates
cite the cover record through `c64_branch_outcomes` (fallback fields) and the
VICE log through `vice_targets`.

COVER_TABLE

## Flight edges

The reviewer asked for the edges a flight reaches that a random image rarely
does. `tools/lane_a_edge_survey.py` (with `platform/host/lane_a_edge_scan.c`)
counts them over every case of each corpus and writes
`evidence/exec-closure/kernel-edges.json`. The generators now produce them on
purpose:

- `lm_exec_services_cases.c`, `cm_exec_services_cases.c`: after the canonical
  image, by `(n/13)%8`: TIME3 at 037774-037777 (wrap), two tasks 1 cs apart
  (LST1 077776), equal job priorities (011110), NEWJOB -0, TIME3 0 with LST1[0]
  077776, and (LM) tasks due together with TIME3 at 037740 and up.
- `lm_exec_jobs_cases.c`: equal priorities for JOBSLEEP, PRIOCHNG and ENDOFJOB
  (`ties`).
- `lm_t6rupt_cases.c`: TIME6 1-3 and 037777.
- `cm_exec_services_cases.c` DISPATCH and RESUME cohorts: Q holding data
  (037777, and an overflowing word) and QRUPT holding data (037777, 070000+).

A full waitlist, NEWJOB +0 and -0, no free core set and no free VAC area were
already common. EDGE_TABLE

## #143: QXCH moves data, not just addresses

`QXCH QRUPT` puts Q into QRUPT overflow-corrected and QRUPT into Q
sign-extended; Q may hold data. The CM T3RUPT body (`cm_exec_routines.c`) and
the CM task resume (`cm_task_transfer.c`) refused Q or QRUPT above 07777. They
now swap any value as the hardware does, and the DISPATCH and RESUME cohorts
compare such values against yaAGC.

## #144: RESUME continues at ZRUPT-1 only when BRUPT is that word

A native RESUME set Z to ZRUPT-1 and dropped BRUPT. The hardware re-executes
BRUPT, which equals the word at ZRUPT-1 only when nothing changed it.
`agc_resume_exact` (`agc_transfer.c`) reads the word at ZRUPT-1 under the banks
being restored (erasable, switched erasable or fixed through the program's
fixed reader) and the RESUME exits refuse when it differs: `lm_t6rupt.c`
(BANKRUPT's banks, or the current ones for NOQBRSM and NOQBRSM+1),
`rupt_lead_ins.c` (CM RESUME), `cm_exec_routines.c` and `lm_exec_tasks.c`
(TASKOVER). The generators set BRUPT to that word, so these cases still run;
`test_rupt_lead_ins.c` checks the refusal with erasable and fixed words. See
`docs/NATIVE_TRANSFER_ABI.md`.
