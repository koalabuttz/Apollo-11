# SPDX-License-Identifier: MIT
native_source_adoption-host:
	python3 tools/native_source_adoption_test.py
native_source_adoption-sanitize:
	python3 tools/native_source_adoption_test.py --sanitize
native_source_adoption-c64:
	python3 tools/native_source_adoption_build.py --cc $(MOS_CC)
.PHONY: native_source_adoption-host native_source_adoption-sanitize native_source_adoption-c64
