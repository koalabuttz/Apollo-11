# SERVICER in two flight modules

Pinball module split lane, for the steward's flight-store overflow list
(`/mnt/project-files/pin-split/store-overflow.md`).

With interpretive code left to the native interpreter
([FLIGHT-INTERP-STOP.md](FLIGHT-INTERP-STOP.md)), SERVICER's flight module
holds only its basic code, and it was still 5,642 bytes over the 16 KB job
slot. About 8 KB of that is the basic code itself; the rest (the transfer
engine, the image-op helpers, the sites and the io record) every module
links. So the file links as two modules, each with half the basic code.

`src/lm_servicer.c` with `-DAGC_FLIGHT_INTERPRETER -DLMSV_FLIGHT_PART=1`
or `=2`:

- Part 1 holds the basic words below `LMSV_PART_CUT` (bank 33 `03420`):
  banks 21, 22, 23, 27 and 32 and bank 33 up to `03417`, with the first 85
  basic keys.
- Part 2 holds the rest of bank 33 and banks 34 and 37, with the other 80
  keys.
- `lm_servicer_routines` is the part's slice of the sorted `basic_entries`;
  the interpretive table is empty.
- A run that reaches a word of the other part (a goto, a fall-through, a
  `TC Q` return) stops there with Z at that word when the word is a basic
  key, so the other part's module takes over through the transfer engine.
  At any other word it is refused. The other part's code is dead and the
  compiler drops it.

Without `LMSV_FLIGHT_PART` the file preprocesses to the same text as
before, in the host build and in every C64 segment group.

| Part | Keys | Flight link (LM, bytes) |
|---|---|---|
| 1 | 85 | 14,581 |
| 2 | 80 | 14,302 |

The sizes come from `tools/flight_store_build.py`'s `link_module`, with the
part's flags added to the common flags. The flight store needs to link the
file twice with those flags, as two modules.

## Where the cut is safe

Every transfer in the basic code that crosses the cut was checked against
the file's keys. That covers each `TC` or `TCF` target, each `TC`'s return
word, each fall-through, and each `to()` target: all of them are keys. So a
flight run that goes from one part to the other always lands on a key.

## Evidence

`tools/lm_servicer_flight_parts.py` (run by `make flight-interp-check`)
builds each part's host snapshot. It also runs the oracle
(`tools/lm_servicer_reference.c`) with a segment file: the part's basic key
slots, a stop at every basic word of the other part, and a stop at every
order pair. Every case must equal the original program stopped there.

| Part | Cases | Equal | Refused unchanged | Refused at a non-key word across the cut |
|---|---|---|---|---|
| 1 | 10,880 | 10,752 | 128 | 8 |
| 2 | 10,241 | 9,970 | 271 | 19 |

The 27 refusals at the cut all come from a `TC Q`, `TC A` or `DTCB` whose
target the case drew at random, landing mid-routine in the other part.
No transfer in the file's own code does that, as the check above shows.
