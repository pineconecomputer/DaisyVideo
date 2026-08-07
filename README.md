# DaisyVideo

Video coprocessor firmware for **Daisy**, a homebrew personal computer.
Runs on an Arduino Mega 2560 (ATmega2560) and generates a composite video
signal in software, driven entirely by cycle-counted timer interrupts.

There is no display hardware behind it: the NTSC signal, including sync,
blanking, and pixel output, is produced by the CPU. DaisyVideo holds the
framebuffer and renders it; DaisyOS sends screen updates and drawing
commands over a hardware UART.

## Role in the system

| Unit       | MCU         | Role                                          |
|------------|-------------|-----------------------------------------------|
| [DaisyOS](https://github.com/pineconecomputer/DaisyOS) | SAM3X (Due) | Brain. BASIC, editor, terminal, keyboard |
| DaisyVideo | ATmega2560  | This repo. Text + graphics, composite video   |
| [DaisySound](https://github.com/pineconecomputer/DaisySound) | ATmega328 | 3-voice synthesizer |

DaisyOS drives this board over its `Serial2` at 115200 baud using the
`video_messages` framing described in
[DaisyOS's ARCHITECTURE.md](https://github.com/pineconecomputer/DaisyOS/blob/main/ARCHITECTURE.md).

## Building

Requires [PlatformIO](https://platformio.org/install) and an Arduino Mega
2560. PlatformIO fetches the AVR toolchain on first build; there are no
library dependencies.

```sh
git clone https://github.com/pineconecomputer/DaisyVideo.git
cd DaisyVideo
pio run                  # build
pio run -t upload        # build and flash
```

Current footprint: ~20 KB flash (8% of 248 KB), ~6.9 KB RAM (85% of
8 KB), most of it framebuffer. `platformio.ini` enables
`--print-memory-usage`.

## Timing

The scanline generator is hand-tuned against the ATmega2560's cycle
timing. Instruction counts in the output loops, the `asm("nop")` padding,
and the empty `while` spin-waits on the USART flags all affect the
output signal.

## Layout

```
include/
  buffer.h        ring buffer for incoming serial commands
  pollserial.h    polled (non-interrupt) serial, to stay out of the ISR
  timer.h         timer helpers
src/
  main.cpp        NTSC signal generation, text/graphics rendering, command loop
  buffer.cpp
  pollserial.cpp
  timer.cpp
```

## Code style

Formatted to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
via the checked-in `.clang-format`:

```sh
clang-format -i $(find include src -name '*.cpp' -o -name '*.h')
```

Include sorting is off, since include order matters in Arduino sources.
`src/pollserial.cpp` is left unformatted as third-party-derived code.

## License

DaisyVideo is licensed under the **GNU General Public License, version 3**.
See [LICENSE](LICENSE) for the full text.

    Copyright (C) 2026 Joe Cassara

### Third-party code

`include/pollserial.h` and `src/pollserial.cpp` are heavily modified
derivatives of Arduino's `HardwareSerial`, Copyright (c) 2006 Nicholas
Zambetti, originally licensed under the **GNU LGPL version 2.1 or later**.
Their original notices are preserved verbatim at the top of both files and
must be kept there in any redistribution.

LGPL-2.1-or-later is compatible with GPLv3. The combined firmware is
distributed under GPLv3; those two files remain independently available
under their original LGPL terms.
