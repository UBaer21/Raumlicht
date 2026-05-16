# Raumlicht

> Ambient light that thinks for itself — and knows when you're in the room.

A smart lighting controller for ESP32-C3/S3 that goes well beyond a dimmer switch. Raumlicht measures the actual light in your room with a calibrated lux sensor, detects your presence with a radar module, and adjusts a 150W LED strip accordingly — from near-invisible star-stripe glow to full motion-triggered brightness, all on a perceptual curve your eyes will never notice adjusting.

Built because commercially available smart lighting either can't see in the dark or blinds you when it does.

---

## What makes it different

Most dimmers map a slider to a voltage. Raumlicht maps **measured lux to a calibrated PWM lookup table** — which means the light responds to what your eyes actually perceive, not what the hardware happens to output.

**The floor never turns off.** At PWM 10–12 (0.24% duty on 12-bit), the LED strip runs as a permanent ceiling glow — invisible as illumination, present as atmosphere. Motion doesn't turn the light on; it raises it from that floor.

**The sensor can't see its own light.** In active mode, the system briefly kills PWM, takes a clean ambient reading with the LED off, then restores brightness. Self-contamination eliminated.

**Switching between sensor gain modes doesn't flicker.** A 5 lux hysteresis band around each gain transition prevents the oscillation that makes most auto-ranging sensors useless at mode boundaries.

---

## Hardware

| Component | Role |
|---|---|
| ESP32-C3 Super Mini | MCU, WiFi, web server |
| VEML7700 | Ambient lux sensor (I²C, 0.001–167,000 lux) |
| LD2420 | 24GHz radar presence and range detection |
| MOSFET + 150W LED strip | Main lighting output |
| WS2812B (×1) | System status indicator |
| Single button | Mode switching and brightness adjust |

---

## Modes

### DUNKEL — ambient floor
The room is lit at the minimum perceptual level. The LUT is calibrated for 0–50 lux with fine resolution at the bottom — 0.001 lux reads differently from 0.01 lux, and the PWM reflects that. The star stripe at the ceiling glows. Nothing more.

### HELL — motion-aware
The LD2420 radar tracks presence and direction of movement. Both the floor and ceiling are lux-driven — the DUNKEL LUT determines the dim floor, the HELL LUT determines the bright ceiling, both evaluated against the current ambient lux measured in darkness. Motion fades between them. The light is always contextually appropriate: a bright afternoon and a dim evening produce different ceilings, automatically.

The fade duration is calculated from the distance traveled divided by a tunable velocity constant — so a slow walk gets a slow fade, a fast entry gets a fast response.

```
ENTERING  →  fade DUNKEL → HELL LUT  (duration ∝ distance)
INSIDE    →  hold at HELL LUT ceiling
LEAVING   →  fade HELL → DUNKEL LUT  (duration ∝ distance)
OUTSIDE   →  complete fade to DUNKEL floor
STALE     →  radar silent > 1.5s → treat as outside
```

### LUX — closed loop
Set a target lux value. The controller runs a proportional feedback loop — every sensor reading adjusts PWM up or down toward the target. Gain is higher at low lux (where a 0.1 lux error matters) and lower at high lux (where it doesn't). The DUNKEL LUT sets a hard floor; the loop cannot dim below ambient.

If the VEML7700 goes stale or fails, LUX mode falls back gracefully — PWM is mapped directly from the target lux value without feedback. The light stays at a reasonable level rather than failing dark or failing bright. Useful if someone disconnects the sensor.

---

## Sensor strategy

The VEML7700 covers an enormous dynamic range through four gain/integration modes:

| Mode | Integration | Gain | Range | Factor |
|---|---|---|---|---|
| HIGH | 800ms | ×1 | 0–550 lux | 0.0084 |
| NORMAL | 100ms | ×1 | 0–3700 lux | 0.0672 |
| FAST | 25ms | ×2 | 0–550 lux | 0.1344 |
| DSUN | 25ms | ×1/8 | 0–167k lux | 2.1504 |

Auto-ranging switches between modes with a hysteresis band to prevent oscillation at boundaries. Transition settle times are conservative — 1600ms for HIGH mode — to ensure the previous integration cycle has fully flushed before a reading is trusted.

Above 1000 lux, a 4th-order polynomial correction from the 2025 Vishay application note is applied. Below 1000 lux (98% of operating time), the 2025 revision factors are used directly.

All reads are non-blocking — the main loop never waits on a sensor.

---

## Web interface

A WiFi-accessible dashboard for calibration and monitoring:

- Live lux graph (logarithmic scale — because lux is meaningless on linear)
- Live PWM graph
- Mode selector
- Target lux slider with dual-range mapping (fine control below 5 lux, coarse above)
- LD2420 presence badge and radar range bar with adjustable in/out thresholds
- Full config page — LUT editing, velocity constant, WiFi credentials
- Falls back to AP mode if no network is found within 15 seconds

The web interface is a calibration instrument. The system is designed to run fully autonomous — the button handles everything needed in daily use.

---

## Status LED

The single WS2812B onboard LED shows system state at a glance:

| Color | Pattern | State |
|---|---|---|
| Blue | Slow pulse | DUNKEL — ambient floor active |
| Green | Fast pulse | HELL — motion detection active |
| Cyan | Solid | LUX — closed-loop targeting |
| Red | Fast pulse | OFF — standby |

Color also tracks the lux-to-color curve during button brightness adjustment — midnight blue at moonlight, through cyan and warm white, to orange-red at high lux. The boundary flashes white when you hit the limit.

---

## Button

| Press | Action |
|---|---|
| Short | Cycle modes: DUNKEL → HELL → LUX |
| Long (>1s) | Enter LUX adjust — ramp target lux up/down |
| Release after long | Reverse ramp direction |

Step size adapts to range: 0.01 lux steps below 5 lux, 1 lux steps above. No menu required.

---

## Configuration persistence

All calibration values survive power cycles via ESP32 NVS (non-volatile storage):

- In/out range thresholds for LD2420
- Velocity constant for fade duration
- Target lux
- Full DUNKEL and HELL lookup tables (up to 20 points each)
- WiFi credentials

NVS writes are throttled on frequently-changing values to stay within flash write cycle limits.

---

## Project status

Currently Arduino framework + PlatformIO (v5/v6). A port to pure ESP-IDF with proper component separation is in progress — the single-file architecture has done its job as a development instrument and is ready to grow up.

---

## What I learned building this

Light is not linear. A kitchen in direct sun sits at 50,000 lux. A doorway two meters away is 20 lux. Your eyes hide this from you completely. A sensor shows you what's actually there — and once you see it, you can't un-see it. The LUT calibration values in this project were not calculated; they were measured, observed, and adjusted until the room felt right.

That process is why the web interface exists.
