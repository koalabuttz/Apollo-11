# Native machine-address display caller

`dsky_machine_cadr` and `cm_machine_cadr` execute actual INTMCTBS41,2116
and INTMCTBS+2 at2120 through actual VERBFAN41,2133 or source-selected
CCSHOLE. The full entry stores MPAC+2 in NOUNCADR, selects its source EB,
derives NOUNADD and preserves SETNCADR return debris. Both entries perform
the original VB5 signed-zero test. VB5 bypasses display and accepts unused
clean positive-zero relay words; other verbs display the machine CADR via
the unchanged five-character octal service and execute the actual following
TC VERBFAN. The body makes no scheduler check or callback.

Inputs accept canonical ordinary image words, raw16 A/L/Q, FB062000 with
any aligned retained EB, SUPER100/120/140/160 and IRQ0/1. MPAC+2/NOUNCADR
and VERBREG accept full ordinary words, including both zeros and sign flags.
The +2 entry retains caller-owned NOUNCADR/NOUNADD state. Invalid inputs
leave image, external I/O and route unchanged. Restoring producer exclusion
belongs to the caller throughout native composition. A selected clean+0
relay word retains the actual exceptional formatter state at LMCCSHOLE05705
or CMCCSHOLE05747 with inhibited IRQ, rather than advancing into VERBFAN.
Actual verb bodies and exceptional recovery are separate continuations.

8192 complete2051-field independently executed original/native/fullASan+
UBSan checkpoints match all2048 words plus SUPER, IRQ and actual route.
Full SHA256:
`95854b1c0d3949d984ca34647dc6c27f287ab6b002b3fab2c6139d553df93505`.
Both original missions rebuild with seven-file source audits and byte-exact
pinned rope equality. The unchanged source oracle links no native body.
The corpus covers both entry addresses, VB5 bypass, ordinary verb sign/zero
boundaries, new/source-retained EB, all selecting SUPER forms, raw CPU debris,
NOUT edges and selected/unused clean+0 relay words.

Sixty-four frozen complete checkpoints pass strict host, sanitizers and
native stock-ROM C64 VICE, with both entries, bypass/display and actual
exceptional/normal routes in both missions. Mailbox a500004002;
PRG9307 loaded9305 bytes; hardware/software stack extents20/52.
The diagnostic owns two canonical images and frozen expected changes;
production owns one source image. tools/dsky_machine_cadr.mk supplies
portable host aliases and an explicit target. Existing formatter bytes and
all previously published proof identities remain unchanged.
