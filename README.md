## CUEParser
### Simple CUE sheet parser suitable for embedded systems written in C++.

To use in PlatformIO, add to the following to `lib_deps` in your `platformio.ini` file
```
lib_deps = CUEParser=https://github.com/rabbitholecomputing/CUEParser
```

Otherwise download the source and simply use `#include <CUEParser.h>`

For usage examples, checkout:
 - https://github.com/ZuluIDE/ZuluIDE-firmware
 - https://github.com/ZuluSCSI/ZuluSCSI-firmware

#### For further information on CUE sheets
 Refer to e.g. https://www.gnu.org/software/ccd2cue/manual/html_node/CUE-sheet-format.html#CUE-sheet-format

 Example of a CUE file:
```
 FILE "foo bar.bin" BINARY
   TRACK 01 MODE1/2048
     INDEX 01 00:00:00
   TRACK 02 AUDIO
     PREGAP 00:02:00
     INDEX 01 02:47:20
   TRACK 03 AUDIO
     INDEX 00 07:55:58
     INDEX 01 07:55:65
```

Authored by Petteri Aimonen \
Licensed under GPL3, see: [LICENSE.md](LICENSE.md) \
Copyright (c) 2023 Rabbit Hole Computing™