# V06N60 data preparation

`p66_nv_data_prepare` follows actual NVSUB03604 through ENTPAS0, original
noun60 table reload, MIXNOUN and DECDSP to DSPDCPUT02537. It retains fresh
canonical physical VHORIZ, HDOTDISP and HCALC pairs for subsequent READLO,
loads the original minor-word mixed data and complements XREG as source.
The full image, edited registers, SUPER and IRQ are authoritative. This
fixed service does not decode arbitrary nouns or verbs.

Independent original source proof compares4,096 complete2,050-word images,
SHA256 `a6f0742ccbc774a0c5eb3e66b62289b6832efcbdfd1c3f6424aeffe53bfd0089`.
Strict tests cover64 literal original image deltas and10 unchanged invalid
boundaries; all source inputs pass ASAN/UBSAN. C64 PAL VICE passes64 complete
images, mailbox `a500004000`, PRG12,274 bytes, no extra zero page, hardware
stack29 bytes plus14 ROM and software stack43 bytes. Recipes and original
comparison: `tools/nv_rows.mk`, `tools/nv_rows_differential.py`.

Entry is NVSUB03604, bank41/EB0/SUPER100 after the noun update. Exit is
DSPDCPUT02537, bank41/EB4/SUPER100, DECOUNT2, A0/L03025/Q02537. Numeric
scaling, decimal rendering and the actual NVSUB return are subsequent stages.
