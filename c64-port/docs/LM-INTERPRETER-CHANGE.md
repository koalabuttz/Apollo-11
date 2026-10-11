# Canonical Luminary CHANG2

`lm_interpreter_change` translates the literal `CHANG2` entry at05126, through `CHANJOB`/`ENDPRCHG` and either basic `DTCB` or interpreter `INTRSM` at `NEWOPS`06070. It exchanges the complete source MPAC/MODE/PUSHLOC/priority context directly in the single2048-word image and returns an explicit native `exec_transfer`. No operation is fetched or decoded. Every invalid entry is rejected before mutation.

The source preserves Q, stores outgoing BANKSET as the active core's basic-return debris, and leaves L equal to EXECBANK02001. Incoming negative PUSHLOC encodes saved OVFIND, including the negative-zero route. Interpreter returns complement/increment saved LOC and restore BANKSET, INTBIT15 and EDOP. These details are independently observed from the unchanged original assembly, rather than inferred from the typed Executive view.

The accepted domain is a literal completed-pair CHANG2 entry with EDOP+0, positive active and selected priorities, a real12-word selected core offset, valid source addresses/banks and explicit SUPER/IRQ metadata. Caller owns the image and excludes producers. Channel14 drive-enable bits are clear in the proof corpus, so hardware IMU/gyro/optics counter producers are quiesced; this helper does not emulate those devices. Channels11/14 are retained external metadata.

Validation covers16,384 complete2052-word source transitions: all2048 raw erasable words plus SUPER, source IRQ, channel11 and channel14. Strict and full ASAN/UBSAN streams match SHA256 `376bc4b403a955b1cf6f78563dc483ae09f7d5d666a94067c00859d039e57b54`. Separate tests verify all16,384 transfer payloads,27 unchanged invalid contexts and3 null guards.

The stock PAL C64 test loads32 real frozen-input disk modules and checks64 complete2052-word source fingerprints and selected-transfer payloads. It alternates native IRQ enabled/masked and verifies restoration of I and $01. Mailbox is `a50000400002`; measured software stack extent56 bytes and hardware extent25 bytes, with no additional zero page. The canonical image remains$7000, slot$8000 and normal software stack$C800. The resident ends$25E2; metadata uses$c000..c01f and mailbox$c080..c085. Fixture module C frames return normally before another slot load.

This packet qualifies the source exchange helper. Existing native scheduler actions and their frozen evidence are unchanged. Integrating an interpreter-origin source-selected adoption requires a separately qualified scheduler action; this packet does not claim the complete EXECUTIVE source file or full multi-job application integration.

Reproduce host tests with `make -f Makefile -f tools/lm_interpreter_change.mk test-lm-interpreter-change sanitize-lm-interpreter-change differential-lm-interpreter-change`. Target builder and VICE runner are in that same fragment. Retained artifact identities and dependencies are recorded in `evidence/lm-interpreter-change-proof.json`.
