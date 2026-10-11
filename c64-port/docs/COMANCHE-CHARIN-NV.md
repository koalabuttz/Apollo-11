# Comanche VERB/NOUN keyboard input

`cm_charin_nv` executes actual Comanche055 CHARIN40,2000 for VERB021 and NOUN037 through VERB/NOUN, NVCOM and 2BLANK to real ENDOFJOB5217. It retains actual CM ENTRET5217 and Q2271 stores, channel11 lamp effects, signed-zero blanking and overflow-corrected NOUT increments. The shared native digit/blanking body is qualified by independent execution of the original CM assembly, rather than assuming LM addresses or state effects.

The domain is actual bank40/SUPER100 with any retained EB0..7, either IRQ state, ordinary blank/NOUT words including +0 and the overflow boundary, and raw L. The caller excludes producers. Unsupported input leaves the complete image, I/O and event unchanged. Actual ISR admission, selected foreground CHARIN dispatch and source Executive completion remain composition work; NUM and other key bodies are separate native entries.

The independent original oracle compares4096 complete2052-field checkpoints: all2048 physical words, SUPER/IRQ and channel11/event. Snapshot SHA256:
`7a3704304f8cc0d3aeff9e733d745ef95807b52b6b0c517a98b9d855a88a4e51`.
The complete ASan/UBSan stream equals the original byte for byte. The oracle links no native service bodies. Original Pinball/T4/Interpreter bytes, indentation-only assembler normalization and the byte-identical pinned CM rope are recorded in `build/cm-charin-nv-source-audit.json`. The captured differential log uses an inherited NUM label; its constructor and oracle execute only VERB/NOUN and the preserved source files identify the actual tested domain.

Frozen64 complete source images pass strict host, sanitizer and native C64. VICE PAL mailbox `a500004002`; PRG11622 bytes (loaded11620); hardware/software stack10/22 bytes. Target bounds and extra zero-page storage are checked. `tools/cm_charin_nv.mk` provides portable host-only all/test/sanitize and explicit C64/differential targets. Evidence uses `build/cm-charin-nv-*` and `build/cm_charin_nv-*`, retaining complete streams and original assembly listing compressed.
