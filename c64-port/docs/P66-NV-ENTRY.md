# Fixed V06N60 display entry

`p66_nv_enter` translates actual NVMONOPT entry04155 with A01474, L0 and
Q03121. It tests the keyboard interlock and external monitor bit, either
returns to the real busy caller03121 or saves the actual caller/bank,
terminates the prior monitor and enters UPDATVB-1 at02337 for V06N60.
Canonical erasable memory and SUPER/IRQ remain authoritative. The header
states ordinary interlock count and supported original control boundary;
unsupported inputs are unchanged. This is a fixed compiled source service,
not a general noun/verb decoder.

`tools/nv_entry_differential.py` compares8,192 original full2,050-word source
images, SHA256 `188f356b2c434205fc424a1eca0b54097d83576087b8209ea8f253276155c8fe`.
All raw registers, scratch, edited EDOP, SUPER and IRQ are compared. The
modern fixture renames only the old DSPOCTWO symbol to DSPOCTWD; instruction
bodies are unchanged. Strict tests cover64 frozen complete images and13
invalid boundaries; all source cases pass ASAN/UBSAN. C64 PAL VICE passes64
states, mailbox `a500004000`, PRG4,367 bytes, no extra zero page, hardware
stack14 bytes plus14 ROM and software stack12 bytes. Recipes:
`tools/nv_entry.mk`. MAKEPLAY/GOPLAY admission and subsequent noun data
rendering are separate source services.
