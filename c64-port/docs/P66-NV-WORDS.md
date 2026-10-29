# V06N60 verb and noun display

`p66_nv_verb` and `p66_nv_noun` translate original UPDATVB/UPDATNN through
their actual NVSUB caller returns03602/03604. They compose source DSPDECVN
with its real POSTJUMP and return effects, and load literal original noun60
addresses and scale metadata. Noun60 reads VHORIZ, HDOTDISP and HCALC;
HCALC1 belongs to noun63. The canonical image is authoritative.

`tools/nv_words_differential.py` compares8,192 original complete2,050-word
states, SHA256 `29c140f45d7a4d5ff1c2d3f2b18759bdc6e584c1370d196e9b6f071af3b4ece9`.
Strict tests cover64 frozen images and20 unchanged rejected boundaries.
All source states pass ASAN/UBSAN. C64 PAL VICE passes64 states, mailbox
`a500004000`, PRG11,091 bytes, no extra zero page, hardware stack33 bytes
plus14 ROM and software stack47 bytes. Recipes: `tools/nv_words.mk`.
The subsequent three numeric rows and MAKEPLAY job lifecycle are separate
services. No general noun/verb instruction decoding is introduced.
