# Comanche diagnostic VAC5 snapshot body

`cm_vac5_store` executes the actual VAC5STOR (`20,2000`) body including its
GENTRAN caller. It saves all seven core LOC/BANKSET/priority triples, copies
the twelve phase CADR words in descending source transfer order, saves six
phase selectors, MPAC+3, NEWLOC, NEWJOB and NEWPRIO, and reaches the genuine
SWRETURN entry (`04675`) with source Q=`02064`.

The source writes the diagnostic data area starting at `0661`, after the
fifth VAC sentinel at `0660`. NEWJOB and NEWPRIO overwrite designated words
inside the phase-copy area exactly as in the original. The routine leaves
EBANK3 selected and interrupts inhibited by GENTRAN, with actual ITEMP1/2,
raw A/L and physical BBCON effects. Its diagnostic snapshot remains ordinary
canonical flight data; it creates no second authoritative image. Producer
exclusion is caller-owned. The subsequent actual SWRETURN/caller recovery
is a separate native source group.

The independent original-layout source rope has SHA256
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
All 8,192 randomized complete physical-image checkpoints, including raw A/L/Q
and external SUPERBNK/IRQ, match the original body, with lossless stream SHA256
`8c01b70eea25e4eef62bb960699b6febc19c76aac85660ca4a8e822dfda287f7`.
The corpus varies the whole ordinary erasable image, entry Ebank/SUPER/IRQ
and raw registers. Full strict and ASAN/UBSAN validation pass.

The C64 diagnostic loads native control module 215 and proof-only input/checker
254/255 through the checked D64 loader and replays 64 representative source
contexts with alternating incoming C64 interrupt state. Every complete state
is fingerprinted; two complete image/I/O samples and the final physical image
are compared directly. The retained report records actual outcome, payload
sizes and software stack extent.

Run `make -f Makefile -f tools/cm_vac5_store.mk test-cm-vac5-store` and
`sanitize-cm-vac5-store`. This packet qualifies the complete VAC5STOR body and
its actual GENTRAN use. It does not certify every GENTRAN caller, the whole
startup/restart file, or subsequent abort recovery.
