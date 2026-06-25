## 🔌 Hardware Wiring & Connections Diagram

To keep the physical setup organized, use the following pin connection map between the Linux Host, the STM32F401RE Microcontroller, the DRV8825 stepper driver, and the power delivery blocks.

### 📋 Main Interconnection Table

| From Device | From Pin / Interface | To Device | To Pin / Interface | Description / Purpose |
| :--- | :--- | :--- | :--- | :--- |
| **Linux Host** | USB Type-A Port | **STM32F401RE** | On-board Mini/Micro-USB | Serial Comm Pipeline (`/dev/ttyACM0`) & Logic Power |
| **STM32F401RE** | `PA4` (GPIO Output) | **DRV8825 Driver** | `STEP` Pin | Generates high-frequency step pulse clocks |
| **STM32F401RE** | `PA5` (GPIO Output) | **DRV8825 Driver** | `DIR` Pin | Toggles directional rotation polarity (High/Low) |
| **STM32F401RE** | `GND` | **DRV8825 Driver** | `GND` (Logic Ground) | Establishes a common ground reference for signals |
| **DRV8825 Driver**| `RST` (Reset) | **DRV8825 Driver** | `SLP` (Sleep) | **Jumper Wire:** Bridges pins together to enable driver |
| **DRV8825 Driver**| `A1` (Output) | **Stepper Motor** | Coil A+ (Black / Pair 1) | Powers Motor Phase A positive leg |
| **DRV8825 Driver**| `A2` (Output) | **Stepper Motor** | Coil A- (Green / Pair 1) | Powers Motor Phase A negative leg |
| **DRV8825 Driver**| `B1` (Output) | **Stepper Motor** | Coil B+ (Red / Pair 2)   | Powers Motor Phase B positive leg |
| **DRV8825 Driver**| `B2` (Output) | **Stepper Motor** | Coil B- (Blue / Pair 2)  | Powers Motor Phase B negative leg |
| **Power Supply** | `VMOT` ($8\text{V} - 45\text{V}$) | **DRV8825 Driver** | `VMOT` Input | High-voltage motor power source input rail |
| **Power Supply** | `GND` | **DRV8825 Driver** | `GND` (Power Ground) | High-current power return loop ground point |

---

### ⚠️ Critical Hardware Connection Rules

> 🛑 **IMPORTANT safety check before flipping the switch:**
> 1. **Never disconnect the stepper motor while the driver is powered!** Unplugging the motor wires while running will generate massive inductive voltage spikes that instantly fry the DRV8825 driver. Always turn off the external power supply *first*.
> 2. **Tie `RESET` and `SLEEP` together:** The DRV8825 turns off output currents automatically if the `SLEEP` pin drops low. Bridging `RST` to `SLP` pulls both high, keeping the chip awake and responsive to your STM32 step signals.
> 3. **Isolate Grounds properly:** Ensure your high-current external power supply ground connects back to the DRV8825 power ground pin to prevent electrical noise from leaking back into your laptop or STM32 USB board link.
