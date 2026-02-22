# LED System Refactor Plan: Shift Register Migration

## Executive Summary

Migrate from 8 direct GPIO LEDs to 16 LEDs controlled via two cascaded 74HC595 shift registers. This is part of a larger hardware redesign that includes multiplexed inputs. The shift register reduces LED GPIO usage from 8 pins to 3 pins (GPIO 22, 27, 28) while doubling LED capacity to 16.

---

## Current System Analysis

### Current GPIO Allocation (Legacy)

| GPIO Range | Function | Count | Notes |
|------------|----------|-------|-------|
| 4-11 | Buttons | 8 | Input (to be replaced by multiplexer) |
| 12-19 | LEDs | 8 | **Output (to be replaced by shift registers)** |
| 18-19 | I2S Audio | 2 | Conflicts with LEDs when I2S enabled |
| 20 | Audio PWM / I2S LCK | 1 | Output |
| 21 | Trigger Out | 1 | Output (TRIGO_PIN) → will be Keyboard MUX COM |
| 22 | Clock In | 1 | Input (CLOCK_PIN) → will be SR SER |
| 26-28 | ADC Knobs | 3 | Analog Input |

### Target GPIO Allocation (New Hardware Design)

Refer to [target_architecture.md](target_architecture.md) for complete GPIO mapping.

**Key Changes:**
- **GPIO 22, 27, 28**: Shift register control (SER, SRCLK, RCLK)
- **GPIO 14-17**: Multiplexer select bits S0-S3 (shared by potentiometer and keyboard muxes)
- **GPIO 21**: Keyboard multiplexer COM
- **GPIO 26**: Potentiometer multiplexer COM
- **GPIO 18-20**: I2S audio (BCK, DIN, LCK)
- **GPIO 1-5**: MIDI, Clock, Reset I/O
- **GPIO 7-13**: Available for future use

### Current LED Implementation

**File: `doth/led.h`**
- Individual LED class
- Software PWM dimming (0-255 brightness levels)
- Direct GPIO control via `gpio_put()`
- Update pattern: `dim_i` counter for PWM generation

**File: `doth/ledarray.h`**
- Manages 8 LEDs as array
- D✅ GPIO Assignments - Confirmed

### Shift Register GPIO Mapping

Based on [target_architecture.md](target_architecture.md), the shift register will use:

| GPIO Pin | Function | Details |
|----------|----------|---------|
| **GPIO 22** | SER | Serial data input to first shift register |
| **GPIO 27** | SRCLK | Shift register clock (shared by both registers) |
| **GPIO 28** | RCLK | Storage register clock/latch (shared by both registers) |

**Note:** These pins are part of a comprehensive hardware redesign. Previous GPIO assignments (buttons on GPIO 4-11, knobs on GPIO 26-28, etc.) are being replaced with multiplexed inputs.

### Hardware Context

This LED refactor is part of a larger redesign that includes:
- **16 potentiometers** via analog multiplexer (GPIO 26 COM, GPIO 14-17 select)
- **16 buttons** via digital multiplexer (GPIO 21 COM, GPIO 14-17 select)  
- **16 LEDs** via cascaded shift registers (GPIO 22, 27, 28)
- **I2S audio** output (GPIO 18-20)
- **MIDI, Clock, Reset** I/O (GPIO 1-5)
- GPIO 13: SRCLK (Shift Register Clock)
- GPIO 14: RCLK (Register/Latch Clock)
- Rationale: These pins currently drive individual LEDs and will be freed

**Option B: Use upper GPIOs (if available)**
- GPIO 15-17: Available (not in use)
- Check hardware schematic for availability

**Option C: Reassign non-critical pins**
- Consider moving CLOCK_IN or other features if necessary

**⚠️  ACTION REQUIRED:** Resolve GPIO conflicts before implementation begins.

---

## Target Architecture

### New Hardware Configuration

```
┌─────────────────┐
│  Raspberry Pi   │
│     Pico        │
└─────────────────┘
      │  │  │
      │  │  └─── GPIO 28 (RCLK)  ──────┐
      │  └────── GPIO 27 (SRCLK) ───┐  │
      └───────── GPIO 22 (SER)  ─┐  │  │
                                 │  │  │
                    ┌────────────┼──┼──┼──────────────┐
                    │   First 74HC595 Shift Register  │
                    │                                  │
                    │  QA(0)  → LED STEP 8            │
                    │  QB(1)  → LED STEP 1            │
                    │  QC(2)  → LED STEP 5            │
                    │  QD(3)  → LED STEP 2            │
                    │  QE(4)  → LED STEP 6            │
                    │  QF(5)  → LED STEP 3            │
                    │  QG(6)  → LED STEP 7            │
                    │  QH(7)  → LED STEP 4            │
                    │  QH' ──────────────────────┐    │
                    └────────────────────────────┼────┘
                                                 │
                    ┌────────────────────────────┼────┐
                    │  Second 74HC595 Shift Register  │
                    │                                  │
                    │  QA(8)  → LED Y4                │
                    │  QB(9)  → LED PLAY/STOP         │
                    │  QC(10) → LED Y1                │
                    │  QD(11) → LED SEQ REC           │
                    │  QE(12) → LED Y2                │
                    │  QF(13) → LED SEQ ERASE         │
                    │  QG(14) → LED Y3                │
                    │  QH(15) → LED SEQ ON/OFF        │
                    │  QH' → (unused)                 │
                    └─────────────────────────────────┘
```

### Signal Description

- **SER (Serial Data)**: Data input, shifted in on SRCLK rising edge
- **SRCLK (Shift Register Clock)**: Shifts data through register
- **RCLK (Register/Latch Clock)**: Transfers shift register to output latches
- **QH'**: Serial output to cascade to next register

### LED Mapping Table

| Logical LED | Physical Bit | Shift Register | QX Pin | Description |
|-------------|--------------|----------------|--------|-------------|
| STEP_1 | 1 | First | QB | Step sequencer position 1 |
| STEP_2 | 3 | First | QD | Step sequencer position 2 |
| STEP_3 | 5 | First | QF | Step sequencer position 3 |
| STEP_4 | 7 | First | QH | Step sequencer position 4 |
| STEP_5 | 2 | First | QC | Step sequencer position 5 |
| STEP_6 | 4 | First | QE | Step sequencer position 6 |
| STEP_7 | 6 | First | QG | Step sequencer position 7 |
| STEP_8 | 0 | First | QA | Step sequencer position 8 |
| Y1 | 10 | Second | QC | Parameter/Y indicator 1 |
| Y2 | 12 | Second | QE | Parameter/Y indicator 2 |
| Y3 | 14 | Second | QG | Parameter/Y indicator 3 |
| Y4 | 8 | Second | QA | Parameter/Y indicator 4 |
| PLAY_STOP | 9 | Second | QB | Playback state indicator |
| SEQ_REC | 11 | Second | QD | Sequencer recording mode |
| SEQ_ERASE | 13 | Second | QF | Sequencer erase mode |
| SEQ_ON_OFF | 15 | Second | QH | Sequencer enable/disable |

---

## Refactor Plan - Non-Invasive Approach

### Design Principles

1. **Preserve existing API** - No changes to calling code
2. **Backward compatibility** - Use conditional compilation
3. **Testable** - Can switch between old/new implementations
4. **Clean abstractions** - Separate concerns into layers
5. **Non-invasive** - Minimal changes to main.cpp

### Architecture Layers

```
┌──────────────────────────────────────────┐
│         Application Code (main.cpp)       │
│  ledarray.Set(), Update(), Clear(), etc.  │
└────────────────┬─────────────────────────┘
                 │
┌────────────────┴─────────────────────────┐
│      LEDArray Class (ledarray.h)         │
│  - 16 LED management                     │
│  - Software PWM control                  │
│  - API compatibility layer               │
└────────────────┬─────────────────────────┘
                 │
        ┌────────┴────────┐
        │                 │
┌───────┴──────┐  ┌──────┴──────────────┐
│  LEDMapper   │  │   ShiftRegister      │
│  (mapper.h)  │  │  (shift_register.h)  │
│              │  │                      │
│  - LED index │  │  - 74HC595 driver    │
│    mapping   │  │  - Bit manipulation  │
│  - Physical  │  │  - GPIO control      │
│    to logical│  │  - Fast updates      │
└──────────────┘  └─────────────────────┘
```

---

## Implementation Phases

### Phase 1: Create Shift Register Driver

**New File: `doth/shift_register.h`**

```cpp
#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include "hardware/gpio.h"
#include "pico/stdlib.h"

class ShiftRegister {
 private:
  uint8_t gpio_ser;    // Serial data input
  uint8_t gpio_srclk;  // Shift register clock
  uint8_t gpio_rclk;   // Register/latch clock
  uint16_t state;      // Current 16-bit state

  // Fast bit-bang shift out 16 bits
  inline void shiftOut16(uint16_t data) {
    // Shift out MSB first (bit 15 to bit 0)
    for (int8_t i = 15; i >= 0; i--) {
      // Set data bit
      gpio_put(gpio_ser, (data >> i) & 0x01);
      // Pulse shift clock
      gpio_put(gpio_srclk, 1);
      gpio_put(gpio_srclk, 0);
    }
    // Pulse latch clock to update outputs
    gpio_put(gpio_rclk, 1);
    gpio_put(gpio_rclk, 0);
  }

 public:
  void Init(uint8_t ser_pin, uint8_t srclk_pin, uint8_t rclk_pin) {
    gpio_ser = ser_pin;
    gpio_srclk = srclk_pin;
    gpio_rclk = rclk_pin;
    state = 0;

    // Initialize GPIO pins as outputs
    gpio_init(gpio_ser);
    gpio_set_dir(gpio_ser, GPIO_OUT);
    gpio_put(gpio_ser, 0);

    gpio_init(gpio_srclk);
    gpio_set_dir(gpio_srclk, GPIO_OUT);
    gpio_put(gpio_srclk, 0);

    gpio_init(gpio_rclk);
    gpio_set_dir(gpio_rclk, GPIO_OUT);
    gpio_put(gpio_rclk, 0);

    // Clear all outputs
    Clear();
  }

  // Set individual LED on/off
  void SetBit(uint8_t bit_index, bool on) {
    if (bit_index < 16) {
      if (on) {
        state |= (1 << bit_index);
      } else {
        state &= ~(1 << bit_index);
      }
    }
  }

  // Get current bit state
  bool GetBit(uint8_t bit_index) {
    if (bit_index < 16) {
      return (state >> bit_index) & 0x01;
    }
    return false;
  }

  // Update hardware with current state
  void Update() {
    shiftOut16(state);
  }

  // Clear all outputs
  void Clear() {
    state = 0;
    Update();
  }

  // Set all 16 bits at once
  void SetAll(uint16_t value) {
    state = value;
    Update();
  }

  // Get current state
  uint16_t GetState() {
    return state;
  }
};

#endif  // SHIFT_REGISTER_H
```

**Features:**
- Hardware abstraction for 74HC595 control
- Fast bit-banging (can be optimized with PIO later if needed)
- 16-bit state buffer
- Simple API for bit manipulation

---

### Phase 2: Create LED Mapping Layer

**New File: `doth/led_mapper.h`**

```cpp
#ifndef LED_MAPPER_H
#define LED_MAPPER_H

#include "pico/stdlib.h"

// Maps logical LED indices (0-15) to physical shift register bits
class LEDMapper {
 private:
  // Lookup table: logical_index -> physical_bit
  static const uint8_t led_to_bit[16];

 public:
  // Convert logical LED index (0-15) to shift register bit position (0-15)
  static uint8_t LogicalToBit(uint8_t logical_index) {
    if (logical_index < 16) {
      return led_to_bit[logical_index];
    }
    return 0;  // Default to bit 0 if out of range
  }
};

// LED mapping based on hardware wiring
// Index:  Logical LED position (0=STEP_1, 1=STEP_2, ..., 7=STEP_8, 8=Y1, etc.)
// Value:  Physical shift register bit position
const uint8_t LEDMapper::led_to_bit[16] = {
    1,   // 0: STEP_1 -> QB (bit 1)
    3,   // 1: STEP_2 -> QD (bit 3)
    5,   // 2: STEP_3 -> QF (bit 5)
    7,   // 3: STEP_4 -> QH (bit 7)
    2,   // 4: STEP_5 -> QC (bit 2)
    4,   // 5: STEP_6 -> QE (bit 4)
    6,   // 6: STEP_7 -> QG (bit 6)
    0,   // 7: STEP_8 -> QA (bit 0)
    10,  // 8: Y1 -> Second:QC (bit 10)
    12,  // 9: Y2 -> Second:QE (bit 12)
    14,  // 10: Y3 -> Second:QG (bit 14)
    8,   // 11: Y4 -> Second:QA (bit 8)
    9,   // 12: PLAY_STOP -> Second:QB (bit 9)
    11,  // 13: SEQ_REC -> Second:QD (bit 11)
    13,  // 14: SEQ_ERASE -> Second:QF (bit 13)
    15   // 15: SEQ_ON_OFF -> Second:QH (bit 15)
};

// Named constants for LED indices (optional, for code clarity)
#define LED_STEP_1      0
#define LED_STEP_2      1
#define LED_STEP_3      2
#define LED_STEP_4      3
#define LED_STEP_5      4
#define LED_STEP_6      5
#define LED_STEP_7      6
#define LED_STEP_8      7
#define LED_Y1          8
#define LED_Y2          9
#define LED_Y3          10
#define LED_Y4          11
#define LED_PLAY_STOP   12
#define LED_SEQ_REC     13
#define LED_SEQ_ERASE   14
#define LED_SEQ_ON_OFF  15

#endif  // LED_MAPPER_H
```

**Features:**
- Handles non-sequential LED-to-bit mapping
- Compile-time mapping table
- Named constants for code readability
- Easy to modify if hardware changes

---

### Phase 3: Update LEDArray Class

**Modified File: `doth/ledarray.h`**

Add shift register implementation alongside existing GPIO implementation:

```cpp
// At top of file, after includes
#include "shift_register.h"
#include "led_mapper.h"

class LEDArray {
#if I2S_AUDIO_ENABLED == 1
  // LEDs disabled when I2S is active (GPIO 18-19 conflict)
 public:
  void Init() {}
  bool Continue() { return false; }
  void LedSet(uint8_t i, uint8_t v) {}
  void LedUpdate(uint8_t i) {}
  void Update() {}
  void Clear() {}
  void On(uint8_t j) {}
  void Set(uint8_t i, uint16_t v) {}
  void Add(uint8_t i, uint16_t v) {}
  void SetBinary(uint8_t v) {}
  void SetAll(uint16_t v) {}
};

#elif SHIFT_REGISTER_ENABLED == 1
  // === NEW SHIFT REGISTER IMPLEMENTATION ===
  ShiftRegister shift_reg;
  uint8_t vals[16];      // Brightness values 0-255 for 16 LEDs
  uint8_t dim_i;         // PWM counter for software dimming
  uint16_t do_leds;      // Update counter
  LEDMapper mapper;

 public:
  void Init() {
    shift_reg.Init(SR_SER_PIN, SR_SRCLK_PIN, SR_RCLK_PIN);
    for (uint8_t i = 0; i < 16; i++) {
      vals[i] = 0;
    }
    dim_i = 0;
    do_leds = 0;
  }

  bool Continue() {
    do_leds++;
    if (do_leds % 1000 == 0) {
      return true;
    }
    return false;
  }

  void LedSet(uint8_t i, uint8_t v) {
    if (i < 16) {
      uint8_t bit = mapper.LogicalToBit(i);
      shift_reg.SetBit(bit, v > 0);
    }
  }

  void LedUpdate(uint8_t i) {
    // Software PWM: called frequently
    if (i < 16) {
      uint8_t bit = mapper.LogicalToBit(i);
      shift_reg.SetBit(bit, dim_i < vals[i]);
    }
  }

  void Update() {
    dim_i++;
    // Apply software PWM to all LEDs
    for (uint8_t i = 0; i < 16; i++) {
      uint8_t bit = mapper.LogicalToBit(i);
      shift_reg.SetBit(bit, dim_i < vals[i]);
    }
    // Single hardware update for all LEDs
    shift_reg.Update();
  }

  void Clear() {
    for (uint8_t i = 0; i < 16; i++) {
      vals[i] = 0;
    }
  }

  void On(uint8_t j) {
    if (j < 16) {
      for (uint8_t i = 0; i < 16; i++) {
        vals[i] = (i == j) ? 255 : 0;
      }
    }
  }

  // sets between 0 and 1000
  void Set(uint8_t i, uint16_t v) {
    if (i < 16) {
      v = v * 255 / 1000;
      if (v != vals[i]) {
        vals[i] = v;
      }
    }
  }

  // sets between 0 and 1000
  void Add(uint8_t i, uint16_t v) {
    if (i < 16) {
      v = v * 255 / 1000;
      if (vals[i] + v > 255) {
        vals[i] = 255;
      } else {
        vals[i] += v;
      }
    }
  }

  void SetBinary(uint8_t v) {
    uint8_t j = 0;
    for (uint8_t i = 128; i > 0; i = i / 2) {
      if (j < 8) {  // Only first 8 LEDs for binary display
        vals[j] = (v & i) ? 255 : 0;
      }
      j++;
    }
  }

  // SetAll sets between 0 and 1000
  void SetAll(uint16_t v) {
    v = v * 4080 / 1000;  // 16 LEDs: 0-4080 (255*16)
    for (uint8_t i = 0; i < 16; i++) {
      if (v > 255) {
        vals[i] = 255;
        v -= 255;
      } else if (v > 0) {
        vals[i] = v;
        v = 0;
      } else {
        vals[i] = 0;
      }
    }
  }

  // === NEW METHODS FOR ADDITIONAL LEDs ===
  
  // Set Y/parameter LEDs (0-3)
  void SetYLED(uint8_t y_index, uint16_t brightness) {
    if (y_index < 4) {
      Set(8 + y_index, brightness);  // Y1-Y4 are LEDs 8-11
    }
  }

  // Set control LEDs
  void SetPlayStop(bool on) {
    Set(LED_PLAY_STOP, on ? 1000 : 0);
  }

  void SetSeqRec(bool on) {
    Set(LED_SEQ_REC, on ? 1000 : 0);
  }

  void SetSeqErase(bool on) {
    Set(LED_SEQ_ERASE, on ? 1000 : 0);
  }

  void SetSeqOnOff(bool on) {
    Set(LED_SEQ_ON_OFF, on ? 1000 : 0);
  }
};

#else
  // === ORIGINAL GPIO IMPLEMENTATION (8 LEDs) ===
  LED led[8];
  uint8_t vals[8];
  uint16_t do_leds;

 public:
  void Init() {
    for (uint8_t i = 0; i < 8; i++) {
      led[i].Init(i + 12);
      vals[i] = 0;
    }
    do_leds = 0;
  }

  // ... (keep existing implementation unchanged)
  
#endif  // I2S_AUDIO_ENABLED / SHIFT_REGISTER_ENABLED
```

**Key Changes:**
- Three-way conditional: I2S disabled / Shift Register / Original GPIO
- Software PWM preserved (same algorithm)
- API remains compatible with existing code
- Extended to 16 LEDs
- New helper methods for control LEDs

---

### Phase 4: Update main.cpp

**File: `main.cpp`**

Add configuration defines:

```cpp
// Near top of file, after includes
#define SHIFT_REGISTER_ENABLED 1  // Enable shift register LEDs

#if SHIFT_REGISTER_ENABLED == 1
// Shift register GPIO pins (UPDATE THESE AFTER GPIO CONFLICT RESOLUTION!)
#define SR_SER_PIN    12   // Serial data (choose available GPIO)
#define SR_SRCLK_PIN  13   // Shift clock (choose available GPIO)
#define SR_RCLK_PIN   14   // Latch clock (choose available GPIO)
#endif

#define NUM_LEDS 16  // Changfrom target_architecture.md)
#define SR_SER_PIN    22   // Serial data input
#define SR_SRCLK_PIN  27   // Shift register clock
#define SR_RCLK_PIN   28   // Storage register clock (latch

```cpp
// In main() function, LED initialization section
#if I2S_AUDIO_ENABLED == 0
  ledarray.Init();  // Works for both GPIO and shift register modes
  printf("LEDs initialized (");
#if SHIFT_REGISTER_ENABLED == 1
  printf("Shift Register mode, 16 LEDs)\n");
#else
  printf("Direct GPIO mode, 8 LEDs)\n");
#endif
#endif
```

**No other changes needed** - existing `ledarray.Set()`, `Update()`, etc. calls work as-is!

---

### Phase 5: Update CMakeLists.txt (if needed)

If new header files need to be tracked:

```cmake
# Add to target sources if using .cpp implementations
target_sources(pikocore PRIVATE
    main.cpp
    doth/i2s_audio.cpp
    doth/WS2812.cpp
    doth/usb_descriptors.c
    # Add if converted to .cpp:
    # doth/shift_register.cpp
)
```

---

## Software PWM Implementation Notes

### Current PWM Mechanism

The existing LED PWM uses a simple counter-based approach:

```cpp
void Update() {
  dim_i++;  // Increment PWM counter
  if (dim_i < dim) {
    gpio_put(gpio, 1);  // LED on
  } else {
    gpio_put(gpio, 0);  // LED off
  }
}
```

### Shift Register PWM Adaptation

With shift registers, we need to:

1. **Update all LEDs simultaneously** (not one at a time)
2. **Maintain same PWM frequency** for consistency
3. **Single shift-out per update** for efficiency

**Implementation:**

```cpp
void Update() {
  dim_i++;
  
  // Build 16-bit state based on PWM comparison
  for (uint8_t i = 0; i < 16; i++) {
    uint8_t bit = mapper.LogicalToBit(i);
    bool on = (dim_i < vals[i]);
    shift_reg.SetBit(bit, on);
  }
  
  // Single hardware update (shift out 16 bits)
  shift_reg.Update();
}
```

**Performance Considerations:**
- Each `Update()` call shifts out 16 bits
- At ~4Hz main loop, this is negligible
- If called from interrupt (faster), consider PIO optimization
- Current bit-banging should be fast enough for typical update rates

---

## Testing Strategy

### Phase 1: Hardware Verification

1. **Shift Register Test**
   - Simple pattern: light LEDs 0-15 sequentially
   - Verify correct wiring and cascading
   - Test: `for (i=0; i<16; i++) { shift_reg.SetBit(i, 1); shift_reg.Update(); sleep_ms(200); }`

2. **LED Mapping Test**
   - Light one logical LED at a time
   - Verify physical LED matches expected position
   - Test: `for (i=0; i<16; i++) { ledarray.On(i); sleep_ms(500); }`

### Phase 2: Functional Testing

3. **PWM Dimming Test**
   - Verify brightness levels 0-255 work correctly
   - Test: `ledarray.Set(0, 500); ledarray.Update();` (repeated)
   - Visual: LED should be at 50% brightness

4. **API Compatibility Test**
   - Run existing LED control code
   - Verify: `SetBinary()`, `Add()`, `Clear()`, `SetAll()` work
   - Compare behavior to original GPIO implementation

### Phase 3: Integration Testing

5. **Sequencer LED Test**
   - Run sequencer with recording/playback
   - Verify step LEDs show current position correctly
   - Test all 8 step LEDs in sequence

6. **New LED Features Test**
   - Test Y1-Y4 LEDs for parameter indication
   - Test PLAY/STOP, SEQ_REC, SEQ_ERASE, SEQ_ON_OFF
   - Verify correct mapping to hardware

### Phase 4: Performance Testing

7. **Update Rate Test**
   - Measure time for `Update()` call
   - Ensure no timing regressions
   - Profile: main loop timing, interrupt timing

8. **PWM Flicker Test**
   - Check for visible flicker at various brightness levels
   - Test edge cases: very dim (vals[i]=1-10), mid-range, near max
   - Adjust update rate if needed

### Test Checklist

- [ ] All 16 LEDs light correctly
- [ ] LED mapping matches hardware wiring
- [ ] Software PWM brightness levels work (0-255)
- [ ] Existing sequencer code works unchanged
- [ ] Binary display (BPM indicator) works
- [ ] New control LEDs (Y1-Y4, etc.) function
- [ ] No GPIO conflicts with buttons/knobs/audio
- [ ] Performance is acceptable (no lag/flicker)
- [ ] Can toggle between GPIO/shift register builds
- [ ] I2S_AUDIO_ENABLED build still compiles

---

## Performance Optimization (Future)

### PIO-Based Shift Register

If bit-banging is too slow or causes timing issues:

**Option: Use PIO for shift out**
- Dedicated PIO state machine for shift register
- DMA-based updates for zero CPU overhead
- Frees CPU for audio processing

**Implementation:**
1. Create `shift_register.pio` program
2. Use PIO FIFO for data transfer
3. Trigger shifts with minimal CPU intervention

Example PIO structure:
```pio
.program shift_register
.side_set 1  ; SRCLK on side-set pin

.wrap_target
    out pins, 1   side 0  ; Output bit to SER, clock low
    nop           side 1  ; Clock high (shift)
.wrap

public entry_point:
    set pins, 0           ; Start with RCLK low
    ; ... shift 16 bits
    set pins, 1           ; Pulse RCLK high
    set pins, 0           ; RCLK low
```

---

## Rollback Plan

If issues arise during implementation:

1. **Immediate Rollback**: Disable shift register
   ```cpp
   #define SHIFT_REGISTER_ENABLED 0  // Back to GPIO mode
   ```

2. **Partial Rollback**: Keep 8 LEDs on GPIO, ignore new 8
   - Modify `#if` conditions to use GPIO for steps, dummy for control

3. **Debug Mode**: Parallel operation
   - Drive both GPIO and shift register simultaneously
   - Compare outputs for debugging

---

## Timeline Estimate

| Phase | Task | Estimated Time | Dependencies |
|-------|------|----------------|--------------|
| 0 | Resolve GPIO conflicts | 1-2 hours | Hardware schematic review |
| 1 | Create `shift_register.h` | 2-3 hours | Phase 0 |
| 2 | Create `led_mapper.h` | 1 hour | - |
| 3 | Test shift register (hardware) | 2-3 hours | Phases 1, 2 |
| 4 | Update `ledarray.h` | 3-4 hours | Phase 3 |
| 5 | Update `main.cpp` | 1 hour | Phase 4 |
| 1 | Create `shift_register.h` | 2-3 hours | - |
| 2 | Create `led_mapper.h` | 1 hour | - |
| 3 | Test shift register (hardware) | 2-3 hours | Phases 1, 2, hardware ready |
| 4 | Update `ledarray.h` | 3-4 hours | Phase 3 |
| 5 | Update `main.cpp` | 1 hour | Phase 4 |
| 6 | Integration testing | 2-3 hours | Phase 5 |
| 7 | New LED features | 2-3 hours | Phase 6 |
| 8 | Performance optimization | 2-4 hours | Phase 7 (optional) |
| **Total** | | **15-23steps + 8 control)
- ✅ GPIO usage reduced from 8 to 3 pins
- ✅ No changes to existing application code (beyond defines)
- ✅ Software PWM dimming preserved
- ✅ Performance acceptable (no lag/flicker)
- ✅ Backward compatible with conditional compilation
- ✅ All existing LED features work as before
- ✅ New control LEDs functional
- ✅ Can toggle I2S/GPIO/ShiftRegister modes

---

## Next Steps - IMMEDIATE ACTIONS

1. **CRITICAL: Resolve GPIO conflicts**
   - ✅ GPIO Assignment** (Complete)
   - [x] GPIO pins defined in target_architecture.md
   - [x] GPIO 22 (SER), GPIO 27 (SRCLK), GPIO 28 (RCLK)
   - [x] LED mapping documented

2. **Hardware preparation**
   - [ ] Wire shift registers to GPIO 22, 27, 28
   - [ ] Connect 16 LEDs to shift register outputs (QA-QH on both registers)
   - [ ] Cascade QH' from first register to SER of second register
   - [ ] Verify power supply adequate for 16 LEDs
   - [ ] Test with multimeter/logic analyzer

3. **Software implementation**
   - [ ] Create `doth/shift_register.h`
   - [ ] Create `doth/led_mapper.h`
   - [ ] Test basic shift register functionality (simple patterns)
   - [ ] Update `ledarray.h` with shift register code
   - [ ] Update `main.cpp` with defines (GPIO 22, 27, 28)

4. **Testing and integration**
   - [ ] Hardware verification (all 16 LEDs light correctly)
   - [ ] LED mapping verification (correct physical LEDs light)
   - [ ] PWM dimming verification (brightness levels work)
   - [ ] Integration testing with existing sequencer code
   - [ ] Test new control LEDs (Y1-Y4, PLAY/STOP, etc.)
---

## Risk Mitigation

| Risk | Likelihood | Impact | Mitigation |
|-PWM flicker with shift registers | Medium | High | Test early, adjust update rate or use PIO |
| Performance degradation | Low | Medium | Profile and optimize (PIO if needed) |
| LED mapping errors | Medium | Low | Comprehensive testing with lookup table |
| Hardware wiring issues | Medium | Medium | Test with simple patterns first |
| Cascade timing issues | Low | Medium | Verify QH' connection between registers |
| Code regression | Low | High | Keep old code path available via #ifdef patterns first |
| Code regression | Low | High | Keep old code path available |

---

## References

- **74HC595 Datasheet**: Shift register timing and specifications
- **RP2040 GPIO**: Pin capabilities and conflicts
- **Current Implementation**: `doth/led.h`, `doth/ledarray.h`
- **Pico SDK**: GPIO, PIO documentation

---
Complete GPIO Mapping (Target Architecture)

See [target_architecture.md](target_architecture.md) for full details.

### Shift Register Final Assignments

```
SR_SER_PIN = GPIO 22     // Serial Data Input
SR_SRCLK_PIN = GPIO 27   // Shift Register Clock (both registers)
SR_RCLK_PIN = GPIO 28    // Storage/Latch Clock (both registers)
```

### LED Physical Mapping (Shift Register → Physical LED)

**First Register (LEDS_1):**
- QA (bit 0) → LED STEP 8
- QB (bit 1) → LED STEP 1
- QC (bit 2) → LED STEP 5
- QD (bit 3) → LED STEP 2
- QE (bit 4) → LED STEP 6
- QF (bit 5) → LED STEP 3
- QG (bit 6) → LED STEP 7
- QH (bit 7) → LED STEP 4
- QH' → Cascaded to second register

**Second Register (LEDS_2):**
- QA (bit 8) → LED Y4
- QB (bit 9) → LED PLAY/STOP
- QC (bit 10) → LED Y1
- QD (bit 11) → LED SEQ REC
- QE (bit 12) → LED Y2
- QF (bit 13) → LED SEQ ERASE
- QG (bit 14) → LED Y3
- QH (bit 15) → LED SEQ ON/OFF

---

**Document Version:** 2.0  
**Date:** February 22, 2026  
**Status:** Ready for Implementation - GPIO Assignments Confirmed
**Status:** Planning Phase - Awaiting GPIO Conflict Resolution
