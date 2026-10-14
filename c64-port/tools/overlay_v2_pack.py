#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Pack overlay ABI 2 module PRGs into an REU store image and a disk plan.

The REU image is what the resident runtime reads by DMA (OVERLAY_ABI_V2.md,
"REU store image"). The same image, split into page-aligned segment files,
is what goes on D81 media so a boot loader can stash it into the REU on real
hardware; VICE can attach the image directly with -reuimage.

Inputs are linked module PRGs (two-byte load address, then the 24-byte
NATV header and payload) and an optional resolver JSON list of
{"bank":int,"loc":int,"module":int,"selector":int} records.

--tail adds raw data at the very end of the image, ending on its last page
and inside one 64 KB REU bank, so a reader finds it from the image's page
count alone, with 16-bit address arithmetic (the flight resident's
boundary-word table: entries, then a count, tools/resident_boundary_words.py).
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

ABI = 2
HEADER = 24
# Slot class 2: the fixed extension in RAM under I/O, loaded once at boot
# (include/overlay_v2.h APOLLO_OV2_SLOT_FIXED); no resolver record names one.
# Slot class 3: pinned modules at $C000, loaded once at boot by plain DMA
# (APOLLO_OV2_SLOT_PINNED, BSS zeroed) and only called directly; no resolver
# record names one either.
# Slot class 4: the pinned task region, the task slot's RAM at $E000 kept for
# one module (APOLLO_OV2_SLOT_PINNED_TASK): loaded at boot by plain DMA, BSS
# zeroed, no main; the class-3 module's table list names its tables. A store
# with one has no task-slot module but one boot-only module (build_image).
SLOTS = {0: (0x8000, 0x4000), 1: (0xE000, 0x1FFA), 2: (0xD000, 0x1000), 3: (0xC000, 0x0C00),
         4: (0xE000, 0x1FFA)}
TASK = 1
FIXED = 2
PINNED = 3
PINNED_TASK = 4
BOOT_LOADED = (FIXED, PINNED, PINNED_TASK)
INDEX_ENTRIES = 4096
INDEX_RECORD = 16
RESOLVER_RECORD = 8
BANK_PAGES = 512 * 1024 // 256  # the REU's DMA address wraps inside 512 KB
REU_SIZES_KB = (128, 256, 512, 1024, 2048, 4096, 8192, 16384)
D81_BLOCKS = 3160            # free data blocks on a formatted 1581 image
D81_DIR_ENTRIES = 296
BLOCK_BYTES = 254


def crc16(data, crc=0xFFFF):
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def parse_module(path, mission, services):
    data = Path(path).read_bytes()
    if len(data) < 2 + HEADER:
        raise ValueError(f'{path}: too short for an ABI 2 header')
    load = struct.unpack_from('<H', data, 0)[0]
    payload = data[2:]
    h = payload[:HEADER]
    if h[:4] != b'NATV' or h[4] != ABI or h[5] != HEADER:
        raise ValueError(f'{path}: not an ABI 2 module')
    slot, module_mission = h[6], h[7]
    ident, entry, bss, bss_bytes, load_bytes, svc, selectors, tables = struct.unpack_from('<8H', h, 8)
    if slot not in SLOTS:
        raise ValueError(f'{path}: unknown slot class {slot}')
    base, capacity = SLOTS[slot]
    problems = []
    if load != base: problems.append(f'load address ${load:04x} is not slot base ${base:04x}')
    if module_mission != mission: problems.append(f'mission {module_mission} != {mission}')
    if not 0 < ident < INDEX_ENTRIES or not ident >> 8: problems.append(f'module id ${ident:04x} outside lane ranges')
    if entry != base + HEADER: problems.append('entry is not slot base + 24')
    if load_bytes != len(payload): problems.append(f'header load bytes {load_bytes} != file payload {len(payload)}')
    if load_bytes > capacity: problems.append(f'{load_bytes} bytes exceed slot capacity {capacity}')
    if not base + load_bytes <= bss <= base + capacity or bss_bytes > base + capacity - bss:
        problems.append('BSS outside the slot')
    if slot == FIXED and bss_bytes:
        problems.append('a fixed-extension module has BSS (boot loads it once and clears nothing)')
    if svc >> 8 != services >> 8 or svc & 0xFF > services & 0xFF:
        problems.append(f'service table ${svc:04x} not compatible with resident ${services:04x}')
    if not selectors: problems.append('no selectors')
    if tables and not base + HEADER <= tables < base + capacity:
        problems.append(f'routine-table list ${tables:04x} outside the slot')
    if problems:
        raise ValueError(f'{path}: ' + '; '.join(problems))
    return dict(path=str(path), id=ident, slot=slot, bytes=load_bytes, selectors=selectors,
                crc16=crc16(payload), payload=payload, sha256=hashlib.sha256(data).hexdigest())


def build_image(modules, resolver, mission, services, tail=b''):
    ids = [m['id'] for m in modules]
    if len(set(ids)) != len(ids):
        raise ValueError('duplicate module identity')
    # Classes 2 to 4 each load at one fixed address at boot: a second
    # module of one class would silently replace the first.
    for slot in BOOT_LOADED:
        if sum(m['slot'] == slot for m in modules) > 1:
            raise ValueError(f'more than one slot class {slot} module')
    # Class 4 keeps the task slot's RAM: a task-slot load would overwrite it.
    # The one exception is boot's own: a task-slot module no resolver record
    # names, which boot runs once before it loads class 4 over it (the flight
    # resident's boundary-word module; boot itself runs in the job slot).
    task = [m for m in modules if m['slot'] == TASK]
    if any(m['slot'] == PINNED_TASK for m in modules) and (
            len(task) > 1 or any(r['module'] == m['id'] for m in task for r in resolver)):
        raise ValueError('a slot class 4 module shares $E000 with a task-slot module other than one boot-only module')
    by_id = {m['id']: m for m in modules}
    keys = set()
    for r in resolver:
        key = (r['bank'], r['loc'])
        if key in keys:
            raise ValueError(f'duplicate resolver key {key}')
        keys.add(key)
        target = by_id.get(r['module'])
        if target is None or not 0 <= r['selector'] < target['selectors'] or target['slot'] in BOOT_LOADED:
            raise ValueError(f'resolver record {r} names no valid continuation')
    resolver = sorted(resolver, key=lambda r: (r['bank'], r['loc']))
    index_page = 1
    resolver_page = index_page + INDEX_ENTRIES * INDEX_RECORD // 256
    resolver_pages = -(-len(resolver) * RESOLVER_RECORD // 256)
    page = resolver_page + resolver_pages
    placed = []
    for m in sorted(modules, key=lambda m: m['id']):
        size = -(-m['bytes'] // 256)
        # One DMA loads a module, and the REU's address counter wraps inside
        # a 512 KB bank (17xx REC; VICE too): never let a module cross one.
        if page // BANK_PAGES != (page + size - 1) // BANK_PAGES:
            page = (page // BANK_PAGES + 1) * BANK_PAGES
        placed.append(dict(m, page=page))
        page += size
    tail_pages = -(-len(tail) // 256)
    if len(tail) > 0x10000:
        raise ValueError(f'tail of {len(tail)} bytes exceeds one 64 KB REU bank')
    start = page * 256 + (-len(tail)) % 256
    if tail and start >> 16 != (start + len(tail) - 1) >> 16:
        page = (page * 256 >> 16) + 1 << 8
        start = page * 256 + (-len(tail)) % 256
    pages = page + tail_pages
    image = bytearray(pages * 256)
    struct.pack_into('<4sBBHHHHHH', image, 0, b'NREU', 1, mission, services, len(modules),
                     index_page, resolver_page, len(resolver), pages)
    for m in placed:
        struct.pack_into('<5HBB', image, index_page * 256 + m['id'] * INDEX_RECORD,
                         m['id'], m['page'], m['bytes'], m['crc16'], m['selectors'], m['slot'], 1)
        image[m['page'] * 256:m['page'] * 256 + m['bytes']] = m['payload']
    if tail:
        image[start:start + len(tail)] = tail
    for i, r in enumerate(resolver):
        struct.pack_into('<4H', image, resolver_page * 256 + i * RESOLVER_RECORD,
                         r['bank'], r['loc'], r['module'], r['selector'])
    return image, placed


def reu_size_kb(image_bytes):
    for kb in REU_SIZES_KB:
        if image_bytes <= kb * 1024:
            return kb
    raise ValueError('image exceeds a 16 MB REU')


def disk_plan(image_bytes, resident_bytes, prefix):
    """Split the store into segment files across as few D81 images as fit.

    Disk 1 also carries the resident boot program. Segments are whole pages so
    the boot loader can stash each 256-byte page straight into the REU."""
    disks, offset, disk, used = [], 0, 1, 0
    resident_blocks = -(-(resident_bytes + 2) // BLOCK_BYTES)
    files = [dict(disk=1, name=prefix + '.BOOT', kind='resident', bytes=resident_bytes,
                  blocks=resident_blocks)]
    used = resident_blocks
    segment = 0
    while offset < image_bytes:
        free_blocks = D81_BLOCKS - used
        # Leave room for nothing else; cap a segment to whole pages.
        capacity = (free_blocks * BLOCK_BYTES - 2) // 256 * 256
        if capacity < 256:
            disks.append(disk); disk += 1; used = 0; continue
        size = min(capacity, image_bytes - offset)
        blocks = -(-(size + 2) // BLOCK_BYTES)
        files.append(dict(disk=disk, name=f'{prefix}.S{segment:02d}', kind='store-segment',
                          reu_offset=offset, bytes=size, blocks=blocks))
        used += blocks; offset += size; segment += 1
    disks.append(disk)
    return dict(media='D81', data_blocks_per_disk=D81_BLOCKS, disks=max(disks), files=files)


def synthetic_module(ident, slot, mission, services, selectors, body_bytes, bss_bytes):
    base, capacity = SLOTS[slot]
    load_bytes = HEADER + body_bytes
    header = struct.pack('<4sBBBB8H', b'NATV', ABI, HEADER, slot, mission, ident, base + HEADER,
                         base + load_bytes, bss_bytes, load_bytes, services, selectors, 0)
    body = bytes((ident * 7 + i) & 0xFF for i in range(body_bytes))
    return struct.pack('<H', base) + header + body


def selftest(out_dir, mission=1, services=0x0201):
    """Synthetic modules for the host runtime test (tests/test_overlay_v2.c)."""
    out_dir.mkdir(parents=True, exist_ok=True)
    specs = [(0x0101, 0, 4, 300, 16), (0x0102, 0, 2, 0x4000 - HEADER, 0),
             (0x0301, 1, 3, 200, 8), (0x0F01, 0, 1, 40, 0), (0x0A02, FIXED, 1, 100, 0),
             (0x0A03, PINNED, 1, 120, 8)]
    paths = []
    for ident, slot, selectors, body, bss in specs:
        p = out_dir / f'm{ident:04x}.prg'
        # The proof module is built against minor version 0 of the same
        # service table, which an append-only resident still serves.
        version = services & 0xFF00 if ident >> 8 == 0xF else services
        p.write_bytes(synthetic_module(ident, slot, mission, version, selectors, body, bss))
        paths.append(p)
    resolver = [dict(bank=0o24000, loc=0o2000, module=0x0101, selector=3),
                dict(bank=0o24000, loc=0o2100, module=0x0301, selector=1),
                dict(bank=0o02000, loc=0o4000, module=0x0102, selector=1)]
    (out_dir / 'resolver.json').write_text(json.dumps(resolver, indent=1) + '\n')
    return paths, out_dir / 'resolver.json'


def selftest_pinned_task(out_dir, mission=1, services=0x0201):
    """A store with a class-4 module (tests/test_overlay_v2.c, second image):
    a job module, a boot-only task-slot module, the class-3 module and the
    class-4 module. Checks first that the packer refuses class 4 beside a
    task-slot module a record names, two task-slot modules or a second class 4."""
    def parsed(ident, slot, body, bss):
        data = synthetic_module(ident, slot, mission, services, 1, body, bss)
        return dict(id=ident, slot=slot, bytes=len(data) - 2, selectors=1, crc16=crc16(data[2:]),
                    payload=data[2:], path='', sha256='')
    named = [dict(bank=0o24000, loc=0o2100, module=0x0301, selector=0)]
    for extra, records in (([parsed(0x0301, TASK, 200, 8)], named),
                           ([parsed(0x0301, TASK, 200, 8), parsed(0x0302, TASK, 60, 0)], []),
                           ([parsed(0x0A05, PINNED_TASK, 60, 0)], [])):
        try:
            build_image([parsed(0x0101, 0, 300, 16), parsed(0x0A04, PINNED_TASK, 150, 8), *extra], records,
                        mission, services)
        except ValueError:
            continue
        raise SystemExit(f'overlay_v2_pack: class 4 packed beside slot class {extra[0]["slot"]}')
    out_dir.mkdir(parents=True, exist_ok=True)
    paths = []
    for ident, slot, selectors, body, bss in [(0x0101, 0, 4, 300, 16), (0x0301, TASK, 1, 200, 8),
                                              (0x0A03, PINNED, 1, 120, 8), (0x0A04, PINNED_TASK, 1, 150, 8)]:
        p = out_dir / f'm{ident:04x}.prg'
        p.write_bytes(synthetic_module(ident, slot, mission, services, selectors, body, bss))
        paths.append(p)
    resolver = [dict(bank=0o24000, loc=0o2000, module=0x0101, selector=3)]
    (out_dir / 'resolver.json').write_text(json.dumps(resolver, indent=1) + '\n')
    return paths, out_dir / 'resolver.json'


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('modules', nargs='*', type=Path)
    p.add_argument('--mission', choices=('LM', 'CM'), default='LM')
    p.add_argument('--services', type=lambda v: int(v, 0), default=0x0201,
                   help='resident service-table version every module must import')
    p.add_argument('--resolver', type=Path)
    p.add_argument('--resident-bytes', type=int, default=26624)
    p.add_argument('--output', type=Path, default=Path('build/overlay-v2.reu'))
    p.add_argument('--manifest', type=Path)
    p.add_argument('--tail', type=Path, help='raw data the image ends with (see above)')
    p.add_argument('--selftest', type=Path, help='write synthetic modules here and pack them')
    p.add_argument('--selftest-pinned-task', type=Path,
                   help='write the synthetic class-4 store\'s modules here and pack them')
    args = p.parse_args()
    mission = 1 if args.mission == 'LM' else 2
    if args.selftest:
        args.modules, args.resolver = selftest(args.selftest, mission, args.services)
    if args.selftest_pinned_task:
        args.modules, args.resolver = selftest_pinned_task(args.selftest_pinned_task, mission, args.services)
    if not args.modules:
        p.error('no modules')
    modules = [parse_module(path, mission, args.services) for path in args.modules]
    resolver = json.loads(args.resolver.read_text()) if args.resolver else []
    tail = args.tail.read_bytes() if args.tail else b''
    image, placed = build_image(modules, resolver, mission, args.services, tail)
    kb = reu_size_kb(len(image))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    # VICE wants the image file to match the configured REU size.
    args.output.write_bytes(bytes(image) + bytes(kb * 1024 - len(image)))
    manifest = dict(schema_version=1, abi=ABI, mission=args.mission, services=args.services,
        reu_image=str(args.output), image_bytes=len(image), reu_size_kb=kb,
        image_sha256=hashlib.sha256(image).hexdigest(),
        modules=[{k: m[k] for k in ('id', 'slot', 'bytes', 'selectors', 'crc16', 'page', 'path', 'sha256')}
                 for m in placed],
        resolver_records=len(resolver), tail_bytes=len(tail),
        disk_plan=disk_plan(len(image), args.resident_bytes, args.mission))
    out = args.manifest or args.output.with_suffix('.json')
    out.write_text(json.dumps(manifest, indent=1) + '\n')
    print(f'{args.output}: {len(placed)} modules, {len(resolver)} resolver records, '
          f'{len(image)} image bytes, REU {kb} KB, {manifest["disk_plan"]["disks"]} D81')
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except ValueError as error:
        print(f'overlay_v2_pack: {error}', file=sys.stderr)
        sys.exit(1)
