#ifndef MULTIPLEXER_BUTTON_H
#define MULTIPLEXER_BUTTON_H

#include "hardware/gpio.h"
#include "pico/stdlib.h"

// 74HC4067 16-Channel Multiplexer for Button Reading
// Based on target_architecture.md GPIO assignments
// COM pin (GPIO 21) reads the current selected button
// Select pins (GPIO 14-17) choose which of 16 channels to read

class MultiplexerButton {
 private:
  uint8_t gpio_com;    // Common pin (GPIO 21)
  uint8_t gpio_s0;     // Select bit 0 (GPIO 14)
  uint8_t gpio_s1;     // Select bit 1 (GPIO 15)
  uint8_t gpio_s2;     // Select bit 2 (GPIO 16)
  uint8_t gpio_s3;     // Select bit 3 (GPIO 17)
  
  bool val[2];         // Current and previous state
  bool rising;
  bool falling;
  bool changed;
  uint16_t debounce;
  uint16_t debounce_time;
  uint8_t channel;     // Which channel (0-15) this button is on

  // Select a channel on the multiplexer
  inline void SelectChannel(uint8_t ch) {
    gpio_put(gpio_s0, (ch >> 0) & 0x01);
    gpio_put(gpio_s1, (ch >> 1) & 0x01);
    gpio_put(gpio_s2, (ch >> 2) & 0x01);
    gpio_put(gpio_s3, (ch >> 3) & 0x01);
    // Small delay to allow multiplexer to settle
    // 74HC4067 typical propagation delay: ~200ns, but add margin for safety
    // Increased to 5µs for more reliable readings with capacitance on lines
    sleep_us(5);
  }

 public:
  void Init(uint8_t channel_, uint8_t com_pin, uint8_t s0, uint8_t s1, 
            uint8_t s2, uint8_t s3, uint16_t debounce_time_) {
    channel = channel_;
    gpio_com = com_pin;
    gpio_s0 = s0;
    gpio_s1 = s1;
    gpio_s2 = s2;
    gpio_s3 = s3;
    debounce_time = debounce_time_;
    
    // Note: GPIO pins are initialized once in main.cpp, not per button
    // This just stores the configuration
    val[1] = false;
    rising = false;
    falling = false;
    changed = false;
    debounce = 0;
  }

  bool On() { return val[0]; }

  void Set(bool v) {
    val[0] = v;
    rising = val[0] > val[1];
    falling = val[0] < val[1];
    changed = rising || falling;
    if (changed) debounce = debounce_time;
    val[1] = val[0];
  }

  void Read() {
    if (debounce == 0) {
      // Select this button's channel
      SelectChannel(channel);
      // Read the COM pin (active LOW with pull-up)
      bool button_state = (bool)(1 - gpio_get(gpio_com));
      Set(button_state);
    } else {
      changed = false;
      rising = false;
      falling = false;
      debounce--;
    }
  }

  bool Changed(bool reset) {
    if (changed && reset) {
      changed = false;
      return true;
    }
    return changed;
  }

  bool ChangedHigh(bool reset) {
    if (changed && reset) {
      changed = false;
      return true && val[1];
    }
    return changed && val[1];
  }

  bool Rising() { return rising; }
  bool Falling() { return falling; }
};

// Multiplexer Manager Class (singleton pattern)
// Manages the shared GPIO pins for all multiplexed buttons
class MultiplexerManager {
 private:
  static bool initialized;

 public:
  static void Init(uint8_t com_pin, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3) {
    if (!initialized) {
      // Initialize COM pin as input with pull-up
      gpio_init(com_pin);
      gpio_set_dir(com_pin, GPIO_IN);
      gpio_pull_up(com_pin);
      
      // Initialize select pins as outputs
      gpio_init(s0);
      gpio_set_dir(s0, GPIO_OUT);
      gpio_put(s0, 0);
      
      gpio_init(s1);
      gpio_set_dir(s1, GPIO_OUT);
      gpio_put(s1, 0);
      
      gpio_init(s2);
      gpio_set_dir(s2, GPIO_OUT);
      gpio_put(s2, 0);
      
      gpio_init(s3);
      gpio_set_dir(s3, GPIO_OUT);
      gpio_put(s3, 0);
      
      initialized = true;
    }
  }
};

// Static member initialization
bool MultiplexerManager::initialized = false;

#endif  // MULTIPLEXER_BUTTON_H
