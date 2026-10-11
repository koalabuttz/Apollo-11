# Comanche display restart with mixed priority selection

`cm_init_display_mixed_stage` composes the actual INITDSP restart entry
(`10,3261`) with source PRIOCHNG (`05210`) and the suspended INITDSP+6
continuation (`10,3267`). Stage 0 restores EBANK from EBANKTEM and SUPERBNK
from RESTREG, then reaches PRIOCHNG with its actual Q, priority and register
image. Stage 1 executes the qualified seven-core scan and returns the genuine
current, selected basic, or selected interpreter transfer. Stage 2 executes
the genuine `3267` BANKJUMP return when that continuation is selected.

The stages use the sole canonical 2,048-word physical image and explicit
SUPERBNK/IRQ latches. Each native frame returns before a loader swaps its
slot. Producer exclusion is caller-owned. A selected basic or interpreter
application body is a subsequent explicit native continuation; this API does
not invoke it or synthesize its completion. A suspended INITDSP+6 remains
in the outgoing source core until genuine selection restores it.

The supported priority/context domain is the independently qualified
`cm_priority_complete` domain. The entry prefix requires a consistent bank
image, positive requested upper priority in RESTREG, and source-width saved
words. The final BANKJUMP wrapper requires an ordinary compiled caller CADR
whose low ten bits are at least four, so subtracting three remains in its
compiled bank. Unsupported inputs preserve the full image, external latches
and output transfer. Reserved-register callers, CCSHOLE/+0 fault candidates,
and selected application/alarm body execution remain separate domains.

The independent original-layout assembly is authenticated as
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The 7,168 cases cover current selection or each of six saved slots, basic and
interpreter selection, and 512 priority/register/bank/PUSH/OVFIND/restore
combinations. All 15,360 complete source checkpoints match every physical
word, raw A/L/Q and SUPERBNK/IRQ, with lossless stream SHA256
`f3faa2f98ec421c7c868185bb19e4107d8a04a1bbbbe789f3d562d4a3e4ab15f`.
The selected native transfer and kind are checked independently against the
source CPU image. Full strict and ASAN/UBSAN validation pass.

The C64 diagnostic loads native control module 213 and proof-only input and
checker modules 254/255 from a D64 through the checked loader. It replays 56
representative actual source contexts and 120 phase checkpoints, alternating
incoming C64 interrupt state. Complete-state fingerprints and actual selected
tuples cover every phase; two complete image/I/O samples and the final full
image are compared directly. The retained report records the actual verdict,
mailbox, payload bytes and measured software stack extent.

Run `make -f Makefile -f tools/cm_init_display_mixed.mk
test-cm-init-display-mixed` and `sanitize-cm-init-display-mixed`.
`tools/cm_freeze_init_display_mixed.py` regenerates the target vectors from
byte-identical retained source/native streams. This is a source-qualified
restart/priority bridge; the complete Executive file and all mission display
restart callers require their own domain closure.
