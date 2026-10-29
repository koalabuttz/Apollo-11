# Luminary single-precision source coverage

The complete original Luminary099 SINGLE_PRECISION_SUBROUTINES source is mapped to native `lm_single_trig` and `lm_single_trig_return`. Its only public entries are SPCOS and SPSIN, with shared SPT, POLLEY and ARG90 paths. BLOCK/COUNT are source placement/accounting directives. No other routine or emitted data belongs to this file.

| Original lines | Native source behavior |
|---|---|
| 36 SPCOS | HALF addition into raw A, entering SPSIN |
| 37–50 SPSIN / SPT | TS skip/overflow, sign and quadrant reduction, TEMK and LIMITS |
| 51–63 POLLEY | Source MP coefficients, SQ, DDOUBL, final TS and overflow branch |
| 65–66 ARG90 | Indexed positive/negative limit result |
| 64 / 67 TC Q | Source basic return through register Q; PC=savedQ, Q3, corrected L, retained A/banks/IRQ |

The original LM file is independently assembled over pinned LM support, with no source or indentation edits. Original SHA256 `bebe6cbf186bed7fcc35f9aad8a392ccbe3cb31154ad8267d01a4926cc0dd3e4` matches the assembled file. Complete rope SHA256 remains `1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`. The equations agree with Comanche, but this independent qualification uses LM source entries05033/05032, returns05066/05071, TEMK01076 and SQ01077. Fixed original coefficients are01124,065552,031103; HALF020000 and LIMITS040000/037777 retain original raw arithmetic semantics.

Every sixteen-bit raw A input is executed for each entry, including signed zeros and overflow patterns. Original assembly and native composition each record the state before TC Q and the state after the saved fixed-code return instruction. The resulting262,144 complete2048-word-plus-IRQ states match the strict native and whole ASAN/UBSAN streams byte-for-byte (SHA256 `48222b46a09e4d6fe71e08b6cc77891b4f113b69b9e9a42b3e9b508067d99703`). Every fixed saved return address02000..07777 occurs for each public entry; all32 FB/all8 EB contexts, MODE, raw L, shared scratch and every unowned source image word are compared. Source branch counts are retained in the proof object.

Four actual C64 PRGs verify128 complete states: numerical SPSIN/SPCOS and each routine composed with its final return. All pass. Maximum loaded end is$391b, measured hardware/software stacks are17/45 bytes, and compiler-reserved zero-page use is0. Target fixtures reconstruct complete source images with lossless literal source-output deltas from the qualified deterministic input constructor; they do not supply arithmetic results to native routines.

The numerical entry accepts REGZERO0, raw A/L, ordinary saved Q<=07777 and the selected original entry PC. The final return accepts only source-produced PC05066/05071 and fixed-code saved TC addresses02000..07777. Unsupported entry states reject unchanged. Synthetic erasable return code and the Q2 self-loop are outside the original fixed-code caller domain. There is no AGC instruction decoder, host floating point, or copied GPL instruction engine in the native implementation. The oracle alone links the host Virtual AGC engine. No Executive check occurs in this basic routine or its return; canonical ownership and exclusion of scratch/input producers span the whole call. IRQ is retained externally.

Reproduce source qualification with `python tools/lm_single_trig_prepare_reference.py` and the pinned host reference prerequisite `python tools/prepare_hardware_dv.py`. Run `make -f tools/lm_single_trig.mk test sanitize check-host`; regenerate frozen target rows with `python tools/freeze_lm_single_trig.py`. The freezer also accepts the committed compressed original corpus if the raw build stream has been removed after verification. The `c64`/`vice-test` targets accept SELECTOR0..3 (default0), MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR.

One deterministic compressed object retains the byte-identical original/native/sanitized logical streams. Exact SHA identities, source audit, independent assembly listing, actual target PRGs/maps/mailboxes/stacks and logs belong to `evidence/lm-single-trig-proof.json` and its manifest. The separate whole-file reviewed certificate supports inventory integration. These standalone numerical diagnostics do not certify either complete resident Apollo program or another source file.
