# Running the flight program on a C64

This is how to boot the Apollo 11 flight program (the LM's Luminary 99 or
the CM's Comanche 55) on a stock Commodore 64 from disk, with nothing
prepared on a PC except the disk images. Everything here was checked in
VICE with true 1581 and 1541 drive emulation (`make flight-disk-vice`),
not on real hardware yet.

## What you need

- A stock C64, PAL or NTSC (the build is for the stock 1 MHz C64 only).
- A 1581 disk drive as device 8 and one 3.5-inch double-density disk a
  mission. A 1541 works too, with 5 5.25-inch disks a mission, but
  slowly, as does any other drive device 8 that takes the images (an
  SD2IEC, say): see [Without a 1581](#without-a-1581).
- A 2 MB RAM Expansion Unit (REU). Commodore's own REUs are smaller (1700:
  128 KB, 1764: 256 KB, 1750: 512 KB); a 1750 expanded to 2 MB or a modern
  2 MB REU works. The loader checks the size and stops if it is smaller.

## Making the disks

```sh
make flight-disk          # builds the flight store first (make flight-store)
```

This writes `build/flight-disk/lm-1.d81` and `cm-1.d81` (and `test-1.d81`, the
[hardware check](#check-on-real-hardware)). Write each
image to a disk with any tool that writes 1581 images. Disk 1 of each set
also holds the loader, `APOLLO LM` or `APOLLO CM`. For a 1541,
`tools/flight_disk.py --drive 1541` writes `lm-1.d64` to `lm-5.d64` and
`cm-1.d64` to `cm-5.d64` instead: label them, the loader asks for them by
number.

| Set | 1581 disks | 1541 disks | Bytes to the REU | Loader | Resident |
|---|---|---|---|---|---|
| LM | 1 | 5 | 1,902,053 | 6,392 bytes | 37,701 bytes |
| CM | 1 | 5 | 1,837,143 | 6,392 bytes | 37,659 bytes |

The data on the disks is compressed (tools/fdz.c, an LZ77 coder written
for this loader; the LM's 1.84 MB of store and rope take 757 KB), and the
loader expands it as it reads (platform/c64/flight_disk_unz.s), through a
32 KB buffer at $4000-$BFFF on its way to the REU. The loader runs before
the flight resident, so this costs the resident nothing.

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
3. with a set of more than one disk (the 1541's), asks `INSERT DISK 2 OF
   5 AND PRESS RETURN`, and so on for each disk. A wrong disk is asked for
   again; a read error asks for the same disk again;
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
| Reading and expanding the disk | 262 s (7.3 KB/s) | 261 s (7.0 KB/s) |
| Copying the resident over the loader | 0.08 s | 0.08 s |
| Verifying the store (rk_boot) | 94 s | 91 s |
| From `LOAD` to the AGC running | about 6.1 minutes | about 6.1 minutes |

Before the compression the same load took 279 s (LM) and 290 s (CM) from
3 disks each. For comparison, the stock KERNAL reads a 1581 at about 490
bytes a second: the LM's 1.84 MB, uncompressed, would take about an
hour.

The fast loader is a 1581 program (platform/c64/flight_disk_drive.s) that
reads each sector with the drive's own job queue and sends it two bits at a
time on the serial bus's CLK and DATA lines, one handshake a byte
(platform/c64/flight_disk_rx.s on the C64). It needs the screen blanked
while it runs (the VIC's bad lines would steal the C64's cycles mid-byte),
which is why the screen goes blank. The drive reads the next sector while
the C64 expands the last one.

## Without a 1581

The loader asks the drive on device 8 what it is (it reads the 1581 DOS
ROM's version string with `M-R`). Anything else, such as a 1541, an
SD2IEC, or an emulated drive that does not run the 1581's own 6502 code,
gets the KERNAL instead: the loader prints `NO 1581 ON DEVICE 8: READING
WITH THE KERNAL (SLOW)` and reads the same files by name (`LM DISK 1`,
`LM DATA 1` and so on). Everything after that is the same. Times in VICE,
reading the disks:

| Drive | LM | CM |
|---|---|---|
| 1541, true drive emulation (`tools/flight_disk.py --vice --drive 1541`) | 2,161 s (about 36 minutes) | 2,161 s (about 36 minutes) |
| 1581 images, the IEC device emulation (`--vice --slow`) | 1,262 s (about 21 minutes) | not measured |

Before the compression the `--slow` path took 2,429 s (about 40 minutes)
for the LM, and there were no 1541 disks. The 1541's KERNAL load moves
about 350 bytes a second, so most of its time is the transfer. An SD2IEC
has not been tried. `make flight-disk-vice` runs only the 1581's fast
path.

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

To try the keys in VICE without a C64: `tools/flight_disk.py --missions lm
--vice --keys V37E00E --screenshot` types them into the DSKY's key queue
through VICE's monitor. With `--xtest`, run under `xvfb-run -a -s
"-screen 0 1024x768x24"` (on Xvfb's default 8-bit screen VICE's window
gets no keys), it presses them on VICE's window instead, so they go through the C64's own keyboard
scan as typed keys would: `R`, then `V37E00E` on the LM clears RESTART and
PROG and reaches P00, and `V35E` on the CM runs the lights test.

CI boots both mission disks this way in `make flight-disk-vice`
(`tools/flight_disk.py --vice --p00`): after the boot it types `R` and
`V37E00E` into the key queue and fails unless the AGC is in P00 (MODREG 0,
`00` under PROG on the screen). Each run writes
`build/flight-disk/boot-lm.json` and `boot-cm.json`: the commit, the
SHA-256 of the disks, the loader and resident PRGs, the REU store and the
VICE log, and the screen's text. CI keeps them with its logs.

The DSKY costs the flight resident no memory and the AGC no time: its code
runs from $0200-$03FF and its tables sit in the screen's last eight rows,
in the background colour; it draws from the AGC's idle loop, only what
changed. Under the work clock the AGC's timing does not depend on how fast
the C64 is, so drawing changes only how fast the C64 runs, not what the AGC
does.

RSET turns RESTART off, as the AGC's hardware does. If the AGC reaches a
point the C64 port does not cover yet, it stops there and the display stays
as it was.

## Check on real hardware

The test disk checks a real C64 and REU against VICE in about 7 minutes,
with one screen to photograph at the end. `make flight-disk` writes it
as `build/flight-disk/test-1.d81` beside the mission disks. Without a
build, take the images CI booted: each port-ci run that runs
`flight-disk-vice` keeps `test-1.d81`, `lm-1.d81` and `cm-1.d81` in its
`c64…-logs` artifact (on the run's GitHub Actions page), once they pass
in VICE. With the REU in and the 1581 on device 8:

```text
LOAD"APOLLO TEST",8
RUN
```

It loads like the LM disk (the same 1.9 MB store, about 4.4 minutes on a
1581) and verifies the store. Then, at the stock 1 MHz:

1. it measures the video standard (from the raster line count), the
   REU's size (up to "2048 KB OR MORE"), and the REU's DMA speed both
   ways (32 KB each way);
2. it runs one fixed piece of flight: the LM's P63 powered descent for
   200 cs (2 s of AGC time) from a snapshot of the host run at 17,513 cs,
   with the host run's inputs (engine, PIPAs, keys) fed at the same AGC
   times;
3. it prints the C64's time for that window, its speed against real
   time, and a CRC of the end state (the erasable words, timers and
   centiseconds that `tools/flight_scenario_vice.py --replay` compares).

The screen in VICE (`tools/flight_disk.py --missions test --vice`; CI
runs it in `make flight-disk-vice`):

```text
APOLLO 11 ON THE C64: HARDWARE CHECK

VIDEO: PAL, 985248 HZ
REU: 2048 KB OR MORE
REU DMA, BYTES/S: C64>REU 923472
                 REU>C64 928145

RUNNING LM P63 FROM 17532 CS...

AGC TIME: 200 CS IN 1218 STEPS
C64 TIME: 43.50 S, 21.75 X REAL TIME

END STATE CRC C4D5 (VICE C4D5)
HOST RUN'S CRC C4D5 (THE SAME)

PASS: THIS C64 COMPUTES WHAT VICE DOES
```

The CRCs, step count and times above are from one build. The CRC VICE gives
is baked into the disk when it is made, so it moves with every store build;
the latest recorded screen is in
[acceptance/Luminary099-bootable_c64_application.md](acceptance/Luminary099-bootable_c64_application.md).

**PASS** means this C64 ended the window in exactly the state VICE's run of
the same program did: the CRC VICE gave is built into the disk when it is
made. FAIL means it differs, and is worth a photograph. The AGC's timing
does not depend on the C64's speed (the work clock), so a faster or slower
machine still ends in the same state; only "C64 time" and "x real time"
change. The times are counted in C64 cycles by the CIA timers, divided by
the PAL or NTSC clock.

The host run's CRC is shown for reference. Since base 08c07360 (the screen
above) the window ends in the host's state too, so the line reads "THE
SAME". If a later build shows "NOT YET THE SAME", a timing change moved the
C64 off the host in this window (a row for the shared replay mismatch list),
not a fault of the C64; PASS or FAIL still says whether this C64 computes
what VICE does.

The test disk only checks: when the report is up, the program stops there.
Switch off or reset to load a mission disk.

From a real machine, the useful numbers are the whole screen: the video
line, the REU size and both DMA speeds, the C64 time and x real time, the
CRC and PASS or FAIL. A photograph of the screen carries all of them.

## The C64 Ultimate (unverified)

None of this has been tried on a C64 Ultimate. The test disk above is
the quickest first try: it shows the REU's size and DMA speed, and PASS
or FAIL against VICE. What would differ:

- Its 1581 emulation would have to run the drive's own 6502 code at the
  1581's speed for the fast loader to work, as VICE's true drive emulation
  does. If it answers the `M-R` check like a 1581 but does not run drive
  code at the 1581's speed, the fast loader would hang; if it does not
  answer like a 1581, the loader takes the slow KERNAL path above.
- Its REU emulation can be set to 2 MB or more, which the loader accepts.
- At turbo speeds the C64 side of the fast loader is timed in C64 cycles,
  so it would read the bus too early: the load has to run at 1 MHz.
