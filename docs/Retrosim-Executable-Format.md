## RetroSim Executable Format

### Version 0

This format is only for very simple scenarios: mostly testing and virtual machines without OS.

No distinction of code/data.

#### File Format

#### Header
    3 bytes: 52 53 58 ("RSX") [Magic bytes]
    1 byte:  ASCII '0' [Version number]

#### Chunks
List of chunks consisting of:

    4 bytes: Address
    4 bytes: Length
    x bytes: Data of length x
