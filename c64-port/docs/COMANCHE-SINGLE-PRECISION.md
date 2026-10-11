# Comanche single-precision source coverage

The complete original `Comanche055/SINGLE_PRECISION_SUBROUTINES.agc` is reviewed against native C numerical and return services. The only public entries are SPCOS and SPSIN. Their shared internal SPT, POLLEY and ARG90 paths are retained; the historical SPROOT text explicitly says that routine was deleted, and emits no code. BLOCK and COUNT are assembler placement/accounting directives.

| Original source lines | Native mapping and verified behavior |
|---|---|
| 35 SPCOS | `cm_single_trig(image,1)`: source HALF addition and raw accumulator overflow before SPSIN |
| 36–49 SPSIN / SPT | `cm_single_trig`: TS skip behavior, sign/quadrant reduction, TEMK, LIMITS and ARG90 branch |
| 50–62 POLLEY | `cm_single_trig`: source MP polynomial coefficients, SQ, DDOUBL, rounding-free raw A/L and final TS overflow |
| 64–65 ARG90 | `cm_single_trig`: source indexed positive/negative limit result |
| 63 and 66 TC Q | `cm_single_return`: source basic return through register Q, including retained A, corrected L, PC=saved Q and Q=3 |

The separate immutable `cm-trig-proof.json` qualifies all65,536 raw sixteen-bit accumulator inputs for each public numerical entry, every source branch, all2048 physical image words and external IRQ, with whole strict/sanitizer output equality. The final-return extension executes6,144 independent original calls, covering every fixed caller address02000..07777 for each entry. All32 FB and8 EB contexts, raw L patterns, MODE/scratch and unowned source words are preserved. Both final return paths05130/05133 occur in the source corpus and frozen target rows.

Each original call records its complete source-produced state immediately before TC Q and its complete state after the saved fixed-code return instruction in Q. The native return receives that independently computed entry. A separate composed native run starts from the original public-entry inputs and invokes both `cm_single_trig` and `cm_single_return`; it does not receive a precomputed numerical result. All6,144 final2048-word-plus-IRQ states from original execution, native return, native composed calls, and both whole ASAN/UBSAN builds are byte-identical (SHA256 `c422cb8f6c20eefe2be81d47d3c9baf38edf85a373e808565ce951193e66846f`).

Four actual C64 PRGs verify128 complete source states:32 each for return-only SPSIN, return-only SPCOS, composed SPSIN and composed SPCOS. All pass. Maximum loaded end is$3cf5; measured hardware/software stack use is20/50 bytes, and compiler-reserved zero-page use is0. These are independent standalone diagnostics. Complete resident application loading, boot, device bindings and integrated scheduling remain application acceptance work.

The return API accepts source-produced PC05130 or05133, REGZERO0, and ordinary fixed-code saved return addresses02000..07777. It rejects unsupported entry states unchanged. Arbitrary synthetic erasable-code returns and the Q2 self-loop are outside the source fixed-code caller domain. The numerical API already preserves any ordinary saved Q until this explicit return. The return consists of native C state transfer; no AGC opcode decoder or copied instruction engine is used. There is no Executive check inside this basic service or its return; the caller retains canonical ownership and excludes scratch/input producers throughout. IRQ is retained externally. This API does not implement the distinct QUICTRIG TC A/SWRETURN/BUF2 bridge owned by CLOCKJOB control.

Original source SHA256 is `768303431e1f6f0d83b63b8d68663755167e9dd4c869f79d364b0e82ef9aea64`, from pinned original revision `911e5c0283c629c50cb97666f34065e8c07d71a5`. There are no source edits or indentation changes in this file. Original whole-rope SHA256 remains `2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79`; fixture/source audit and host-only reference-engine qualification belong to the unchanged CM trig package. The historical source tree remains untouched.

Reproduce the numerical prerequisite with `tools/cm_trig.mk`. Then run `make -f tools/cm_single_return.mk test sanitize complete-test complete-sanitize check-host complete-check-host`; regenerate target rows with `python tools/freeze_cm_single_return.py`. The `c64` and `vice-test` targets accept SELECTOR0..3 (default0), MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR. The reference prerequisite is `tools/prepare_hardware_dv.py` and the independently assembled `deps/cm-trig-fixture/MAIN.agc.bin`.

`cm-single-return-reference.snapshots.gz` retains12,288 source entry/exit states. One deterministic compressed final-state object serves all five equal output streams, with explicit logical names and decoded SHA identities in `cm-single-return-proof.json`. Actual PRGs, maps, source comparison/sanitizer logs, mailboxes and stack captures are retained under the manifest. The separate whole-file certificate is ready for inventory review; it does not certify another original source file or either complete Apollo program.
