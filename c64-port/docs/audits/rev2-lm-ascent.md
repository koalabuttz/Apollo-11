# Certificate review: Luminary099 ASCENT_GUIDANCE (lm-ascent)

Reviewed 2026-10-14 by the second certificate reviewer, at 4d2103b5 (#474).
The certificate was rejected in #403, because ENGOFF1 runs as a WAITLIST task
and its entry checked the job-only domain. #474 fixed that. Verdict:
**accepted**.

## What was checked

| Check | Result |
|---|---|
| `evidence.py fetch`, cited sha256 | all 6 match |
| `cert_precheck.py` | 0 FAIL; the VICE manifest WARN (proxy) checked by hand; 226 refusal sites (as in #403) |
| `ledgers.py --validate` | PASS |
| yaAGC differential | 107,008 cases equal over 209 routines |
| `cert_selfcheck.py lm-ascent` | OK: 3,594 rows re-run on yaAGC equal their digests; host 1,717 = C64 1,717 gcov outcomes (shared-source NOTEs only) |
| Strict `--cover-check` | verified, cases match, 250 rows, 10 truncated rows rebuilt, 0 missing |
| Host and sanitizer | 3,344 checks each, 0 failures |
| VICE 3.7.1 | 29 of 29 PRGs byte-identical and PASS, 250 cases = `vice_targets` = proof rows = header rows; 1,728 host arcs = 1,728 C64 arcs in the cover record |

## Rule #403

ENGOFF1 runs as a WAITLIST task, as the listing shows: TWIDDLE at 14,2375
(ADRES ENGOFF1), the group 4 restart at 14,2377 (PHASCHNG OCT 47014, 2CADR
ENGOFF1), and TERMASC's POSTJUMP at 14,2434. A task interrupts any job, so the
spec now sets `any_fixloc` and `any_newjob`. ENGOFF1 and the three routines
it returns through (14,2410, 14,2412, 14,2416 and 14,2423) check with
`at_basic_nv` (`src/lm_ascent.c` 446-475). The host cases cover NEWJOB −0 and
a FIXLOC outside any VAC area at each of them. On C64, 14,2416 runs with a
FIXLOC outside a VAC area. Other `sane_nv` entries take the NEWJOB −0 arm, and
gcov shows nothing missing.

The five routines that keep the job check all enter the interpreter and run
only in jobs: 14,2404, 30,2722, 30,3074, 34,3644 and 34,3663. There is no
other task or interrupt entry: ATMAG runs in the SERVICER job, CUTOFF is
started with FINDVAC, and TERMASC and CUTOFF2 run in the CUTOFF job. #385 and
#414 do not apply: there is no compiled RESUME and no BOVB.

## Domain

The spec pins no erasables. The case generator's defaults were checked
against the program's writers:

- NEWJOB and FIXLOC cover what flight holds (see above).
- PUSHLOC stays inside the file's push depth.
- MODE takes all three modes, and EDOP is +0 at NEWOPS as INTPRET leaves it.
- EB is random 0-7, a superset of flight's 7. The superbank test reads only
  bit 0100.
- The file reads only TIME2/TIME1 among 010-061, and supported_domain states
  how TIME1 spans 0..037777.

No pin or default excludes a value flight can hold.

## Fixed in this PR

- `docs/LM-ASCENT.md` and `docs/milestones/lm-ascent.md`: 10 truncated C64
  rows, not 21 (21 is the number of host cut candidates).
- `docs/LM-ASCENT.md`: the 1,728 arcs are the cover record's figure. The
  proof's `c64_gcov_branches` block gives 1,685.
