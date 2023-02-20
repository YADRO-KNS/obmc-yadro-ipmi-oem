# YADRO IPMI OEM support library

This component is intended to provide YADRO-specific IPMI command handlers for OpenBMC,

## Overview

`obmc-yadro-ipmi-oem` serves as an extension`[1]` to OpenBMC IPMI daemon`[2]`. It is
compiled as a shared library and intended to provide implementation for non-standard
OEM extensions.

## Capabilities

Related features provided by the library are grouped in separate source files.
Main extensions to vanilla OpenBMC IPMI stack are the following:

- SMBIOS dump (processed by MDR parser);
- Connected storage list.
- Report PCIe bifurcation mode to UEFI

## References

1. [OpenBMC IPMI Architecture](https://github.com/openbmc/docs/blob/master/architecture/ipmi-architecture.md)
2. [Phosphor IPMI Host](https://github.com/openbmc/phosphor-host-ipmid)
