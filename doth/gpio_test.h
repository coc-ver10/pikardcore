#ifndef GPIO_TEST_H
#define GPIO_TEST_H

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Ultra-simple GPIO test - manually toggle shift register pins
// to verify hardware connections
void test_gpio_manual() {
    printf("\n=== Manual GPIO Test ===\n");
    printf("This will toggle pins and shift data to verify hardware\n\n");
    
    const uint8_t SER = 22;
    const uint8_t SRCLK = 27;
    const uint8_t RCLK = 28;
    
    // Initialize pins
    gpio_init(SER);
    gpio_set_dir(SER, GPIO_OUT);
    gpio_init(SRCLK);
    gpio_set_dir(SRCLK, GPIO_OUT);
    gpio_init(RCLK);
    gpio_set_dir(RCLK, GPIO_OUT);
    
    printf("GPIOs initialized (SER=22, SRCLK=27, RCLK=28)\n");
    
    // Clear everything first
    gpio_put(SER, 0);
    gpio_put(SRCLK, 0);
    gpio_put(RCLK, 0);
    sleep_ms(100);
    
    // Test 1: Shift out all 1s to light all LEDs
    printf("\n[Test 1] Shifting out all 1s - ALL 16 LEDs should turn ON\n");
    
    for (int i = 0; i < 16; i++) {
        gpio_put(SER, 1);  // Set data HIGH
        sleep_us(10);
        gpio_put(SRCLK, 1);  // Pulse clock
        sleep_us(10);
        gpio_put(SRCLK, 0);
        sleep_us(10);
    }
    
    // Latch the data to outputs
    gpio_put(RCLK, 1);
    sleep_us(10);
    gpio_put(RCLK, 0);
    
    printf("        All 16 LEDs should be ON now!\n");
    sleep_ms(3000);
    
    // Test 2: Shift out all 0s to turn off all LEDs
    printf("\n[Test 2] Shifting out all 0s - ALL LEDs should turn OFF\n");
    
    for (int i = 0; i < 16; i++) {
        gpio_put(SER, 0);  // Set data LOW
        sleep_us(10);
        gpio_put(SRCLK, 1);  // Pulse clock
        sleep_us(10);
        gpio_put(SRCLK, 0);
        sleep_us(10);
    }
    
    gpio_put(RCLK, 1);
    sleep_us(10);
    gpio_put(RCLK, 0);
    
    printf("        All LEDs should be OFF now\n");
    sleep_ms(2000);
    
    // Test 3: Walking bit pattern
    printf("\n[Test 3] Walking bit - one LED at a time (bit 15 to 0)\n");
    
    for (int bit = 15; bit >= 0; bit--) {
        printf("        Lighting LED at bit position %d\n", bit);
        
        // Shift out the pattern (MSB first, so shift from bit 15 to 0)
        for (int i = 15; i >= 0; i--) {
            gpio_put(SER, (i == bit) ? 1 : 0);
            sleep_us(10);
            gpio_put(SRCLK, 1);
            sleep_us(10);
            gpio_put(SRCLK, 0);
            sleep_us(10);
        }
        
        gpio_put(RCLK, 1);
        sleep_us(10);
        gpio_put(RCLK, 0);
        
        sleep_ms(500);
    }
    
    // Clear at end
    for (int i = 0; i < 16; i++) {
        gpio_put(SER, 0);
        sleep_us(10);
        gpio_put(SRCLK, 1);
        sleep_us(10);
        gpio_put(SRCLK, 0);
        sleep_us(10);
    }
    gpio_put(RCLK, 1);
    sleep_us(10);
    gpio_put(RCLK, 0);
    
    printf("\n=== Manual GPIO Test Complete ===\n");
    printf("If LEDs didn't respond, check:\n");
    printf("  - OE pin (pin 13) must be connected to GND\n");
    printf("  - VCC and GND connected to shift register\n");
    printf("  - LED polarity (anode to SR output, cathode to GND via resistor)\n");
    printf("  - GPIO connections (22=SER/pin14, 27=SRCLK/pin11, 28=RCLK/pin12)\n\n");
}

#endif  // GPIO_TEST_H
