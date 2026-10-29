# Native Pinball display selection

`dsky_pinbranch` / `cm_pinbranch` execute actual PINBRNCH at LM10,3050 /
CM10,3272 with any retained aligned EB and legal SUPER/interrupt state.
They restore MARK2PAC into MPAC+2, execute RELINT and the original signed CCS
selection, and execute the normal UPFLAG path with exact ITEMP1/2/3, A/L/Q
and flag-word debris. Source flag bits select actual ERASER, MARKPLAY,
KEEPPRIO or PLAYJUM1; no job completion or extra scheduler poll is inferred.

`dsky_pinbranch_resume` / `cm_pinbranch_resume` execute those real selected
continuations. Normal and priority paths store actual COPINDEX2/0 and reach
GOPLAY (LM10,3070 / CM10,3312). MARKPLAY executes its original flag changes,
conditional MARKNV complement and COPINDEX1, reaching the same GOPLAY entry.
ERASER sets source A=-3 and calls actual NVSUB (LM4154 / CM4170), preserving its
real source return Q. Display rendering, noun tables, the broader GOPLAY
paths and negative NVSUB erase body are separate continuations; these entries
are source transfer boundaries, not substitutes for those services.

The canonical 2048-word image is authoritative. Source channels are untouched;
SUPER and interrupt enable are explicit metadata. All calls require restoring
producer exclusion owned by the caller. Unsupported bank/PC/index/IO input
preserves image, metadata and route. Neither native body fetches or decodes
original instructions.

Independent references rebuild both unchanged original programs. Original
Pinball, T4, Interpreter, Executive, Display Interface and Fresh Start sources
are restored before assembly; CM indentation repair is recorded. Whole rebuilt
rope bytes must equal the separately pinned reference fixtures. Each of16384
checkpoints compares all2048 words, SUPER, interrupt enable and selected route,
covering all eight upper flag combinations, all EB/SUPER domains, both signed
zeros in MARK2PAC/MARKNV, raw L values, all four selected continuation entries
and both conditional mark branches. Full native and ASan/UBSan streams match
the original byte for byte; the exact SHA is recorded by the retained logs and
frozen vector header.

`tools/dsky_pinbranch.mk` supplies portable host all/test/sanitize aliases and
an explicit C64 target. Sixty-four frozen complete original states are tested
on strict host, sanitizers and actual compiled C64 code under stock-ROM VICE.
Target checks include mailbox, RAM image limit, zero-page budget and hardware
and software stack sentinels. These services extend KEYREL control coverage;
ENTER, generic erase and the remaining display/user application fanout are
still separate work.
