# Certificate review: Lane G after PR #359

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
7aeecf3f (PR #359 merged), with the yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Comanche055/ORBITAL_INTEGRATION | `cm-orbital-integration` | accepted |
| Comanche055/TIME_OF_FREE_FALL | `cm-time-free-fall` | re-confirmed (C64 rows re-picked) |
| Comanche055/TVCINITIALIZE | `cm-tvc-init` | re-confirmed (C64 rows re-picked) |

## ORBITAL_INTEGRATION

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL (the one WARN is the uncached nightly VICE manifest). The
  `source_revision` is 911e5c02. The 368 refusal sites are the `sane` and
  `basic_sane` entry guards, `cmi_switch` guards, TC Q guards and dispatcher
  defaults.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py cm-orbital-integration --cases 256`
  (the frozen size) prints PASS with all 85,248 cases equal across 333
  routines. The case set hashes to the proof's `cases_sha256` (c74290e6), and
  the vectors came out unchanged. The strict `--cover-check` verifies 444
  rows (32 truncated rows rebuilt, 1 resumed row by digest), 1,732 host arcs,
  0 missing.
- **Host and sanitizer.** `make test-cm-orbital-integration
  sanitize-cm-orbital-integration` gives 666 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL. Its
  27 PASS lines sum to 444 checks, equal to `vice_targets`, the proof's C64
  rows and the rows in `tests/cm_orbital_integration_c64_vectors.h`.
- **Independent C64 branch check.** All 443 non-resumed header rows match a
  regenerated case. The per-group build with KEEP flags and `-DIRC_CUTS` takes
  all 1,759 host outcomes, 0 missing. The resumed row's snapshot was
  re-derived from yaAGC (152 words equal), and yaAGC run from it to the cut at
  11,3110 equals the native run and the digest. That row alone takes the
  `sgndvovf` BZMF false arm (src/lm_interp_orders.c 658), so it is needed.
  All 33 truncated rows were rerun on yaAGC to their cuts and agree.
- **Domain.** The file is entered only from interpretive integration jobs
  (INTEGRATION_INITIALIZATION RECTIFY/KEPPREP/TIMESTEP,
  MEASUREMENT_INCORPORATION 402, P76 127), so the entries see a VAC FIXLOC and
  NEWJOB +0 or positive. ENDSTATE's phase change (ORBITAL_INTEGRATION.agc
  733-738) re-enters through INTEGRATION_INITIALIZATION, not mid-file.
- **Note (not blocking).** The BDDV `resume_states` at 11,2736 force MPAC's
  high word to +0. Of 141 such resumes only 1 is a true zero divisor; the
  rest are tiny divisors that overflow. Flight cannot reach either after NORM
  ROUND, so these widen the tested domain beyond flight. They stay inside the
  stated domain and yaAGC agrees on all of them. The wording in
  `tools/interp_specs.py` ("divide-by-zero") overstates it and can be
  corrected at the next refreeze.

## TIME_OF_FREE_FALL and TVCINITIALIZE (row re-pick)

Both case sets are unchanged (56173aff, 7cbccf93), and both differentials
pass at the frozen 256 cases per routine (30,976 and 2,560). Host and
sanitizer give 968 and 80 checks with 0 failures. The VICE logs match the
certificate hashes, and the rebuilt PRGs are byte-identical to the logged
ones. PASS sums are 193 and 15, equal to `vice_targets`, the proof rows and
the header rows. The strict `--cover-check` now verifies 1,017 of 1,017 and
413 of 413. The shared `src/agc_transfer.c` find() arcs at lines 25-26, which
had been missing since 2db70a83, are now taken on the C64. Per-object and
per-group gcov show 0 missing outcomes and lines. The generated-C changes
(`image_w`, `index_carry16`) do not change behaviour in these builds, since
`RK_FIXED_IMAGE` is unset and `ixw()` is not called.
