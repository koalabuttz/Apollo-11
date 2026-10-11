# SPDX-License-Identifier: MIT
# Host-only retained qualification; no target or disk mutation.
.PHONY: p66-pending-port-test p66-pending-port-sanitize
p66-pending-port-test:
	python3 tools/p66_pending_port_host.py
p66-pending-port-sanitize:
	python3 tools/p66_pending_port_host.py --sanitize
