# Both-program invalid keyboard input

`dsky_charin_alarm` and `cm_charin_alarm` execute actual LM CHARIN40,2077 and CM CHARIN40,2000 for illegal keycodes0,012..017,023..030 and035. They retain the original display lock, old-lock register, key-release lamp path and key fanout, then execute CHARALRM/FALTON and reach genuine ENDOFJOB5155/5217 with the actual literal return words. Channel11 operator-error lights while all display and numeric state remains unchanged. The source IRQ state and raw L normalization are preserved.

The source 89TEST path is also qualified for low DECBRNCH bits0: positive/+0 DSPCOUNT rejects8/9 through CHARALRM, while negative/-0 DSPCOUNT exits directly to genuine ENDOFJOB without operator-error. The latter branch retains the source diminished-absolute accumulator and distinct negative/-0 return words. Accepted decimal8/9 remains the separately translated NUM service's domain.

CADRSTOR=-0 is retained accurately: CCS skips the CS ELRCODE1 instruction. Illegal key0 then produces +0 and bypasses key-release lighting, while the other invalid keycodes still light it. Ordinary occupied CADRSTOR values use the usual key-release output. Final channel11 and its real output-event flag are checked independently, including initially lit lamps and both signed-zero paths.

Bank40/SUPER100, every retained EB0..7, IRQ0/1, arbitrary ordinary display/count words and explicit channel11 are qualified. Unsupported keys or metadata are rejected before mutating image or I/O; the caller excludes producers. No source instruction or address word is decoded into a native callback.

Independent unchanged original assembly compares8192 complete2052-field checkpoints (2048 physical words plus SUPER/IRQ/channel11/event), with all illegal keys, octal8/9 rejection and blocked branches, every EB, raw L boundaries and CADRSTOR sign/zero cases. Full source stream SHA256:
`811b97bc597ae218be8e10f1f743ad3e9b10b8cf9994045f3d00815669a02b7d`.
The complete ASan/UBSan stream matches byte for byte. Original Pinball/T4/Interpreter bytes, CM indentation-only repairs and byte-identical pinned assembled ropes are audited; the oracle links no native body.

Frozen64 complete images pass strict host, ASan/UBSan and actual native C64. VICE PAL mailbox `a500004002`; PRG6164 bytes (loaded6162); hardware/software stack10/26 bytes. All sections remain belowC000 and additional compiler zero-page storage is rejected. `tools/dsky_charin_alarm.mk` provides portable host-only all/test/sanitize and explicit differential/C64 targets. Evidence uses build/dsky-charin-alarm-* and build/dsky_charin_alarm-*.

This module stops at the real ENDOFJOB entry. Actual Executive completion, hardware keyboard admission and C64 channel-output routing belong to the separately composed application lifecycle; unsupported ENTER/KEYREL are not silently treated as invalid-key completion.
