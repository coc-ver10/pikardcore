// I2S Audio Output Implementation

#include "i2s_audio.h"
#include "i2s_audio.pio.h"
#include "hardware/clocks.h"
#include <stdio.h>

void I2SAudio::Init(uint32_t sample_rate_, PIO pio_instance, uint state_machine,
                    uint data_pin_, uint bck_pin_, uint lck_pin_) {
    pio = pio_instance;
    sm = state_machine;
    data_pin = data_pin_;
    bck_pin = bck_pin_;
    lck_pin = lck_pin_;
    sample_rate = sample_rate_;
    
    // Load PIO program
    offset = pio_add_program(pio, &i2s_audio_program);
    
    // Get system clock frequency
    uint32_t system_clock_hz = clock_get_hz(clk_sys);
    
    // Initialize PIO state machine (enables it at end)
    i2s_audio_program_init(pio, sm, offset, data_pin, bck_pin, lck_pin, 
                          sample_rate, system_clock_hz);
    
    initialized = true;
    
    // Pre-fill FIFO with silence to prevent stalling
    // This ensures smooth startup when timer begins
    for (int i = 0; i < 4; i++) {
        pio_sm_put(pio, sm, 0x00000000);  // Silence
    }
    
#ifdef DEBUG_I2S
    printf("I2S Audio initialized:\n");
    printf("  PIO: %s, SM: %d\n", pio == pio0 ? "pio0" : "pio1", sm);
    printf("  Data pin: %d, BCK pin: %d, LCK pin: %d\n", data_pin, bck_pin, lck_pin);
    printf("  Sample rate: %d Hz\n", sample_rate);
    printf("  System clock: %d Hz\n", system_clock_hz);
    printf("  BCK frequency: %d Hz\n", sample_rate * 32);
    printf("  Clock divider: %.2f\n", (float)system_clock_hz / (sample_rate * 64));
#endif
}

void I2SAudio::WriteSample(uint8_t sample_8bit) {
    if (!initialized) return;
    
    // Convert 8-bit unsigned (0-255, center at 128) to 16-bit signed
    // 8-bit: 0 = most negative, 128 = silence, 255 = most positive
    // 16-bit: -32768 = most negative, 0 = silence, +32767 = most positive
    int16_t sample_16bit = ((int16_t)sample_8bit - 128) << 8;
    
    // Create 32-bit I2S data: [Left 16-bit][Right 16-bit]
    // Duplicate mono to both channels
    uint32_t i2s_data = ((uint32_t)(uint16_t)sample_16bit << 16) | (uint32_t)(uint16_t)sample_16bit;
    
    // Write to PIO FIFO (non-blocking, caller should check CanWrite first)
    pio_sm_put(pio, sm, i2s_data);
}

void I2SAudio::WriteSilence() {
    if (!initialized) return;
    
    // Silence is 0x00000000 (both channels at 0)
    pio_sm_put(pio, sm, 0x00000000);
}

void I2SAudio::Start() {
    if (!initialized) return;
    pio_sm_set_enabled(pio, sm, true);
}

void I2SAudio::Stop() {
    if (!initialized) return;
    pio_sm_set_enabled(pio, sm, false);
}
