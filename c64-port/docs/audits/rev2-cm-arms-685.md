# Second review: CM uncovered flight arms in cm-p30-p37, cm-p40-p47 and cm-tpi-search (#685)

Second certificate reviewer, 2026-10-19. The Certificate fixes lane's #685 (merged c5edf5b9) adds case states so that every FLIGHT arm in the 10-18 uncovered-arms triage of these three Comanche055 specs is taken, then refreezes each one. The bar is the one the steward relayed for #656-#661. **Verdict: accepted and re-cited.** The previous records are [rev2-recite-p30-p33.md](rev2-recite-p30-p33.md) (P30-P37.agc), [rev2-cm-p40-p47-616.md](rev2-cm-p40-p47-616.md) (P40-P47.agc) and [rev2-cm-r31-tpi-630.md](rev2-cm-r31-tpi-630.md) (TPI_SEARCH.agc). Reviewer 1 re-cites the LM specs and r30/r60-r62 from the same PR.

## Code

The native sources are unchanged apart from regeneration. The only changes are case states in `tools/interp_specs.py`, and none of them narrows the domain: each one sets a value in only part of the draw, and the rest of the cases keep their random values.

- **cm-p30-p37.** At AUGEKUGL's last `DSU BMN V(400)` (32,2123, a CHANG2 resume), `resume_states` puts MPAC in the 28,000-28,400 ft/s entry-speed band.
- **cm-p40-p47.** NBRCYCLS (E6,1742, held under every EBANK by `cells`) is -1 in about an eighth of the cases, as P40SXTY and every S40.9 pass leave it. CLOKTASK's return from WAITLIST (24,3301) also runs with NVWORD1 = V06N85B, as in P41.
- **cm-tpi-search.** GOFLASH's third return (VNCOMP17 +8, `CS MPAC`) runs with MPAC = 040, the crew's V32.

## Evidence (my own runs at base 29c47284, which carries #685)

- **Cases.** I rebuilt every frozen case set with allcases3p. Each sha equals the proof's `cases_sha256`:

| spec | cases | sha | C64 rows |
|---|---|---|---|
| cm-p30-p37 | 73,216 | abc13b2b… | 180 in 6 groups |
| cm-tpi-search | 101,888 | 05241d59… | 249 in 16 groups |
| cm-p40-p47 | 126,976 | 1f36f1d9… | 646 in 51 groups |

- **FLIGHT arms**, checked by gcov. Each one is taken on the host, and the same arm is taken by the C64 rows:
  - cm-p30-p37: AUGEKUGL `DSU BMN V(400)` to LOOPSET (src/cm_p30_p37.c:1133) is taken 386 times on the host.
  - cm-tpi-search: VNCOMP17 `BZF P17.1` (35,3252; src/cm_tpi_search.c:852) is taken 353 times.
  - cm-p40-p47: UPDATEVG `SLOAD BMN NBRCYCLS` to SETUP.9 (24,3110; src/cm_p40_p47.c:9224) is taken 141 times. CLOKTASK `BZF SETUPDYN` (24,3310; :4557) is taken 182 times, and SETUPDYN's `CAF PRIO27` / `TC FINDVAC` (:4591-4596) run 182 times.
- **C64, per group.** These checks use the KEEP masks (cov_groups):
  - cm-p40-p47's C64 rows take all 2,767 host branch outcomes.
  - cm-tpi-search's take all 1,301. The one missed line is in the test harness (tools/interp_routine_native.c:110, the uncut runner path), not in port code.
  - cm-p30-p37's take 949 of 950. The one untaken outcome is agc_transfer.c:98 branch 5, the known NOTE.
- **Arc diff against the previous frozen cases (base 7566bdb4).** The diff is keyed by AGC comment, so it survives line shifts.
  - cm-p30-p37: 0 lost, 1 gained.
  - cm-tpi-search: 0 lost, 1 gained.
  - cm-p40-p47: 0 lost in the cert's files, 14 gained. Five arcs in the shared lm_interp_orders.c (included by cm_interp_orders) drop out of the reshuffled draw. This doesn't block, because the interpreter specs certify those paths.
- **The lane's runs.** All cases equal yaAGC, and `cert_selfcheck` gives OK for each spec (#685).
