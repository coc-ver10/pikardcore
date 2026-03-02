# GPIO Allocation - Pikardcore

**Date:** 2 mars 2026  
**Status:** Configuration actuelle après résolution conflits

## GPIO Utilisés (14 pins)

| GPIO | Fonction | Direction | Module | Statut |
|------|----------|-----------|--------|--------|
| **2** | CLOCK_IN | Input | External sync | ✅ OK |
| **3** | RESET_IN | Input | External reset | ✅ OK |
| **4** | TRIGO / CLOCK_OUT | Output | TriggerOut | ✅ OK (analog sync) |
| **5** | RESET_OUT | Output | (reserved) | ⚠️ Défini mais non utilisé |
| **14** | Mux S0 | Output | MultiplexerKnob | ✅ OK (shared avec buttons futur) |
| **15** | Mux S1 | Output | MultiplexerKnob | ✅ OK (shared avec buttons futur) |
| **16** | Mux S2 | Output | MultiplexerKnob | ✅ OK (shared avec buttons futur) |
| **17** | Mux S3 | Output | MultiplexerKnob | ✅ OK (shared avec buttons futur) |
| **18** | I2S BCK | Output (PIO) | I2S Audio | ✅ OK (bit clock) |
| **19** | I2S DIN | Output (PIO) | I2S Audio | ✅ OK (data) |
| **20** | I2S LCK | Output (PIO) | I2S Audio | ✅ OK (word select) |
| **21** | Button Mux COM | Input ADC | *FUTUR MultiplexerButton* | 🔮 Réservé |
| **22** | SR SER | Output | Shift Register LEDs | ✅ OK (serial data) |
| **25** | LED | Output | Pico onboard LED | ✅ OK |
| **26** | Knob Mux COM | Input ADC | MultiplexerKnob | ✅ OK (ADC0) |
| **27** | SR SRCLK | Output | Shift Register LEDs | ✅ OK (shift clock) |
| **28** | SR RCLK | Output | Shift Register LEDs | ✅ OK (latch clock) |

## GPIO Libres (13 pins)

| GPIO | Notes |
|------|-------|
| **0-1** | Réservés UART (USB debug) |
| **6-13** | Libres (anciens boutons/LEDs GPIO direct) |
| **23-24** | Libres |
| **29** | Libre (ADC3) |

## GPIO Désactivés (Ancien Picocore)

| GPIO | Ancienne fonction | Raison désactivation |
|------|------------------|---------------------|
| **4-11** | Boutons directs | Conflit avec nouvelle architecture |
| **12-19** | LEDs directes | Shift register remplace |
| **23** | WS2812 legacy | WS2812_ENABLED=0 |
| **26-28** | ADC knobs directs | Multiplexeur remplace |

## Conflits Résolus

### ❌ Conflit 1 : GPIO 4-11 (Boutons)
- **Problème :** Anciens boutons GPIO direct sur 4-11
- **Impact :** GPIO 4 (CLOCK_OUT), GPIO 5 (RESET_OUT), GPIO 14-17 (Mux select)  
- **Solution :** BUTTONS_ENABLED=0 → Désactivation complète

### ❌ Conflit 2 : GPIO 21 (TriggerOut vs Button Mux)
- **Problème :** TRIGO_PIN=21 bloquait GPIO 21 pour MultiplexerButton COM
- **Solution :** TRIGO_PIN routé vers GPIO 4 (CLOCK_OUT analog sync)

### ❌ Conflit 3 : GPIO 23 (WS2812 legacy)
- **Problème :** Initialization inutile de GPIO 23
- **Solution :** Commenté (WS2812_ENABLED=0)

### ❌ Conflit 4 : GPIO 27-28 (ADC vs Shift Register)
- **Problème :** Anciens knobs ADC utilisaient GPIO 27-28
- **Solution :** Multiplexeur utilise uniquement GPIO 26 (ADC0)

## État Actuel Audio

| Paramètre | Source | Valeur Initiale |
|-----------|--------|-----------------|
| **I2S BCK** | GPIO 18 | PIO contrôlé |
| **I2S DIN** | GPIO 19 | PIO contrôlé |
| **I2S LCK** | GPIO 20 | PIO contrôlé |
| **Sample Rate** | Config | 48000 Hz |
| **Bit Depth** | Config | 16-bit |

## Modules Actifs

✅ **MultiplexerKnob** : GPIO 26 (COM) + 14-17 (S0-S3)  
✅ **Shift Register LEDs** : GPIO 22, 27, 28  
✅ **I2S Audio** : GPIO 18, 19, 20  
✅ **TriggerOut** : GPIO 4  
❌ **Buttons** : Désactivés (attente MultiplexerButton)

## Prochaines Étapes

1. **Task 2** : Implémenter MultiplexerButton  
   - GPIO 21 (COM digital input)
   - GPIO 14-17 (S0-S3 shared)

2. **Vérifier shift register**  
   - Tester 16 LEDs cascade
   - Vérifier mapping non-séquentiel

## Notes

- **Aucun conflit GPIO détecté** dans la configuration actuelle
- Tous les modules initialisent leurs GPIO correctement
- ADC utilisé uniquement par MultiplexerKnob
- PIO utilisé uniquement par I2S audio
