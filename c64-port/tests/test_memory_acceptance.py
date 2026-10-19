#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/memory_acceptance.py on made-up runs, linker scripts and stores."""
import pathlib
import re
import shutil
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import memory_acceptance as ma  # noqa: E402
import reu_layout  # noqa: E402

ARMED_LOG = ''.join(f'TRACE: {n}  C:$df01  (Trace store)\nSetting checkpoint {n} condition to: x\n' for n in (2, 3, 4))
# The bundle's pattern for an image a doc names (tools/acceptance_bundle.py IMAGE).
IMAGE = re.compile(r'(flight-(?:store|disk)/[A-Za-z0-9][A-Za-z0-9._-]*\.(?:reu|d81|d64|prg))\b[^\n]*?\b([0-9a-f]{64})\b')


def run(replay=True, want=200, reached=200, ended=True, armed=True, ok=True, outside=0):
    r = dict(scenario='lm-p63-descent-thrust', snap_cs=20010, requested_cs=want, replay=replay,
             reu_bounds=dict(ok=ok, dma_outside=outside, dma_trace_armed=armed, outside=[], touched=['scratch']))
    if replay:
        r.update(replay_ended=ended, replay_end_cs=reached, agc_cs=want + 500)
    else:
        r.update(agc_cs=reached)
    return r


def manifest(sizes):
    """sizes: {slot class: largest module bytes}."""
    return dict(modules=[dict(slot=s, bytes=b) for s, b in sizes.items()], rope_reu_base='0x1e0000',
                image_bytes=0x1000, reu_size_kb=2048, resolver_records=1)


class Runs(unittest.TestCase):
    def test_full_armed_run_passes(self):
        self.assertEqual(ma.run_verdict(run()), (True, ''))
        self.assertTrue(ma.run_verdict(run(replay=False))[0])

    def test_short_run_fails(self):
        ok, why = ma.run_verdict(run(reached=3))
        self.assertFalse(ok)
        self.assertIn('covered 3 of 200', why)
        self.assertFalse(ma.run_verdict(run(ended=False))[0])
        self.assertFalse(ma.run_verdict(run(replay=False, reached=3))[0])

    def test_unarmed_trace_fails(self):
        ok, why = ma.run_verdict(run(armed=False))
        self.assertFalse(ok)
        self.assertIn('not armed', why)
        r = run()
        del r['reu_bounds']['dma_trace_armed']
        self.assertFalse(ma.run_verdict(r)[0])

    def test_other_commit_or_store_fails(self):
        r = dict(run(), commit='abc123', store_sha256='s' * 64)
        self.assertTrue(ma.run_verdict(r, 'abc123', 's' * 64)[0])
        ok, why = ma.run_verdict(r, 'def456', 's' * 64)
        self.assertFalse(ok)
        self.assertIn('not def456', why)
        self.assertFalse(ma.run_verdict(r, 'abc123', 't' * 64)[0])
        self.assertFalse(ma.run_verdict(dict(run(), commit=None), 'abc123', None)[0])

    def test_host_match_shown(self):
        self.assertEqual(ma.host_match(dict(run(), replay_differences=0)), 'yes')
        self.assertIn('61 differences', ma.host_match(dict(run(), replay_differences=61, replay_differ_at_cs=150)))
        self.assertEqual(ma.host_match(run(replay=False)), 'not compared')

    def test_outside_fails(self):
        self.assertFalse(ma.run_verdict(run(ok=False, outside=3))[0])
        self.assertFalse(ma.run_verdict(dict(run(), reu_bounds=None))[0])

    def test_control_must_fail(self):
        self.assertTrue(ma.control_verdict(run(ok=False, outside=48))[0])
        self.assertFalse(ma.control_verdict(run())[0], 'a control with nothing outside proves nothing')
        self.assertFalse(ma.control_verdict(run(ok=False, outside=48, armed=False))[0])
        self.assertFalse(ma.control_verdict(None)[0])

    def test_trace_armed(self):
        self.assertTrue(reu_layout.trace_armed(ARMED_LOG))
        self.assertFalse(reu_layout.trace_armed(ARMED_LOG.replace('TRACE: 3', 'TRACE: 9')))
        self.assertFalse(reu_layout.trace_armed(''))


class Rope(unittest.TestCase):
    def test_header_gives_flight_base(self):
        if shutil.which(ma.CC) is None:
            self.skipTest(f'no {ma.CC}')
        self.assertEqual(ma.rope_base_c64('lm'), 0x1e0000)
        self.assertEqual(ma.rope_base_c64('cm'), 0x1e0000)

    def test_cross_check(self):
        regions = [('rope', 0x1e0000, 0x1f2000, False)]
        self.assertTrue(ma.rope_check('lm', manifest({}), regions, read=lambda m: 0x1e0000)[1])
        self.assertFalse(ma.rope_check('lm', manifest({}), regions, read=lambda m: 0x1c0000)[1])
        self.assertFalse(ma.rope_check('lm', manifest({}), [('rope', 0x1c0000, 0x1d2000, False)],
                                         read=lambda m: 0x1e0000)[1])
        self.assertFalse(ma.rope_check('lm', manifest({}), regions, read=lambda m: None)[1])


class Ram(unittest.TestCase):
    def setUp(self):
        self.tmp = pathlib.Path(tempfile.mkdtemp())
        self.platform = self.tmp / 'platform'
        shutil.copytree(ma.PLATFORM, self.platform, ignore=shutil.ignore_patterns('*.c', '*.s', '*.h'))
        self.header = self.tmp / 'overlay_v2.h'
        shutil.copy(ma.OV2, self.header)

    def tearDown(self):
        shutil.rmtree(self.tmp)

    def failed(self, sizes={0: 16000, 1: 8000, 2: 4000, 3: 2000}):
        _, checks = ma.ram_layout(manifest(sizes), self.platform, self.header)
        return [c[0] for c in checks if not c[1]]

    def edit(self, name, old, new):
        path = self.platform / name
        text = path.read_text()
        self.assertIn(old, text)
        path.write_text(text.replace(old, new))

    def test_committed_layout_passes(self):
        self.assertEqual(self.failed(), [])
        self.assertEqual(self.failed({0: 16000, 2: 4000, 3: 2000, 4: 8000}), [])

    def test_module_over_its_slot(self):
        self.assertIn('class 0 job slot: largest module fits', self.failed({0: 0x4001, 3: 10}))

    def test_slot_grown_into_c000(self):
        self.edit('overlay_v2_slot_job.ld', 'LENGTH = 0x4000', 'LENGTH = 0x4400')
        bad = self.failed()
        self.assertIn('class 0 job slot: linker script matches include/overlay_v2.h', bad)
        self.assertIn('slots do not overlap', bad)

    def test_slot_in_io_area(self):
        self.edit('overlay_v2_slot_pinned.ld', 'ORIGIN = 0xc000', 'ORIGIN = 0xd800')
        self.assertIn('class 3 pinned slot: I/O area $D000-$DFFF only for class 2', self.failed())

    def test_slot_over_vectors(self):
        self.edit('overlay_v2_slot_task.ld', 'LENGTH = 0x1ffa', 'LENGTH = 0x2000')
        self.assertIn('class 1 task slot: below the vectors and RAM end', self.failed())

    def test_header_and_script_agree(self):
        text = self.header.read_text().replace('#define APOLLO_OV2_PINNED_BYTES 0x0c00u',
                                               '#define APOLLO_OV2_PINNED_BYTES 0x0800u')
        self.header.write_text(text)
        self.assertIn('class 3 pinned slot: linker script matches include/overlay_v2.h', self.failed())


class Doc(unittest.TestCase):
    def setUp(self):
        self.tmp = pathlib.Path(tempfile.mkdtemp())
        self.saved = ma.ROOT, ma.STORE
        ma.ROOT, ma.STORE = self.tmp, self.tmp / 'build/flight-store'
        ma.STORE.mkdir(parents=True)
        (ma.STORE / 'lm.reu').write_bytes(b'\0' * 64)

    def tearDown(self):
        ma.ROOT, ma.STORE = self.saved
        shutil.rmtree(self.tmp)

    def doc(self, control_ok=True, run_ok=True):
        row = dict(scenario='s', snap_cs=1, requested_cs=200, agc_cs=200, armed=True, replay=True, touched=[],
                   outside=0, dma_outside=0, dma_late=0, ok=run_ok, why='' if run_ok else 'covered 3 of 200 cs',
                   evidence='evidence/lm-acceptance-memory/s-1.json', sha256='a' * 64)
        control = dict(ok=control_ok, detail='48 DMAs outside', scenario='s', snap_cs=1, agc_cs=200,
                       evidence='evidence/lm-acceptance-memory/control-s-1.json', sha256='b' * 64)
        sizes = dict(resident_bytes=1, resident_capacity=2, free_bytes=1, components={'k': 1},
                     ram=[('class 0 job slot', 0x8000, 0xC000, 16000)])
        path, ok = ma.write_doc('lm', [('x', True, 'd')], manifest({0: 1}), [('store', 0, 16, False)], sizes,
                                [row], 'abc123', control)
        return path.read_text(), ok

    def test_one_commit_and_full_store_sha(self):
        text, ok = self.doc()
        self.assertTrue(ok)
        self.assertEqual(IMAGE.findall(text), [('flight-store/lm.reu', ma.sha(ma.STORE / 'lm.reu'))])
        self.assertEqual(re.findall(r'commit `([0-9a-f]+)', text), ['abc123'])
        self.assertNotIn('built at', text)
        self.assertNotIn('48 DMAs outside and fails', text.split('## Control')[0])

    def test_control_or_short_run_fails_the_doc(self):
        self.assertFalse(self.doc(control_ok=False)[1])
        text, ok = self.doc(run_ok=False)
        self.assertFalse(ok)
        self.assertIn('FAIL: covered 3 of 200 cs', text)


if __name__ == '__main__':
    unittest.main()
