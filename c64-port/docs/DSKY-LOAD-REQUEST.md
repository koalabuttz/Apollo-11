# Original operator load request controls

`dsky_load_request` and `cm_load_request` execute ABCLOAD, ABLOAD, ALOAD,
BLOAD, and CLOAD at actual bank 41 PCs 2616/2703/2732/2743/2760, plus
next-component continuations 2624/2627/2711. They compose independently
qualified COMPTEST/NOUNTEST, real UPDATVB-1 formatting, REQDATX/Y/Z,
five-character blanking, FLASHON, and the actual selected ENTRET target.
No callback, new scheduler poll, or synthetic job finish is inserted.

ABCLOAD and ABLOAD display each next component's original verb and save
the correct source continuation into REQRET. BLOAD/CLOAD perform the
original component test and set CLPASS for pass zero only. ALOAD bypasses
those tests exactly as the source does. Component/no-load failures retain
the source GODSPALM transfer; clean-zero relay words retain the actual
formatter/blanking CCSHOLE prelude and inhibited IRQ. Normal prompts stop
at real ENDOFJOB, NVSUBEND, or PASTEVB, selected by the source ENTRET.

The image is the sole authoritative flight state and the caller excludes
producers. Ordinary RAM, raw CPU registers, all EBANKs, selecting SUPER
100/120/140/160 and IRQ 0/1 are qualified. MIXBR 1/2 is required only when
an actual component/noun test reads it; ALOAD and the three resume entries
accept opaque unused MIXBR. A supported compiled ENTRET descriptor is
required only when the prompt actually returns through it. Early noun
alarms and clean-zero relay holes leave an otherwise unsupported saved
return opaque. The full corpus includes those unused minus-zero/zero
saved-return words. Rejection preserves image, IO, channels/events,
restart flip-flop, and route.

8,192 independent unchanged-original checkpoints compare all 2,048
words, SUPER/IRQ, channels 11/12/13, event mask, restart flip-flop, and
selected route (2,056 fields). All five initial entries and three resume
entries, all 32 type codes, normal/mixed/unused noun selectors, relay
sign/zero classes, NOUT overflow edges, raw registers and selecting
latches are sampled. Original, native, and full ASan/UBSan streams match
byte for byte, SHA-256:

`27634eb114c27ef7cb85e1fe6ba4f5f14ee28add7a76d69a77dbea047cf80eb2`.

Frozen 64 states and 16 atomic guards pass host/sanitizer. Actual stock-ROM
PAL C64 runs compare 32 full states per mission: LM `a500002000`, PRG
19,319 bytes (19,317 loaded), stack extents 21/102; CM `a500002001`, PRG
19,572 bytes (19,570 loaded), stack extents 20/127. Both stay below C000
with no extra zero-page allocation. These linked test diagnostics do
not establish a production 16 KB disk overlay. The independent oracle
links no tested native bodies; fresh fixture ropes match the pinned
originals byte for byte.

`tools/dsky_load_request.mk` supplies isolated host-only `all`, host tests,
sanitizer, source comparison and target builds. This closes the load
request/control stages under their stated contracts. PUTXYZ/PUTXY and
single-component store continuations, PUTCOM/channel writes, all input
scales, real subsequent keyboard entry, and canonical scheduler adoption
remain follow-on work. This is not a completed load/scaling or Pinball
file certificate.
