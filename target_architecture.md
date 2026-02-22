# Target Architecture - GPIO Pin Mappings

## MULTIPLEXER POTARDS

| GPIO | Function | Details |
| --- | --- | --- |
| GPIO26 | COM | Common pin |
| GPIO14 | S0 | Select bit 0 |
| GPIO15 | S1 | Select bit 1 |
| GPIO16 | S2 | Select bit 2 |
| GPIO17 | S3 | Select bit 3 |

### Multiplexer Inputs (Potentiometers)

| Input | Function | Notes |
| --- | --- | --- |
| I0 | REVERSE PROB | |
| I1 | TUNNEL PROB | |
| I2 | RETRIG PROB | |
| I3 | JUMP PROB | |
| I4 | SAMPLE | |
| I5 | BREAK | |
| I6 | FILTER | |
| I7 | STRETCH | |
| I8 | GATE | |
| I9 | GATE PROB | |
| I10 | VOLUME / FOLD | |
| I11 | TEMPO | |
| I12 | X4 | to define |
| I13 | X3 | to define |
| I14 | X2 | to define |
| I15 | X1 | to define |

## MULTIPLEXER KEYBOARD

| GPIO | Function | Details |
| --- | --- | --- |
| GPIO21 | COM | Common pin |
| GPIO14 | S0 | Select bit 0 (shared) |
| GPIO15 | S1 | Select bit 1 (shared) |
| GPIO16 | S2 | Select bit 2 (shared) |
| GPIO17 | S3 | Select bit 3 (shared) |

### Multiplexer Inputs (Buttons)

| Input | Function | Notes |
| --- | --- | --- |
| I0 | STEP 1 | |
| I1 | STEP 2 | |
| I2 | STEP 3 | |
| I3 | STEP 4 | |
| I4 | STEP 5 | |
| I5 | STEP 6 | |
| I6 | STEP 7 | |
| I7 | STEP 8 | |
| I8 | Y1 | to define |
| I9 | Y2 | to define |
| I10 | Y3 | to define |
| I11 | Y4 | to define |
| I12 | PLAY / STOP | |
| I13 | SEQ REC | |
| I14 | SEQ ERASE | |
| I15 | SEQ ON / OFF | |

## SHIFT REGISTER LEDS_1

| GPIO | Function | Details |
| --- | --- | --- |
| GPIO22 | SER | Serial data input |
| GPIO27 | SRCLK | Shift register clock |
| GPIO28 | RCLK | Storage register clock |

### Shift Register Outputs

| Output | Function | Notes |
| --- | --- | --- |
| QA (0) | LED STEP 8 | |
| QB (1) | LED STEP 1 | |
| QC (2) | LED STEP 5 | |
| QD (3) | LED STEP 2 | |
| QE (4) | LED STEP 6 | |
| QF (5) | LED STEP 3 | |
| QG (6) | LED STEP 7 | |
| QH (7) | LED STEP 4 | |
| QH' | SER SECOND SHIFT | Cascaded to LEDS_2 |

## SHIFT REGISTER LEDS_2

| GPIO | Function | Details |
| --- | --- | --- |
| QH' LEDS_1 | SER | Serial input from LEDS_1 |
| GPIO27 | SRCLK | Shift register clock (shared) |
| GPIO28 | RCLK | Storage register clock (shared) |

### Shift Register Outputs

| Output | Function | Notes |
| --- | --- | --- |
| QA | LED Y4 | |
| QB | LED PLAY / STOP | |
| QC | LED Y1 | |
| QD | LED SEQ REC | |
| QE | LED Y2 | |
| QF | LED SEQ ERASE | |
| QG | LED Y3 | |
| QH | LED SEQ ON / OFF | |
| QH' | (unused) | |

## DAC AUDIO OUT - I2S PCM5102

| GPIO | Function | Details |
| --- | --- | --- |
| GPIO18 | BCK | Bit clock |
| GPIO19 | DIN | Data in |
| GPIO20 | LCK | Left/Right clock (LRCLK) |

## OTHER GPIO

| GPIO | Function | Notes |
| --- | --- | --- |
| GPIO1 | MIDI INPUT | |
| GPIO4 | CLOCK OUT | |
| GPIO5 | RESET OUT | |
| GPIO2 | CLOCK IN | |
| GPIO3 | RESET IN | |
| GPIO7 | (to define) | |
| GPIO8 | (to define) | |
| GPIO9 | (to define) | |
| GPIO10 | (to define) | |
| GPIO11 | (to define) | |
| GPIO12 | (to define) | |
| GPIO13 | (to define) | |

## GPIO Summary

### Used Pins
- **GPIO1**: MIDI INPUT
- **GPIO2**: CLOCK IN
- **GPIO3**: RESET IN
- **GPIO4**: CLOCK OUT
- **GPIO5**: RESET OUT
- **GPIO14**: S0 (Multiplexer select - shared)
- **GPIO15**: S1 (Multiplexer select - shared)
- **GPIO16**: S2 (Multiplexer select - shared)
- **GPIO17**: S3 (Multiplexer select - shared)
- **GPIO18**: I2S BCK
- **GPIO19**: I2S DIN
- **GPIO20**: I2S LCK
- **GPIO21**: Keyboard multiplexer COM
- **GPIO22**: Shift register SER
- **GPIO26**: Potentiometer multiplexer COM
- **GPIO27**: Shift register SRCLK (shared)
- **GPIO28**: Shift register RCLK (shared)

### Available Pins
- GPIO7, GPIO8, GPIO9, GPIO10, GPIO11, GPIO12, GPIO13
- 4 potentiometer inputs (X1-X4)
- 4 button inputs (Y1-Y4)
