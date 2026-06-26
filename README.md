# OpenMotion

**A complete embedded Linux motion control system — built from scratch.**

A stepper motor controlled by bare-metal STM32F401RE firmware, commanded over a custom binary UART protocol, from a C application cross-compiled with Yocto and running natively on an STM32MP257F-DK industrial Linux board.

---

## What This Project Does

A G-code file (the same format used by 3D printers and CNC machines) is parsed on the Linux host. Each movement command is converted into a precise 10-byte binary packet and sent over USB-UART to an STM32F401RE Nucleo board, which pulses a stepper motor driver to physically move a motor.

Everything was written from scratch: the bare-metal firmware drivers, the binary protocol, the Linux C application, and the Yocto layer that cross-compiles and bakes the application into the embedded Linux image.

---

## System Architecture

```
┌──────────────────────────────────┐     USB-UART (115200 8N1)    ┌──────────────────────────────┐
│   STM32MP257F-DK                 │ ── 10-byte binary packet ──► │   STM32F401RE Nucleo         │
│   (ST OpenSTLinux Weston image)  │ ◄── ACK + COMPLETE bytes ─── │   (Bare-metal C firmware)    │
│                                  │                               │                              │
│  openmotion-host (C app)         │                               │  Custom register-level       │
│  ├─ G-code parser                │                               │  drivers (no HAL):           │
│  ├─ Motion planner               │                               │  ├─ UART (interrupt-driven)  │
│  └─ Protocol serializer          │                               │  ├─ TIM3 (step pulses)       │
└──────────────────────────────────┘                               │  └─ GPIO (direction pin)     │
                                                                   └──────────────┬───────────────┘
                                                                                  │ STEP / DIR pins
                                                                                  ▼
                                                                       [ Stepper Motor Driver ]
                                                                                  │
                                                                                  ▼
                                                                          [ Stepper Motor ]
```

---

## Project Structure

```
OpenMotion/
├── firmware/               # Bare-metal STM32F401RE firmware (STM32CubeIDE)
│   ├── Src/
│   │   ├── main.c              # Protocol parser + command dispatcher
│   │   ├── uart_driver.c       # UART driver — interrupt-driven RX buffer
│   │   ├── timer_driver.c      # TIM3 step pulse generator
│   │   ├── gpio_driver.c       # Direction pin control
│   │   └── protocol.c          # 10-byte packet decoder
│   └── Inc/
│       ├── uart_driver.h
│       ├── timer_driver.h
│       ├── gpio_driver.h
│       └── protocol.h
├── host/                   # Linux host application (C)
│   ├── main.c                  # G-code parser + motion planner + serial sender
│   └── Makefile
├── meta-openmotion/        # Custom Yocto layer
│   ├── conf/layer.conf
│   ├── recipes-openmotion/openmotion-host/
│   │   ├── openmotion-host_1.0.bb
│   │   └── files/
│   │       ├── main.c
│   │       └── Makefile
│   └── recipes-st/
│       └── images
|   	    |__ st-image-weston.bbappend
├── docs/
│   ├── protocol.md             # Binary protocol specification
│   ├── wiring.md               # Hardware wiring guide
│   
└── tools/
    └── test_motion.py          # Python test script
```

---

## The Protocol

A custom 10-byte big-endian binary protocol over UART at 115200 baud (8N1).

### MOVE Command (Host → STM32)

| Byte(s) | Field      | Description                                     |
|---------|------------|-------------------------------------------------|
| 0       | Command ID | `0x01` = MOVE                                   |
| 1       | Direction  | `0x01` = Forward, `0x00` = Backward             |
| 2–5     | Step Count | `uint32_t`, big-endian — total steps to pulse   |
| 6–9     | Step Delay | `uint32_t`, big-endian — microseconds per pulse |

### Response (STM32 → Host)

| Code   | Meaning                                       |
|--------|-----------------------------------------------|
| `0x55` | `STATUS_OK` — command accepted, motor running |
| `0x77` | `STATUS_MOVE_COMPLETE` — motion finished      |
| `0x99` | `ERR_INVALID_COMMAND` — unknown command ID    |
| `0xAA` | `ERR_INVALID_PARAMETER` — value out of range  |

### Why Big-Endian?
Serial lines carry one byte at a time. The 32-bit step count and delay values are split across 4 bytes using bit shifts (`>> 24`, `>> 16`, `>> 8`). Big-endian (MSB first) keeps the byte order predictable on both sides and matches network byte order convention.

### Synchronization
The host performs two blocking reads per command — the first waits for `STATUS_OK` (command accepted), the second waits for `STATUS_MOVE_COMPLETE` (motor physically finished). This guarantees the host never sends a second command while the motor is still running.

---

## Motion Planning Math

The host app converts G-code coordinates and feedrates into hardware step parameters:

```
Steps      = distance_mm × STEPS_PER_MM          (configured: 80 steps/mm)
Step_freq  = (feedrate_mm_min ÷ 60) × STEPS_PER_MM
Delay_us   = 1,000,000 ÷ Step_freq
```

**Example:** `G1 X10.5 F600`
- Distance: 10.5 mm → **840 steps**
- Feedrate: 600 mm/min → step frequency = 800 Hz → **delay = 1250 µs**

---

## Firmware Drivers (Bare-Metal, No HAL)

All drivers were written directly against the STM32F401RE reference manual — no STM32 HAL, no CubeMX-generated code, no BSP abstractions. Direct register access only.

### UART Driver
- Configured at 115200 baud via direct `USART2` register writes
- Interrupt-driven RX — incoming bytes land in a buffer through `USART2_IRQHandler`
- Blocking TX for sending ACK and status response bytes back to the host

### Timer Driver (TIM3)
- Configured in output compare mode to generate timed STEP pulses
- The `delay_us` value from the protocol packet is loaded directly into the timer ARR register
- Counts down the total step count and sends `STATUS_MOVE_COMPLETE` (`0x77`) when done

### GPIO Driver
- Direct register writes to `GPIOA`
- Controls the motor direction pin (High = Forward, Low = Backward)
- STEP pin toggled on each TIM3 interrupt

---

## Yocto Layer: `meta-openmotion`

A custom OpenEmbedded layer that cross-compiles the host C application for the ARM Cortex-A35 inside the STM32MP257F-DK and includes it in the Linux image.

**Layer structure:**
```
meta-openmotion/
├── conf/layer.conf
├── recipes-openmotion/openmotion-host/
│   ├── openmotion-host_1.0.bb      # BitBake recipe
│   └── files/
│       ├── main.c
│       └── Makefile
└── recipes-st/
    └── images
	|___ st-image-weston.bbappend
```

---

## Hardware

| Component             | Part                                              |
|-----------------------|---------------------------------------------------|
| Linux SoC Board       | STM32MP257F-DK                                    |
| Microcontroller Board | STM32F401RE Nucleo                                |
| Motor Driver          | A4988 / DRV8825                                   |
| Motor                 | NEMA 17 Stepper                                   |
| Connection            | USB (MP257 USB Host port → Nucleo ST-LINK USB)    |

---

## How to Reproduce

### 1. Flash the Firmware

Open the `firmware/` folder in STM32CubeIDE, build, and flash to the STM32F401RE Nucleo board.

### 2. Set Up the ST OpenSTLinux Yocto Environment

Follow ST's official setup guide to download the OpenSTLinux Yocto distribution for the STM32MP257F-DK. Once downloaded, source the ST environment setup script:

```bash
source layers/meta-st/scripts/envsetup.sh
```

This opens an interactive menu — select the `stm32mp25-disco` build configuration. It will set up and switch into the build directory automatically.

### 3. Add meta-openmotion to Your Build

Clone this repo and copy `meta-openmotion/` into your Yocto workspace. Then register the layer with BitBake:

```bash
bitbake-layers add-layer /path/to/meta-openmotion
```

This automatically adds the layer to `bblayers.conf`.

### 4. Build and Flash

**Build the image:**
```bash
bitbake st-image-weston
```

**Flash the image to an SD card** using `dd` (replace `mmcblk0` with your device — could be `sda`, `sdb`, etc.):
```bash
sudo dd if=flashlayout_st-image-weston/optee/../../FlashLayout_sdcard_stm32mp257f-dk-optee.raw \
        of=/dev/mmcblk0 \
        bs=8M conv=fdatasync status=progress
```

> **Note:** Replace `/dev/mmcblk0` with the correct device for your SD card reader. Use `lsblk` before and after inserting the card to identify it. Common alternatives: `/dev/sda`, `/dev/sdb`, `/dev/sdc`. **Double-check before running — dd will overwrite whatever device you point it at.**

Insert the SD card into the STM32MP257F-DK and power on.

### 5. Run on the Board

Connect the Nucleo to the MP257 USB Host port, then on the MP257:

```bash
openmotion-host /dev/ttyACM0 test.gcode
```

### G-code Format

```gcode
; Move forward 10mm at 600 mm/min
G1 X10.0 F600
; Move backward 5mm at 1200 mm/min
G1 X-5.0 F1200
```

---

## Development Notes

Issues and workarounds encountered during the build process — kept here in case they help someone else.

### Ubuntu 24.04 LTS: BitBake AppArmor Error

On Ubuntu 24.04, BitBake can fail with a namespace-related permission error due to a stricter AppArmor policy. Fix it by running:

```bash
echo 0 | sudo tee /proc/sys/kernel/apparmor_restrict_unprivileged_userns
```

This disables the restriction on unprivileged user namespaces for the current session. You may need to re-run it after a reboot.

### Pre-fetching All Sources Before Building

If you want to download all source tarballs first without compiling anything (useful on slow or metered connections, or to verify you can fetch everything before a long build), run:

```bash
bitbake st-image-weston --runall=fetch
```

BitBake will walk the entire dependency graph of the image and fetch every source file it needs, without running any compile tasks.

---



This is a learning project, not a production system. Known limitations:

- **Single axis only.** The system controls one stepper motor on one axis. A real CNC or 3D printer requires coordinated multi-axis motion — that would need a motion queue, lookahead, and synchronized multi-channel timers.
- **No acceleration/deceleration.** The motor starts and stops at full speed. Real motion controllers use trapezoidal or S-curve velocity profiles to avoid missed steps and mechanical stress.
- **Bare-metal by design.** The STM32 firmware uses no HAL or RTOS — this was intentional to practice direct register programming. In a production system you would likely use FreeRTOS for command queuing and HAL for portability.
- **Blocking protocol.** The host waits for each move to fully complete before sending the next one. A production system would use a command queue and non-blocking I/O.
- **No position tracking.** There is no encoder feedback — the system is open-loop. If the motor misses steps, there is no way to detect or recover from it.

---

## What I Learned

- Writing bare-metal C drivers from register-level documentation — UART, timers, GPIO — with no HAL
- Designing a binary communication protocol and implementing it end-to-end on two different processors
- Linux serial programming with `termios` — raw mode, why `ICANON` breaks binary reads, why `ICRNL` corrupts packets
- The Yocto build system — layers, BitBake recipes, `bblayers.conf`, cross-compilation for ARM
- Debugging across a custom Linux image, a serial protocol, bare-metal firmware, and real hardware simultaneously

---

## Author

**Abdelrahman**
Embedded Linux & Firmware Engineer — open to opportunities

[LinkedIn](https://www.linkedin.com/in/abdelrahman-abdellatif-93371a405/) · [GitHub](#)
