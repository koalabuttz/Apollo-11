#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Build a mission's flight disks (docs/RUN-ON-C64.md): 1581 (or 1541)
disk images that boot the flight store on a stock C64 with a 2 MB REU,
with no VICE snapshot and no REU image prepared on the host.

For each mission this links the flight resident on the flight store
(platform/c64/resident_flight_main.c, as tools/dap_pass_vice.py links its
check) and writes

  build/flight-disk/<m>-1.d81 ... <m>-<n>.d81   the disk set (.d64, --drive 1541)
  build/flight-disk/<m>-loader.prg              the loader (also on disk 1)
  build/flight-disk/<m>-resident.prg            the resident it starts

Disk 1's first file is the loader (platform/c64/flight_disk_loader.c). It
reads every disk of the set into the REU, with a 1581 fast loader
(flight_disk_drive.s, flight_disk_rx.s) or the KERNAL on any other drive:
the store image and the rope exactly as build/flight-store/<m>.reu holds
them, and the resident, staged in REU bytes the store leaves zero and
zeroed again after the hand-off. Each disk's data is compressed
(tools/fdz.c, expanded by platform/c64/flight_disk_unz.s), so a
mission takes 1 D81 or 5 D64s.

--vice boots the set in VICE as a stock PAL C64 would: the drive with true
drive emulation, a 2 MB REU starting zero, LOAD"*",8,1 and RUN typed at
the READY prompt, the next disk attached at each prompt. It times the load,
the hand-off and the resident's boot (its store check), checks the boot
passed and the AGC then runs (TIME1 counting), compares the REU VICE writes
back on exit with <m>.reu, and writes build/flight-disk/vice.json. Needs
make flight-store first.

  flight_disk.py [--missions lm,cm] [--drive 1581|1541] [--vice]
"""
import argparse
import hashlib
import json
import os
import pathlib
import re
import shutil
import socket
import struct
import subprocess
import sys
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from resident_flight_build import COMMON, FLIGHT_MAIN, MISSION, resident_sources  # noqa: E402

STORE = ROOT / 'build/flight-store'
OUT = ROOT / 'build/flight-disk'
PAL_HZ = 985248
TITLE = {'lm': 'APOLLO 11 LM: LUMINARY 99', 'cm': 'APOLLO 11 CM: COMANCHE 55',
         'test': 'APOLLO 11 HARDWARE CHECK: LM P63'}
# The hardware check set, "test" (docs/RUN-ON-C64.md, Checking real
# hardware): this LM window (scenario, AGC centisecond, length) as
# tools/flight_scenario_vice.py --replay runs it, on the LM store.
CHECK = ('lm-p63-descent-thrust', 17500, 200)
# 1581: 80 tracks of 40 sectors; track 40 holds the header, the BAM and the
# directory.
TRACKS, SECTORS, DIR_TRACK = 80, 40, 40
# Zero runs at least this long are left out of the disks (the loader clears
# the REU first).
GAP = 512


def run(cmd, **kw):
    return subprocess.run([str(c) for c in cmd], cwd=ROOT, check=True, **kw)


def tool(cc, name):
    return pathlib.Path(cc).parent / name


def drive_code(cc):
    """flight_disk_drive.s linked at $0600: the bytes and the two symbols."""
    obj, elf, bin_ = OUT / 'drive.o', OUT / 'drive.elf', OUT / 'drive.bin'
    ld = OUT / 'drive.ld'
    ld.write_text('SECTIONS { . = 0x0600; .text : { *(.text) } }\n')
    run([tool(cc, 'llvm-mc'), '-triple', 'mos', '-mcpu=mos6502', '-filetype=obj',
         'platform/c64/flight_disk_drive.s', '-o', obj])
    run([tool(cc, 'ld.lld'), '-e', 'fd_drive_start', '-T', ld, obj, '-o', elf])
    run([tool(cc, 'llvm-objcopy'), '-O', 'binary', elf, bin_])
    syms = {}
    for line in run([tool(cc, 'llvm-nm'), elf], capture_output=True, text=True).stdout.splitlines():
        addr, _, name = line.split()
        syms[name] = int(addr, 16)
    code = bin_.read_bytes()
    # The 1581's buffers 3 and 4 ($0600-$07FF): DOS is idle while the code
    # runs, and the loader writes it again for each disk.
    assert syms['fd_drive_start'] == 0x0600 and len(code) <= 0x200, (hex(len(code)), syms)
    return code, syms


def link_resident(cc, m):
    out = OUT / f'{m}-resident.prg'
    run([cc, *COMMON, *MISSION[m], '-T', 'platform/c64/resident_flight.ld', f'-Wl,-Map,{out}.map',
         '-DRK_DSKY=1', '-DRK_DOWNRUPT=1', *FLIGHT_MAIN, *resident_sources(m), STORE / 'overlay_v2_services.S', '-o', out])
    prg = out.read_bytes()
    sys_at = re.search(rb'\x9e\s*(\d+)\x00', prg[:32])
    assert prg[:2] == b'\x01\x08' and sys_at, out
    syms = map_symbols(out)
    main = syms['main'] - 0x0801 + 2
    # main starts with JSR rk_boot: the VICE run stops after it.
    assert prg[main] == 0x20 and int.from_bytes(prg[main + 1:main + 3], 'little') == syms['rk_boot'], 'main does not start with rk_boot'
    return prg[2:], int(sys_at[1]), syms['main'] + 3, (syms['rk_dsky_queue'], syms['rk_hardware'])


def map_symbols(prg):
    """A link's symbols from its lld map."""
    return {mt[2]: int(mt[1], 16) for mt in re.finditer(r'^\s*([0-9a-f]+)\s+[0-9a-f]+\s+[0-9a-f]+\s+\d+\s+(\w+)$',
                                                        pathlib.Path(f'{prg}.map').read_text(), re.M)}


def check_crc(words, cs):
    """CRC-16/CCITT of an end state, as platform/c64/dap_pass_check.c
    hw_crc: the erasable words from 020 up but 070-073 (a PIPA's -0 as +0),
    then the centiseconds."""
    c = 0xffff

    def byte(c, b):
        c ^= b << 8
        for _ in range(8):
            c = ((c << 1) ^ 0x1021 if c & 0x8000 else c << 1) & 0xffff
        return c
    for i in range(0o20, 2048):
        if 0o70 <= i <= 0o73:
            continue
        w = 0 if 0o37 <= i <= 0o41 and words[i] == 0o77777 else words[i]
        c = byte(byte(c, w & 0xff), w >> 8)
    return byte(byte(c, cs & 0xff), cs >> 8)


def link_check(a):
    """The check program (platform/c64/dap_pass_check.c DP_HW_CHECK): the
    host run's snapshot at CHECK's start and its inputs over the window in
    the LM store's spare REU bytes. Linked twice: first to run in VICE
    (as tools/flight_scenario_vice.py does) for the end state's CRC, then
    with that CRC to match. Returns the PRG, its SYS address, where
    rk_boot returns to, the two CRCs (VICE's and the host run's), dp_done's
    address, and the REU image to load."""
    import dap_pass_vice as dp
    import flight_host_differential as fhd
    import flight_scenario_vice as fsv
    scen, snap_cs, cs = CHECK
    m, host_cc = 'lm', os.environ.get('CC', 'cc')
    run([sys.executable, 'tools/flight_store_build.py', '--host-only', '--missions', m, '--host-cc', host_cc])
    exe = fhd.build(host_cc, m, fhd.frozen_engine(host_cc))
    dp.WORK.mkdir(parents=True, exist_ok=True)
    tag = f'check-{scen}-{snap_cs}'
    header = fsv.snapshot(exe, scen, snap_cs, tag, cs, cs)
    snap_at = int(re.search(r'^snapshot at (\d+) cs', (dp.WORK / f'{tag}-host.log').read_text(), re.M)[1])
    rows = pathlib.Path(f'{header}.replay').read_bytes()
    host_end = pathlib.Path(f'{header}.end').read_bytes()
    n_states = len(host_end) // fsv.STATE
    end_cs, words = fsv.states(host_end, n_states)[-1]
    host = check_crc(words, end_cs)
    image = bytearray((STORE / f'{m}.reu').read_bytes())
    manifest = json.loads((STORE / f'{m}-manifest.json').read_text())
    at = fsv.replay_place(manifest, m, image, 2 * len(rows) + n_states * fsv.C64_STATE + 256)
    end_at = (at + 2 * len(rows) + 0xff) & ~0xff
    # The check's boot code while the window runs (at most the 16 KB boot
    # region, resident_flight.ld), below the rows.
    save_at = at - 0x4000
    assert save_at > 0 and not any(image[save_at:at]), f'no 16 KB of zeros below 0x{at:x} in the LM store'
    n_c64 = len(fsv.c64_rows(rows, m, 0, 0)) // fsv.ROW.size
    busy = {x[0]: x for x in fhd.FLIGHT}[scen][4].get('FLIGHT_BUSY_SAMPLE')
    defines = ['-DDP_REPLAY', f'-DDP_REPLAY_REU=0x{at:x}ul', f'-DDP_REPLAY_ROWS={n_c64}u', f'-DDP_REPLAY_CS={cs}u',
               f'-DDP_EVERY={cs}u', f'-DDP_END=0x{end_at:x}ul', *([f'-DDP_BUSY={int(busy)}u'] if busy else []),
               '-DDP_HW_CHECK', f'-DDP_HW_SAVE=0x{save_at:x}ul', f'-DDP_HW_HOST=0x{host:04x}u', f'-DDP_HW_WINDOW="LM P63 FROM {snap_at} CS"']

    def link(expect):
        prg = dp.link(a.cc, tag, header, cs + 500, len(image) - dp.SCRATCH_BYTES, m=m,
                      defines=[*defines, f'-DDP_HW_EXPECT=0x{expect:04x}u'])
        syms = map_symbols(prg)
        patch = fsv.c64_rows(rows, m, syms['rk_device'], syms['rk_pending'])
        image[at:at + len(patch)] = patch
        return prg, syms
    prg, syms = link(0)
    vice_args = argparse.Namespace(vice=a.vice_bin, rom_dir=a.rom_dir, data_dir=a.data_dir, cycles=6_000_000_000,
                                   timeout=a.timeout, profile=False)
    # The run's REU regions (tools/reu_layout.py), with the boot stash below
    # the rows: VICE logs any DMA outside them, and every byte the run
    # changed must lie in a writable one.
    import reu_layout
    regions = reu_layout.layout(m, manifest, (at, end_at, reu_layout.free_top(m, manifest)),
                                extra=[('check stash', save_at, at, True)])
    r = dp.vice(vice_args, tag, prg, any(mod['slot'] == 3 for mod in manifest['modules']), m,
                [(0, bytes(image))], [reu_layout.dma_trace(regions)])
    mail = (dp.WORK / f'vice-{tag}' / 'results.bin').read_bytes()
    if r.get('status') != 0 or not mail[0x34]:
        sys.exit(f"FAIL: the check window did not run to its end in VICE: {r.get('error') or r.get('status')}")
    bounds = reu_layout.report(m, regions, image, (dp.WORK / f'vice-{tag}' / 'image.reu').read_bytes(), 'test')
    misses = reu_layout.dma_misses((dp.WORK / f'vice-{tag}' / 'monitor.log').read_text())
    if not bounds['ok'] or misses:
        sys.exit(f'FAIL: the check program used the REU outside its layout ({misses} DMAs outside)')
    expect = mail[0x36] | mail[0x37] << 8
    prg, syms = link(expect)
    data = prg.read_bytes()
    sys_at = re.search(rb'\x9e\s*(\d+)\x00', data[:32])
    assert data[:2] == b'\x01\x08' and sys_at, prg
    main = syms['main'] - 0x0801 + 2
    calls_boot = data[main] == 0x20 and int.from_bytes(data[main + 1:main + 3], 'little') == syms['rk_boot']
    print(f'test: {scen} from {snap_at} cs, {cs} cs, {n_c64} input rows: VICE ends it in CRC {expect:04x}, '
          f'the host run in {host:04x}')
    return (data[2:], int(sys_at[1]), syms['main'] + 3 if calls_boot else None, (expect, host), syms['dp_done'],
            bytes(image))


def link_loader(cc, m, code, syms, disks):
    gen = OUT / m / 'flight_disk_gen.h'
    gen.parent.mkdir(parents=True, exist_ok=True)
    gen.write_text(
        '/* Generated by tools/flight_disk.py. */\n'
        f'#define FD_TITLE "{TITLE[m]}"\n'
        f"#define FD_MISSION '{m[0].upper()}'\n"
        f'#define FD_NAME "{m.upper()}"\n'
        f'#define FD_DISKS {disks}\n'
        f"#define FD_DRIVE_START 0x{syms['fd_drive_start']:04x}u\n"
        f"#define FD_DRIVE_TRK 0x{syms['fd_drive_trk']:04x}u\n"
        'static const unsigned char fd_drive_code[] = {' + ','.join(str(b) for b in code) + '};\n')
    out = OUT / f'{m}-loader.prg'
    run([cc, '-Os', '-std=c99', '-Wall', '-Wextra', '-Werror', '-I', gen.parent,
         'platform/c64/flight_disk_loader.c', 'platform/c64/flight_disk_rx.s',
         'platform/c64/flight_disk_unz.s', '-o', out])
    # The loader's code and data end below the decoder's ring at $4000.
    nm = run([tool(cc, 'llvm-nm'), f'{out}.elf'], capture_output=True, text=True).stdout
    end = int(re.search(r'^([0-9a-f]+) \S __heap_start$', nm, re.M).group(1), 16)
    if end > 0x4000:
        sys.exit(f'{m} loader ends at ${end:04x}, in the ring at $4000')
    return out.read_bytes()


def ranges(image):
    """The image's byte ranges between zero runs of GAP bytes or more."""
    out, at = [], 0
    for z in re.finditer(rb'\0{%d,}' % GAP, image):
        if z.start() > at:
            out.append((at, z.start()))
        at = z.end()
    if at < len(image):
        out.append((at, len(image)))
    return out


def fdz(data):
    """data compressed with tools/fdz.c (built here with the host's cc)."""
    exe, src = OUT / 'fdz', ROOT / 'tools/fdz.c'
    if not exe.exists() or exe.stat().st_mtime < src.stat().st_mtime:
        run([os.environ.get('HOST_CC', 'cc'), '-O2', '-std=c99', '-Wall', '-Wextra', '-Werror', src, '-o', exe])
    (OUT / 'fdz.in').write_bytes(data)
    run([exe, 'c', OUT / 'fdz.in', OUT / 'fdz.out'], stderr=subprocess.DEVNULL)
    return (OUT / 'fdz.out').read_bytes()


# The loader's limit on a 'Z' record's ranges (flight_disk_loader.c).
MAX_RANGES = 512


def pack(recs, capacity_of, res_at, res_len, sys_at):
    """recs, (REU address, bytes) pairs, compressed over the disks: each
    disk's data file (capacity_of(disk) bytes at most) is a 'Z' record (its
    REU ranges, then their bytes as one fdz stream), then on the last disk
    the 'R' record, and 'E'."""
    stream = b''.join(x for _, x in recs)
    spans, o = [], 0  # (stream offset, REU address, length)
    for a, x in recs:
        spans.append((o, a, len(x)))
        o += len(x)
    tail = 1 + 1 + 3 + 2 + 2  # 'E' and the 'R' record
    disks, at = [], 0

    def record(n):
        pieces = [(a + max(s, at) - s, min(s + k, at + n) - max(s, at)) for s, a, k in spans
                  if s < at + n and s + k > at]
        assert len(pieces) <= MAX_RANGES, len(pieces)
        return (b'Z' + len(pieces).to_bytes(2, 'little') +
                b''.join(r.to_bytes(3, 'little') + k.to_bytes(3, 'little') for r, k in pieces) +
                fdz(stream[at:at + n]))

    while at < len(stream):
        cap = capacity_of(len(disks) + 1) - tail
        n = len(stream) - at
        z = record(n)
        if len(z) > cap:
            # The most bytes whose record fits, to 256.
            lo, hi = 0, n // 256
            while lo < hi:
                mid = (lo + hi + 1) // 2
                if len(record(mid * 256)) <= cap:
                    lo = mid
                else:
                    hi = mid - 1
            assert lo, 'a disk holds no data'
            n = lo * 256
            z = record(n)
        at += n
        disks.append(bytearray(z))
    disks[-1] += b'R' + res_at.to_bytes(3, 'little') + res_len.to_bytes(2, 'little') + sys_at.to_bytes(2, 'little')
    return [bytes(d + b'E') for d in disks]


class D81:
    """A 1581 disk image, filled in file by file."""

    def __init__(self, name, disk_id):
        self.img = bytearray(TRACKS * SECTORS * 256)
        self.used = {(DIR_TRACK, s) for s in range(4)}
        self.entries = []
        self.name, self.id = name, disk_id

    def put(self, t, s, data):
        o = ((t - 1) * SECTORS + s) * 256
        self.img[o:o + 256] = data.ljust(256, b'\0')

    @staticmethod
    def blocks():
        return [(t, s) for t in range(1, TRACKS + 1) if t != DIR_TRACK for s in range(SECTORS)]

    def write_file(self, name, kind, data, chain):
        """data over the blocks in chain (one per 254 bytes)."""
        parts = [data[i:i + 254] for i in range(0, len(data), 254)] or [b'']
        assert len(parts) == len(chain), (name, len(parts), len(chain))
        for i, (ts, part) in enumerate(zip(chain, parts)):
            assert ts not in self.used, ts
            self.used.add(ts)
            link = bytes(chain[i + 1]) if i + 1 < len(chain) else bytes([0, len(part) + 1])
            self.put(*ts, link + part)
        self.entries.append((name, kind, chain[0], len(chain)))

    def finish(self):
        pad = lambda s, n: s.encode('ascii')[:n].ljust(n, b'\xa0')  # noqa: E731
        hdr = bytearray(256)
        hdr[0:4] = bytes([DIR_TRACK, 3, ord('D'), 0])
        hdr[4:0x14] = pad(self.name, 16)
        hdr[0x14:0x16] = b'\xa0\xa0'
        hdr[0x16:0x18] = self.id.encode('ascii')
        hdr[0x18] = 0xa0
        hdr[0x19:0x1b] = b'3D'
        hdr[0x1b:0x1d] = b'\xa0\xa0'
        self.put(DIR_TRACK, 0, bytes(hdr))
        for half in (0, 1):
            bam = bytearray(256)
            bam[0:2] = bytes([DIR_TRACK, 2]) if half == 0 else b'\x00\xff'
            bam[2], bam[3] = ord('D'), 0xbb
            bam[4:6] = self.id.encode('ascii')
            bam[6], bam[7] = 0xc0, 0
            for k in range(40):
                t = half * 40 + k + 1
                free = [s for s in range(SECTORS) if (t, s) not in self.used]
                bits = sum(1 << s for s in free)
                bam[0x10 + 6 * k] = len(free)
                bam[0x11 + 6 * k:0x16 + 6 * k] = bits.to_bytes(5, 'little')
            self.put(DIR_TRACK, 1 + half, bytes(bam))
        assert len(self.entries) <= 8
        d = bytearray(256)
        d[0:2] = b'\x00\xff'
        for i, (name, kind, (t, s), n) in enumerate(self.entries):
            e = 32 * i
            d[e + 2] = 0x80 | kind
            d[e + 3], d[e + 4] = t, s
            d[e + 5:e + 0x15] = pad(name, 16)
            d[e + 0x1e:e + 0x20] = n.to_bytes(2, 'little')
        self.put(DIR_TRACK, 3, bytes(d))
        return bytes(self.img)


class D64:
    """A 1541 disk image, filled in file by file: the same interface as
    D81. Each track's blocks go in the 1541 DOS's interleave of 10, so the
    KERNAL reads a file in a few turns of the disk rather than one a
    block."""
    TRACKS, DIR_TRACK, INTERLEAVE = 35, 18, 10

    @staticmethod
    def spt(t):
        return 21 if t <= 17 else 19 if t <= 24 else 18 if t <= 30 else 17

    def __init__(self, name, disk_id):
        self.img = bytearray(sum(self.spt(t) for t in range(1, 36)) * 256)
        self.used = {(18, 0), (18, 1)}
        self.entries = []
        self.name, self.id = name, disk_id

    def put(self, t, s, data):
        o = (sum(self.spt(k) for k in range(1, t)) + s) * 256
        self.img[o:o + 256] = data.ljust(256, b'\0')

    @classmethod
    def blocks(cls):
        out = []
        for t in range(1, cls.TRACKS + 1):
            if t == cls.DIR_TRACK:
                continue
            n, s, seen = cls.spt(t), 0, set()
            for _ in range(n):
                while s in seen:
                    s = (s + 1) % n
                seen.add(s)
                out.append((t, s))
                s = (s + cls.INTERLEAVE) % n
        return out

    write_file = D81.write_file

    def finish(self):
        pad = lambda s, n: s.encode('ascii')[:n].ljust(n, b'\xa0')  # noqa: E731
        bam = bytearray(256)
        bam[0:4] = bytes([18, 1, 0x41, 0])
        for t in range(1, 36):
            free = [k for k in range(self.spt(t)) if (t, k) not in self.used]
            bam[4 * t] = len(free)
            bam[4 * t + 1:4 * t + 4] = sum(1 << k for k in free).to_bytes(3, 'little')
        bam[0x90:0xa0] = pad(self.name, 16)
        bam[0xa0:0xa2] = b'\xa0\xa0'
        bam[0xa2:0xa4] = self.id.encode('ascii')
        bam[0xa4] = 0xa0
        bam[0xa5:0xa7] = b'2A'
        bam[0xa7:0xab] = b'\xa0' * 4
        self.put(18, 0, bytes(bam))
        assert len(self.entries) <= 8
        d = bytearray(256)
        d[0:2] = b'\x00\xff'
        for i, (name, kind, (t, s), n) in enumerate(self.entries):
            e = 32 * i
            d[e + 2] = 0x80 | kind
            d[e + 3], d[e + 4] = t, s
            d[e + 5:e + 0x15] = pad(name, 16)
            d[e + 0x1e:e + 0x20] = n.to_bytes(2, 'little')
        self.put(18, 1, bytes(d))
        return bytes(self.img)


# The disk image for each drive: its class and its file name's extension.
DISK = {'1581': (D81, 'd81'), '1541': (D64, 'd64')}

SEQ, PRG = 1, 2


def build(a, m, code, syms):
    extra = {}
    if m == 'test':
        res, sys_at, boot_return, (expect, host), done, image = link_check(a)
        extra = dict(check_expect=f'{expect:04x}', check_host=f'{host:04x}', done=done)
        dsky_queue = hardware = None
    else:
        image = (STORE / f'{m}.reu').read_bytes()
        res, sys_at, boot_return, (dsky_queue, hardware) = link_resident(a.cc, m)
    assert len(image) == 2 << 20, f'the {m} REU image is not 2 MB'
    # Stage the resident at the top of the REU, in bytes the store leaves 0
    # (the hand-off zeroes them again).
    res_at = (len(image) - len(res)) & ~0xff
    assert not any(image[res_at:res_at + len(res)]), f'{m}.reu uses ${res_at:06x}: no room to stage the resident'
    disk_type, ext = DISK[a.drive]
    blocks = disk_type.blocks()
    recs = [(a0, image[a0:e]) for a0, e in ranges(image)] + [(res_at, res)]
    disks = 1
    while True:
        # The loader holds the disk count: link it again if the set comes
        # out longer than it was linked for.
        loader = link_loader(a.cc, m, code, syms, disks)
        loader_blocks = -(-len(loader) // 254)

        def capacity(d):
            # Every disk's id sector; disk 1's loader.
            return (len(blocks) - 1 - (loader_blocks if d == 1 else 0)) * 254

        datas = pack(recs, capacity, res_at, len(res), sys_at)
        if len(datas) == disks:
            break
        disks = len(datas)
    paths = []
    for d, data in enumerate(datas, 1):
        disk = disk_type(f'APOLLO 11 {m.upper()} {d}/{len(datas)}', m.upper()[:2])
        nblocks = -(-len(data) // 254)
        idb = b'APOLLO11' + bytes([ord(m[0].upper()), d, len(datas), *blocks[1]])
        if d == 1:
            disk.write_file(f'APOLLO {m.upper()}', PRG, loader, blocks[-loader_blocks:])
        disk.write_file(f'{m.upper()} DISK {d}', SEQ, idb, blocks[:1])
        disk.write_file(f'{m.upper()} DATA {d}', SEQ, data, blocks[1:1 + nblocks])
        path = OUT / f'{m}-{d}.{ext}'
        path.write_bytes(disk.finish())
        paths.append(path)
    loaded = sum(len(x) for _, x in recs)
    info = dict(mission=m, disks=[p.name for p in paths], loader_bytes=len(loader) - 2,
                resident_bytes=len(res), resident_start=sys_at, boot_return=boot_return, dsky_queue=dsky_queue, hardware=hardware, resident_staged=f'0x{res_at:06x}',
                bytes_loaded=loaded, data_bytes=[len(x) for x in datas], **extra)
    print(f"{m}: {len(paths)} disks, {loaded} bytes to the REU, loader {info['loader_bytes']} bytes, "
          f"resident {len(res)} bytes (SYS {sys_at})")
    return info


# VICE ---------------------------------------------------------------------

# The 1581's DOS ROM, for true drive emulation: from the VICE source mirror
# at tools/versions.json's ROM commit, as tools/setup_cloud.sh fetches the
# C64's (kept here so the toolchain cache's key does not change).
DRIVE_ROMS = {
    '1581': ('dos1581-318045-02.bin', '8f689277f36be357f5f105e6e606c35c42e73c581cdeec14b58f31850dfbcfcb'),
    '1541': ('dos1541-325302-01+901229-05.bin', 'd1d45afb46fd4e2b48d93ca367b889d75654a3b7acf73044e51f6d880c09369e'),
}


def drive_rom(data_dir, drive):
    name, sha = DRIVE_ROMS[drive]
    path = data_dir / 'DRIVES' / name
    if not path.exists():
        commit = json.loads((ROOT / 'tools/versions.json').read_text())['vice']['rom_commit']
        path.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run(['curl', '-fsSL', '--retry', '4', '--retry-delay', '2', '-o', str(path),
                        f'https://raw.githubusercontent.com/VICE-Team/svn-mirror/{commit}/vice/data/DRIVES/{name}'],
                       check=True)
    if hashlib.sha256(path.read_bytes()).hexdigest() != sha:
        path.unlink()
        sys.exit(f'FAIL: {name}: SHA-256 mismatch')


class Monitor:
    """VICE's remote monitor (text). A command stops the emulation; the
    first one after go() answers with an extra prompt in front."""

    PROMPT = re.compile(rb'\(C:\$[0-9a-f]{4}\) ')

    def __init__(self, port):
        for _ in range(300):
            try:
                self.s = socket.create_connection(('127.0.0.1', port), timeout=120)
                break
            except OSError:
                time.sleep(0.2)
        else:
            raise RuntimeError('no VICE remote monitor')
        self.running = True
        self.hit = False

    def drain(self):
        data = b''
        self.s.setblocking(False)
        try:
            while True:
                chunk = self.s.recv(65536)
                if not chunk:
                    break
                data += chunk
        except BlockingIOError:
            pass
        finally:
            self.s.setblocking(True)
            self.s.settimeout(120)
        return data

    def cmd(self, c):
        if self.running and self.PROMPT.search(self.drain()):
            self.running = False  # a checkpoint stopped it
            self.hit = True
        self.s.sendall(c.encode() + b'\n')
        want = 2 if self.running else 1
        self.running = False
        data = b''
        while len(self.PROMPT.findall(data)) < want or not self.PROMPT.search(data[-12:]):
            chunk = self.s.recv(65536)
            if not chunk:
                break
            data += chunk
        return self.PROMPT.sub(b'', data).decode('latin-1')

    def mem(self, lo, hi):
        out = bytearray()
        for line in self.cmd(f'm {lo:04x} {hi:04x}').splitlines():
            mt = re.match(r'>C:([0-9a-f]{4})((?:\s+[0-9a-f]{2})+)', line)
            if mt:
                out += bytes(int(x, 16) for x in mt[2].split()[:16])
        return bytes(out[:hi - lo + 1])

    def go(self):
        self.s.sendall(b'x\n')
        self.running = True
        self.hit = False

    def wait_stop(self):
        """Wait for a checkpoint to stop the emulation."""
        data = b''
        while not self.PROMPT.search(data[-12:]):
            chunk = self.s.recv(65536)
            if not chunk:
                break
            data += chunk
        self.running = False
        return self.PROMPT.sub(b'', data).decode('latin-1')

    def stopwatch(self):
        return int(re.search(r'Stopwatch:\s*(\d+)', self.cmd('sw'))[1])


# The DSKY's key queue (platform/c64/resident_dsky.S): its head in zero
# page, and the channel 15 codes of the keys it takes (P is PRO).
DSKY_HEAD = 0xf0
# IMODES33 (build/fixed-data/*/assembly.lst): T4RUPT's copy of channel 32 bit 14
IMODES33 = dict(lm=0o1303, cm=0o1321)
DSKY_KEYS = {**{str(d): d or 0o20 for d in range(10)}, 'V': 0o21, 'N': 0o37, '+': 0o32, '-': 0o33, 'E': 0o34,
             'C': 0o36, 'R': 0o22, 'K': 0o31, 'P': 0x40}


def xtest_keys(keys, wait):
    """Type keys on VICE's window through the X server (XTEST), so the
    C64's keyboard scan sees them: E is RETURN (ENTR), the rest as typed."""
    import ctypes
    x11, xtst = ctypes.CDLL('libX11.so.6'), ctypes.CDLL('libXtst.so.6')
    x11.XOpenDisplay.restype = ctypes.c_void_p
    x11.XStringToKeysym.restype = ctypes.c_ulong
    dpy = ctypes.c_void_p(x11.XOpenDisplay(None))
    assert dpy.value, 'no X display'
    xtst.XTestFakeMotionEvent(dpy, -1, 160, 120, 0)  # over VICE's window: focus follows the pointer
    names = {'E': b'Return', '+': b'plus', '-': b'minus'}
    for ch in keys:
        code = x11.XKeysymToKeycode(dpy, ctypes.c_ulong(x11.XStringToKeysym(names.get(ch, ch.lower().encode()))))
        assert code, f'no X keycode for {ch}'
        for down in (1, 0):
            xtst.XTestFakeKeyEvent(dpy, code, down, 0)
            x11.XFlush(dpy)
            time.sleep(0.3)
        time.sleep(wait)
    x11.XCloseDisplay(dpy)


def screen_text(mem, rows=17):
    """Rows of screen codes as text (the DSKY's 17)."""
    ch = lambda c: chr(c + 64) if 1 <= c <= 26 else chr(c) if 32 <= c < 64 else '.'  # noqa: E731
    return [''.join(ch(c) for c in mem[r * 40:r * 40 + 40]).rstrip() for r in range(rows)]


def check_run(mon, info, result):
    """The check program's run, to dp_done: its report from the screen and
    the mailbox (platform/c64/dap_pass_check.c DP_HW_CHECK)."""
    if info['boot_return']:
        mon.cmd(f"break {info['boot_return']:04x}")
        mon.cmd('sw reset')
        mon.go()
        mon.wait_stop()
        result['boot_s'] = round(mon.stopwatch() / PAL_HZ, 1)
        mon.cmd('del')
    mon.cmd(f"break {info['done']:04x}")
    mon.cmd('sw reset')
    mon.go()
    mon.wait_stop()
    result['run_s'] = round(mon.stopwatch() / PAL_HZ, 1)
    box = mon.mem(0xc000, 0xc03f)
    cycles, _, steps, start_cs, end_cs = struct.unpack_from('<3I2H', box, 0x10)
    agc = (end_cs - start_cs) & 0x3fff
    crc = box[0x36] | box[0x37] << 8
    result.update(check_crc=f'{crc:04x}', check_expect=info['check_expect'], check_pass=bool(box[0x38]),
                  window_cs=agc, window_steps=steps, window_x=round(cycles / (agc * PAL_HZ / 100), 2) if agc else None,
                  screen=screen_text(mon.mem(0x0400, 0x0400 + 999), 25))
    result['ok'] = bool(box[0x38])
    return result


def vice(a, m, info):
    rom = a.rom_dir.resolve()
    work = OUT / f'vice-{m}'
    work.mkdir(parents=True, exist_ok=True)
    reu = work / 'image.reu'
    reu.write_bytes(bytes(2 << 20))
    drive_rom(a.data_dir.resolve(), a.drive)
    port = 6510 + (os.getpid() % 500)
    env = dict(os.environ, XDG_CACHE_HOME=str(work / 'cache'), XDG_CONFIG_HOME=str(work / 'config'))
    for key in ('XDG_CACHE_HOME', 'XDG_CONFIG_HOME'):
        pathlib.Path(env[key]).mkdir(parents=True, exist_ok=True)
    disks = [OUT / d for d in info['disks']]
    cmd = [a.vice, '-default', *([] if a.xtest else ['-console']), '-pal', '-warp', '+sound',
           '-kernal', rom / 'kernal-901227-03.bin', '-basic', rom / 'basic-901226-01.bin',
           '-chargen', rom / 'chargen-901225-01.bin', '-directory',
           # the window's shaders and keymaps (/usr/share/vice) with --xtest
           f'{a.data_dir.resolve()}:{rom.parent}' + (':/usr/share/vice:/usr/share/vice/C64' if a.xtest else ''),
           *(['+drive8truedrive', '-virtualdev8', '-iecdevice8'] if a.slow else ['-drive8truedrive', '+virtualdev8']),
           '-drive8type', a.drive, '-8', disks[0],
           '-reu', '-reusize', '2048', '-reuimage', reu, '-reuimagerw',
           '-remotemonitor', '-remotemonitoraddress', f'ip4://127.0.0.1:{port}',
           '-keybuf', 'load"*",8,1\\nrun\\n']
    log = (work / 'vice.log').open('w')
    p = subprocess.Popen([str(c) for c in cmd], stdout=log, stderr=subprocess.STDOUT, env=env)
    result = dict(mission=m, ok=False)
    try:
        mon = Monitor(port)
        # The hand-off ($0340) is a checkpoint: the poll below may come
        # after the loader is gone.
        mon.cmd('break 0340')
        mon.go()
        t0 = time.time()
        state = 0
        while time.time() - t0 < a.timeout:
            time.sleep(2)
            box = mon.mem(0x03f0, 0x03ff)
            state = box[0]
            if state != 1:
                print(f"{time.time() - t0:.0f}s: loader state {state}, mailbox {box.hex()}", flush=True)
            if state == 2:
                d = box[1]
                mon.cmd(f'attach "{disks[d - 1]}" 8')
                # RETURN into the keyboard buffer.
                mon.cmd('> 0277 0d')
                mon.cmd('> 00c6 01')
                mon.cmd('> 03f0 01')
                result.setdefault('swaps', []).append(d)
            elif state == 0xff:
                result['error'] = f'loader stopped, reason {box[1]}'
                break
            elif state == 3 and mon.hit:
                break
            mon.go()
        if state != 3:
            result.setdefault('error', f'timed out in state {state}')
            return result
        load_cycles = int.from_bytes(box[2:6], 'little')
        clear_cycles = int.from_bytes(box[6:10], 'little')
        result['fast_loader'] = bool(box[10])
        result.update(load_cycles=load_cycles, load_s=round(load_cycles / PAL_HZ, 1),
                      reu_clear_s=round(clear_cycles / PAL_HZ, 2))
        # From the hand-off to the resident's entry: the fetch and the
        # zeroing.
        mon.cmd('del')
        mon.cmd(f"break {info['resident_start']:04x}")
        mon.cmd('sw reset')
        mon.go()
        mon.wait_stop()
        result['handoff_cycles'] = mon.stopwatch()
        mon.cmd('del')
        if m == 'test':
            return check_run(mon, info, result)
        # rk_boot: main's first instruction calls it; A holds its result
        # (0, or which check failed) after.
        mon.cmd(f"break {info['boot_return']:04x}")
        mon.cmd('sw reset')
        mon.go()
        mon.wait_stop()
        result['boot_cycles'] = mon.stopwatch()
        result['boot_s'] = round(result['boot_cycles'] / PAL_HZ, 1)
        result['boot_status'] = int(re.search(r'\.;[0-9a-f]{4} ([0-9a-f]{2})', mon.cmd('r'))[1], 16)
        mon.cmd('del')
        # Then the AGC: TIME1 (erasable 025) counts while it runs.
        mon.cmd('sw reset')
        samples = []
        for _ in range(4):
            mon.go()
            time.sleep(4)
            cyc = mon.stopwatch()
            w = mon.mem(0x7000, 0x7000 + 2 * 0o1011 + 1)
            word = lambda e: w[2 * e] | w[2 * e + 1] << 8  # noqa: E731
            samples.append(dict(cycles=cyc, time1=f'{word(0o25):05o}', z=f'{word(5):05o}', fb=f'{word(4):05o}',
                                modreg=f'{word(0o1011):05o}'))
        result['samples'] = samples
        result['ok'] = result['boot_status'] == 0 and len({x['time1'] for x in samples}) > 1
        # The DSKY: keys typed through its queue, as its keyboard scan
        # queues them (a press, then the release), then the screen.
        if a.keys and a.xtest:
            mon.go()
            xtest_keys(a.keys, a.key_wait)
            mon.cmd('r')
        elif a.keys:
            queue = info['dsky_queue']
            for ch in a.keys:
                head = mon.mem(DSKY_HEAD, DSKY_HEAD)[0]
                mon.cmd(f'> {queue + head:04x} {DSKY_KEYS[ch]:02x}')
                mon.cmd(f'> {queue + ((head + 1) & 7):04x} 00')
                mon.cmd(f'> {DSKY_HEAD:04x} {(head + 2) & 7:02x}')
                mon.go()
                time.sleep(a.key_wait)
                mon.cmd('r')  # the first answer after go() can be the stale prompt
                result.setdefault('key_pcs', []).append(re.search(r'\.;([0-9a-f]{4})', mon.cmd('r'))[1])
                # channel 32's high byte (PRO holds bit 14, 0x20, low for 1 s
                # of AGC time), and IMODES33, where T4RUPT keeps the bit it saw
                ch32 = mon.mem(info['hardware'] + 15, info['hardware'] + 15)[0]
                result.setdefault('key_ch32', []).append(f'{ch32:02x}')
                im = mon.mem(0x7000 + 2 * IMODES33[m], 0x7000 + 2 * IMODES33[m] + 1)
                result.setdefault('key_imodes33', []).append(f'{im[0] | im[1] << 8:05o}')
        if a.screenshot:
            mon.go()
            time.sleep(a.settle)
            shot = (work / f'{m}-dsky.png').resolve()
            mon.cmd('r')  # the first answer after go() can be the stale prompt
            result['pc'] = re.search(r'\.;([0-9a-f]{4})', mon.cmd('r'))[1]
            result['screen'] = screen_text(mon.mem(0x0400, 0x0400 + 679))
            f = mon.mem(0x7000 + 2 * 0o375, 0x7000 + 2 * 0o377 + 1)
            result['failreg'] = [f'{f[2 * i] | f[2 * i + 1] << 8:05o}' for i in range(3)]
            f = mon.mem(0x7000 + 2 * 0o1011, 0x7000 + 2 * 0o1011 + 1)
            result['modreg'] = f'{f[0] | f[1] << 8:05o}'
            f = mon.mem(0x7000 + 2 * 4, 0x7000 + 2 * 5 + 1)
            result['fb'], result['z'] = f'{f[0] | f[1] << 8:05o}', f'{f[2] | f[3] << 8:05o}'
            mon.cmd(f'screenshot "{shot}" 2')
            result['screenshot'] = str(shot)
        if a.p00:
            # RSET, V37 ENTR 00 ENTR: the AGC in P00 (MODREG 0), with PROG
            # 00 on the DSKY's screen (docs/RUN-ON-C64.md, The DSKY).
            prog = next((row for i, row in enumerate(result['screen'][:-1])
                         if 'PROG' in row and 'VERB' in row), None)
            digits = result['screen'][result['screen'].index(prog) + 1] if prog else ''
            result['p00'] = result['modreg'] == '00000' and digits[prog.index('PROG'):].strip() == '00'
            result['ok'] = result['ok'] and result['p00']
    finally:
        try:
            mon.cmd('quit')
            p.wait(timeout=60)
        except Exception:
            p.kill()
            p.wait()
        log.close()
    # VICE writes the REU back on exit (-reuimagerw): what the run left in
    # it against the store image, in 256-byte blocks.
    if result.get('samples'):
        got, want = reu.read_bytes(), (STORE / f'{m}.reu').read_bytes()
        result['reu_blocks_changed'] = [f'0x{i:06x}' for i in range(0, len(want), 256)
                                        if got[i:i + 256] != want[i:i + 256]]
    return result


P00_KEYS = 'RV37E00E'


def sha256(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def boot_evidence(a, m, info):
    """build/flight-disk/boot-<m>.json, for the bootable_c64_application
    acceptance item: the commit, the SHA-256 of each file the boot used
    (the disks, the loader and resident PRGs, the REU store) and of the
    VICE log and the screen's text after the keys, with the verdict."""
    r = info['vice']
    head = subprocess.run(['git', 'rev-parse', 'HEAD'], capture_output=True, text=True, cwd=ROOT).stdout.strip()
    dirty = bool(subprocess.run(['git', 'status', '--porcelain', '--untracked-files=no'], capture_output=True,
                                text=True, cwd=ROOT).stdout.strip())
    files = [OUT / d for d in info['disks']] + [OUT / f'{m}-loader.prg', OUT / f'{m}-resident.prg', STORE / f'{m}.reu']
    ev = dict(mission=m, commit=head, dirty=dirty, drive=a.drive, keys=a.keys, ok=r['ok'], p00=r.get('p00'),
              boot_status=r.get('boot_status'), modreg=r.get('modreg'), failreg=r.get('failreg'),
              load_s=r.get('load_s'), boot_s=r.get('boot_s'),
              sha256={f"{f.parent.name}/{f.name}": sha256(f) for f in files if f.exists()},
              vice_log_sha256=sha256(OUT / f'vice-{m}' / 'vice.log'),
              screen_sha256=hashlib.sha256('\n'.join(r.get('screen', [])).encode()).hexdigest(),
              screen=r.get('screen'))
    out = OUT / f'boot-{m}.json'
    out.write_text(json.dumps(ev, indent=1) + '\n')
    # CI keeps build/ci-logs (port-ci.yml): the evidence and the log it hashes.
    logs = ROOT / 'build/ci-logs'
    if logs.is_dir():
        shutil.copy(out, logs / f'flight-disk-boot-{m}.json')
        shutil.copy(OUT / f'vice-{m}' / 'vice.log', logs / f'flight-disk-boot-{m}-vice.log')
    print(f"{m}: {'P00 reached' if ev['p00'] else 'P00 NOT reached'} from the disk at {head[:9]}"
          f"{' (dirty tree)' if dirty else ''}; log sha256 {ev['vice_log_sha256'][:16]}; {out}")


def main():
    global STORE
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--cc', default=os.environ.get('MOS_CC', str(ROOT / 'deps/llvm-mos/bin/mos-c64-clang')))
    p.add_argument('--missions', default='lm,cm,test',
                   help='the sets: lm, cm and test (the hardware check)')
    p.add_argument('--drive', choices=sorted(DISK), default='1581',
                   help='the disk drive: D81 sets with the fast loader, or D64 sets read with the KERNAL')
    p.add_argument('--vice', action='store_true', help='boot each set in VICE (true drive emulation)')
    p.add_argument('--keys', default='', help='DSKY keys to type after the boot (E is ENTR, P is PRO), e.g. V35E')
    p.add_argument('--xtest', action='store_true',
                   help="type --keys on VICE's window through X (run under xvfb-run): the C64's keyboard scan sees them")
    p.add_argument('--key-wait', dest='key_wait', type=float, default=1.0, help='seconds of VICE run after each key')
    p.add_argument('--settle', type=float, default=2.0, help='seconds of VICE run before the screenshot')
    p.add_argument('--screenshot', action='store_true', help="save the DSKY's screen after the boot (and keys)")
    p.add_argument('--p00', action='store_true',
                   help='after the boot type RSET V37E00E, require P00 (MODREG 0, PROG 00 on the screen) and write '
                        'build/flight-disk/boot-<m>.json: the commit and the SHA-256 of the disks, PRGs and VICE log')
    p.add_argument('--slow', action='store_true',
                   help='the VICE run without true drive emulation: the loader falls back to the KERNAL')
    p.add_argument('--vice-bin', dest='vice_bin', default=os.environ.get('VICE', 'x64sc'))
    p.add_argument('--rom-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_ROM_DIR', ROOT / 'deps/vice-data/C64')))
    p.add_argument('--data-dir', type=pathlib.Path,
                   default=pathlib.Path(os.environ.get('VICE_DATA_DIR', ROOT / 'deps/vice-data')))
    p.add_argument('--timeout', type=int, default=3600)
    p.add_argument('--store', type=pathlib.Path, default=STORE, help='the flight store build (make flight-store)')
    a = p.parse_args()
    if a.p00:
        a.keys, a.screenshot = a.keys or P00_KEYS, True
    STORE = a.store.resolve()
    a.vice = a.vice_bin if a.vice else None
    OUT.mkdir(parents=True, exist_ok=True)
    code, syms = drive_code(a.cc)
    report = []
    for m in a.missions.split(','):
        info = build(a, m, code, syms)
        if a.vice:
            info['vice'] = vice(a, m, info)
            print(json.dumps(info['vice'], indent=1))
            if a.p00 and m != 'test':
                boot_evidence(a, m, info)
            # CI keeps build/ci-logs (port-ci.yml): the disks it booted, so a
            # run's artifact holds images ready to write (docs/RUN-ON-C64.md).
            logs = ROOT / 'build/ci-logs'
            if logs.is_dir() and info['vice'].get('ok'):
                for d in info['disks']:
                    shutil.copy(OUT / d, logs / d)
        report.append(info)
    (OUT / ('vice.json' if a.vice else 'disks.json')).write_text(json.dumps(report, indent=1) + '\n')
    return 0 if all(r.get('vice', {}).get('ok', True) for r in report) else 1


if __name__ == '__main__':
    sys.exit(main())
