# Second review: CM uncovered flight arms (#679, issue #659)

Second certificate reviewer, 2026-10-18. Lane G's #679 (head 71dad9df, merged 7bc7c95d) adds case states so that every FLIGHT arm in the #659 triage is taken in four Comanche055 certificates, then refreezes them: CONIC_SUBROUTINES (cm-conics), CM_ENTRY_DIGITAL_AUTOPILOT (cm-entry-dap), REENTRY_CONTROL (cm-reentry) and INTEGRATION_INITIALIZATION (cm-integration-init). This follows the standard the steward relayed for #656-#661. **Verdict: accepted and re-cited.**

## Code

- **No source change.** The PR changes `tools/interp_specs.py`, the group files and the regenerated evidence.
- **New pins.** `joint_states` and, for integration init, `flow_range_states`. Each pin list keeps the random or flow choice beside it, so the domain is not narrowed:
  - reentry: DELV and MPAC at ±0 before GETUNITV.
  - entry DAP: roll phase-plane points at ATTRATES+31.
  - integration init: UPSVFLAG 0/±2, MODREG 0, a small DT/2 and TDEC ≈ TET.
  - conics: VTARGTAG +0 at TARGETV-5.

## Evidence (my own runs at 71dad9df)

- **Cases.** I rebuilt every spec's frozen cases (allcases2p, plus allcases3p with the freeze's loop and refusal filters for integration init). Each sha equals the proof's cases_sha256: 28dc0164…, df0af548…, 0a984fa4… and 8b45962a….
- **FLIGHT arms**, taken on the host and on the full C64 build (gcov):
  - cm_reentry.c:1434 SETMIND (host 153, C64 1).
  - cm_entry_dap.c:3136 CCS Q >0 and :3149 TCF OVRLINE1 (host 34, C64 2); :3328 BZF TOFFOVFL (46/1); :3704 BZMF TIMETST5 (2/1).
  - cm_integration_init.c:2362, :2676, :2694, :2826, :3360, :3511 and :3579, all taken on both.
  - cm_conics.c:5575 BZE TARGETV (68/1).
- **Per-group C64** (KEEP masks, cov_groups_resume / cov_groups). Missing host outcomes: 0 of 2,190 (conics), 0 of 958 (entry DAP), 0 of 2,036 (reentry), 0 of 1,436 (integration init).
- **Arc diff against base a163b162** (host gcov over all frozen cases):

  | spec | file arcs lost | arcs gained |
  |---|---|---|
  | conics | 0 | 1 |
  | entry DAP | 0 | 6 |
  | reentry | 0 | 4 |
  | integration init | 0 | 11 |

  Integration init's reseeded draw no longer takes 9 arcs in the shared `lm_interp_orders.c` (SQRT's smpac, sqrtneg and abrt paths, and DDV's zero-dividend test at :668). This is not blocking, because the interpreter order specs certify those paths.
- **UNSURE.** cm_conics.c:1147 MODDONE BZE BADX (TAU. = 0) stays untaken. The triage names no yaAGC run that reaches it, so under the standard it does not block.
- **Host.** 3,592, 224, 784 and 482 checks, 0 failures.
- **VICE.** 621 / 85 / 544 / 336 checks over 30 / 3 / 47 / 9 diagnostics, 0 failures. These match the PR's vice targets.
- **Ledgers.** `ledgers.py --validate` and `cert_precheck` pass.
