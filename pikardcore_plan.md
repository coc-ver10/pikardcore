# Pikardcore Implementation Plan

## Project Overview

This document outlines the implementation plan for upgrading **Picocore** to **Pikardcore**, expanding from 3 knobs/8 buttons/8 LEDs to 16 knobs/16 buttons/16 LEDs with I2S audio output.

See [target_architecture.md](target_architecture.md) for detailed hardware specifications.

---

## Recent Updates

### March 2, 2026 - PLAY/STOP Button & LED Fixes

**Completed:**
- ✅ Implemented PLAY/STOP button (I12) with full playback reset
- ✅ Fixed critical PWM overflow bug in LED system (caused flickering)
- ✅ Verified 16 LED shift register cascade operation
- ✅ LED 12 (PLAY/STOP) now correctly indicates playback state

**Technical Details:**
- PLAY/STOP button toggles `do_mute` and fully resets playback state
- Playback reset includes: `phase_sample[]`, `phase_retrig`, `select_beat`, `beat_counter`
- Fixed PWM: Added `if (dim_i > 255) dim_i = 0;` to prevent uint8_t overflow
- Added 1μs timing delays in shift register for signal stability
- LEDs 8-11 and 13-15 remain off (functions not yet implemented)

**Next Steps:**
- Implement remaining control buttons (I13-I15: SEQ REC, ERASE, ON/OFF)
- Define functions for Y buttons (I8-I11)
- Consider adding visual feedback modes for active parameters

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
**Status:** ✅ **COMPLETE**  
**Priority:** High (Foundation)  
**File:** Create `doth/multiplexer_button.h`

**Description:**  
Create a class to read 16 digital buttons via 74HC4067 multiplexer.

**Implementation Details:** ✅ **ALL DONE**
- ✅ Initialize GPIO21 (digital COM), share GPIO14-17 (S0-S3) with knob multiplexer
- ✅ Implement channel selection function (0-15)
- ✅ Read digital state for selected channel (active LOW with pull-up)
- ✅ Add debouncing logic (5-frame debounce, ~80ms at 250Hz)
- ✅ Track button press/release events (On(), Rising(), Falling(), Changed())
- ✅ Support edge detection (ChangedHigh/ChangedLow)

**Implementation Notes:**
- Created `MultiplexerButton` class following same API as original `Button` class
- **15μs settling time** after channel selection (increased from 5μs for stability)
- **10μs delay between button reads** for multiplexer stability
- `MultiplexerManager` singleton handles shared GPIO initialization
- Active-LOW buttons with internal pull-up resistors
- API compatible with original button code for easy migration
- Debounce: 5 frames (vs 10 originally) for faster combo detection

**Issues Resolved:**
- Initial 5μs settling time caused unreliable reads → increased to 15μs
- Button 7 worked better due to being read last → added 10μs delays between reads
- Beat/retrigger detection required buttons read at beat onset (not continuous)
- Retrigger effects needed 16-bit audio adaptation (volume shifts * 2)

**Dependencies:** None

---

### **Task 3: Hardware Interface - Shift Register LED Driver**
**Status:** ✅ **COMPLETE**  
**Priority:** High (Foundation)  
**File:** `doth/shift_register.h`, `doth/led_mapper.h`

**Description:**  
Implement 16 LED control via cascaded 74HC595 shift registers.

**Implementation Details:** ✅ **ALL DONE**
- ✅ Initialize GPIO22 (SER), GPIO27 (SRCLK), GPIO28 (RCLK)
- ✅ Support 16-bit output (2 cascaded 8-bit registers)
- ✅ Handle non-sequential LED mapping per target_architecture.md:
  - LEDS_1: QA=LED8, QB=LED1, QC=LED5, QD=LED2, QE=LED6, QF=LED3, QG=LED7, QH=LED4
  - LEDS_2: QA=LED16, QB=LED9, QC=LED13, QD=LED10, QE=LED14, QF=LED11, QG=LED15, QH=LED12
- ✅ Implement efficient bulk update method (ShiftOut16() - shift all 16 bits at once)
- ✅ Add individual LED set/clear methods (SetBit(), ClearBit())
- ✅ LED mapper created in `doth/led_mapper.h` for logical-to-physical mapping

**Implementation Notes:**
- `ShiftRegister` class handles low-level bit shifting
- `led_to_bit[16]` lookup table in `led_mapper.h` maps logical LEDs (0-15) to physical bits
- Non-sequential mapping handled transparently
- GPIO conflict avoided: GPIO 27/28 used for shift register, NOT for ADC
- Update rate: Every control loop (~20Hz), fast enough for smooth visualization

**Dependencies:** None

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
**Status:** 🟡 **PARTIALLY COMPLETE**  
**Priority:** Medium (Expanded Controls)  
**File:** Modify `main.cpp`

**Description:**  
Implement functionality for 8 additional buttons (I8-I15).

**Implementation Details:**

**Implemented Buttons:** ✅
- **I12 (PLAY/STOP):** ✅ **COMPLETE** - Global playback toggle
  - Stop: Sets `do_mute = true`, freezes playback
  - Play: Resets playback completely (phases, beat counter, select_beat)
  - Works with original combo (buttons 0+1+6+7) and dedicated button I12
  - Playback restarts from beat 0 (step 1) on play
  
**Not Yet Implemented:**
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

**Implementation Notes:**
- Button I12 reads via multiplexer at 250Hz (every 4ms)
- `do_start_everything()` fully resets: phase_sample[], phase_retrig, select_beat, beat_counter
- Reset flag `btn_reset` properly skips beat increment to start at beat 0
- Compatible with existing play/stop combo logic

**Dependencies:** Task 2 (Multiplexer Button Reader) ✅ Complete

---

### **Task 7: LED Visualization - 16 LED Support**
**Status:** ✅ **COMPLETE**  
**Priority:** Medium (Visual Feedback)  
**File:** Modify `main.cpp`, `doth/ledarray.h`, `doth/shift_register.h`

**Description:**  
Update LED feedback logic to support 16 LEDs with appropriate behaviors.

**Implementation Details:** ✅ **ALL DONE**

**First 8 LEDs (0-7):** ✅ **DONE**
- Keep existing beat visualization behavior
- Light up to show currently selected beat/step (`select_beat % 8`)
- Works correctly with multiplexer button input
- Brightness: 1000 (maps to 255 internally for full brightness)

**Second 8 LEDs (8-15):** ✅ **DONE**
- **LEDs 8-11 (Y1-Y4):** Off (functions not yet defined)
- **LED 12 (PLAY/STOP):** ✅ ON when playing (`!do_mute`), OFF when stopped
- **LEDs 13-15 (SEQ controls):** Off (functions not yet implemented)
- Persistent state indicators (not momentary button feedback)

**Critical Fixes Applied:** ✅
- **PWM Overflow Fix:** Added `if (dim_i > 255) dim_i = 0;` to prevent PWM counter overflow
  - Previous issue: dim_i would overflow uint8_t causing rapid flickering
  - Result: All LEDs now display steadily without flickering
- **Shift Register Timing:** Added 1μs delays between clock pulses for signal stability
- **Cascade Verification:** Confirmed bits 0-7 → LEDS_1, bits 8-15 → LEDS_2
- **LED Mapping:** Verified against target_architecture.md non-sequential output mapping

**Implementation Notes:**
- Both LED groups update at control loop rate (~20Hz in main loop)
- Software PWM: `dim_i < vals[i]` determines LED state each cycle
- LED mapping handled by `led_mapper.h` lookup table
- Shift register updates entire 16-bit state efficiently
- All 16 LEDs tested and working correctly

**Current LED Behaviors:**
- **LEDs 0-7:** Beat position indicator (one LED lit, cycles with beat)
- **LED 12:** Play/Stop status (ON=playing, OFF=stopped)
- **LEDs 8-11, 13-15:** Reserved for future functions (currently off)

**Dependencies:** Task 3 (Shift Register LED Driver) ✅ Complete

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

## Implementation Issues & Resolutions

### Multiplexer Button Reading (Task 2)

**Issue 1: Unreliable Button Detection**
- **Problem:** Buttons not detected reliably, especially combinations. Button 7 worked better than others.
- **Root Cause:** 5μs settling time insufficient for 74HC4067 multiplexer channel switching. Button 7 read last had no subsequent switches interfering.
- **Solution:** Increased settling time to 15μs + added 10μs delay between button reads.

**Issue 2: GPIO Pin Conflict**
- **Problem:** Initial design had TRIGO_PIN on GPIO 21, conflicting with BTN_MUX_COM.
- **Solution:** Moved TRIGO_PIN from GPIO 21 to GPIO 10.

**Issue 3: Retrigger Effects Not Audible**
- **Problem:** Single button jumps worked, but two-button retrigger effects were barely noticeable (except button 7-x combinations).
- **Root Cause:** Retrigger effects calibrated for 8-bit PWM audio (0-255 unsigned, center=128). I2S uses 16-bit signed audio (-32768 to +32767, center=0). Volume reduction by bit-shifting had different effect.
- **Solution:** 
  - Doubled volume reduction shift amount (`total_volume_reduce * 2` instead of `* 1`)
  - Re-enabled bitcrush effect, adapted for signed 16-bit (center at 0, not 128)
  - Kept button 7-x having longer durations (`retrig_max * 2`) for most obvious effects
- **Status:** Retriggers now audible, 7-x combinations work best (as designed)

**Issue 4: Beat Detection Required for Retriggers**
- **Problem:** Initially tried detecting button combinations continuously (250Hz main loop). This broke both jumps and retriggers.
- **Root Cause:** Original Picocore logic detects buttons only at beat onset, not continuously. Beat timing critical for effect synchronization.
- **Solution:** Restored button detection to beat onset block. Single button = jump to beat. Two buttons = retrigger effect.

### 16-Bit Audio Adaptation

**Audio Format Changes:**
- **Old (Picocore):** 8-bit PWM, unsigned (0-255), center = 128
- **New (Pikardcore):** 16-bit I2S, signed (-32768 to +32767), center = 0

**Effect Adaptations Required:**
- Volume reduction: Bit shifts need to account for 8-bit more dynamic range
- Bitcrush: Updated to work with signed values (mask around 0, not 128)
- Filter: Disabled pending adaptation (uses 8-bit lookup tables)

---

## Implementation Priority Order

### Phase 1: Foundation (High Priority) - ✅ **COMPLETE**
1. ~~**Task 1** - Multiplexer Knob Reader~~ ✅ **COMPLETE**
2. ~~**Task 2** - Multiplexer Button Reader~~ ✅ **COMPLETE**
3. ~~**Task 3** - Shift Register LED Driver~~ ✅ **COMPLETE**

### Phase 2: Integration (High Priority) - **IN PROGRESS**
4. **Task 9** - GPIO Pin Assignment Migration - **PARTIALLY DONE**
   - ✅ Button multiplexer GPIO assigned (21, 14-17)
   - ✅ Shift register GPIO assigned (22, 27, 28)
   - ✅ I2S GPIO confirmed (18, 19, 20)
   - ⏳ Remaining: Full knob multiplexer integration
5. **Task 4** - Remove Menu Navigation - **NOT STARTED**
6. **Task 5** - Direct Knob Parameter Mapping - **NOT STARTED**

### Phase 3: Expansion (Medium Priority)
7. **Task 6** - New Button Functions - **NOT STARTED**
   - Buttons 0-7: Jumps/retriggers work ✅
   - Buttons 8-15: Functions to be defined
8. ~~**Task 7** - 16 LED Visualization~~ ✅ **COMPLETE**
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

---

## Current Configuration Summary (as of March 2, 2026)

### Hardware Working ✅
- **16 Buttons via Multiplexer:** GPIO 21 (COM), GPIO 14-17 (S0-S3)
  - Buttons 0-7: Beat jumps + retrigger combos functional
  - Buttons 8-15: Read correctly, functions to be defined
  - Settling: 15μs after channel switch, 10μs between reads
  - Debounce: 5 frames (~80ms at 250Hz)

- **16 LEDs via Shift Register:** GPIO 22 (SER), GPIO 27 (SRCLK), GPIO 28 (RCLK)
  - LEDs 0-7: Beat position visualization
  - LEDs 8-15: Mirror button 8-15 states
  - Non-sequential physical mapping handled by `led_mapper.h`

- **I2S Audio:** GPIO 18 (BCK), GPIO 19 (DIN), GPIO 20 (LCK)
  - GY-PCM5102 DAC module
  - 48kHz, 16-bit stereo (mono duplicated)
  - Effects adapted for signed 16-bit audio

- **3 Knobs (Legacy):** GPIO 26, 27, 28 (ADC) - **CONFLICT with shift register!**
  - Currently disabled due to GPIO 27/28 conflict with shift register
  - Need multiplexer implementation to resolve

### Known Issues ⚠️
- Retrigger effects: Button 7-x combinations most audible, others less distinct
- Filter DSP: Disabled (8-bit lookup tables need 16-bit adaptation)
- Knobs: 3 direct knobs disabled due to GPIO conflict with shift register

### Next Steps 🎯
1. Implement 16-knob multiplexer (resolve GPIO conflict)
2. Define functions for buttons 8-15 (PLAY/STOP, SEQ_REC, etc.)
3. Adapt filter for 16-bit audio (or rebuild for I2S)
4. Remove menu navigation, implement direct parameter mapping
