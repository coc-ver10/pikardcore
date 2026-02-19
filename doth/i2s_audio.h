// I2S Audio Output Module
// Provides interrupt-driven I2S audio output via PIO
// Converts 8-bit unsigned audio to 16-bit signed stereo I2S

#ifndef I2S_AUDIO_H
#define I2S_AUDIO_H

#include "hardware/pio.h"
#include "pico/types.h"

class I2SAudio {
private:
    PIO pio;
    uint sm;
    uint offset;
    uint data_pin;
    uint bck_pin;
    uint lck_pin;
    uint32_t sample_rate;
    bool initialized;
    
public:
    // Constructor
    I2SAudio() : pio(nullptr), sm(0), offset(0), initialized(false) {}
    
    // Initialize PIO state machine
    // sample_rate: Audio sample rate in Hz (e.g., 31000)
    // pio_instance: PIO instance to use (pio0 or pio1)
    // state_machine: State machine number (0-3)
    // data_pin_: GPIO for DIN (data out)
    // bck_pin_: GPIO for BCK (bit clock)
    // lck_pin_: GPIO for LCK (word select)
    void Init(uint32_t sample_rate, PIO pio_instance, uint state_machine,
              uint data_pin_, uint bck_pin_, uint lck_pin_);
    
    // Convert 8-bit sample to 16-bit and output to both L/R channels
    // sample_8bit: Unsigned 8-bit audio (0-255, 128=silence)
    void WriteSample(uint8_t sample_8bit);
    
    // Check if FIFO has space
    // Returns true if we can write without blocking
    inline bool CanWrite() {
        if (!initialized) return false;
        return !pio_sm_is_tx_fifo_full(pio, sm);
    }
    
    // Start audio output (enable state machine)
    void Start();
    
    // Stop audio output (disable state machine)
    void Stop();
    
    // Output silence (0x0000 for both channels)
    void WriteSilence();
    
    // Get FIFO level (for debugging)
    inline uint GetFifoLevel() {
        if (!initialized) return 0;
        return pio_sm_get_tx_fifo_level(pio, sm);
    }
};

#endif // I2S_AUDIO_H
