# Second review: lm-llge uncovered flight arms (#667, issue #660)

Second certificate reviewer, 2026-10-18. Lane F's #667 (head 0c8088f8) adds case states so that every FLIGHT arm in the #660 triage of Luminary099/LUNAR_LANDING_GUIDANCE_EQUATIONS.agc is taken, then refreezes. This follows the standard the steward relayed for #656-#661. **Verdict: accepted and re-cited.** The previous record is [rev2-lm-llge-563.md](rev2-lm-llge-563.md).

## Code

- **No source change.** src/lm_llge.c is byte-identical. The PR changes the spec, the docs and the regenerated evidence.
- **New pins.** None of them narrows the domain, because each list keeps a random third or keeps the plain cases:
  - P67NOW? +2: MODREG 66 and RODCOUNT +0, each in two thirds of the cases, with the rest random.
  - GUILDRET +6: FLPASS0 +0 in two thirds of the cases.
  - DERCLOOP +3: PWRCNT 0-2 with the PWRPTR and DERPTR walked to, in four of seven cases.
  - ROOTLOOP +19/+20: |DX| near DXCRIT (LLGE_CONVERGE). These are values the Newton loop produces at convergence.

## Evidence (my own runs at 0c8088f8)

- **Cases.** I rebuilt all 127,488 cases (allcases2p). The sha a14e65c9… equals the proof's cases_sha256.
- **FLIGHT arms on host** (gcov, src/lm_llge.c):
  - P66NOW?/RESTART? :727, both arms.
  - RODCOUNT BZF :733, both arms.
  - CCS FLPASS0 :1167, the +0 arm taken 350 times.
  - DERCLOOP CCS PWRCNT :5628, the +0 arm taken 152 times.
  - CCS MPAC :5745, all four arms. TESTLODX is taken too.
  - In each CCS, only the IMPOSSIBLE -0 arm stays untaken.
- **C64.** Per group, with the KEEP masks and the 33 resumed or truncated rows (cov_groups_resume), the 397 rows take 2,479 of the 2,480 host outcomes. The one left is agc_transfer.c:98 branch 5, the shared-source table-linkage NOTE from earlier reviews. Every lm_llge.c arc is taken on the C64.
- **Arc diff against the previous frozen cases.** I measured host gcov over all cases at base c16c3ec1, with the same lm_llge.c. 0 arcs are lost and 20 are gained (lines 727, 733, 744, 801, 1167, 1173, 1174, 5628, 5745, 5757). Lane F reports that its traced set loses VERTGUID's CCS WCHVERT -0 refusal arm. That arm is IMPOSSIBLE, and my all-cases measure doesn't lose it.
- **Host.** 3,984/3,984, re-run locally.
- **VICE.** 46 diagnostics, 397 checks, 0 failures, matching vice_targets.
- **Ledgers.** `ledgers.py --validate` passes and `cert_precheck` gives 0 FAIL.
