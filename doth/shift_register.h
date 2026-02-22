#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#include "hardware/gpio.h"
#include "pico/stdlib.h"

// 74HC595 Shift Register Driver
// Supports two cascaded shift registers for 16-bit output
// Based on target_architecture.md GPIO assignments
class ShiftRegister {
 private:
  uint8_t gpio_ser;    // Serial data input (GPIO 22)
  uint8_t gpio_srclk;  // Shift register clock (GPIO 27)
  uint8_t gpio_rclk;   // Storage/latch clock (GPIO 28)
  uint16_t state;      // Current 16-bit state buffer

  // Fast bit-bang shift out 16 bits
  // Shifts MSB first (bit 15 down to bit 0)
  // Data flows: bit 15 → first register QH → second register QA → ... → bit 0
  inline void shiftOut16(uint16_t data) {
    // Shift out MSB first (bit 15 to bit 0)
    for (int8_t i = 15; i >= 0; i--) {
      // Set data bit on serial line
      gpio_put(gpio_ser, (data >> i) & 0x01);
      
      // Pulse shift clock to move data into register
      gpio_put(gpio_srclk, 1);
      gpio_put(gpio_srclk, 0);
    }
    
    // Pulse latch clock to transfer shift register to output latches
    gpio_put(gpio_rclk, 1);
    gpio_put(gpio_rclk, 0);
  }

 public:
  // Initialize shift register GPIOs
  void Init(uint8_t ser_pin, uint8_t srclk_pin, uint8_t rclk_pin) {
    gpio_ser = ser_pin;
    gpio_srclk = srclk_pin;
    gpio_rclk = rclk_pin;
    state = 0;

    // Initialize GPIO pins as outputs, all low
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

  // Set individual bit on/off (updates internal state only)
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

  // Update hardware with current state buffer
  // Call this after setting bits to actually update the LEDs
  void Update() {
    shiftOut16(state);
  }

  // Clear all outputs (both state and hardware)
  void Clear() {
    state = 0;
    Update();
  }

  // Set all 16 bits at once and update hardware
  void SetAll(uint16_t value) {
    state = value;
    Update();
  }

  // Get current state buffer
  uint16_t GetState() {
    return state;
  }
};

#endif  // SHIFT_REGISTER_H
