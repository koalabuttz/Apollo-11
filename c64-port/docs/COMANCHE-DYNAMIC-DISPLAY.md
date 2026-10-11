# Actual Comanche dynamic display composition

`cm_dynamic_display_step` composes the already qualified numerical DYNDISP
caller with the normal V06N85 source display consumer. The actual path starts
at DYNDISP24,3325/E6, computes VGBODY through P40CNV85/S41.1 and live CDU
trig, two source matrix products and *SMNB*, enters CKNVWRD1, executes the
real display admission/priority/wake bodies, displays all three fresh VGBODY
pairs and reaches real ENDOFJOB and idle ADVAN03231. Native routes0..26
invoke `cm_dyndisp_step`; routes27..52 invoke `cm_nv85_step`. No qualified
helper or fixed source fixture is modified by the composition.

The producer owns the sole canonical image and external metadata. Every
linked route returns before any overlay replacement. The domain is an
actual admitted normal job with one allocated VAC0401/0455/0531/0605/0661,
NVWORD1=01525, NEWJOB+0, no display lock or external monitor, and six
negative-zero unused core priorities. Initial Pinball dirty blank relay
words and NOUT11 follow STARTSUB; unrelated caller state is retained.
Source-interleaved scheduler requests and other selected job continuations
need a separate resident binding qualification. The composition does not
replace them with a fictitious basic poll.

Sixty-four continuous original source paths cover all five allocated VACs,
all four source SUPER variants and both initial IRQ states. Every one of
53 boundaries matches all2048 erasable words plus SUPER, IRQ and channel11
without normalization: 3,392 complete states. ASAN/UBSAN matches the whole
source stream. SHA256 is
`6b4ea94180b8d18bde051667536b49126e181c83e22d2131308359b4be15ae92`.
The ordinary regression also checks actual VAC release, negative-zero core
release, normal verb/noun and source idle banks/IRQ, plus unchanged invalid
route handling. The unchanged numerical and display packets retain their
independent target qualifications; this new uninterrupted composition is
currently a host source proof, not a whole resident C64 scheduling proof.
Device relay draining and screen rendering remain separately qualified
native device services.

Use `tools/cm_dynamic_display_differential.py` and
`tools/cm_dynamic_display_sanitize_stream.py` for the complete compressed
source stream. `tools/cm_dynamic_display.mk` provides linked host and
sanitizer regressions. The evidence manifest records exact dependency and
artifact identities; the original pinned rope is the powered fixture
`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
