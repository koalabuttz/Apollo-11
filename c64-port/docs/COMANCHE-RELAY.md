# Comanche relay output source qualification

`cm_relay_drain` executes original Comanche055 DSPOUTSB06,2024 through the actual caller Q or Q+1. It shares the fixed native relay scan only after independent execution of the actual CM assembly verifies every physical word. The CM Q+1 helper preserves its literal source Q=6710. Raw L normalization, source banking, IRQ state, relay stores, and the real channel 10 write event are retained. An emitted word can be passed directly to the native C64 DSKY renderer after the source return.

The qualified domain is bank06/EB2, SUPER0..160 in steps of020, normal nonnegative A=NOUT-1, DSPCNT0..10 or negative zero, nonzero/non-negative-zero DSPTAB entries, and ordinary caller Q0100..07776. These relay-table exclusions avoid the original CCSHOLE paths. Unsupported input leaves image, I/O and output unchanged. The caller owns exclusion around the canonical image.

The independent oracle executes 4096 original states, comparing all2048 physical words, SUPER/IRQ, the write-event flag and channel10 latch: 2052 fields per checkpoint. Cases include all scan indices, dirty and clean entries, no-output paths, all SUPER settings, ordinary return locations, raw L normalization, and A corners0,1,037776,037777. Original and native snapshot SHA256:
`2597ce3724b0e690186e0fba733a88d6dd8032226075917d5520f483c18052fe`.
The complete ASan/UBSan stream matches the original byte for byte. No native service body is linked into the original oracle. Original Pinball and T4 source bytes, indentation-only assembler normalization, and the byte-identical pinned CM rope are audited in `build/cm-relay-source-audit.json`.

Frozen64 complete source images pass strict host, sanitizer, and native C64 checks. Case0 obtains its real MODE89 channel10 write from the native source body; the diagnostic consumes that event and checks screen RAM0400 and color RAMD800 for digits8 and9. Color comparisons use the defined low nibble. VICE PAL mailbox `a500004002`; PRG13495 bytes (loaded13493); hardware/software stack16/110 bytes. Target bounds remain belowC000 with no additional compiler zero-page allocation.

Evidence uses `build/cm-relay-*` and `build/cm_relay-*`; full streams and original assembly listing are retained compressed. `tools/cm_relay.mk` provides portable host-only all/test/sanitize and explicit c64-cm-relay/differential-cm-relay targets. This body is available for real CM display consumption; full CM T4 scheduling and application display composition remain separate work.
