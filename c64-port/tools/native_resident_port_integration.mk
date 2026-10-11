# SPDX-License-Identifier: MIT
# Additive host-only targets. Never rebuild the qualified PRGs or disks.
.PHONY: native-resident-port-test native-resident-port-sanitize
native-resident-port-test:
	python3 tools/native_resident_port_host.py
native-resident-port-sanitize:
	python3 tools/native_resident_port_host.py --sanitize
