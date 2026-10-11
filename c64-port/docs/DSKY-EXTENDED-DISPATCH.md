# Native extended-verb transfer and fanout

`dsky_extended_dispatch` and `cm_extended_dispatch` execute actual VERBFAN
return41,2143 through POSTJUMP and GOEXTVB43,2000, or start directly at
GOEXTVB. They execute the indexed LST2FAN branch to each mission's actual
selected verb-body entry. All60 source branches compile as explicit native C
cases with source address constants; no executable AGC words are retained,
fetched or decoded at runtime. The offline authoring audit records original
source labels, TC/TCF distinctions and addresses against unchanged assembly.

Both missions use TCF for verb49; Comanche additionally uses TCF for verb96.
Those paths retain original Q02002. Other branches produce Q02003+index.
Verb69 reaches the actual self-loop body with source Q02040, rather than
pretending the hardware restart has completed. Raw A survives POSTJUMP/fanout,
L is corrected as in original instructions, and actual BB/FB/IRQ/SUPER effects
remain in the canonical image and explicit I/O.

Inputs are sourcephysicalbank41 for2143 or43 for2000, all EB,
SUPER100/120/140/160, IRQ0/1 and MPAC0..59 or negative zero. Unsupported
image/I/O remains unchanged. Caller holds restoring producer exclusion.

8192 complete2050-field original/native/fullASan/UBSan checkpoints independently
verify all60 branches, both entries, both TC/TCF forms, negative-zero indexing,
all EB/SUPER and raw A/L/Q edges. Full SHA256:
`1cb51342a5c0ab17f9d491f39c37eadad4e02d27a37d8594fb7237e2b02f0dd7`.
The oracle stops after the actual selected source TC/TCF instruction, based
on source control progression, without importing the native target map.
Rebuilt unchanged original extended verbs, interbank controls and seven
Pinball-related source files must produce complete pinned ropes unchanged.

Sixty-four frozen complete states pass strict host, sanitizers and actual
stock-ROM C64 VICE. Mailbox a500004002, PRG3705 loaded3703 bytes, hardware/
software stack extents10/3. tools/dsky_extended_dispatch.mk provides portable
host aliases and an explicit C64 target. Actual selected verb bodies remain
separate continuations; no whole extended-verb/program completion is claimed.
