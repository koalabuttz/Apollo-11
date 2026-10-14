# Certificate review: Luminary099 P70-P71 after PR #408

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
72b40a63 (PR #408 merged), with the yaAGC reference rebuilt from the tip.
It was rejected in #385 because the compiled RESUME at CONTABRT left
`in_interrupt` set and InIsr was not compared.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/P70-P71 | `lm-p70-p71-file` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL. The one WARN is the nightly VICE manifest, which could not be
  fetched. The `source_revision` is 911e5c02.
- **The #385 fix.**
  - RESUME at CONTABRT 05,3404 (`src/lm_p70_p71_file.c` 205) now clears the
    program's `agc_hardware` `in_interrupt`. That matches yaAGC's RESUME
    (`NextZ = ZRUPT-1`, `InIsr = 0`, substitute instruction).
  - Through the spec key `in_isr`, every case starts with InIsr 0 or 1 on both
    sides (`tests/interp_case.c` 116/121, `tools/interp_routine_reference.c`
    82/126), and both digests cover it.
  - With the clear removed from `tools/basic_compile.py`, the differential
    fails 1,045 cases.
  - `tools/interp_compile.py --check` regenerates all 79 specs unchanged, and
    all 79 pass under `make test`.
  - The only other compiled RESUME (`src/lm_p70_p71.c` 134) also clears. The
    handwritten RESUMEs (WHIMPER, WAITLIST, T6RUPT and the lead-ins) already
    did.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py lm-p70-p71-file --cases 512` prints
  PASS with all 39,936 cases equal across 78 routines.
  - The case set hashes to the proof's `cases_sha256` (4c8314f8).
  - The strict `--cover-check` verifies 125 rows, 0 missing.
  - The traced closure rerun reproduces `lm-p70-p71-file-closure.json` byte
    for byte: 169 executed, 91 interpreted, 0 unreached.
- **Host and sanitizer.** `make test-lm-p70-p71-file
  sanitize-lm-p70-p71-file` gives 2,506 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL. Its
  3 PASS lines sum to 125 checks, equal to `vice_targets`, the proof's C64 rows
  and the rows in `tests/lm_p70_p71_file_c64_vectors.h`. All 3 PRGs rebuilt at
  the tip are byte-identical to the logged ones.
- **Independent C64 branch check.**
  - All 125 header rows match a regenerated case, and their expected digests
    equal yaAGC's.
  - The 3 KEEP-group builds take all 1,034 host outcomes and 1,953 lines, 0
    missing.
  - There are no truncated or resumed rows.
  - CONTABRT has a C64 row with InIsr 0 and one with InIsr 1.
- **Interrupt context.**
  - R10,R11 and its TWIDDLE/FLIP return points, CONTABRT and ABRTJASK are
    basic entries under `basic_sane`, which accepts any FIXLOC and NEWJOB
    (#403).
  - The file has no BOVB or BOV, so #414 does not apply.
  - RTB LOADTIME runs natively.
- **Accounting and domain.**
  - SERVICER schedules R10,R11 as a task and reaches R10,R11A by TCF from
    GRP2OFF.
  - RESTART_TABLES restarts R10,R11 (2.21SPOT, task), GOABORT (4.3SPOT) and
    P70A/P71A (4.27/4.31, jobs). All are registered entries.
  - CONTABRT is reached only by P70A/P71A's DTCB, inside or outside an
    interrupt, as the domain text says.
  - The divide edges (zero divisor, DVOVF, equal majors) equal yaAGC.
- **Not blocking.**
  - The proof's `compared` string does not name `in_interrupt`; this is the
    generator text at `tools/interp_routine_differential.py` 1759.
  - The cover-check counts 1,006 outcomes against the proof's 1,013, 0
    missing either way.
  - The uncited `lm-p70-p71-c64-cover.json` has stale `linked_sources`
    hashes, and 901 arcs against the doc's 872. It still re-derives with 0
    missing.
  - The generators add the clear only with `lm_channels`/`in_isr`. A future
    spec with a RESUME but neither key would skip it silently, so the
    generators should refuse that case.
