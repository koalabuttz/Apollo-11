# Certificate review: Comanche055 P37_P70 after PR #372

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
9f3affb7 (PR #372 merged), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Comanche055/P37_P70 | `cm-p37-p70` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL (the one WARN is the uncached nightly VICE manifest). The
  `source_revision` is 911e5c02. The 634 refusal sites are the `sane` and
  `at_basic` entry guards, `cmi_switch` and RTB-mode guards, TC Q and
  index-return guards, and dispatcher defaults. The TS-overflow skip at
  `src/cm_p37_p70.c` 7794 cannot trigger (A is +0 after CAF ZERO).
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py cm-p37-p70 --cases 512` (the frozen
  size) prints PASS with all 304,640 cases equal across 595 routines. The case
  set hashes to the proof's `cases_sha256` (4e954996), and the 1,190 frozen
  vectors are every 256th case. The strict `--cover-check` verifies 700 rows
  (3 truncated rows rebuilt, 1 resumed row by digest), 0 missing.
- **Host and sanitizer.** `make test-cm-p37-p70 sanitize-cm-p37-p70` gives
  1,190 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL. Its
  56 PASS lines sum to 700 checks, equal to `vice_targets`, the proof's C64
  rows and the rows in `tests/cm_p37_p70_c64_vectors.h`. All 56 PRGs rebuilt
  at the tip are byte-identical to the logged ones.
- **Independent C64 branch check.** All 699 non-resumed header rows match a
  frozen case. The 56 KEEP-group builds with `-DIRC_CUTS` take all 2,476 host
  outcomes and 6,069 lines, 0 missing. The resumed row's snapshot was
  re-derived from yaAGC (44 pairs equal), and yaAGC run from it to the cut at
  32,3656 equals the native run and the digest. Only that row takes GAMDV40's
  BMN (`src/cm_p37_p70.c` 5065), so it is needed. The 3 plain truncated rows
  rerun on yaAGC to their cuts and agree.
- **Accounting.** The 1,368 words in banks 16, 31, 32, 35 and 36 are 1,263
  words in 595 routines, 98 fixed operand constants, 7 inline VN operands and
  32,3107 OCT 00610, the alarm code POODOO reads after V2TERROR.
- **Domain.** P37 is started only by V37 (`FRESH_START_AND_RESTART.agc` 1276,
  PREMM1 EBANK 7) through SPVAC, so the entries see a VAC FIXLOC and NEWJOB +0
  or positive; Comanche has no P70 or P71 in its major-mode table. The file's
  one phase change makes group 4 inactive (P37_P70.agc 113-114, "P37 IS NOT
  RESTARTABLE"), and the CSMPREC calls clear DIM0FLAG so ENDSTATE skips its
  phase change; no restart re-enters mid-file. The GOFLASH and GOFLASHR waits
  resume in the same job, and all 409 CHANG2 resume points start from flowed
  states. The ALARM, POODOO and VARALARM stops of the divide, square-root and
  arc orders all equal yaAGC.
- **Doc fix in this PR.** `docs/CM-P37-P70.md` and the `native_coverage`
  scope still described the earlier freeze (194 routines, 99,328 cases, a
  10,000-routine limit, 40 diagnostics and 17 host-only routines). They now
  match the evidence: 595 routines, 304,640 cases, `max_steps` 40,000, 56
  diagnostics with 700 rows and no host-only routine.
- **Not blocking.** The cover-check counts 2,447/2,461 taken host/C64 arcs
  against the proof's 2,443/2,457, likely a gcc version difference (13.3
  here); 0 missing either way.
