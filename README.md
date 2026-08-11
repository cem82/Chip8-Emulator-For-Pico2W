# CHIP-8 Emulator on Raspberry Pi Pico 2

A CHIP 8 emulator written in C and running on a Raspberry Pi Pico 2 with an SSD1306 display.

The project was built as a hands on introduction to low level programming, CPU architecture, memory, registers, bitwise operations, and embedded systems.

The emulator is capable of running CHIP 8 programs directly on the Pico 2. Currently has Breakout in it but is open to new games. (Though I only have 2 buttons currently)

## Features

CHIP 8 CPU emulation

4 KB CHIP-8 memory

16 general-purpose 8-bit registers (`V0`–`VF`)

16-bit program counter

16-level stack

16-bit `I` register

CHIP 8 delay and sound timers

64×32 CHIP-8 display

CHIP 8 keypad input

Sprite rendering with XOR drawing and collision detection

Built in CHIP-8 font data

Embedded CHIP 8 ROM

SSD1306 output over I2C

Physical button input

Runs on Raspberry Pi Pico 2W

## Hardware

Raspberry Pi Pico 2W

SSD1306 128×64 OLED display

2 push buttons

Jumper wires / breadboard

The OLED is connected through I2C. The current implementation uses GPIO 0 for SDA and GPIO 1 for SCL.

### Controls

| Button | CHIP-8 Key |
| ------ | ---------- |
| Left   | `4`        |
| Right  | `6`        |

The current implementation maps the physical buttons to CHIP 8 keys `4` and `6`.

## CHIP 8 Architecture

The emulator represents the main CHIP 8 components with a `Chip8` structure:

```text
Chip8
├── Memory      4096 bytes
├── V Registers 16 × 8-bit
├── PC          16-bit
├── Stack       16 × 16-bit
├── I Register  16-bit
├── Display     64 × 32
├── Delay Timer 8-bit
├── Sound Timer 8-bit
└── Keypad      16 keys
```

These components are represented directly in the emulator's state structure.

## CPU Cycle

Each CHIP-8 instruction goes through the classic:

```text
FETCH
  ↓
DECODE
  ↓
EXECUTE
  ↓
NEXT INSTRUCTION
```

The emulator fetches two bytes from memory, combines them into a 16 bit opcode, advances the program counter, and then decodes the opcode.

For example:

```text
0x600A
```

is decoded as:

```text
6XNN
```

which means:

```text
VX = NN
```

The emulator extracts the different parts of the opcode using bitwise operations and executes the corresponding instruction.

## Opcode Support

The emulator implements the main CHIP 8 instruction groups, including:

`6XNN` Set register

`7XNN` Add value to register

`3XNN` / `4XNN` Conditional skips

`5XY0` / `9XY0` Register comparisons

`1NNN` Jump

`2NNN` Subroutine call

`00E0` Clear screen

`00EE` Return from subroutine

`8XYN` Arithmetic and bitwise operations

`ANNN` Set `I` register

`BNNN` Jump with offset

`CXNN` Random number

`DXYN` Sprite drawing and collision detection

`EX9E` / `EXA1` Key handling

`FX07`, `FX15`, `FX18` Timer operations

`FX0A` Wait for key press

`FX1E` Add register to `I`

`FX29` Font character address

`FX33` Binary-coded decimal conversion

`FX55` / `FX65` Memory/register transfers

The instruction decoder is implemented using a `switch` structure based on the first hexadecimal digit of the opcode, with nested decoding for instruction groups such as `8XYN`, `EXNN`, and `FXNN`.

## Graphics

CHIP 8 uses a 64×32 monochrome display.

The emulator stores the display as:

```c
uint8_t screen[32][64];
```

Sprites are read from memory starting at the address stored in `I`.

Each sprite byte represents 8 horizontal pixels. Drawing is performed using XOR, allowing the emulator to detect collisions through the `VF` register.

The CHIP 8 display is then scaled to fit the 128×64 OLED by drawing each CHIP 8 pixel as a 2×2 block.

```text
CHIP-8              OLED

64 × 32     →      128 × 64

  ██                  ████
  ██                  ████
```

## Input

The emulator maintains a 16 key CHIP 8 keypad:

```c
uint8_t keys[16];
```

The physical buttons are read using the Pico SDK's GPIO functions and mapped to CHIP 8 keypad values.

## Timers

CHIP 8 provides two timers:

Delay Timer

Sound Timer

Both timers are decremented while they are above zero.

## Memory Layout

The emulator uses the standard CHIP 8 program start address:

```text
0x000
System / reserved

0x050
Font data

0x200
CHIP-8 program
ROM

0xFFF
```

The built in font is loaded at `0x50`, while the ROM is loaded starting at `0x200`.

## Running a CHIP-8 Program

The current implementation embeds the CHIP 8 ROM directly into the firmware as a byte array and copies it into CHIP 8 memory at runtime.

A complete game of Breakout was successfully tested on the emulator.

## Build

This project uses the Raspberry Pi Pico SDK and the Pico C toolchain.

The firmware uses:

C

Raspberry Pi Pico SDK

I2C

SSD1306 driver

Hardware GPIO

`stdint.h`

Standard C library

The OLED is initialized through the I²C peripheral at 400 kHz.

Build the project using the standard Pico SDK CMake workflow, then flash the resulting firmware to the Pico 2.

## Project Structure

```text
CHIP-8-Pico2/
├── CMakeLists.txt
├── main.c
├── ssd1306.c
├── ssd1306.h
└── README.md
```

## What I Learned

This project was primarily built as a learning project for embedded systems and low-level programming.

Binary and hexadecimal representation

Bitwise operators

Bit masking

Bit shifting

Fixed-width integer types

Registers

Memory addressing

Program counters

Stack-based subroutines

CPU instruction decoding

Fetch Decode Execute architecture

Emulator design

Embedded C

GPIO

I2C communication

OLED display control

Hardware input handling

## Future Improvements

Support for more physical CHIP-8 keys

A complete 16-button keypad

Better timing accuracy

Sound output

External ROM loading

Multiple ROM support

More CHIP-8 compatibility testing

A custom PCB

A dedicated handheld enclosure

## Demo

A playable CHIP-8 **Breakout** game is currently running on the Raspberry Pi Pico 2 with the SSD1306 OLED display.

<img width="595" height="795" alt="image" src="https://github.com/user-attachments/assets/01eb28a3-8f44-4f86-a3e6-1125d9250b67" />

<img width="442" height="791" alt="image" src="https://github.com/user-attachments/assets/dac15120-1d95-4d9a-974e-d7d3d8a1735a" />



https://github.com/user-attachments/assets/2ab30b52-054f-4e9b-bafb-76a39644e9f9



## AI Disclosure

AI tools were used as a learning and development aid during this project.

**Learning:** AI was used to help explain concepts related to hexadecimal and binary representation, bitwise operations, CPU architecture, and CHIP-8 emulation.

**Porting:** AI assisted with adapting the emulator from a desktop environment to the Raspberry Pi Pico 2 and its hardware peripherals.

**ROM preparation:** AI assisted with converting and preparing ROM data so it could be embedded into the Pico 2 firmware.

**Documentation:** A large portion of this README was initially generated with AI assistance. I reviewed, edited, and verified the content before including it in the repository.

Rest of the code, structure, comments were all made by me (mostly on paper) 

The emulator's concepts, implementation, debugging, and hardware integration were studied and tested during the development process. AI was used as an assistant rather than as a replacement for understanding or testing the code.

This project is primarily intended as an educational and experimental project.
