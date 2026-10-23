# Review: PR #789, lm-powered-subs re-cite (Luminary099 POWERED_FLIGHT_SUBROUTINES.agc, Lane B)

Reviewer 1, 2026-10-23. The PR refreezes lm-powered-subs with `outside_off_keys` (as #781 did for cm-powered-subs). Reviewer 2 accepted the whole PR at 949ba7cf and is handling the three Comanche055 entries; this review covers only the Luminary099 re-cite.

- **Base:** 97e71f5a, which is still the tip of port/c64-native-c, so the PR is not behind.
- **Head moved:** the PR head is now **307d7bc0**, not 949ba7cf. 307d7bc0 is one docs-only commit on top of 949ba7cf. It adds the domain sentence to the four per-spec docs, which was reviewer 2's nit, and that includes `docs/LM-POWERED-SUBS.md:31`. Nothing else changed after 949ba7cf.
- **How I checked:** the head tree was exported (without the bulk evidence store) to the scratchpad. The shared checkout and its branch were not touched.

## Verdict

**Luminary099/POWERED_FLIGHT_SUBROUTINES.agc: ACCEPT (re-cite).** There are no blockers. Merge once the in-progress `c64 (0)` job is green.

## What the PR changes for lm-powered-subs

These are all the LM-relevant changes.

- `tools/interp_specs.py:932`: `'outside_off_keys': True` on the lm-powered-subs spec.
- `tests/lm_powered_subs_vectors.h`: exactly **one** of the 1,248 frozen cases changed.
  - Case 971 (`:976`) is the basic entry 23,3662, the word after EXIT. Its seed 0x958beb51 is unchanged, and it gains one edit, `{023,00}`, which sets EDOP to +0. That is the `pending_on_key` zeroing.
  - Its expected digest changes from 0xe9363aee to 0x54ec172b.
  - No other seed, edit list or digest changed.
- `tests/lm_powered_subs_c64_vectors.h` and `tools/mk/lm-powered-subs.groups` are unchanged. The 51 C64 case ids do not include 971.
- `evidence/lm-powered-subs/lm-powered-subs-proof.json`: only `cases_sha256` (6955864… → 1c4a7926…) and `oracle_sha256` (8d892b1f… → 737ba65b…) changed.
  - The case count (19,968), frozen_vectors (1,248), c64 case ids and the stops table are unchanged.
  - c64_coverage is unchanged: 861/861 edges and 221 labels.
  - gcov is unchanged: 527/527.
  - outcome_cover is unchanged: 407.
- `evidence/lm-powered-subs/SHA256.json`: new hashes for the host log, vice log and proof.
  - The differential log is byte-identical (8e254313…). It records no case hash, and its stops table is unchanged.
  - The closure (3c478315…) and the source map (23534223…) are unchanged.
- `tools/cert_arms_allow.py:814`: only `cases_sha256` moved. `sources_sha256` and the 7 arms are untouched, and `src/lm_powered_subs.c` is unchanged (44106c94…).
- `docs/port-coverage.json:11960`: a new `supported_domain.basic_entries` sentence. The four evidence cites at :11975-11996 are updated.
- `docs/LM-POWERED-SUBS.md:31`: the same sentence, word for word.

## Reproduced (head 307d7bc0, on the host)

| Check | Result |
|---|---|
| `interp_routine_differential.py lm-powered-subs --cases 512 --freeze` | PASS, 19,968 cases, 39 routines. The regenerated `lm_powered_subs_vectors.h`, `_c64_vectors.h` and `.groups` are byte-identical to the committed files. The log is byte-identical to the release differential log. |
| **Fresh salt** `IRC_SALT=rev1-789`, 512 cases per routine | **PASS, 19,968 cases equal yaAGC.** The stop distributions at 23,3605, 3606 and 3635 differ from the frozen set, which confirms that the salt took effect. |
| **Flag off** (the flag removed from the scratch spec, then refrozen) | Reproduces **base 97e71f5a's** `lm_powered_subs_vectors.h` byte for byte. So the only thing the refreeze changes is the flag's effect. |
| `make test-lm-powered-subs sanitize-lm-powered-subs` | 1,248 checks, 0 failures (both) |
| `make vice-lm-powered-subs` (1 PRG) | PASS, 51 checks, 0 failures. The PRG sha256 d17f64fd… equals the one in the committed vice log. |
| `evidence.py fetch` / `verify lm-powered-subs` | 3 release logs fetched. All 7 files match SHA256.json and the port-coverage.json cites. |
| `cert_precheck.py Luminary099/POWERED_FLIGHT_SUBROUTINES.agc` | PASS, 0 FAIL. The 1 WARN is only that there is no cached vice manifest offline. |
| `cert_arms.py check` | 0 failures |

**Arc diff.** I built the host test with `gcc -O0 --coverage`, once with the base frozen vectors and once with the head frozen vectors, against the head sources. I ran gcov from the c64-port root (build/gcov-rev1-{base,head}).
- The executed lines and taken branch outcomes are identical over all 11 linked non-harness sources.
- Each side has 1,824 lines and arms, of which 527 are taken branch outcomes, matching the proof's 527. Nothing was lost and nothing was gained.
- Combined with the unchanged proof coverage counts over all 19,968 cases (861 edges, 527 gcov, 407 outcomes), **no arm and no case was dropped**.

**Mutation probes.** Each mutated file was restored afterwards and its sha re-checked.
1. **QUICTRIG.** `src/lm_powered_subs.c:960` changed from `ai_mpf(s,020000u)` (MP BIT14) to `017777u`.
   - The host test fails: 1,248 checks, 27 failures.
   - The differential refuses to run because the source is "not what tools/interp_compile.py generates".
2. **DMPSUB2** (hand-written, run in place by AX\*SR\*T and FLESHPOT). I dropped the final `DAS MPAC` at `src/lm_dmpsub.c:41`.
   - The yaAGC differential at 23,3675 AX\*SR\*T fails. Case 2432 has MPAC/VAC words 0122-0130 that differ from the oracle.
   - The host test fails: 159 failures.

## Checks against the brief

1. **The refreeze changes only what the flag should change.** Verified directly: with the flag off, the base vectors come back exactly; with it on, the head vectors come back exactly.
   - In the frozen set the visible change is one EDOP zeroing at a basic entry, and its seed is unchanged.
   - The PR's "55 of 59,904 outside draws redrawn, 4 EDOP zeroed" is consistent with this. I did not recount it.
   - The filter code (`interp_routine_differential.py` `outside()` and `pending_on_key`) is unchanged from #781, which reviewer 2 accepted there.
2. **Counts and arcs: nothing lost.** See above.
3. **Evidence ids and shas exist and match.**
   - Every path cited in the certificate exists, and its sha256 matches: source map, proof (cited twice), host log, vice log, closure and `docs/audits/rev2-recite-b506.md` (ab238c13…).
   - The release logs fetched are the ones in SHA256.json.
   - The host and vice logs record e159f0de, the PR's first commit. That is correct, because no source or vector changed after it.
4. **Coverage entry, docs and stale cites.**
   - The four evidence cites in the LM entry use the new hashes.
   - The old proof, host and vice hashes, the old `cases_sha256`, the old `oracle_sha256` and the old digest 0xe9363aee no longer appear anywhere in docs/, tools/, tests/ or the LM evidence of the head tree. I grepped the exported tree.
   - The LM doc's evidence counts (19,968 / 1,248 / 51 / 527 / 861) match the proof.
5. **ABI and domain.**
   - Sources and entries are unchanged, so the transfer ABI review from earlier re-cites still holds.
   - The new domain sentence describes the filter accurately: outside CADRs avoid file words, registered keys and DANZIG; EDOP is zeroed at basic entries with LOC on such a word; RTB pairs are excepted.
   - The flight justification holds for this file. QPRET is the word after a CALL. FLESHPOT's own EDOP/LOC state at 10,2173 is an interpretive (`edop_exits`) entry, which the basic-entry zeroing does not touch, because the zeroing applies only where `kinds[point] != 'i'`.

## CI and mergeability (at 09:08 UTC)

- Run 39927438110 on 307d7bc0:
  - `plan`, `format`, `host (0)` and `ledgers-and-coverage` passed.
  - `c64-legacy`, `c64-maps` and `cover-check` were skipped.
  - **`c64 (0)` is still in progress** (started 08:50).
- GitHub reports `mergeable_state: unstable` only because of that pending check. There are no conflicts, and the base is current.

## Blocking

None.

## Not blocking

1. **Audit record.** The certificate's `independent_review` (`docs/port-coverage.json:11991`) still cites `docs/audits/rev2-recite-b506.md`. That record describes the older 25,088-case / 71-VICE freeze. Its sha matches, and the PR leaves it unchanged, as #789 should; reviewer 2's comment also says re-cites follow the merge. The LM re-cite after merge should add a reviewer-1 record of this review, as #781 did with `rev2-cm-conics-tff-powered-781.md`, and point `independent_review` at it.
2. **Key name.** `supported_domain.basic_entries` (`port-coverage.json:11960`) also covers the outside-return draws, which are not only a basic-entry matter. The name is cosmetic.
3. **Head moved after reviewer 2's acceptance.** The acceptance covered 949ba7cf. The only delta, 307d7bc0, is the four doc sentences. Its LM part is verified above.
