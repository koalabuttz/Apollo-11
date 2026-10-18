# Running the flight program on a C64

This is how to boot the Apollo 11 flight program (the LM's Luminary 99 or
the CM's Comanche 55) on a stock Commodore 64 from disk, with nothing
prepared on a PC except the disk images. Everything here was checked in
VICE with true 1581 drive emulation (`make flight-disk-vice`), not on real
hardware yet.

## What you need

- A stock C64, PAL or NTSC (the build is for the stock 1 MHz C64 only).
- A 1581 disk drive as device 8, and 3.5-inch double-density disks: three
  for the LM, three for the CM. Another drive that takes 1581 images as
  device 8 (an SD2IEC, say) also works, but slowly: see
  [Without a 1581](#without-a-1581).
- A 2 MB RAM Expansion Unit (REU). Commodore's own REUs are smaller (1700:
  128 KB, 1764: 256 KB, 1750: 512 KB); a 1750 expanded to 2 MB or a modern
  2 MB REU works. The loader checks the size and stops if it is smaller.

## Making the disks

```sh
make flight-disk          # builds the flight store first (make flight-store)
```

This writes `build/flight-disk/lm-1.d81` to `lm-3.d81` and `cm-1.d81` to
`cm-3.d81`. Write each image to its own disk with any tool that writes
1581 images, and label them; the loader asks for them by number. Disk 1 of
each set also holds the loader, `APOLLO LM` or `APOLLO CM`.

| Set | Disks | Bytes to the REU | Loader | Resident |
|---|---|---|---|---|
| LM | 3 | 1,710,384 | 5,569 bytes | 36,231 bytes |
| CM | 3 | 1,781,342 | 5,569 bytes | 36,197 bytes |

What goes to the REU is exactly what `build/flight-store/<m>.reu` holds: the
module store (the flight modules, their index and resolver, the erasable
image's boundary words) and the rope image. The flight resident (the C64
program that runs the AGC) travels on the last disk.

## Loading

With the REU in and the 1581 on device 8, put in disk 1 and type:

```text
LOAD"APOLLO LM",8
RUN
```

(`LOAD"*",8` loads the same file: the loader is the first one on disk 1.)

The loader then:

1. checks the REU is 2 MB, and clears it (2.3 s);
2. reads disk 1 with its own fast loader: the screen goes blank and the
   border changes colour every 16 KB;
3. asks `INSERT DISK 2 OF 3 AND PRESS RETURN`, and so on for each disk. A
   wrong disk is asked for again; a read error asks for the same disk again;
4. prints `LOADED IN n S. STARTING THE AGC`, copies the flight resident
   from the REU over itself and starts it.

The flight resident then verifies the whole store in the REU (each module's
header and CRC, the resolver, the rope's site words) and starts the AGC as
switching it on would: the hardware restart (GOJAM) runs the rope's
power-on code, which ends in a fresh start (MODREG -0, no program). As on
the AGC after power-up, the RESTART lamp is then lit until the crew presses
RSET: that is not a fault.

### How long it takes (PAL, VICE true drive emulation)

| Step | LM | CM |
|---|---|---|
| `LOAD"APOLLO LM",8` (the KERNAL's own 1581 load, 6 KB) | about 10 s | about 10 s |
| Clearing the REU | 2.3 s | 2.3 s |
| Reading the disks (not counting the swaps) | 279 s (6.1 KB/s) | 290 s (6.1 KB/s) |
| Copying the resident over the loader | 0.08 s | 0.08 s |
| Verifying the store (rk_boot) | 84 s | 88 s |
| From `LOAD` to the AGC running, not counting the swaps | about 6.3 minutes | about 6.5 minutes |

For comparison, the stock KERNAL reads a 1581 at about 490 bytes a
second: the LM's 1.7 MB would take about 58 minutes.

The fast loader is a 1581 program (platform/c64/flight_disk_drive.s) that
reads each sector with the drive's own job queue and sends it two bits at a
time on the serial bus's CLK and DATA lines, one handshake a byte
(platform/c64/flight_disk_rx.s on the C64). It needs the screen blanked
while it runs (the VIC's bad lines would steal the C64's cycles mid-byte),
which is why the screen goes blank. About two thirds of the time is the
transfer and one third the drive reading its tracks.

## Without a 1581

The loader asks the drive on device 8 what it is (it reads the 1581 DOS
ROM's version string with `M-R`). Anything else, such as an SD2IEC, or
an emulated drive that does not run the 1581's own 6502 code, gets the
KERNAL instead: the loader prints `NO 1581 ON DEVICE 8: READING WITH THE
KERNAL (SLOW)` and reads the same files by name (`LM DISK 1`, `LM DATA 1`
and so on). Everything after that is the same. In VICE (its drive
emulation off, the IEC device emulation on: `tools/flight_disk.py --vice
--slow`) the KERNAL path read the LM's disks in 2,429 s (about 40
minutes, 700 bytes a second) and the AGC then started as above. An SD2IEC
has not been tried. `make flight-disk-vice` runs
only the fast path.

## The DSKY

Once the AGC runs, the screen is its DSKY. Here is the CM in VICE after the
boot and `V37` ENTR `00` ENTR, with PROG and RESTART lit (yellow on the
screen):

```text
 APOLLO 11 CM DSKY

 UPLINK ACTY  TEMP         COMP ACTY
 NO ATT       GIMBAL LOCK  PROG
 KEY REL      RESTART      TRACKER
 OPR ERR      ALT          VEL

        VERB    NOUN    PROG
                         00

            R1
            R2
            R3

 V VERB  N NOUN  RETURN ENTR  C CLR
 R RSET  K KEY REL  P PRO  0-9 + -
```

The digits are what the AGC sent to the DSKY's
relays (channel 10, through T4RUPT), so they change as the real ones did,
a row at a time; the lamps are channel 11's and row 12's, and RESTART is
the restart flip-flop. VERB and NOUN flash when the AGC asks, as do KEY REL
and OPR ERR. PRIO DISP, NO DAP and STBY are not shown.

The keys:

| C64 key | DSKY key |
|---|---|
| `0`-`9`, `+`, `-` | the same |
| `V` | VERB |
| `N` | NOUN |
| RETURN | ENTR |
| `C` or DEL | CLR |
| `R` | RSET |
| `K` | KEY REL |
| `P` | PRO (held down for 1 s of AGC time) |

So `V37` RETURN `00` RETURN selects P00, and `V35` RETURN runs the lights
test. One key at a time: the C64 sees a key 25 times a second and passes
it on when the AGC is next idle.

The DSKY costs the flight resident no memory and the AGC no time: its code
runs from $0200-$03FF and its tables sit in the screen's last eight rows,
in the background colour; it draws from the AGC's idle loop, only what
changed. Under the work clock the AGC's timing does not depend on how fast
the C64 is, so drawing changes only how fast the C64 runs, not what the AGC
does.

RSET turns RESTART off, as the AGC's hardware does. If the AGC reaches a
point the C64 port does not cover yet, it stops there and the display stays
as it was.

## The C64 Ultimate (unverified)

None of this has been tried on a C64 Ultimate. What would differ:

- Its 1581 emulation would have to run the drive's own 6502 code at the
  1581's speed for the fast loader to work, as VICE's true drive emulation
  does. If it answers the `M-R` check like a 1581 but does not run drive
  code at the 1581's speed, the fast loader would hang; if it does not
  answer like a 1581, the loader takes the slow KERNAL path above.
- Its REU emulation can be set to 2 MB or more, which the loader accepts.
- At turbo speeds the C64 side of the fast loader is timed in C64 cycles,
  so it would read the bus too early: the load has to run at 1 MHz.
