# SPDX-License-Identifier: MIT
.PHONY: p66-application-port-host p66-application-port-host-sanitize
p66-application-port-host:
	python3 tools/p66_application_port_host.py --cc "$(CC)"
p66-application-port-host-sanitize:
	python3 tools/p66_application_port_host.py --cc "$(CC)" --sanitize
