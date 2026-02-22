// Include shift register headers only when needed
#if defined(SHIFT_REGISTER_ENABLED) && SHIFT_REGISTER_ENABLED == 1
#include "shift_register.h"
#include "led_mapper.h"
#endif

class LEDArray {
#if defined(SHIFT_REGISTER_ENABLED) && SHIFT_REGISTER_ENABLED == 1
  // ========================================================================
  // PATH 2: NEW - Shift Register Implementation (16 LEDs - two cascaded registers)
  // ========================================================================
  ShiftRegister shift_reg;
  uint8_t vals[16];      // Brightness values 0-255 for 16 LEDs
  uint8_t dim_i;         // PWM counter for software dimming
  uint16_t do_leds;      // Update counter

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
      uint8_t bit = LEDMapper::LogicalToBit(i);
      shift_reg.SetBit(bit, v > 0);
    }
  }

  void LedUpdate(uint8_t i) {
    // Software PWM: called frequently for individual LED
    if (i < 16) {
      uint8_t bit = LEDMapper::LogicalToBit(i);
      shift_reg.SetBit(bit, dim_i < vals[i]);
    }
  }

  void Update() {
    dim_i++;
    
    // Debug: Print state periodically
    static uint32_t debug_counter = 0;
    if (++debug_counter >= 20000) {  // Every 20000 calls (~1 second at 20kHz)
      debug_counter = 0;
      printf("[LED Update] dim_i=%d, vals[0]=%d, vals[1]=%d, vals[7]=%d\n", 
             dim_i, vals[0], vals[1], vals[7]);
    }
    
    // Apply software PWM to all LEDs
    for (uint8_t i = 0; i < 16; i++) {
      uint8_t bit = LEDMapper::LogicalToBit(i);
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

  // === DIAGNOSTIC METHOD ===
  // Direct hardware control bypassing PWM for testing
  void DirectTest(uint8_t led_index) {
    shift_reg.Clear();
    if (led_index < 8) {
      uint8_t bit = LEDMapper::LogicalToBit(led_index);
      shift_reg.SetBit(bit, true);
    }
    shift_reg.Update();
  }
  
  // === NEW METHODS FOR ADDITIONAL LEDs (8-15) ===
  
  // Set Y/parameter LEDs (0-3 corresponds to Y1-Y4)
  void SetYLED(uint8_t y_index, uint16_t brightness) {
    if (y_index < 4) {
      Set(LED_Y1 + y_index, brightness);
    }
  }

  // Set control LEDs with simple on/off
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

#elif I2S_AUDIO_ENABLED == 1
  // ========================================================================
  // PATH 2: LEDs disabled when I2S is active without shift registers
  // (GPIO 18-19 would conflict with legacy GPIO LEDs)
  // ========================================================================
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

#else
  // ========================================================================
  // PATH 3: ORIGINAL - Direct GPIO Implementation (8 LEDs)
  // ========================================================================
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

  bool Continue() {
    do_leds++;
    if (do_leds % 1000 == 0) {
      return true;
    }
    return false;
  }

  void LedSet(uint8_t i, uint8_t v) { led[i].Set(v); }

  void LedUpdate(uint8_t i) { led[i].Update(); }

  void Update() {
    for (uint8_t i = 0; i < 8; i++) {
      if (vals[i] != led[i].Val()) {
        led[i].SetDim(vals[i]);
      }
      led[i].Update();
    }
  }

  void Clear() {
    for (uint8_t i = 0; i < 16; i++) {
      vals[i] = 0;
    }
  }

  void On(uint8_t j) {
    for (uint8_t i = 0; i < 8; i++) {
      led[i].Set(i == j);
    }
  }

  // sets between 0 and 1000
  void Set(uint8_t i, uint16_t v) {
    v = v * 255 / 1000;
    if (v != vals[i]) {
      vals[i] = v;
    }
  }

  // sets between 0 and 1000
  void Add(uint8_t i, uint16_t v) {
    v = v * 255 / 1000;
    if (v != vals[i]) {
      if (vals[i] + v > 255) {
        vals[i] = 255;
      } else {
        vals[i] = v;
      }
    }
  }

  void SetBinary(uint8_t v) {
    uint8_t j = 0;
    for (uint8_t i = 128; i > 0; i = i / 2) {
      vals[j] = 255 * (v & i);
      j++;
    }
  }

  // SetAll sets between 0 and 1000
  void SetAll(uint16_t v) {
    v = v * 2040 / 1000;  // sets between 0 and 2040 to divide between 8 leds
    for (uint8_t i = 0; i < 8; i++) {
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
};
#endif  // I2S_AUDIO_ENABLED / SHIFT_REGISTER_ENABLED
