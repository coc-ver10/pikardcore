# Pikardcore Implementation Plan

## Project Overview

This document outlines the implementation plan for upgrading **Picocore** to **Pikardcore**, expanding from 3 knobs/8 buttons/8 LEDs to 16 knobs/16 buttons/16 LEDs with I2S audio output.

See [target_architecture.md](target_architecture.md) for detailed hardware specifications.

---

## Task List

### **Task 1: Hardware Interface - Multiplexer for Knobs** 
**Status:** ✅ **COMPLETE**  
**Priority:** High (Foundation)  
**File:** Create `doth/multiplexer_knob.h`

**Description:**  
Create a class to read 16 analog knobs via 74HC4067 multiplexer.

**Implementation Details:** ✅ **ALL DONE**
- ✅ Initialize GPIO26 (ADC COM), GPIO14-17 (S0-S3 select lines)
- ✅ Implement channel selection function (0-15)
- ✅ Read ADC value for selected channel
- ✅ Add smoothing/filtering for stable readings
- ✅ Support sequential or selective channel reading
- ✅ Return 12-bit ADC values (0-4095)

**Implementation Notes:**
- Created `MultiplexerKnob` class following same pattern as original `Knob` class
- 10μs settling time after channel selection (74HC4067 has ~200ns propagation delay)
- Change detection threshold: 100 (same as original)
- Startup delay: 800 cycles per channel to prevent spurious readings
- API: `ReadAll()` for sequential reading, `Read(channel)` for selective reading
- Values are inverted (4095 - adc_read) like original implementation

**Dependencies:** None

---

### **Task 2: Hardware Interface - Multiplexer for Buttons**
**Status:** Not Started  
**Priority:** High (Foundation)  
**File:** Create `doth/multiplexer_button.h`

**Description:**  
Create a class to read 16 digital buttons via 74HC4067 multiplexer.

**Implementation Details:**
- Initialize GPIO21 (digital COM), share GPIO14-17 (S0-S3) with knob multiplexer
- Implement channel selection function (0-15)
- Read digital state for selected channel (with pull-up/pull-down)
- Add debouncing logic for each button (time-based or state-machine)
- Track button press/release events
- Support edge detection (rising/falling)

**Dependencies:** None

---

### **Task 3: Hardware Interface - Shift Register LED Driver**
**Status:** Not Started  
**Priority:** High (Foundation)  
**File:** Update `doth/shift_register.h` or extend `doth/ledarray.h`

**Description:**  
Implement 16 LED control via cascaded 74HC595 shift registers.

**Implementation Details:**
- Initialize GPIO22 (SER), GPIO27 (SRCLK), GPIO28 (RCLK)
- Support 16-bit output (2 cascaded 8-bit registers)
- Handle non-sequential LED mapping per target_architecture.md:
  - LEDS_1: QA=LED8, QB=LED1, QC=LED5, QD=LED2, QE=LED6, QF=LED3, QG=LED7, QH=LED4
  - LEDS_2: QA=LEDY4, QB=PLAY/STOP, QC=LEDY1, QD=SEQ_REC, QE=LEDY2, QF=SEQ_ERASE, QG=LEDY3, QH=SEQ_ON/OFF
- Implement efficient bulk update method (shift all 16 bits at once)
- Add individual LED set/clear methods
- Add brightness control if needed (PWM-based)

**Dependencies:** None

**Note:** Existing implementation in `doth/shift_register.h` may already support this. Review and extend if needed.

---

### **Task 4: Control Logic - Remove Menu Navigation**
**Status:** Not Started  
**Priority:** High (Core Refactoring)  
**File:** Modify `main.cpp`

**Description:**  
Refactor main.cpp control loop to remove menu-based knob selector logic.

**Implementation Details:**
- Remove menu knob selector logic
- Remove `param_set_*` function calls tied to menu position
- Remove mode switching between Function A/Function B
- Simplify control flow - each knob directly controls one parameter
- Remove knob position threshold-based save/load triggers
- Clean up unused selector variables

**Dependencies:** Task 1 (Multiplexer Knob Reader)

**Impact:** Major refactoring of main loop logic

---

### **Task 5: Parameter Mapping - Direct Knob Access**
**Status:** Not Started  
**Priority:** High (Core Functionality)  
**File:** Modify `main.cpp`

**Description:**  
Update main loop to map all 16 knobs to direct parameter access.

**Implementation Details:**

Map knobs to parameters:
- **K11 (I11):** BPM (tempo) - 50-360 BPM in 5 BPM increments
- **K8 (I8):** Gate threshold (noise_gate_thresh)
- **K3 (I3):** Jump probability (probability_jump)
- **K1 (I1):** Tunnel probability (probability_tunnel)
- **K0 (I0):** Reverse probability (probability_direction)
- **K9 (I9):** Gate probability (probability_gate)
- **K2 (I2):** Retrigger probability (probability_retrig)
- **K5 (I5):** Break effects macro (param_set_break)
- **K7 (I7):** Stretch (stretch_change)
- **K6 (I6):** Low-pass filter cutoff (46 positions)
- **K10 (I10):** Volume/wave-folding distortion (param_set_volume)
- **K4 (I4):** Sample selection

**K12-K15 (I12-I15) - To Be Defined:**
- Potential options: Reverb/delay mix, swing/groove, quantize, sample bank select, preset select, etc.

**Dependencies:** Task 1, Task 4

---

### **Task 6: Button Functions - 8 New Controls**
**Status:** Not Started  
**Priority:** Medium (Expanded Controls)  
**File:** Modify `main.cpp`

**Description:**  
Implement functionality for 8 additional buttons (I8-I15).

**Implementation Details:**

**Defined Buttons:**
- **I12 (PLAY/STOP):** Global playback toggle
  - Stop: Freeze playback, silence output
  - Play: Resume playback from current position
  
- **I13 (SEQ REC):** Sequencer record mode
  - Enter sequencer record mode
  - Record button press patterns
  - Exit record mode on second press
  
- **I14 (SEQ ERASE):** Clear sequence pattern
  - Erase current sequencer pattern
  - Confirmation mechanism (hold or double-press?)
  
- **I15 (SEQ ON/OFF):** Toggle sequencer playback
  - Enable: Play recorded pattern
  - Disable: Manual button control only

**To Be Defined (I8-I11 / Y1-Y4):**
- Potential options: 
  - Sample bank select (4 banks?)
  - Preset save/load slots
  - Mute/unmute
  - Tap tempo
  - Effect bypass
  - Quantize on/off

**Dependencies:** Task 2 (Multiplexer Button Reader)

---

### **Task 7: LED Visualization - 16 LED Support**
**Status:** Partially Complete (First 8 LEDs Done)  
**Priority:** Medium (Visual Feedback)  
**File:** Modify `main.cpp`

**Description:**  
Update LED feedback logic to support 16 LEDs with appropriate behaviors.

**Implementation Details:**

**First 8 LEDs (I0-I7):** ✅ **DONE**
- Keep existing beat visualization behavior
- Light up to show currently selected beat/step
- Blink/pulse patterns for active beats

**Additional LEDs:**
- **LED 9-12 (Y1-Y4):** Match button states or status indicators
- **LED 13 (PLAY/STOP):** 
  - Flash on beat when playing
  - Solid when stopped
- **LED 14 (SEQ REC):** 
  - Solid when recording
  - Off when not recording
- **LED 15 (SEQ ERASE):** 
  - Flash during erase operation
  - Off otherwise
- **LED 16 (SEQ ON/OFF):** 
  - Solid when sequencer active
  - Off when sequencer disabled

**Dependencies:** Task 3 (Shift Register LED Driver)

---

### **Task 8: I2S Audio - Verify Implementation**
**Status:** ✅ **COMPLETE**  
**Priority:** Medium (Verification)  
**Files:** Check `doth/i2s_audio.cpp`, `doth/i2s_audio.h`

**Description:**  
Verify existing I2S implementation for GY-PCM5102 16-bit DAC.

**Verification Checklist:** ✅ **ALL VERIFIED**
- ✅ Confirm GPIO18 (BCK), GPIO19 (DIN), GPIO20 (LCK) configuration
- ✅ Verify 16-bit sample format (vs old 8-bit PWM)
- ✅ Confirm sample rate matches target (48kHz per target_architecture.md)
- ✅ Check PIO state machine configuration for I2S protocol
- ✅ Test audio quality vs old PWM output
- ✅ Verify stereo output (mono duplicated to both channels?)
- ✅ Check for audio artifacts, pops, clicks

**Dependencies:** None (existing code)

**Note:** According to project description, I2S audio is "already functional". This task is verification only.

---

### **Task 9: GPIO Pin Assignment Migration**
**Status:** Not Started  
**Priority:** High (Integration)  
**File:** Modify `main.cpp`

**Description:**  
Update all GPIO initializations to match new Pikardcore pinout.

**Migration Details:**

**Remove (Old Picocore):**
- Buttons: GPIO 4-11 (8 direct pins) → Replace with multiplexer
- Knobs: GPIO 26-28 (3 ADC pins) → Replace with multiplexer  
- LEDs: GPIO 12-19 (8 direct pins) → Replace with shift register
- Audio: GPIO 20 (PWM) → Replace with I2S

**Add (New Pikardcore):**
- Multiplexer Select: GPIO 14-17 (S0-S3, shared)
- Knob Multiplexer: GPIO 26 (COM, ADC)
- Button Multiplexer: GPIO 21 (COM, digital input)
- Shift Register: GPIO 22 (SER), GPIO 27 (SRCLK), GPIO 28 (RCLK)
- I2S Audio: GPIO 18 (BCK), GPIO 19 (DIN), GPIO 20 (LCK)

**Keep (Unchanged):**
- MIDI Input: GPIO 1
- Clock In: GPIO 2
- Reset In: GPIO 3
- Clock Out: GPIO 4
- Reset Out: GPIO 5

**Available for Future Use:**
- GPIO 7, 8, 9, 10, 11, 12, 13

**Dependencies:** Tasks 1, 2, 3 (hardware drivers must exist first)

---

### **Task 10: Integration Testing**
**Status:** Not Started  
**Priority:** Medium (Validation)  
**File:** Test on hardware

**Description:**  
Comprehensive hardware validation sequence.

**Test Sequence:**

1. **Multiplexer Knob Test:**
   - Read all 16 knob channels sequentially
   - Verify correct channel selection
   - Check for crosstalk between channels
   - Verify ADC values are stable and responsive

2. **Multiplexer Button Test:**
   - Read all 16 button channels sequentially
   - Verify debouncing works correctly
   - Test rapid button presses
   - Check for missed or double-triggers

3. **Shift Register LED Test:**
   - Light each LED individually (0-15)
   - Verify correct LED mapping (non-sequential)
   - Test patterns (chase, blink, etc.)
   - Check for ghosting or incorrect states

4. **I2S Audio Test:**
   - Play test tones at various frequencies
   - Check for audio artifacts, pops, clicks
   - Verify no dropouts or stuttering
   - Compare quality to old PWM output

5. **Integration Test:**
   - Verify knob → parameter → audio pipeline
   - Test button → LED feedback
   - Check for GPIO conflicts
   - Verify timing/performance (no slowdowns)

**Dependencies:** All previous tasks

---

### **Task 11: Flash Persistence Update**
**Status:** Not Started  
**Priority:** Low (Enhancement)  
**File:** Modify `main.cpp`

**Description:**  
Modify save/load routines to handle new parameters and button states.

**Implementation Details:**
- Expand saved state to include all 16 knob values (vs 3 previously)
- Add sequencer button states (PLAY/STOP, SEQ REC, SEQ ON/OFF)
- Check if 256-byte flash page is sufficient:
  - Old: ~10 parameters × 1-2 bytes + sequencer data
  - New: 16 knob values × 2 bytes + additional button states = ~40+ bytes
  - Should still fit in 256 bytes
- Update save trigger mechanism (no more knob threshold method)
- Consider: Button combo for save (e.g., hold button 1+8 for 2 seconds)
- Consider: Button combo for load (e.g., hold button 1+16 for 2 seconds)

**Dependencies:** Tasks 5, 6 (new parameters defined)

---

### **Task 12: Documentation & Build System**
**Status:** Not Started  
**Priority:** Low (Finalization)  
**Files:** Various documentation and build files

**Description:**  
Final documentation updates and build system verification.

**Checklist:**

1. **Update architecture.md:**
   - Add note about Pikardcore upgrade
   - Reference target_architecture.md
   - Keep as historical reference for Picocore

2. **Create Hardware Guide:**
   - Wiring diagram for 74HC4067 multiplexers
   - Wiring diagram for 74HC595 shift registers
   - GY-PCM5102 DAC connections
   - Bill of Materials (BOM)

3. **Update README.md:**
   - Add Pikardcore features
   - List new controls and capabilities
   - Update build instructions if needed

4. **Build System:**
   - Update CMakeLists.txt if new files added
   - Test build targets: build16, build2
   - Verify all compile-time options work
   - Update requirements.txt if needed

5. **Code Comments:**
   - Document multiplexer timing requirements
   - Add comments for LED mapping (non-sequential)
   - Document I2S configuration

**Dependencies:** All previous tasks

---

## Implementation Priority Order

### Phase 1: Foundation (High Priority)
1. ~~**Task 1** - Multiplexer Knob Reader~~ ✅ **COMPLETE**
2. **Task 2** - Multiplexer Button Reader
3. **Task 3** - Shift Register LED Driver

### Phase 2: Integration (High Priority)
4. **Task 9** - GPIO Pin Assignment Migration
5. **Task 4** - Remove Menu Navigation
6. **Task 5** - Direct Knob Parameter Mapping

### Phase 3: Expansion (Medium Priority)
7. **Task 6** - New Button Functions
8. **Task 7** - 16 LED Visualization (8 LEDs remaining)
9. ~~**Task 8** - Verify I2S Audio~~ ✅ **COMPLETE**

### Phase 4: Finalization (Low Priority)
10. **Task 10** - Integration Testing
11. **Task 11** - Flash Persistence
12. **Task 12** - Documentation & Build System

---

## Estimated Complexity

### High Complexity (Major Refactoring)
- Task 1, 2, 3: New hardware drivers
- Task 4, 5: Control logic refactoring
- Task 9: GPIO migration

### Medium Complexity (New Features)
- Task 6, 7: Button/LED expansion
- Task 11: Flash persistence update

### Low Complexity (Verification)
- Task 8: I2S verification
- Task 10: Testing
- Task 12: Documentation

---

## Notes & Considerations

### Multiplexer Timing
- 74HC4067 has ~200ns propagation delay
- May need small delay after channel selection before ADC read
- Test for crosstalk between channels

### Button Scanning Performance
- 16 buttons + 16 knobs = 32 channels to scan
- Control loop runs at ~20Hz (50ms period)
- Budget: ~1.5ms per channel scan (should be plenty)
- Consider: Interleave button/knob scanning across multiple loops

### LED Update Rate
- Shift register update is very fast (<1μs for 16 bits)
- Can update every control loop iteration (20Hz)
- Consider: Higher update rate for smooth animations?

### Memory Considerations
- Adding 13 more knobs adds minimal RAM overhead
- Flash usage unchanged (code size may increase slightly)
- 256-byte flash persistence page should be sufficient

### Future Expansion Ideas
- MIDI CC output for each knob
- Preset system (save/recall multiple configurations)
- CV input/output (requires additional hardware)
- Screen/OLED display for parameter values
