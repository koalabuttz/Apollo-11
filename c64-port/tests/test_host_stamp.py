#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/flight_store_build.py host object stamps (#810): a host/*.o is
stale when its compiler, an input's text or the object's own bytes change,
so a store rebuild can trust objects already in build/ instead of wiping
it."""
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import flight_store_build as fsb  # noqa: E402

CC = os.environ.get('HOST_CC', 'cc')


@unittest.skipUnless(shutil.which(CC), f'no {CC}')
class Stamp(unittest.TestCase):
    def setUp(self):
        self.d = pathlib.Path(tempfile.mkdtemp())
        (self.d / 'src').mkdir()
        (self.d / 'include').mkdir()
        (self.d / 'include/a.h').write_text('#define A 1\n')
        (self.d / 'src/a.c').write_text('#include "a.h"\nint a(void) { return A; }\n')
        self.root, fsb.ROOT = fsb.ROOT, self.d
        self.obj, self.dep = self.d / 'a.o', self.d / 'a.d'
        subprocess.run([CC, '-Iinclude', '-MMD', '-MF', 'a.d', '-c', 'src/a.c', '-o', 'a.o'], cwd=self.d, check=True)
        self.obj.with_suffix('.sha').write_text(fsb.host_stamp('a', self.obj, self.dep, CC) + '\n')

    def tearDown(self):
        fsb.ROOT = self.root
        fsb._CC_IDS.pop('fake-cc', None)

    def test_fresh_object_is_current(self):
        self.assertTrue(fsb.stamp_ok(self.obj, CC))

    def test_object_bytes_changed(self):
        self.obj.write_bytes(self.obj.read_bytes() + b'\0')
        self.assertFalse(fsb.stamp_ok(self.obj, CC))

    def test_header_text_changed(self):
        (self.d / 'include/a.h').write_text('#define A 2\n')
        self.assertFalse(fsb.stamp_ok(self.obj, CC))

    def test_other_compiler(self):
        fsb._CC_IDS['fake-cc'] = '/usr/bin/fake-cc\nfake 1.0\n'
        self.assertFalse(fsb.stamp_ok(self.obj, 'fake-cc'))

    def test_old_one_field_stamp_rebuilds(self):
        self.obj.with_suffix('.sha').write_text(fsb.host_inputs_hash('a', self.dep, CC) + '\n')
        self.assertFalse(fsb.stamp_ok(self.obj, CC))

    def test_missing_input(self):
        (self.d / 'include/a.h').unlink()
        self.assertFalse(fsb.stamp_ok(self.obj, CC))


if __name__ == '__main__':
    unittest.main()
