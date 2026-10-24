# Certificate review: Luminary099 and Comanche055 PINBALL_GAME_BUTTONS_AND_LIGHTS.agc

Reviewed 2026-10-24 by certificate reviewer 1, against milestone
`lanec-pin-cert` (Lane C), at #674 head 790beaf7 (on base d39f6871, with
#824 and #825), merged as b56219b3.

Earlier rounds: rejected 2026-10-18 at f0092478 (the flight verb/noun address
and bank guards, the cover record crediting an unrecorded run, three suites
missing); a code pre-review at 65a5c1cd2 found the code clean, with the
EV refreeze and the C64 evidence still to do. The Pinball code is unchanged
since that pre-review.

Verdict: **accepted**, for both files.

## What was checked

- **Store rebuilt from the 790beaf7 tree** (`tools/flight_store_build.py
  --force`: lm 167 modules, cm 161, 0 failed). It reproduces every hash the
  evidence cites:
  - lm.reu `ac161b96d39775dc…` and cm.reu `14b3c88738a0b881…`;
  - the manifests `65c778c3…` (lm) and `f7fae0f6…` (cm);
  - the ropes `1f5326e0…` (lm) and `6e3150ae…` (cm);
  - the store prefixes in `lanec-pin-c64-results.json`, whose 214 result
    lines are all PASS (pin-keys and pin-key-labels 1,408 cases per mission,
    pin-chains, pin-flight, pin-gaps and the pw and transfer programs).
- **Full DSKY sweep.** The release asset
  `evidence-dsky-sweep/d96a1cb841d86050-dsky-sweep.json` hashes to
  `d96a1cb8…f4889`. That equals the release digest, `SHA256.json` and the
  tree summary's `full_sha256`.
  - Recomputing the summary from the full report gives the same keys and
    counts: lm 1,830 of 1,830 and cm 1,830 of 1,830 end in the host state,
    with 0 skipped and 0 failed cases.
  - The mode is `all` on both missions, item 6 holds, and
    `tools/dsky_sweep_known.json` is empty.
- **VICE on the rebuilt store.** cm V83E + PRO, cm V37E06E and lm V37E00E
  each end in the host state. These are the cases #824 and #825 fixed, and
  the one D1b traced.
- **cert_precheck** with the rebuilt store in place, both with the summary
  only and with the full report: 0 FAIL, and the same output both ways. The
  WARNs are the uncached vice-manifest, the release-stored source maps, and
  the refusal count. The refusals are argued in docs/LANEC-PIN-CERT.md.
- **C64 branch cover.** 0 host arcs are missing on the C64. The 117 host-only
  arcs (43 lines) are in other certified files' routines (the Executive,
  waitlist and interpreter that pooled transfer chains run on into, and
  `lanec_di_mark.c`) or in test-harness code. None is in a Pinball module.
- **Tool changes in the PR.**
  - `module_test.py`'s base-kind exemption is sound: `lm_dntm_io` opens with
    `p66_bias_io`, and `cm_dntm_io` with `cm_native_io`.
  - The sweep's busy sample is applied the same way on both sides.
  - cert_precheck's new summary path catches a wrong `full_sha256`, a
    non-clock failed case, a sample-mode run, a stale store hash and a missing
    mission, each probed.

## Open follow-up (not blocking)

- #843: in summary-only mode, cert_precheck does not check that the summary's
  counts add up (`passed + len(failed) == cases`). This certificate does not
  depend on that check, because the summary was recomputed from the full
  report above.

## Not run

- The VICE suites themselves were not rerun here; their PASS lines name the
  rebuilt store's hashes, and the c64 CI shards were green on 790beaf7.
- The `--replay-exact` sweep (acceptance item 6) is a separate follow-up.
