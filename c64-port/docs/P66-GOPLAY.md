# Normal V06N60 GOPLAY consumer

`p66_goplay_enter` follows actual GOPLAY03070 through COPYPACS and NVDSP
stores to NVMONOPT04155. `p66_goplay_finish` follows actual NVSUB return03122
through FLASHOFF, COPYTOGO, three DOWNFLAG calls, zero BLANKSUB and the
nonflashing branch to original ENDOFJOB05155. Canonical memory, SUPER/IRQ and
an explicit channel11 word preserve source registers, scratch and flags.
The supported source normal display is COPINDEX2, flags023400 and V06N60;
unsupported boundaries are unchanged.

The complete GOPLAY consumer composes the verified NV entry, verb/noun,
data preparation and three-row scale/decimal services. An independent
original source proof compares18,432 full2,051-word checkpoints on1,024
inputs, SHA256 `aa9fae39cf9cd916ce565bfb73ad771b41a91606a335c87505258a79d66044e4`.
Separate entrance/finish proof compares8,192 full2,051-word original states,
SHA256 `e291c7b650412a3cf3e27f46a74315104fcba180dedba38c785dd8f97ced23f1`.
Both include all2,048 erasable words, raw A/L/Q, SUPER, IRQ and channel11.
Strict tests cover64 literal complete images and24 unchanged invalid inputs;
ASAN/UBSAN matches both entire source corpora. C64 PAL VICE passes64 original
boundary images, mailbox `a500004000`, PRG5,193 bytes, no extra zero page,
hardware stack14 bytes plus14 ROM and software stack15 bytes. Standalone
recipes: `tools/goplay.mk`; independent comparisons: `goplay_differential.py`
and `goplay_full_differential.py`.

This milestone reaches actual ENDOFJOB for an available nonflashing display.
MAKEPLAY priority change/admission and busy sleep/wake require their source
control services. Runtime job dispatch and ENDOFJOB completion remain owned
by the verified Executive rather than a synthesized callback or return.
