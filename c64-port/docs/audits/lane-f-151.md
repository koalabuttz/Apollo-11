# Certificate review: ten Lane F Luminary099 files after PR #151

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
0f8483f0 (PR #151 merged at 23b88743). This is a re-review after issues #76,
#99, #108, #109, #123, #127 and #147.

| File | Milestone | Verdict |
|---|---|---|
| AGS_INITIALIZATION | `lm-ags-init` | accepted |
| BURN_BABY_BURN--MASTER_IGNITION_ROUTINE | `lm-burn-baby` | accepted |
| DAPIDLER_PROGRAM | `lm-dapidler` | accepted (re-confirmed) |
| LANDING_ANALOG_DISPLAYS | `lm-landing-displays` | accepted (re-confirmed) |
| THE_LUNAR_LANDING | `lm-lunar-landing` | accepted |
| P-AXIS_RCS_AUTOPILOT | `lm-p-axis` | accepted |
| Q_R-AXIS_RCS_AUTOPILOT | `lm-q-r-axis` | accepted |
| THROTTLE_CONTROL_ROUTINES | `lm-throttle` | accepted (re-confirmed) |
| TJET_LAW | `lm-tjet-law` | accepted |
| TRIM_GIMBAL_CONTROL_SYSTEM | `lm-trim-gimbal` | accepted (re-confirmed) |
| SERVICER | `lm-servicer` | **not accepted**, see the auditor issue filed with this review |

DAPIDLER, LANDING_ANALOG_DISPLAYS, THROTTLE and TRIM_GIMBAL were already
complete (`lane-f-dap.md`, `lm-landing-displays.md`). PR #151 replaced their
evidence, so this record re-confirms them against the new evidence and becomes
their `independent_review`.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on every file. The four evidence
  categories cite four distinct records, and none cites the source map. Every
  `source_revision` is 911e5c02.
- **yaAGC comparison, rerun by the reviewer.** `make <m>-differential` rebuilt
  each yaAGC oracle with the hardware divide on the unmodified Luminary099 rope.
  Every case equals yaAGC, and the frozen vectors and C64 vectors came out
  byte-identical (`git diff --exit-code`):

  | Milestone | Equal | Refused |
  |---|---|---|
  | lm-tjet-law | 16,384 | 0 |
  | lm-p-axis | 16,235 | 149 |
  | lm-q-r-axis | 15,547 | 837 |
  | lm-throttle | 16,384 | 0 |
  | lm-dapidler | 9,216 | 0 |
  | lm-trim-gimbal | 16,211 | 173 |
  | lm-landing-displays | 16,384 | 0 |
  | lm-ags-init | 29,952 | 0 |
  | lm-lunar-landing | 44,758 | 1,322 |
  | lm-burn-baby | 21,676 | 596 |

  The regenerated `lm-tjet-law`, `lm-p-axis` and `lm-q-r-axis` proofs are
  byte-identical to the committed evidence, which closes #99 item 7.
- **Host and sanitizer.** `make test-<m> sanitize-<m>` passes with 0 failures
  on all ten, including each source checker and the trim-gimbal CA table check.
- **VICE.** Every log's PASS totals equal the certificate's `vice_targets`:
  lunar landing 81+38, burn baby 246+21, the rest one PRG each. Every log
  records the PRG sha256.
- **Independent C64 branch check.** This used the reviewer's own tool,
  `lanef_cov.py`, kept in the reviewer's notes. It is gcc `--coverage` with
  `gcov -b` per object, not Lane F's clang trace-pc-guard. The run covered
  every linked `src/` file and ran the full build over every host case. Each
  C64 build (per PRG for burn baby and lunar landing, with the group macro)
  ran only its own rows from `tests/<m>_c64_vectors.h`. Every branch outcome
  and executed line the host cases take is also taken by a C64 row: 0 missing
  on all ten. Switch lines whose arm count differs between the full and a PRG
  build are judged by arm-line execution.
- **Transfer ABI.** In lunar landing and burn baby, every word after a DANZIG is
  a key, including the CHANG2 resume LOCs listed in #109 and #123 (36,2152,
  36,2233, and DDUMCALC/DDUMGOOD/P63SPOT2). The P66 items moved to
  LUNAR_LANDING_GUIDANCE_EQUATIONS (#109 item 4).

## Prior blockers

- #76: THROTTLE, DAPIDLER and TJET C64 gaps are closed by outcome-chosen rows
  over every linked source. The TJET FORMULA2 `DAS HH` no-carry outcome is
  taken on the C64.
- #99: the Q_R refusals at the return entries are now the documented SKIPU/
  SKIPV, ALLOWGTS and channel 5 exclusions, each with its producer argument.
  RETJADR is handled. P-AXIS detent ratedamp and the mixed-sign DV dividend are
  taken on the C64. `native_files` lists `agc_hardware_divide.c`, `agc_word.c`
  and `agc_multiword.c`.
- #108: the shared-source C64 gaps are closed. The DDV ±0 dividend and equal
  majors are compared (a sixth of the E4 states). The VAD AGSRND1/2 carry into
  037777 is generated. The remaining unreached GENDDV/MAXTEST/ROUNDSUB outcomes
  are argued in `docs/LM-AGS-INIT.md`.
- #109: the BMN at exactly DDUMCRIT is compared with both signs. The 1,322
  refusals are keys inside DDUMCALC's pushdown arithmetic. The generator
  draws PUSHLOC anywhere in the VAC area, and those cases pop below FIXLOC.
  The live job resumes there holding the pushes made before the key, so it
  never pops below FIXLOC.
- #123 and #147: the C64 gaps and the bank 36 resume LOCs are closed.
  KILLTSK2/DEAD refuses L>16 at ADRSCAN, TSTFBANK and KILLDEAD, so the
  INDEX L carry into the opcode is never admitted. The 596 refusals are
  these plus the producer-argued ones in `docs/LM-BURN-BABY.md` (INDEX WHICH
  tables, negative PHASE words, DISPDEX, P40/RET, overflowed TS).

## Notes (not blocking)

- The cover JSON files are still not byte-reproducible, because trace-pc-guard
  block numbering depends on ASLR (#108 item 4). The rows themselves
  reproduce, and the vectors are frozen.
- `cert_precheck` counts Q_R refusals as 1,674 where the differential reports
  837. The differential's count is the one checked here.
