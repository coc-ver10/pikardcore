// MultiplexerKnob - Read 16 analog knobs via 74HC4067 multiplexer
// 
// Hardware connections:
// - GPIO26 (ADC0): COM pin - analog input from multiplexer
// - GPIO14-17: S0-S3 select pins (shared with button multiplexer)
// 
// Usage:
//   MultiplexerKnob knobs;
//   knobs.Init(200);  // alpha smoothing factor
//   
//   // In main loop:
//   knobs.ReadAll();  // Read all 16 knobs
//   uint16_t value = knobs.Value(channel);  // Get value 0-4095
//   bool changed = knobs.Changed(channel);  // Check if changed

#ifndef MULTIPLEXER_KNOB_H
#define MULTIPLEXER_KNOB_H

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

class MultiplexerKnob {
 private:
  // GPIO pins
  static const uint8_t PIN_COM = 26;    // ADC0 - COM pin
  static const uint8_t PIN_S0 = 14;     // Select bit 0
  static const uint8_t PIN_S1 = 15;     // Select bit 1
  static const uint8_t PIN_S2 = 16;     // Select bit 2
  static const uint8_t PIN_S3 = 17;     // Select bit 3

  // Number of channels
  static const uint8_t NUM_CHANNELS = 16;

  // Settling time after channel selection (microseconds)
  static const uint8_t SETTLING_TIME_US = 10;

  // State for each channel
  struct KnobState {
    uint16_t val_current;   // Current smoothed value
    uint16_t val_last;      // Last reported value
    uint16_t startup;       // Startup delay counter
    bool changed;           // Change flag
  };

  KnobState channels[NUM_CHANNELS];
  uint16_t alpha;          // Smoothing factor (0-1024)
  uint16_t val_max;        // Maximum value (typically 4095)

  // Select a channel on the multiplexer
  void SelectChannel(uint8_t channel) {
    if (channel >= NUM_CHANNELS) return;
    
    // Set S0-S3 pins to select the channel
    gpio_put(PIN_S0, (channel >> 0) & 0x01);
    gpio_put(PIN_S1, (channel >> 1) & 0x01);
    gpio_put(PIN_S2, (channel >> 2) & 0x01);
    gpio_put(PIN_S3, (channel >> 3) & 0x01);
    
    // Wait for signal to settle (74HC4067 propagation delay ~200ns)
    sleep_us(SETTLING_TIME_US);
  }

  // Read ADC value from currently selected channel
  uint16_t ReadADC() {
    adc_select_input(0);  // Select ADC0 (GPIO26)
    return 4095 - adc_read();  // Inverted like original Knob class
  }

 public:
  // Initialize the multiplexer and ADC
  void Init(uint16_t alpha_ = 200) {
    alpha = alpha_;
    val_max = 4095;

    // Initialize select pins as outputs
    gpio_init(PIN_S0);
    gpio_set_dir(PIN_S0, GPIO_OUT);
    gpio_init(PIN_S1);
    gpio_set_dir(PIN_S1, GPIO_OUT);
    gpio_init(PIN_S2);
    gpio_set_dir(PIN_S2, GPIO_OUT);
    gpio_init(PIN_S3);
    gpio_set_dir(PIN_S3, GPIO_OUT);

    // Initialize ADC on COM pin
    adc_init();
    adc_gpio_init(PIN_COM);

    // Initialize all channel states
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
      channels[i].val_current = 0;
      channels[i].val_last = 0;
      channels[i].startup = 800;  // Startup delay like original Knob
      channels[i].changed = false;
    }
  }

  // Read a specific channel
  void Read(uint8_t channel) {
    if (channel >= NUM_CHANNELS) return;

    SelectChannel(channel);
    uint16_t adc = ReadADC();

    // Store raw value (simplified - no smoothing for now)
    channels[channel].val_current = adc;

    // Detect significant change (threshold = 100, same as original Knob)
    channels[channel].changed = 
        abs(channels[channel].val_current - channels[channel].val_last) > 100;
    
    if (channels[channel].changed) {
      channels[channel].val_last = channels[channel].val_current;
    }
  }

  // Read all 16 channels sequentially
  void ReadAll() {
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
      Read(i);
    }
  }

  // Get current value for a channel (0-4095)
  uint16_t Value(uint8_t channel) {
    if (channel >= NUM_CHANNELS) return 0;
    return channels[channel].val_last;
  }

  // Get maximum value
  uint16_t ValueMax() { return val_max; }

  // Check if a channel has changed
  bool Changed(uint8_t channel) {
    if (channel >= NUM_CHANNELS) return false;

    // Handle startup delay
    if (channels[channel].startup > 0) {
      channels[channel].startup--;
      return false;
    }

    return channels[channel].changed;
  }

  // Reset startup delay for a channel
  void Reset(uint8_t channel) {
    if (channel >= NUM_CHANNELS) return;
    channels[channel].startup = 800;
  }

  // Reset all channels
  void ResetAll() {
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
      Reset(i);
    }
  }

  // Get number of channels
  uint8_t NumChannels() { return NUM_CHANNELS; }
};

#endif  // MULTIPLEXER_KNOB_H
