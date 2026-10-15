# Certificate review: Luminary099 LUNAR_LANDING_GUIDANCE_EQUATIONS (lm-llge)

Reviewed 2026-10-14 by the second certificate reviewer, on #462: first at
583d9cd2 (rejected in #468 for its domain), then at the refreeze 2da08bfa, and
the domain text at bc3c6f85. Verdict: **accepted**.

## What was checked (at 2da08bfa, and on the tree merged with the base)

| Check | Result |
|---|---|
| `evidence.py fetch`/`verify`, cited sha256 | source and 6 evidence files match |
| `cert_precheck.py` | 0 FAIL; the VICE manifest WARN (proxy) checked by hand; 445 refusal sites |
| `ledgers.py --validate` | PASS |
| yaAGC differential | 125,952 cases (246 routines x 512) equal yaAGC in Z, all 2,048 erasable words, the interrupt-enable state, channels 13, 30 and 31 and the trap flags; case set matches the proof |
| `cert_selfcheck.py lm-llge` | OK. `agc_transfer.c` 26/5 (find()'s not-found exit, which a KEEP-mask group table cannot take) is a NOTE under #466, taken on C64 by other specs on the same source |
| Strict `--cover-check` | verified, 0 missing; 29 truncated rows (24 rebuilt, 5 resumed) |
| Host and sanitizer | 3,936 checks each, 0 failures |
| VICE 3.7.1 | 45 of 45 PRGs byte-identical and PASS, 379 checks = `vice_targets` = proof rows = header rows |
| `interp_compile --check` | 79 of 79 specs |

On the tree merged with the base, `interp_compile --check lm-llge`, the
differential (same case set), the row re-runs and the host tests give the same
results.

## Domain

Every pin was checked against every writer in Luminary099, aliases included
(WCHVERT and NGUIDSUB are ELVDUMMY, NIGNLOOP is ZERDUMMY, WCHPHOLD is
OURPERMS):

| Pin | Flight values | Spec |
|---|---|---|
| WCHVERT / NGUIDSUB | +10 (STARTP67), +0 (P66), -2 (P65START), 2/1/0 (GUIDSUB) | +10, 2, 1, 0, -1, -2 |
| NIGNLOOP | 40 (IGNALG), counted down to 0 (DDUMCALC) | 0..40 |
| WCHPHASE | -1 (P63), 0 (BURNBABY), WCHPHOLD+1 (EXTLOGIC), 2 (VRTSTART) | -1..2 |
| WCHPHOLD at CGCALC..EXGSUB | equal to WCHPHASE (AFCCLEND copies it) | 0..2, or both -1 (a third of cases) |
| WCHPHOLD at EXBRAK..P63DISPS, 1406P00..FASTCHNG | 0..2 (-1 there loops for ever on the AGC too, and BURNBABY zeroes both at ignition) | 0..2 |
| X1 | -TARGTDEX (INTPRETX, its only writer) | -0, -034, +1 |
| PUSHLOC, MODE at the resume points, ROOTPSRS's entry | SERVOUT/FINDVAC give vac+0; DP and vector pushes only; TTF/8CL is ROOTPSRS's caller | as flight leaves them |

supported_domain states each pin with its reason (added at bc3c6f85, text
only).

## Rules

- #403: PITFALL (RUPT10), RODTASK (WAITLIST) and their returns, REDESMON and
  DESCBITS use `at_basic_nv`, with `any_fixloc` and `any_newjob` (46 of 84
  basic routines). RODCOMP (the FINDVAC job) and DISPEXIT +3 use the job check.
- #385 and #414 do not apply: no RESUME instruction, no BOV/BOVB/DANZIG.
- WOR CHAN13 and READ CHAN30/31 match yaAGC's channel model (trap bits 12-14
  kept and read back as 0). The DISPEXIT +3 entry is VHORCOMP's BANKCALL return.

## Shared runner

#462's `lm_cut` rewrite in `tests/interp_cut.c` changes the PRG bytes of
lm-p70-p71-file, lm-r31-file, lm-p40-p47 and lm-p32-p35. All 111 rebuilt PRGs
pass VICE with per-PRG results identical to their records. Under the
steward's ruling the recorded sha256 refreshes at each record's next refreeze;
#462's body lists the four specs passing.
