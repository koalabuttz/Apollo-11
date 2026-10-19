# Second review: uncovered flight arms in p51-p53 and cm-p20-p25 (#707)

Second certificate reviewer, 2026-10-19. Lane D1b's #707 (issue #658, merged 96265f41 from head 3d234883e) adds case states so that the five FLIGHT arms of the 10-18 uncovered-arms triage of these two specs are taken. It then refreezes both specs: p51-p53 at 512 cases per routine and cm-p20-p25 at 256. **Verdict: accepted and re-cited.** The previous records are [rev2-recite-p51-p53-486.md](rev2-recite-p51-p53-486.md) (Luminary099 and Comanche055 P51-P53.agc) and [rev2-cm-p20-p25-617.md](rev2-cm-p20-p25-617.md) (Comanche055 P20-P25.agc).

## Code

The native sources are unchanged. The only changes are case states in `tools/interp_specs.py`, and none of them narrows the domain: each one sets a value in only part of the draw (or adds flowed cases), and the rest of the cases keep their random values.

- **p51-p53 (LM).** OPTION2, the P57 technique, is 0-3 in part of the draw at SURFDISP and P57POST.
- **cm-p20-p25.** MRKBUF1 is R23CSM's -1 at the two TCF R21CSMA, and WWOPT is 1/2/3 at V6N99INP. V67WW's CHANG2 resume points (40,3701-3740) also get flowed cases with a small W matrix, small running sums and OVFIND clear.

## Evidence (my own runs at 3d234883e)

- **Cases and C64 cover.** I ran `interp_routine_differential.py <spec> --cover-check` against the committed cover records:

| spec | cases | cases_match | C64 rows | host arcs missing on C64 |
|---|---|---|---|---|
| p51-p53 | 655,872 | true (afb443cb…) | 1,562 | 0 of 5,665 |
| cm-p20-p25 | 288,768 | true (c85823ce…) | 1,460 (31 cut rows rebuilt, 2 resumed digests) | 0 of 4,982 |

- **FLIGHT arms**, checked by my own gcov over the regenerated cases of the touched routines:
  - LM SURFDISP `CCS OPTION2` (15,3174): the +0 arm to `TCF P57POST +1` (src/lm_p51_p53.c:7835) is taken 470 times on the host and on the C64 rows.
  - LM P57POST `BZF +2` (15,3217) to `TC PHASCHNG` (15,3221; :7926/:7932) is taken 186 times on the host and on the C64 rows.
  - CM R21CSMA `BZF R21END` (37,2542; src/cm_p20_p25.c:15006) is taken 256 times on the host and on the C64 rows.
  - CM V67 `BHIZ` (40,3654; :15721) is taken on the host (1,293) and on the C64 rows.
  - CM V67WW `BOV` falling through to V67XXX (40,3737; :16057) is taken 2,716 times on the host. The cover check above confirms the C64 rows take it.
- **The one new OFFNOM.** `CCS OPTION2` -0 is correctly classed OFFNOM. ALIGNOPT (15,3331) masks only the copy in A, so a crew-keyed 77777 stays in OPTION2.
- **Untaken arms.** `tools/cert_arms_allow.py` loses the five FLIGHT arms and gains no untaken arm, so the refreeze lost no host arm. CI's cert_arms check measures this.
- **The lane's runs.** All cases equal yaAGC, the host and C64 runs pass, and CI is green on 3d234883e.
