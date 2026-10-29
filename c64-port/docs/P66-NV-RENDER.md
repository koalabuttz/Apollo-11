# V06N60 numeric rows and NVSUB return

`p66_nv_scale_select` follows original DSPDCPUT through the literal noun60
scale tables and BANKJUMP to DP1OUTSF or DP3OUTSF. The canonical image keeps
the physical VHORIZ, HDOTDISP and HCALC pairs; `dsky_scale_output` performs
original fresh READLO, sign agreement, multiplication and rescaling.
`p66_nv_decimal_row` implements actual DSPDCEND BANKCALL, composes source
DSPDECWD and applies actual SWRETURN. `p66_nv_next_row` follows CCS/row-loop
or actual NVSUBEND/SUPDXCHZ to caller03122, restoring original SUPER/banks.
No general noun, verb or instruction decoder is introduced.

Independent original proof compares4,096 complete2,050-word scale-selection
states, SHA256 `e54ee64d7d42c186babf4293016fb2219be8ce12eac2fce82653ad0aca5be01d`.
A complete three-row source proof compares12,288 complete2,050-word images
(1,024 inputs, four boundaries per row), SHA256
`2ecaa7528c4f5ae8281952543f146c7390eb9a281d610fbdef49fbc25f819a58`.
This includes original BANKCALL/SWRETURN registers and scratch, all numeric
row debris, loop state and actual NVSUB caller return. Strict tests cover64
literal source entry/exit deltas and30 unchanged rejected inputs; ASAN/UBSAN
matches the entire full-row source corpus.

Four C64 PAL VICE images each pass16 complete original boundary images:
mailboxes `a500001000` through `a500001003`, PRG8,843/11,922/12,296/9,067
bytes, no extra zero page, hardware stack29 bytes plus14 ROM and software
stack45/153/60/44 bytes. `tools/nv_render.mk` supplies standalone recipes;
`tools/nv_render_differential.py` and `nv_render_full_differential.py` run
independent original assembly. Parent integration composes these source
stages with the producer exclusion and actual Executive callback lifecycle.

The service headers define source boundary admission. DSPTAB words must be
nonzero for the verified Pinball formatter. The canonical image remains the
only authoritative numerical and basic control state across calls. The
MAKEPLAY admission, PRIOCHNG, sleep/wake paths and GOPLAY epilogue are separate
control services; completing these row stages alone does not complete that
job lifecycle.
