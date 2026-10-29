# Original operator recall, both missions

`dsky_response_recall` and `cm_response_recall` translate RECALTST,
RECAL1/2/3, DOPROC, and DOTERM through actual ENDOFJOB. The source entries
are LM bank 40 PCs 3547/3552/3563/3575/3601/3577 and CM bank 40 PCs
3454/3457/3470/3502/3506/3504. The body composes the unchanged canonical
JOBWAKE and RELDSP services without inserting a scheduler poll.

RECALTST plus-zero CADRSTOR ends immediately, leaving unrelated Executive
state opaque. Other values, including minus zero, select the original
wake and clear CADRSTOR. LOADSTAT plus zero takes the pathological source
exit; positive values advance the saved location by one, negative nonzero
values by zero, and minus zero by two. The original noun and verb are
saved into the selected context through DXCH. RELDSP then releases the
display and performs its source-selected second wake.

An unmatched wake returns LOCCTR minus one. The original INDEX instructions
then update LOC minus one and MPAC minus one/MPAC. This native body preserves
those stores and scratch debris. Direct RECAL2 accepts raw A and ordinary
saved LOC, including the actual TS overflow branch that skips CA NOUNREG
and stores the overflow unit into the noun slot. No unmatched-wake shortcut
or synthetic job completion is substituted.

The caller owns one canonical image and excludes producers. Both missions
cover all EBANKs, selecting SUPER 100/120/140/160, IRQ 0/1, raw registers,
ordinary CADRSTOR and LOADSTAT classes. Direct saved-context entries accept
aligned core displacements or minus one. Selected wakes retain the frozen
kernel domain: positive active core 0, nonzero other priorities, canonical
NEWJOB, and legal first matching dormant bank. A second wake checks the
next actual dormant match after the first slot has been awakened. Unused
scheduler state remains opaque; plus-zero CADRSTOR and empty direct
RECAL3 include otherwise invalid plus-zero core state. Rejection preserves
all image, IO, channel-event, and restart state.

The original-source oracle is separate from the tested native body and
uses fresh assemblies with byte-identical pinned ropes. All 8,192 native,
original, and full ASan/UBSan checkpoints agree over 2,048 source words,
SUPER, IRQ, channels 11/12/13, channel-event mask, restart flip-flop, and
the fixed END route (2,056 fields). SHA-256:

`414fba3ad772e2830f733e19ab522898a065d34780d7feb1b702539ccd045ba8`.

Frozen 64 source cases and 16 atomic rejection cases pass host/sanitizer.
Stock-ROM PAL C64 diagnostics compare 32 full states per mission: LM
`a500002000`, PRG 14,773 bytes (14,771 loaded), stack extents 16/735;
CM `a500002001`, PRG 15,079 bytes (15,077 loaded), stack extents 16/714.
Both remain below C000 and allocate no extra zero page. The isolated
`tools/dsky_response_recall.mk` supplies host/test/sanitizer/differential
and C64 targets. Canonical END/idle adoption and the calling Proceed,
Terminate, resequence, and PRO key bodies are separate composition stages.
