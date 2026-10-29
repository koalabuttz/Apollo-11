# EXEC-CLOSURE: whole-file maps of EXECUTIVE.agc and WAITLIST.agc

Lane A, milestone `exec-closure`.

The exec milestones made every entry of EXECUTIVE.agc and WAITLIST.agc a native routine in both missions:

- `cm-exec-services` and `lm-exec-services`;
- `cm-exec-jobs` and `lm-exec-jobs`;
- `cm-exec-idle` and `lm-exec-idle`;
- the RESUME family in `rupt-lead-ins`.

This milestone checks the whole files against those routines. Every assembled word of the four files is listed with its source line, and with the milestones whose oracle comparison ran it in the original program. Each comparison runs the original from the routine's entry to where the native routine ended. So a word that run executed lies on a path the native routine reproduced, and was compared in all 2,048 erasable words.

## How the words are traced

`tools/oracle_trace/` holds `oracle_trace.c` and a `cc`/`gcc` shim. `make exec-closure-trace` puts the shim first on `PATH` and reruns each milestone's own differential. When the shim sees a link of a `*_reference.c` oracle, it adds `oracle_trace.c` and `-Wl,--wrap=agc_engine`.

The wrapper records the physical fixed address of Z before each instruction and appends the set to `$AGC_EXECUTED_WORDS` at exit. It records banks 030-037 +8 under SUPERBNK, and fixed-fixed as banks 2-3. The differentials themselves are unchanged and still assert that their frozen vectors are current. `rupt-lead-ins` already writes its own executed-words files.

`tools/exec_closure_source_map.py` (`make exec-closure-source-map`) matches each listing line of the four files to the repository source in order. It records the line's words and owners, and writes:

- `build/exec-closure-source-map-{lm,cm}-{executive,waitlist}.json`;
- `build/exec-closure-proof.json`, with per-file counts and the hashes of the milestones' evidence manifests.

The script fails if any instruction word was executed by no milestone, unless the word is named below.

## Results

| File | Words | Instruction lines | Executed | Unreached (named) | Pending |
|---|---|---|---|---|---|
| Luminary099/EXECUTIVE.agc | 345 | 336 | 326 | 10 | 0 |
| Luminary099/WAITLIST.agc | 267 | 240 | 238 | 2 | 0 |
| Comanche055/EXECUTIVE.agc | 337 | 328 | 320 | 8 | 0 |
| Comanche055/WAITLIST.agc | 252 | 225 | 224 | 1 | 0 |

**Unreached instruction words.** These are the original's own traps:

- **`TC CCSHOLE` on CCS classes the Executive never produces.**
  - A +0 PRIORITY register in EJSCAN: a core set's priority is -0 (free), positive (active) or negative (asleep).
  - A positive or +0 BUF +1 after the scan.
  - SPECTEST's CCSHOLEs on NEWJOB, which is -0 or a core-set location outside CHANJOB.
- **The `NOOP`s the source marks CAN'T GET HERE.**
  - LM WTABORT follows `TC FILLED`, which does not return.
  - LONGCYCL's +0 class of LONGTIME +1, in both missions.

**Read as data.** `DELAYEX TCF TASKOVER -2` is never executed. VARDELAY reads it with `CAF DELAYEX` into WAITEXIT, as the address word for DTCB. The map lists it as a constant, like `ENDTASK -2CADR SVCT3` and `-CCSPR -CCS PRIORITY`.

**Luminary099 core set 0.** `lm_basic_wake` used to refuse a sleeping job in core set 0, which the CM routine already accepts. That is the state a task's JOBWAKE meets while the dummy job runs. Its domain now admits that state. The `lm-exec-jobs` JOBWAKE cases put the matching job in every core set, core set 0 included, so WAKETEST's `TCF SPECTEST` and SPECTEST's `TCF SETLOC` run. The case changes are:

- `lm-exec-jobs`: every slot (`j*9`), and core set 0 asleep in one case in 16;
- `lm-exec-services`: a dormant job in a third of the NOVAC/FINDVAC scans.

Their vectors and evidence are refrozen. All four files have every reachable instruction word in a native routine's compared path. Certificates for all four are drafted in `docs/port-coverage.json`.

## Evidence

Hashes: `evidence/exec-closure/SHA256.json`.

- **Source to native map.** `evidence/exec-closure/exec-closure-source-map-{lm,cm}-{executive,waitlist}.json`.
- **Independent assembly comparison.**
  - `evidence/exec-closure/exec-closure-proof.json` points at the comparisons and hashes their evidence manifests. Those comparisons are the milestones' differentials, rerun under the tracer. The proof also lists `waitlist-entries` (manifest hash and case count, `traced: false`). That milestone compares the WAITLIST entries other files reach with yaAGC, and it also checks TASKOVER's RESUME superbank check against yaAGC's own T3RUPT (`docs/WAITLIST-ENTRIES.md`). Its words are already among the traced runs.
  - `evidence/exec-closure/exec-closure-trace.log` is the run's output: every differential passes with its vectors unchanged.
- **Host and sanitizer results.** `make test-exec-closure sanitize-exec-closure` runs all seven milestones' strict and sanitizer checks (`evidence/exec-closure/exec-closure-host.log`).
- **C64 execution results.** `make exec-closure-vice` runs all seven milestones' VICE programs (`evidence/exec-closure/exec-closure-vice.log`).
