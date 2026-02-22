#ifndef LED_MAPPER_H
#define LED_MAPPER_H

#include "pico/stdlib.h"

// LED Mapper: Maps logical LED indices to physical shift register bits
// Based on target_architecture.md hardware wiring
//
// Logical indices (what the code uses):
//   0-7:  STEP_1 through STEP_8 (sequencer steps)
//   8-11: Y1 through Y4 (parameter indicators)
//   12:   PLAY_STOP
//   13:   SEQ_REC
//   14:   SEQ_ERASE
//   15:   SEQ_ON_OFF
//
// Physical bits (shift register outputs):
//   First register (bits 0-7):   STEP LEDs in non-sequential order
//   Second register (bits 8-15): Control and Y LEDs
//
class LEDMapper {
 public:
  // Convert logical LED index (0-15) to shift register bit position (0-15)
  static uint8_t LogicalToBit(uint8_t logical_index) {
    if (logical_index < 16) {
      return led_to_bit[logical_index];
    }
    return 0;  // Default to bit 0 if out of range
  }

 private:
  // Lookup table: logical_index -> physical_bit
  // Based on target_architecture.md SHIFT REGISTER sections
  static const uint8_t led_to_bit[16];
};

// LED mapping implementation
// Index:  Logical LED position
// Value:  Physical shift register bit position
const uint8_t LEDMapper::led_to_bit[16] = {
    1,   // 0: STEP_1 -> First:QB (bit 1)
    3,   // 1: STEP_2 -> First:QD (bit 3)
    5,   // 2: STEP_3 -> First:QF (bit 5)
    7,   // 3: STEP_4 -> First:QH (bit 7)
    2,   // 4: STEP_5 -> First:QC (bit 2)
    4,   // 5: STEP_6 -> First:QE (bit 4)
    6,   // 6: STEP_7 -> First:QG (bit 6)
    0,   // 7: STEP_8 -> First:QA (bit 0)
    10,  // 8: Y1 -> Second:QC (bit 10)
    12,  // 9: Y2 -> Second:QE (bit 12)
    14,  // 10: Y3 -> Second:QG (bit 14)
    8,   // 11: Y4 -> Second:QA (bit 8)
    9,   // 12: PLAY_STOP -> Second:QB (bit 9)
    11,  // 13: SEQ_REC -> Second:QD (bit 11)
    13,  // 14: SEQ_ERASE -> Second:QF (bit 13)
    15   // 15: SEQ_ON_OFF -> Second:QH (bit 15)
};

// Named constants for LED indices (for code clarity)
// These match the logical indices above
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
