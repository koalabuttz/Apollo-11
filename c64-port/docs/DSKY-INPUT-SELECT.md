# Original input scale selection and cyclic primitives

This additive native C service translates both original missions' SFRUTNOR,
SFRUTMIX, SFCONUM, CONUMNOR, SFRET, SFRET1 and GOQ, together with the actual
fixed-fixed RIGHT5, LEFT5 and SLEFT5 source bodies. It reads and writes only the
canonical image; neither native target nor algorithm decodes instructions.

`dsky_input_select(image, dsky_lamp_io*, route*)` and
`cm_input_select(image, cm_native_io*, event*, restart*, route*)` report actual
source caller return. Producer exclusion is owned by the caller. Selected
saved Q/EXITEM must hold an ordinary logical return0100..7777; unsupported
contexts preserve image, IO, events, restart and route atomically.

The bank41 entries are identical logical addresses in both missions:
SFRUTNOR03026, SFRUTMIX03034, SFCONUM03047, CONUMNOR03071, SFRET03064,
SFRET103046, GOQ03025. They require FB062000 and selecting
SUPER100/120/140/160, any retained aligned EBANK and coherent BBANK.
Fixed-fixed RIGHT5/LEFT5/SLEFT5 are LM04322/04331/04340 and
CM04336/04345/04354; these accept all32 retained FBANKs and all eight legal
SUPER latch values. IRQ0/1, ZERO+0 and ordinary image words are source contracts.
Raw A/L register values, both zeros, all packed five-bit fields and independent
old editing-register contents are supported.

Only SFCONUM selects MIXBR1 normal or2 mixed. SFRUTMIX and selected mixed
SFCONUM require DECOUNT0..2. Other entries retain these fields as opaque words,
including signed-zero values. Direct CONUMNOR/SFRET/SFRET1 use the already saved
EXITEM caller; other entries use incoming Q. The original EXITEM stored TC
return executes its actual instruction, leaving Q0115. Direct fixed-fixed
primitives and GOQ leave Q3. The native body reproduces the source editing
register debris, saved EXITEM and L dispatch word. SFRUTNOR extracts middle5
of NNTYPTEM; SFRUTMIX selects the component field of RUTMXTEM; SFCONUM extracts
the component field of NNTYPTEM and doubles the constant pair index.

RIGHT5/LEFT5 retain the exact edit-on-access ordering: normal five-bit output
and its complemented final editing-register store, plus the conditional TS
skip when raw A has overflow. SLEFT5 performs five original DOUBLE operations,
retaining register overflow and signed-zero semantics. SFRET directly doubles
raw A; SFRET1 preserves it. Source dispatch is compiled control flow, not a
runtime instruction interpreter.

The independent oracle freshly assembles the unchanged original LM and CM
programs, verifies each rope against its pinned fixture, then runs8192 complete
source states. It links no native service. Every2048 image word and SUPER,
IRQ, channels11/12/13, write mask, restart lamp and route match strict native
C and the entire ASan/UBSan stream. Snapshot SHA256:
`d424442854c24506199defcba238753e5810476a03c4b3347ca9e491c56daf01`.
The corpus spans all entry and component selections, packed-field values,
retained banks/latches, raw register classes, signed zeros and unused fields;
it samples combinations, rather than certifying their Cartesian product.
Frozen64 whole source states and16 atomic guard cases pass host tests.
Separate native LM/CM C64 diagnostics run32 whole frozen states each with
actual PAL VICE mailbox, no extra zero-page storage, below-C000 image budget,
and hardware/software stack sentinels. Linked diagnostics are component proofs,
not a production disk-overlay integration certificate.

`tools/dsky_input_select.mk` provides host-only all/test/sanitize aliases,
explicit c64 and fresh differential/freeze targets. The immutable packet
retains source/dependency identities, original fixture audits and listings,
full compressed source/native/sanitizer streams, host logs, PRG/maps and raw
mailbox/stack evidence. Input constant-table loading, data stores, selected
input scale bodies and application caller execution remain separate work.
