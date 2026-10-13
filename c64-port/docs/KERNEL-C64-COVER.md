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

Every C64 program now takes every host branch arc:

| milestone | host cases | host branch arcs | stride rows | cover rows | VICE checks |
|---|---|---|---|---|---|
| `lm-t6rupt` | 3,840 | 198 | 240 | 2 | 242 |
| `rupt-lead-ins` | 3,072 | 168 | 192 | 1 | 193 |
| `keyrupt-uprupt` | 3,072 | 137 | 192 | 0 | 192 |
| `lm-exec-services` | 9,208 | 620 | 576 | 5 | 581 |
| `lm-exec-jobs` | 9,216 | 1110 | 576 | 20 | 596 |
| `lm-exec-idle` | 4,864 | 289 | 304 | 6 | 310 |
| `cm-exec-services` | 7,648 | 662 | 478 | 9 | 487 |
| `cm-exec-jobs` | 6,144 | 670 | 384 | 4 | 388 |
| `cm-exec-idle` | 5,120 | 294 | 320 | 5 | 325 |

`evidence/exec-closure/exec-closure-c64-cover.json` gathers the six Executive milestones and `rupt-lead-ins` for the EXECUTIVE and WAITLIST certificates. `waitlist-entries` already runs every case on the C64 (five programs, one per entry group).

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
already common. Cases holding each edge (an image can hold several):

| milestone | cases | TIME3 037770-037777 | TIME6 1-3 | LST1 tasks due together (+0) | LST1 tasks 1 cs apart | Waitlist full (9 tasks) | NEWJOB +0 | NEWJOB -0 | equal busy priorities | no free core set | no free VAC area |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `lm-exec-jobs` | 9,216 | 3,191 | 0 | 898 | 898 | 7,881 | 4,014 | 346 | 513 | 149 | 169 |
| `lm-exec-services` | 9,208 | 1,447 | 0 | 1,144 | 1,165 | 948 | 8,051 | 1,157 | 723 | 877 | 1,264 |
| `lm-exec-idle` | 4,864 | 2,449 | 0 | 2 | 0 | 4,864 | 1,549 | 499 | 9 | 125 | 172 |
| `cm-exec-services` | 7,648 | 1,042 | 0 | 756 | 841 | 687 | 6,686 | 962 | 413 | 257 | 516 |
| `cm-exec-jobs` | 6,144 | 3,061 | 0 | 2 | 1 | 6,144 | 1,385 | 464 | 344 | 283 | 512 |
| `cm-exec-idle` | 5,120 | 2,048 | 2 | 3 | 0 | 5,120 | 727 | 385 | 1 | 6 | 0 |
| `lm-t6rupt` | 3,840 | 1,920 | 362 | 1 | 1 | 3,840 | 1 | 1 | 8 | 13 | 0 |

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
