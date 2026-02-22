// Shift Register LED Test Functions
// Add these to your main.cpp for hardware verification
// Requires global ledarray instance to be defined in main.cpp

#ifdef SHIFT_REGISTER_ENABLED
#if SHIFT_REGISTER_ENABLED == 1

// Forward declare - ledarray is defined in main.cpp
class LEDArray;
extern LEDArray ledarray;

// Test 1: Sequential LED test (verifies wiring and mapping)
void test_leds_sequential() {
    printf("Testing LEDs sequentially (0-15)...\n");
    for (uint8_t i = 0; i < 16; i++) {
        ledarray.Clear();
        ledarray.Set(i, 1000);  // Full brightness
        ledarray.Update();
        printf("LED %d (", i);
        
        // Print LED name
        const char* names[] = {
            "STEP_1", "STEP_2", "STEP_3", "STEP_4",
            "STEP_5", "STEP_6", "STEP_7", "STEP_8",
            "Y1", "Y2", "Y3", "Y4",
            "PLAY_STOP", "SEQ_REC", "SEQ_ERASE", "SEQ_ON_OFF"
        };
        printf("%s) ON\n", names[i]);
        
        sleep_ms(300);
    }
    ledarray.Clear();
    ledarray.Update();
    printf("Test complete!\n");
}

// Test 2: Step LEDs in chasing pattern
void test_step_leds_chase() {
    printf("Testing step LEDs (chasing pattern)...\n");
    for (uint8_t cycle = 0; cycle < 5; cycle++) {
        for (uint8_t i = 0; i < 8; i++) {
            ledarray.Clear();
            ledarray.Set(i, 1000);
            if (i > 0) {
                ledarray.Set(i - 1, 300);  // Trailing dim LED
            }
            ledarray.Update();
            sleep_ms(100);
        }
    }
    ledarray.Clear();
    ledarray.Update();
}

// Test 3: Binary counter display (verifies SetBinary)
void test_binary_counter() {
    printf("Testing binary display (0-255)...\n");
    for (uint16_t i = 0; i <= 255; i++) {
        ledarray.SetBinary((uint8_t)i);
        ledarray.Update();
        sleep_ms(50);
    }
    ledarray.Clear();
    ledarray.Update();
}

// Test 4: PWM dimming test (verifies software PWM)
void test_pwm_dimming() {
    printf("Testing PWM dimming on STEP_1...\n");
    
    // Fade in
    for (uint16_t b = 0; b <= 1000; b += 10) {
        ledarray.Set(0, b);
        ledarray.Update();
        sleep_ms(5);
    }
    
    // Fade out
    for (uint16_t b = 1000; b > 0; b -= 10) {
        ledarray.Set(0, b);
        ledarray.Update();
        sleep_ms(5);
    }
    
    ledarray.Clear();
    ledarray.Update();
    printf("PWM test complete!\n");
}

// Test 5: All step LEDs at various brightness
void test_brightness_levels() {
    printf("Testing brightness levels...\n");
    
    uint16_t levels[] = {100, 250, 500, 750, 1000};
    for (uint8_t l = 0; l < 5; l++) {
        for (uint8_t i = 0; i < 16; i++) {
            ledarray.Set(i, levels[l]);
        }
        ledarray.Update();
        printf("Brightness: %d/1000\n", levels[l]);
        sleep_ms(1000);
    }
    
    ledarray.Clear();
    ledarray.Update();
}

// Test 7: Stress test (rapid updates)
void test_rapid_updates() {
    printf("Testing rapid updates (stress test)...\n");
    
    for (uint16_t cycle = 0; cycle < 1000; cycle++) {
        // Random pattern
        for (uint8_t i = 0; i < 16; i++) {
            ledarray.Set(i, (rand() % 1000));
        }
        ledarray.Update();
        
        if (cycle % 100 == 0) {
            printf("Cycle %d/1000\n", cycle);
        }
    }
    
    ledarray.Clear();
    ledarray.Update();
    printf("Stress test complete!\n");
}

// Test 8: All LEDs validation
void test_all_leds_on() {
    printf("Testing all 16 LEDs ON...\n");
    ledarray.SetAll(1000);
    ledarray.Update();
    printf("All 16 LEDs should be ON - verify visually\n");
    sleep_ms(3000);
    
    ledarray.Clear();
    ledarray.Update();
}

// Master test function - runs all tests in sequence
void run_all_led_tests() {
    printf("\n=== Starting LED Test Suite ===\n\n");
    
    test_all_leds_on();
    sleep_ms(1000);
    
    test_leds_sequential();
    sleep_ms(1000);
    
    test_step_leds_chase();
    sleep_ms(1000);
    
    test_brightness_levels();
    sleep_ms(1000);
    
    test_pwm_dimming();
    sleep_ms(1000);
    
    test_binary_counter();
    sleep_ms(1000);
    
    test_rapid_updates();
    
    printf("\n=== All Tests Complete! ===\n");
}

#endif  // SHIFT_REGISTER_ENABLED == 1
#endif  // SHIFT_REGISTER_ENABLED

// Usage in main():
// 
// int main() {
//     // ... your initialization code ...
//     
// #if SHIFT_REGISTER_ENABLED == 1
//     // Run single test:
//     test_leds_sequential();
//     
//     // Or run all tests:
//     // run_all_led_tests();
// #endif
//     
//     // ... rest of your code ...
// }
