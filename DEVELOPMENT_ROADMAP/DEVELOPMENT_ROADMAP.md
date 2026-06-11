# Development Roadmap — OpenMotion
 
This file is your personal step-by-step build plan. Each phase has:
- **Goal** — what you're trying to achieve
- **Tasks** — concrete things to build
- **What to learn first** — where to find the knowledge you need, mapped to your books
- **Checkpoint** — how you know this phase is done
Tip: as you finish each phase, write a short note (3-5 sentences) about what you built and what problems you ran into. These notes become great material for your GitHub commit messages, your CV, and interview stories.
 
---
 
## Phase 1 — Hardware setup and wiring
 
**Goal:** Get your STM32F401 Nucleo physically wired to a stepper motor through the A4988/DRV8825 driver, and confirm you can spin the motor with simple test code.
 
**Tasks:**
- Wire the A4988/DRV8825 driver to the NEMA17 stepper motor
- Wire the driver's STEP, DIR, and ENABLE pins to GPIO pins on the STM32F401
- Connect the lab power supply to the driver (set voltage carefully, check the driver's current limit potentiometer first — this protects your motor and driver)
- Write a simple test program that toggles the STEP pin to make the motor spin slowly
- Document the wiring in `docs/wiring.md` (a simple table: signal name, F401 pin, driver pin)
**What to learn first:**
- This is mostly hardware/GPIO work, similar to what you already did in bare-metal programming. Refresh on GPIO output configuration using the STM32F401 reference manual (RM0368) — look at the GPIO chapter.
- A4988/DRV8825 datasheets explain the STEP/DIR/ENABLE logic — these are short and worth reading fully once.
**Checkpoint:** The motor spins continuously when you run your test program, and you can change direction by changing the DIR pin.
 
---
 
## Phase 2 — Design the serial communication protocol
 
**Goal:** Before writing any code for either board, design (on paper / in `docs/protocol.md`) the simple language the two boards will use to talk to each other.
 
**Tasks:**
- Decide on a simple message format, for example: `MOVE <motor_id> <steps> <speed>\n` or a small binary format with a command byte + parameters + checksum
- Define commands you'll need: MOVE, HOME, STOP, STATUS, GET_POSITION
- Define how the microcontroller replies: e.g., `OK`, `ERROR <code>`, or a position report
- Write this all down in `docs/protocol.md` — this document is also great for your portfolio, since it shows you can design clear interfaces
**What to learn first:**
- Keep it simple at first — a text-based protocol (human-readable) is easier to debug with a serial terminal, and you can switch to binary later if needed
- No specific book chapter needed here — this is a design exercise. If you want inspiration, look at how Marlin or Klipper define their G-code/command sets (just for inspiration, don't copy code)
**Checkpoint:** You have a written document describing every message type, with examples of what bytes/text are sent and received.
 
---
 
## Phase 3 — Real-time firmware on the STM32F401
 
**Goal:** The Nucleo board can receive commands over UART and generate precise step pulses for the motor, plus read the encoder and a limit switch.
 
**Tasks:**
- Set up a hardware timer in interrupt mode to generate step pulses at a controlled frequency (this controls motor speed)
- Set up UART receive (interrupt or DMA based) to read incoming commands from the Linux board
- Parse incoming commands using the protocol you designed in Phase 2, and translate them into "move N steps at frequency F"
- Set up the rotary encoder using the timer's encoder mode (STM32 timers can do this in hardware)
- Set up a GPIO interrupt for the limit switch
- Send back simple status replies over UART
**What to learn first:**
- STM32F401 reference manual (RM0368): chapters on **Timers** (for step generation and encoder mode), **UART/USART**, and **EXTI/GPIO interrupts**
- If you've done bare-metal before, this will feel familiar — the new parts are encoder mode and combining UART communication with real-time timing
- Optional: if you want to manage multiple things happening at once more cleanly, this is a good place to learn **FreeRTOS** basics (tasks, queues) — FreeRTOS has excellent free documentation and many STM32-specific tutorials
**Checkpoint:** You can send a text command like `MOVE 1 200 500` over a serial terminal (e.g., from your PC) and the motor moves 200 steps. Turning the encoder by hand prints position changes. Pressing the limit switch is detected.
 
---
 
## Phase 4 — Linux host application: G-code parser and motion planner
 
**Goal:** Write a C/C++ program that runs on Linux, reads a G-code file, calculates motion, and sends commands to the F401 over the serial link.
 
**Tasks:**
- Write a basic G-code parser: read a text file line by line, recognize simple commands (e.g., `G1 X10 Y5 F500` = move to X=10, Y=5 at feedrate 500)
- Write a basic motion planner: convert target positions and feedrates into step counts and speeds for each motor (start simple — constant speed; add acceleration/deceleration later)
- Open and configure the serial port to talk to the F401, send commands, and read replies
- Add basic error handling (what happens if the F401 doesn't respond?)
**What to learn first:**
- **The Linux Programming Interface (TLPI)** is your main resource here:
  - For serial port programming, look up "terminal I/O" / "termios" in the index — this covers how to configure a serial port (baud rate, etc.) on Linux
  - For reading files line by line and general file I/O, see the File I/O chapters
  - For handling signals cleanly (e.g., stopping safely on Ctrl+C), see the Signals chapters
- For the motion planning math (trapezoidal velocity profiles — accelerate, cruise, decelerate), this is closer to your mechanical engineering background — look up "trapezoidal motion profile" as a general robotics/CNC concept (not in your books, but well documented online and a great thing to implement and explain in your README)
**Checkpoint:** Running your program with a simple G-code file (a few `G1` move commands) results in the correct sequence of MOVE commands being sent to the F401, and the motor moves accordingly.
 
---
 
## Phase 5 — Custom Yocto image for the STM32MP257-DK
 
**Goal:** Build a custom embedded Linux image for the STM32MP257-DK that includes your host application from Phase 4, set to start automatically.
 
**Tasks:**
- Set up the Yocto build environment and build the base/reference image for the STM32MP257-DK first (don't customize anything yet — just get a working baseline image)
- Create your own Yocto layer (e.g., `meta-openmotion`) — never edit the vendor layers directly
- Write a recipe that builds your C/C++ host application from source and installs it into the image
- Add a systemd service file so your application starts automatically on boot
- Rebuild the image, flash it, and confirm your application runs on boot
**What to learn first:**
- **Mastering Embedded Linux Development (4th ed)** is your main resource:
  - Read the chapters on the Yocto Project itself (setting up the environment, building an existing image) before trying to customize anything
  - Then read the chapter(s) on creating your own layers and writing recipes — this is the core skill for this phase
  - The chapter on init systems / systemd will help with auto-starting your application
- Go slowly here — the first successful Yocto build (even unmodified) can take hours and uses a lot of disk space. Budget time for this.
**Checkpoint:** You flash an SD card, boot the STM32MP257-DK, and your application starts automatically (you can confirm with `systemctl status` or by checking logs).
 
---
 
## Phase 6 — Integration: first real movement end-to-end
 
**Goal:** Connect everything together — Linux board running your application, talking over UART to the F401, driving the real motor.
 
**Tasks:**
- Wire the UART connection between the STM32MP257-DK and the STM32F401 Nucleo
- Run your G-code file through the full pipeline: Linux parses it, plans motion, sends commands over UART, F401 moves the motor
- Use your **USB logic analyzer** to capture the UART traffic and verify the protocol is working as designed — this is also great documentation material (a screenshot of the logic analyzer capture in your README looks very professional)
- Debug any timing issues or dropped messages
**What to learn first:**
- This phase is mostly about debugging — the chapter on debugging tools in Mastering Embedded Linux Development covers useful Linux-side tools
- For the logic analyzer itself, tools like PulseView (sigrok) are commonly used with cheap USB logic analyzers and have good documentation online
**Checkpoint:** You can run a multi-line G-code file and watch the machine execute the full sequence of moves correctly.
 
---
 
## Phase 7 — Extra features: display, jog control, homing
 
**Goal:** Add the rotary encoder for manual jog control, the limit switch for homing, and the TFT display for status.
 
**Tasks:**
- Implement a "homing" routine: move toward the limit switch slowly until it triggers, then set that as position zero
- Implement "jog mode": when not running a G-code file, turning the encoder moves the motor manually
- Show status information (current position, state: idle/running/error) on the TFT display
**What to learn first:**
- If the display is driven from the Linux side via SPI, the device driver chapters in Mastering Embedded Linux Development cover how Linux talks to SPI devices, and how to use a framebuffer
- If the display is driven directly from the F401, this is similar to Phase 3 work (SPI peripheral in the reference manual)
**Checkpoint:** The machine can home itself, you can jog it manually with the encoder, and the display shows live status.
 
---
 
## Phase 8 — Vibration sensing with the MPU6050
 
**Goal:** Add the MPU6050 to read vibration data — this is a stepping stone toward Project 2 (predictive maintenance), but also useful here for detecting if the machine is shaking too much during fast moves.
 
**Tasks:**
- Connect the MPU6050 over I2C (to either board — Linux side is good practice for I2C drivers)
- Write code to read accelerometer values
- Log this data and observe how it changes at different motor speeds
**What to learn first:**
- Mastering Embedded Linux Development covers I2C device drivers and how Linux exposes I2C devices
- This phase is intentionally light — its main purpose is to get you comfortable with I2C before Project 2, where you'll do more with this sensor
**Checkpoint:** You can read and log accelerometer values while the motor is running.
 
---
 
## Phase 9 — Web interface (optional but high-impact)
 
**Goal:** Add a simple web page where you can upload a G-code file and see machine status, so the project can be controlled remotely — like a mini OctoPrint.
 
**Tasks:**
- Add a small web server to your Linux application (or run a separate lightweight server) that can receive a G-code file upload and show current status
- Display this in a simple web page (basic HTML is enough — this isn't a frontend project)
**What to learn first:**
- TLPI's chapters on sockets cover the fundamentals of network programming in C, if you want to build the server yourself
- Alternatively, many people build the web layer in Python (Flask) while keeping the core motion logic in C/C++ — this is a perfectly valid and common architecture, and worth mentioning in your README as a design decision
**Checkpoint:** You can upload a G-code file from a browser on your laptop and watch the machine execute it, with status visible on the web page.
 
---
