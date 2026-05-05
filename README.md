SMART LED CONTROLLER (ESP32-C3 + VEML7700)

------------------------------------------------------------

HARDWARE
    MCU: ESP32-C3
    Light sensor: VEML7700 (ambient lux sensor)
    LED output: 12-bit PWM (LEDC hardware PWM)
    Input: single button (short + long press)
    Optional: WiFi for web interface

------------------------------------------------------------

INPUT LOGIC (BUTTON)
    Short press:
        Cycle modes:
            Dunkel → Hell → Lux → Dunkel

    Long press (>1s):
        Adjust value depending on mode:
            Dunkel: increase / decrease brightness
            Hell: increase / decrease brightness
            Lux: adjust target lux (0–200 lux)

------------------------------------------------------------

MODES

    1. DUNKEL MODE
        - Low-light optimized mode
        - Uses dedicated LUT for ultra-low brightness (0–1 lux focus)
        - Strong perceptual resolution in darkness
        - Smooth brightness transitions (no stepping)

    2. HELL MODE
        - Normal / bright lighting mode
        - Uses separate LUT optimized for higher lux range
        - Self-light compensation:
            * LED turns off briefly
            * VEML7700 takes clean ambient measurement
            * PWM updates after measurement
        - Prevents LED from influencing sensor readings

    3. LUX MODE
        - Closed-loop automatic brightness control
        - Target range: 0–200 lux
        - Adjusts PWM until measured lux matches target
        - Continuous feedback loop using VEML7700
        - Acts as automatic light regulator

------------------------------------------------------------

SENSOR PROCESSING
    VEML7700 readings: ambient lux
    Processing:
        - smoothing filter (reduces noise)
        - stable update rate (prevents flicker)
        - low-light sensitivity handling

------------------------------------------------------------

PWM OUTPUT
    - 12-bit resolution (0–4095)
    - ESP32-C3 LEDC hardware PWM core 3.x
    - LUT-based brightness mapping
    - Smooth transitions (no visible stepping)
    - Perceptual (nonlinear) brightness correction

------------------------------------------------------------

WEB INTERFACE
    - Tablet-optimized UI
    - WiFi accessible

    Features:
        - Mode selection (Dunkel / Hell / Lux)
        - Live lux display
        - Brightness visualization
        - Manual override controls
        - Target lux adjustment (Lux mode)

------------------------------------------------------------

SYSTEM BEHAVIOR
    - Smooth brightness transitions at all times
    - No flicker or abrupt changes
    - Human-perception optimized (LUT-based)
    - Strong low-light sensitivity (0–1 lux focus)
    - Stable under sensor noise
    - Deterministic mode-based control logic

LUT
    DUNKEL
        PWM: 10 → 200
        Lux: 0 → 50
    HELL
        PWM: 100 → 2000
        Lux: 0 → 500
