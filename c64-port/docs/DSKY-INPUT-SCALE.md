# Original arithmetic and DP input scale bodies

This additive service translates actual ARTHINSF, BINROUND, ARTIN1SF, DPINSF,
DPINSF+2, DPINSF2, DPINSF4 and the DP store continuation (LM DPINSF+9/CM
DPFRACIN). It composes the unchanged verified DMPSUB product kernel and actual
TPSL1/TPLEFTN/2ROUND/TESTOFUF source services. It executes native C arithmetic
and compiled source control, never instruction decoding.

| Entry | LM bank40 PC | CM bank40 PC |
|---|---:|---:|
| ARTHINSF | 03076 | 02766 |
| BINROUND | 03106 | 02776 |
| ARTIN1SF | 03111 | 03001 |
| DPINSF | 03114 | 03021 |
| DPINSF+2 | 03116 | 03023 |
| DPINSF2 | 03141 | 03046 |
| DPINSF4 | 03146 | 03053 |
| DP store/DPFRACIN | 03125 | 03032 |

`dsky_input_scale` uses `dsky_lamp_io`, while `cm_input_scale` uses
`cm_native_io` plus retained write-event and restart-light words. Source image
is canonical; callers retain restoring producer exclusion. Entries require
FB060000, selecting SUPER100/120/140/160, any retained aligned EBANK with
coherent BBANK, ZERO+0, IRQ0/1, ordinary memory and raw A/L registers. TP limbs,
scale constants and overflow flag support independent word signs and either
zero. Incoming Q is opaque because these are source continuation entries.

DP bodies require MIXBR1/2, DECOUNT0..2 only for selected mixed addresses,
and NOUNADD logical0000..1774 such that the selected physical destination is
ordinary RAM1100..3775 with a bank-local pair. Register/editor/scratch alias
stores are outside this caller contract. Arithmetic and BINROUND retain these
unused fields as opaque values. This bounded destination contract applies to
DP entries including possible alarm branches; it is not a whole Pinball
arbitrary-address certificate. Unsupported input preserves image, IO, events,
restart and route atomically.

DMP uses the literal original SFTEMP1 address and records exact ADDRWD,
MPTEMP and A/L/MPAC exchanges. A temporary typed product object exists only
for arithmetic calculation; it is discarded and never owns persistent source
state. Original MODE is untouched because the basic DMP entry never reads it.
ARTHINSF performs all three original XCH operations, then reaches actual
ALMCYCLE if the discarded high word is neither zero. BINROUND and ARTIN1SF
preserve both original conditional TS rounding skips and TESTOFUF returns.

DPINSF rounds the low limb into the middle limb with the actual DOUBLE/TS
conditional skip of CAF ZERO. DPINSF2 and DPINSF4 perform seven or three actual
left shifts, preserving SFTEMP1/2, overflow flag and L/Q debris. The final
normal/mixed branch writes the low word to the source selected erasable
address, preserves the displaced address in MPAC+1 and executes actual
ENDSCALE/ENDSCAL1 POSTJUMP. The completed route stops at PUTCOM241,3155,
FB062000 and Q01155, preserving the scale result in A. PUTCOM2 and its selected
stored caller are separately translated by the unchanged input-store service.
CM DPFRACIN performs the actual direct DP store without a multiply or rounding.

ALMCYCLE (LM04145/CM04161) is a true RECYCLE frontier. TESTOFUF flag-0 reaches
its genuine LM03200/CM03105 fallthrough and reports FALLTHROUGH, preserving
actual CPU state rather than inventing a return. Normal PUTCOM producers clear
this flag before scaling; standalone entries prove both source zeros and all
ordinary sampled flag classes. Channels, SUPER, IRQ, write events and restart
lamp are unchanged by these bodies.

The independent fresh-assembly oracle links only original AGC source and the
constructor, never native kernels. Both fresh ropes match their pinned original
fixtures. All8192 complete LM+CM states match strict native C and the complete
ASan/UBSan stream: all2048 image words plus SUPER, IRQ, channels11/12/13,
events, restart and route. Snapshot SHA256:
`e8f945dc7a0c94fbfed54bdb6167dd4a4469e954656773c798c0b0615c2ce52b`.
The corpus spans all eight entries, limb/sign/zero/extrema classes, raw A/L,
scale factors, flags, normal/mixed destinations, banks/latches and retained
unused fields. It samples combinations; it does not assert their Cartesian
product. Frozen64 whole source states and16 meaningful atomic guards pass
strict host and sanitizers. Separate native LM/CM C64 diagnostics execute32
whole frozen states each with actual PAL VICE mailbox, below-C000 memory
budget, no added zero-page storage and hardware/software stack sentinels.
These linked diagnostics are component proofs, not application disk-overlay
integration evidence.

`tools/dsky_input_scale.mk` supplies host-only all/test/sanitize aliases and
explicit C64/fresh differential/freeze targets. Exact source/dependency hashes,
original fixture audits/listings, full compressed source/native/sanitizer
streams, host logs, PRG/maps and raw mailbox/stack evidence are frozen. Degree,
optics and time input scaling, final load-verb caller stores, channel output
and whole keyboard/application integration remain separate tracked work.
