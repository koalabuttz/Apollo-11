# SPDX-License-Identifier: MIT
# tools/flight_store_build.py host object stamps (compiler, inputs, object
# bytes), tested on a scratch tree.
.PHONY: test-host-stamp
test: test-host-stamp
test-host-stamp:
	python3 tests/test_host_stamp.py
