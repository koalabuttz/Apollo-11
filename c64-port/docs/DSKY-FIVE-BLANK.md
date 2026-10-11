# Native numeric component blanking

`dsky_five_blank` and `cm_five_blank` execute actual source 5BLANK,
5BLANK-1, 5BLANK+2 and CLR5 through ordinary caller or selected-clean+0
CCSHOLE. LM entries are40,2536/2535/2540/2525; CM entries40,2437/2436/
2441/2426. The body derives INREL for the selected numeric component,
zeros its source major/minor X/Y/Z register pair, clears the corresponding
decimal component flag and low two flags, blanks one isolated digit and
two digit pairs, and restores the component's leftmost DSPCOUNT. Original
CODE/COUNT/DSEXIT/DSREL/DSMAG/SR, raw CPU and NOUT dirty-state effects are
preserved. Existing native DSPIN/2BLANK bytes are unchanged.

The qualified calling domain is numeric R1/R2/R3: DSPCOUNT0..14, or ordinary
A0..14 at entry-1. Entry+2 instead uses INREL2..4 and saved L0100..7777;
other entries use ordinary Q0100..7777. Unused input fields and unused
positive-zero relays impose no guard. Canonical ordinary words, physical
bank40, any EB, SUPER100/120/140/160 and IRQ0/1 are accepted. Restoring
producer exclusion belongs to the caller. Invalid inputs preserve the full
image, external metadata and route. Numeric-domain scope does not certify
misuse on verb/noun display positions or out-of-table source INDEX inputs.

A selected isolated positive-zero relay executes the actual DSPIN prelude
and stops at LMCCSHOLE05705/CMCCSHOLE05747 with source Q03354/03261,
inhibited IRQ and all scratch retained. Paired positive-zero words execute
the original corrected 2BLANK path. Ordinary return is actual TC L with Q2;
no callback or scheduler check is inserted. Exceptional recovery remains
its separate source continuation.

8192 complete2051-field unchanged original/native/fullASan+UBSan states
match all2048 image words plus SUPER, IRQ and selected route. Full SHA256:
`e49fae93b230364bbdfbafdb459b0ed4140eb161f4ef800f43a5a9b7e65443b2`.
Both missions rebuild with seven-file original-source audits and byte-exact
pinned ropes; the original oracle links no native body. Corpus coverage
includes all four entries, every numeric position, each component, all EB,
selecting SUPER forms, both IRQ states, NOUT sign boundaries, raw L and
selected/unused clean positive-zero words.

Sixty-four frozen complete original checkpoints pass strict host, sanitizers
and native stock-ROM C64 VICE. Mailbox a500004002; PRG9771 loaded9769 bytes;
hardware/software stack extents16/42. The diagnostic includes two canonical
images and frozen expected changes; production owns one canonical image.
Portable host aliases and an explicit target recipe are provided in
tools/dsky_five_blank.mk. REQDAT and other callers are separate compositions.
