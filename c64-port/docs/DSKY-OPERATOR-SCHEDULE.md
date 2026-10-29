# Native operator scheduler admission and wait requests

`dsky_operator_schedule` and `cm_operator_schedule` execute actual Verb30
VBRQEXEC41,3466 or Verb31 VBRQWAIT41,3512 through real RELDSP/JOBWAKE,
original REQUESTC, NOVAC/FINDVAC or WAITLIST, and literal TC ENDOFJOB at
MPAC+3. Successful calls reach actual LM05155/CM05217 with Q0160.
Resource exhaustion and full/bad wait requests reach the actual source
BAILOUT/POODOO entry with all reservations, partial queue changes and raw
register/scratch effects preserved. Recovery and Executive completion are
separately compiled continuations; this module never invents their return.

The original erasable caller is Q0155. Source MPAC+1/+2 hold the actual
inline pair and MPAC+3 the literal TC ENDOFJOB word. This qualification
independently executes original scheduler inline rereads and scratch aliases
at those addresses rather than assuming an earlier fixed-caller proof.
The caller supplies an explicit compiled request witness equal to noun26
components2/3; native admission records that pair without fetching or
executing an AGC rope instruction. Source ownership remains excluded
through the complete body, with no extra native scheduler/IRQ poll.

The request/wake domain is the released operator-request domain. Executive
requests additionally take packed nonzero priority bits10..14 (bit1 selects
allocation), positive active core0, canonical other nonzero core priorities,
NEWJOB displacements and zero/canonical VAC markers. Actual resource
exhaustion is supported. Zero or malformed priorities, source cleanzero
Executive core markers and noncanonical resource markers remain outside
these immutable scheduler helpers; the broader request-stage API preserves
such actual pending service arguments without a fabricated admission.

Wait requests accept ordinary zero/negative bad-delay arguments or positive
1..16250, canonical task location/bank, and TIME3 within0..037777. The latter
is an existing LM helper qualification even on bad-delay paths where source
TIME3 is unused. Source queue insertion/full/displacement behavior is retained.
CM pending native alarms are checked only by the source-selected Executive,
positive-delay WAITLIST, or nonempty display-wake path. Unsupported inputs
preserve image, external metadata, channel event and route before mutation.

8192 complete2053-field original/native/fullASan+UBSan states match all2048
words, SUPER, IRQ, channel11, its actual event and route. The corpus contains
6242 END returns,864 admission-alarm entries,482 full-queue entries and604
bad-delay entries, plus matching/notfound/no-list display wakes. Both missions
independently assemble nine audited unchanged files including WAITLIST into
byte-exact pinned ropes. The original oracle links no native body.
Full SHA256:
`c207027e5e7a82f8c87b79ceabe497e851ac7c62e5810015855b9ad39daede8d`.

All64 frozen complete checkpoints pass strict host and sanitizers. C64 uses
separate LM32 and CM32 diagnostic PRGs so the two native scheduler kernels
and complete comparison images fit below C000; production logic is unchanged.
LM mailbox a500002000, PRG26594/loaded26592 bytes, hardware/software stack
extents16/1385. CM mailbox a500002001, PRG27086/loaded27084 bytes, stack
extents18/1346. Both stock-ROM PAL VICE checks compare every source state
word and report zero failures. Eight atomic caller-domain checks pass host.
The first target runs found16-bit overflow in the diagnostic case generator's
nonpower-of-two delay/interval products; explicit uint32_t factory arithmetic
now produces identical inputs across host and C64. The original/native/SAN
full-stream identity stayed unchanged. Stable scheduler source was untouched.

Portable host and explicit dual-mission C64 aliases are in
tools/dsky_operator_schedule.mk; tools/dsky_operator_schedule_vice_test.py
requires --mission lm or --mission cm. Comparison images/frozen vectors and
the first-difference diagnostic record are test storage. Production owns one
canonical source image and uses only transient imported scheduler views.
