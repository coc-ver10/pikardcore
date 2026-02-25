# Pikardcore Target Architecture

## Project Overview

**Pikardcore** is a hardware and software upgrade of the **Picocore** project (see [architecture.md](architecture.md) for the original Picocore description).

### Key Upgrades from Picocore

The original Picocore is limited to:
- **3 Knobs** (Menu, Function A, Function B)
- **8 Buttons** (beat/step selection)
- **8 LEDs** (beat visualization)

Pikardcore expands the interface to:
- **16 Knobs** (direct access to all controls)
- **16 Buttons** (8 for beat/step selection + 8 additional)
- **16 LEDs** (corresponding to buttons)
- **I2S Audio Output** (16-bit DAC upgrade from PWM)

---

## Interface Components

### 16 Knobs - Direct Control Access

The objective of the 16 knobs is to provide **direct access to all controls** without needing to navigate the "Menu" knob used in the original Picocore.

**Original Picocore Mapping** (3 knobs):
- **Menu Knob:** Control selector
- **Function A:** BPM, noise gate, jump probability, tunnel probability, record mode, save
- **Function B:** Break effects, stretch, gate/retrig/reverse probabilities, playback mode, load

**Pikardcore Mapping** (16 knobs - one knob per parameter):

| Knob # | Function | Description |
|--------|----------|-------------|
| I11 | TEMPO | BPM control (50-360 BPM) |
| I8 | GATE | Noise gate threshold |
| I3 | JUMP PROB | Jump probability |
| I1 | TUNNEL PROB | Tunnel probability (sample switching) |
| I0 | REVERSE PROB | Direction change probability |
| I9 | GATE PROB | Gate probability |
| I2 | RETRIG PROB | Retrigger probability |
| I5 | BREAK | Break effects macro |
| I7 | STRETCH | Time stretch control |
| I6 | FILTER | Low-pass filter cutoff (46 positions) |
| I10 | VOLUME / FOLD | Volume and wave-folding distortion |
| I4 | SAMPLE | Sample selection |
| I12-I15 | X4-X1 | **To be defined** (4 knobs remaining) |

**Implementation:**
- Interfaced via **74HC4067 16-channel multiplexer**
- Common pin: GPIO26 (ADC capable)
- Select pins: GPIO14-17 (S0-S3)

### 16 Buttons - Beat Selection + Additional Controls

**First 8 Buttons** (I0-I7):
- Retain the exact same function as original Picocore
- **Beat/step selection** for audio manipulation
- Single button: Select specific beat
- Two buttons: Trigger retrig effect between beats

**Additional Buttons** (I8-I15):

| Button # | Function | Description |
|----------|----------|-------------|
| I8-I11 | Y1-Y4 | **To be precisely defined** |
| I12 | PLAY / STOP | Playback control |
| I13 | SEQ REC | Sequencer record mode |
| I14 | SEQ ERASE | Sequencer erase pattern |
| I15 | SEQ ON / OFF | Sequencer enable/disable |

**Implementation:**
- Interfaced via **74HC4067 16-channel multiplexer**
- Common pin: GPIO21 (digital input)
- Select pins: GPIO14-17 (S0-S3, shared with knob multiplexer)

### 16 LEDs - Visual Feedback

Each LED corresponds to a button.

**First 8 LEDs:**
- Function like the original project for **beat visualization**
- Already functional in current implementation
- Light up to show currently selected beat/step

**Additional 8 LEDs:**
- Likely light up when their corresponding buttons are activated
- Visual feedback for sequencer and control states
- **To be confirmed** in final implementation

**Implementation:**
- Controlled by **2× 74HC595 shift registers** (cascaded)
- Serial data: GPIO22 (SER)
- Shift clock: GPIO27 (SRCLK, shared between both registers)
- Storage clock: GPIO28 (RCLK, shared between both registers)
- LEDS_1 (QH') cascades to LEDS_2 (SER)

### Audio Output - I2S DAC

**Upgrade from Original Picocore:**
- Original: 8-bit PWM audio output
- Pikardcore: **16-bit I2S digital audio**

**Hardware:**
- **GY-PCM5102** 16-bit stereo DAC module
- Status: **Already functional**

**I2S Connections:**

| GPIO | Signal | Description |
|------|--------|-------------|
| GPIO18 | BCK | Bit clock (serial clock) |
| GPIO19 | DIN | Data input (serial data) |
| GPIO20 | LCK | Left/Right clock (LRCLK/WS) |

**Audio Processing Pipeline:**
```
Flash Memory (audio samples)
        ↓
Phase Management (dual playback heads)
        ↓
Sample Retrieval & Effects
        ↓
Volume/Distortion Processing
        ↓
Low-Pass Filter (biquad IIR)
        ↓
16-bit I2S Output (GY-PCM5102 DAC)
```

---

## Hardware Architecture Summary

### Multiplexer Strategy
Both knobs and buttons use **74HC4067** multiplexers to expand limited GPIO:
- Reduces GPIO usage from 32 pins (16+16) to 5+5 pins per interface
- Select lines (S0-S3) are **shared** between both multiplexers
- Only the COM pin differs (GPIO26 for knobs, GPIO21 for buttons)
- Allows reading one knob and one button simultaneously

### Shift Register Strategy
LEDs use **cascaded 74HC595** shift registers:
- Chain two 8-bit shift registers for 16 outputs
- Only 3 GPIO pins needed (SER, SRCLK, RCLK)
- Serial data shifts through LEDS_1 → LEDS_2
- Both registers update simultaneously on RCLK pulse

### GPIO Efficiency
- **Original Picocore:** 3 + 8 + 8 = 19 GPIO pins minimum
- **Pikardcore:** 16 + 16 + 16 = 48 controls/outputs
- **Achieved with:** ~15 GPIO pins (multiplexing saves ~33 pins)

---

# GPIO Pin Mappings

## MULTIPLEXER POTARDS

| GPIO | Function | Details |
| --- | --- | --- |
| GPIO26 | COM | Common pin |
| GPIO14 | S0 | Select bit 0 |
| GPIO15 | S1 | Select bit 1 |
| GPIO16 | S2 | Select bit 2 |
| GPIO17 | S3 | Select bit 3 |

### Multiplexer Inputs (Potentiometers)

| Input | Function | Notes |
| --- | --- | --- |
| I0 | REVERSE PROB |probability_direction (reverse) |
| I1 | TUNNEL PROB | probability_tunnel|
| I2 | RETRIG PROB | probability_retrig|
| I3 | JUMP PROB |probability_jump |
| I4 | SAMPLE |sample (sélection d’échantillon) |
| I5 | BREAK |macro FX “break” (param_set_break) |
| I6 | FILTER |Low-Pass Filter (biquad IIR, filter.h) |
| I7 | STRETCH |stretch_change |
| I8 | GATE |noise_gate_thresh (seuil gate) |
| I9 | GATE PROB | probability_gate|
| I10 | VOLUME / FOLD | volume (via param_set_volume)|
| I11 | TEMPO |tempo (BPM) |
| I12 | X4 | to define |
| I13 | X3 | to define |
| I14 | X2 | to define |
| I15 | X1 | to define |

## MULTIPLEXER KEYBOARD

| GPIO | Function | Details |
| --- | --- | --- |
| GPIO21 | COM | Common pin |
| GPIO14 | S0 | Select bit 0 (shared) |
| GPIO15 | S1 | Select bit 1 (shared) |
| GPIO16 | S2 | Select bit 2 (shared) |
| GPIO17 | S3 | Select bit 3 (shared) |

### Multiplexer Inputs (Buttons)

| Input | Function | Notes |
| --- | --- | --- |
| I0 | STEP 1 | |
| I1 | STEP 2 | |
| I2 | STEP 3 | |
| I3 | STEP 4 | |
| I4 | STEP 5 | |
| I5 | STEP 6 | |
| I6 | STEP 7 | |
| I7 | STEP 8 | |
| I8 | Y1 | to define |
| I9 | Y2 | to define |
| I10 | Y3 | to define |
| I11 | Y4 | to define |
| I12 | PLAY / STOP | |
| I13 | SEQ REC | |
| I14 | SEQ ERASE | |
| I15 | SEQ ON / OFF | |

## SHIFT REGISTER LEDS_1

| GPIO | Function | Details |
| --- | --- | --- |
| GPIO22 | SER | Serial data input |
| GPIO27 | SRCLK | Shift register clock |
| GPIO28 | RCLK | Storage register clock |

### Shift Register Outputs

| Output | Function | Notes |
| --- | --- | --- |
| QA (0) | LED STEP 8 | |
| QB (1) | LED STEP 1 | |
| QC (2) | LED STEP 5 | |
| QD (3) | LED STEP 2 | |
| QE (4) | LED STEP 6 | |
| QF (5) | LED STEP 3 | |
| QG (6) | LED STEP 7 | |
| QH (7) | LED STEP 4 | |
| QH' | SER SECOND SHIFT | Cascaded to LEDS_2 |

## SHIFT REGISTER LEDS_2

| GPIO | Function | Details |
| --- | --- | --- |
| QH' LEDS_1 | SER | Serial input from LEDS_1 |
| GPIO27 | SRCLK | Shift register clock (shared) |
| GPIO28 | RCLK | Storage register clock (shared) |

### Shift Register Outputs

| Output | Function | Notes |
| --- | --- | --- |
| QA | LED Y4 | |
| QB | LED PLAY / STOP | |
| QC | LED Y1 | |
| QD | LED SEQ REC | |
| QE | LED Y2 | |
| QF | LED SEQ ERASE | |
| QG | LED Y3 | |
| QH | LED SEQ ON / OFF | |
| QH' | (unused) | |

## DAC AUDIO OUT - I2S PCM5102

| GPIO | Function | Details |
| --- | --- | --- |
| GPIO18 | BCK | Bit clock |
| GPIO19 | DIN | Data in |
| GPIO20 | LCK | Left/Right clock (LRCLK) |

## OTHER GPIO

| GPIO | Function | Notes |
| --- | --- | --- |
| GPIO1 | MIDI INPUT | |
| GPIO4 | CLOCK OUT | |
| GPIO5 | RESET OUT | |
| GPIO2 | CLOCK IN | |
| GPIO3 | RESET IN | |
| GPIO7 | (to define) | |
| GPIO8 | (to define) | |
| GPIO9 | (to define) | |
| GPIO10 | (to define) | |
| GPIO11 | (to define) | |
| GPIO12 | (to define) | |
| GPIO13 | (to define) | |

## GPIO Summary

### Used Pins
- **GPIO1**: MIDI INPUT
- **GPIO2**: CLOCK IN
- **GPIO3**: RESET IN
- **GPIO4**: CLOCK OUT
- **GPIO5**: RESET OUT
- **GPIO14**: S0 (Multiplexer select - shared)
- **GPIO15**: S1 (Multiplexer select - shared)
- **GPIO16**: S2 (Multiplexer select - shared)
- **GPIO17**: S3 (Multiplexer select - shared)
- **GPIO18**: I2S BCK
- **GPIO19**: I2S DIN
- **GPIO20**: I2S LCK
- **GPIO21**: Keyboard multiplexer COM
- **GPIO22**: Shift register SER
- **GPIO26**: Potentiometer multiplexer COM
- **GPIO27**: Shift register SRCLK (shared)
- **GPIO28**: Shift register RCLK (shared)

### Available Pins
- GPIO7, GPIO8, GPIO9, GPIO10, GPIO11, GPIO12, GPIO13
- 4 potentiometer inputs (X1-X4)
- 4 button inputs (Y1-Y4)
