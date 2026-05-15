const char SRC[] PROGMEM = R"SRCEOF(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Source</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#0d1117;color:#e6edf3;font-family:'JetBrains Mono','Fira Code','Courier New',monospace;min-height:100vh}
:root{
  --bg:#0d1117;--bg2:#161b22;--border:#30363d;
  --text:#e6edf3;--muted:#8b949e;
  --kw:#ff7b72;--type:#ffa657;--fn:#d2a8ff;
  --str:#a5d6ff;--num:#79c0ff;--cmt:#8b949e;
  --pp:#ffa657;--ln:#484f58;--accent:#388bfd;
  --pill:#21262d;
}
.viewer{max-width:960px;margin:32px auto;border:1px solid var(--border);border-radius:12px;overflow:hidden}
.toolbar{background:var(--bg2);border-bottom:1px solid var(--border);padding:10px 16px;display:flex;align-items:center;gap:12px}
.dots{display:flex;gap:6px}
.dot{width:12px;height:12px;border-radius:50%}
.dot-r{background:#ff5f57}.dot-y{background:#febc2e}.dot-g{background:#28c840}
.fname{font-size:12px;color:var(--muted);flex:1;text-align:center;letter-spacing:.03em}
.actions{display:flex;gap:8px}
.pill{background:var(--pill);border:1px solid var(--border);border-radius:6px;padding:4px 10px;font-size:11px;color:var(--muted);cursor:pointer;display:flex;align-items:center;gap:5px;transition:color .15s,border-color .15s;font-family:inherit}
.pill:hover{color:var(--text);border-color:var(--accent)}
.pill.ok{color:#3fb950;border-color:#3fb950}
.code-wrap{overflow-x:auto;padding:16px 0;background:var(--bg)}
table{border-spacing:0;width:100%}
tr:hover td{background:rgba(255,255,255,.03)}
.ln{user-select:none;text-align:right;padding:0 16px 0 20px;color:var(--ln);font-size:13px;line-height:1.7;min-width:52px;vertical-align:top;border-right:1px solid var(--border)}
.cd{padding:0 20px 0 16px;font-size:13px;line-height:1.7;color:var(--text);white-space:pre;vertical-align:top}
.kw{color:var(--kw)}.tp{color:var(--type)}.fn{color:var(--fn)}
.st{color:var(--str)}.nm{color:var(--num)}.cm{color:var(--cmt);font-style:italic}.pp{color:var(--pp)}
.stats{background:var(--bg2);border-top:1px solid var(--border);padding:8px 20px;display:flex;gap:20px;font-size:11px;color:var(--muted)}
.sv{color:var(--text);font-weight:500}
textarea{position:absolute;left:-9999px;opacity:0}
</style>
</head>
<body>
<div class="viewer">
  <div class="toolbar">
    <div class="dots"><div class="dot dot-r"></div><div class="dot dot-y"></div><div class="dot dot-g"></div></div>
    <div class="fname" id="fname">main.cpp</div>
    <div class="actions">
      <button class="pill" id="rawBtn" onclick="toggleRaw()">&#60;/&#62; raw</button>
      <button class="pill" id="copyBtn" onclick="copyCode()">&#128203; copy</button>
    </div>
  </div>
  <div class="code-wrap" id="codeWrap"><table id="codeTable"></table></div>
  <div class="stats" id="stats"></div>
</div>
<textarea id="ta"></textarea>

<script>
// ─────────────────────────────────────────────────────────
// PASTE YOUR CPP SOURCE BETWEEN THE BACKTICKS BELOW
// Safe to paste raw C++ — only backtick ` needs escaping as \`
// ─────────────────────────────────────────────────────────
const CODE = `
// SOURCE CODE CC0 HERE

/****************************************************
 RAUMLICHT v5
 ESP32-S3 + VEML7700 + LD2420 v2.1

 MODES:
   DUNKEL  – dim ambient (LUT-based, always floor)
   HELL    – bright ambient (LUT-based) by motion
   LUX     – closed-loop lux target

 PRESENCE (LD2420 via UART pins 6/7):
   ENTERING  → fade UP  to HELL
   INSIDE    → fast fade UP to HELL 
   LEAVING   → fade DOWN to DUNKEL 
   NONE      → hold current PWM level
   OUTSIDE / STALE → continue / complete fade to DUNKEL

 PWM: 12-bit (0–4095) on pin 10, 5 kHz
 VEML7700: I²C 0x10, auto-range

 • non-blocking sensor state machine
 • adaptive gain/integration strategy
 • fade engine separated from sensing
 • stale-state handling

 TODO:
The hardcoded WIFI_SSID / WIFI_PASS from config.h are only used as a fallback when prefs are empty
So right now: wifi fields on the CONFIG page are display-only, saves are silently dropped. 
The hardcoded credentials are safe but the UI is lying about being able to change them.

****************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "config.h"
#include "webpage.h"
#include "webpage2.h"
#include "webpageCONFIG.h"
#include <DNSServer.h>
#include <ArduinoJson.h>

Preferences prefs;

//#define FASTLED_UNUSABLE_PIN_MASK 0ULL   // clear FastLED's pin blacklist for ESP32-S3
#define FASTLED_ESP32_USES_RMT
#include <FastLED.h>


#if defined(CONFIG_IDF_TARGET_ESP32S3)
    #define DATA_PIN 48   // onboard LED is on GPIO48 on S3
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
    #define DATA_PIN 0    // C3 only has GPIO0–21
#endif

//#define DATA_PIN    48      // 48 Change to 21, 38, etc. if your board differs



#define NUM_LEDS    1
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB     // WS2812 is GRB, not RGB

CRGB leds[NUM_LEDS];


// ============================================================
// DEBUG
// ============================================================
#define DEBUG          // uncomment to enable Serial output
#ifdef DEBUG
  #define DBG(x)   Serial.print(x)
  #define DBGLN(x) Serial.println(x)
#else
  #define DBG(x)
  #define DBGLN(x)
#endif


// ROOM settings — mutable, saved via prefs (not const → camelCase)
 int outRange = 350;
 int inRange  = 50;


// ============================================================
// PWM
// ============================================================
const int PWM_FREQ = 5000;
const int PWM_RES  = 12;          // 0–4095






HardwareSerial ldSerial(1);

// ============================================================
// WEB SERVER
// ============================================================
WebServer server(80);


// ============================================================
// AP / WIFI CREDENTIALS  (persistent)
// ============================================================
char wifiSSID[64] = "";
char wifiPASS[64] = "";
bool         apMode  = false;
DNSServer    dnsServer;
const byte   DNS_PORT = 53;

// ============================================================
// ALARM
// ============================================================
bool          alarmEnabled  = false;
bool          alarmArmed    = false;
bool          alarmTripped  = false;
String        alarmPin      = "0000";
String        alarmLog      = "";
unsigned long alarmFlashMs  = 0;
bool          alarmFlashOn  = false;
enum FlashMode { FLASH_OFF, FLASH_ALARM, FLASH_PANIC, FLASH_DISCO, FLASH_STROBE };
FlashMode flashMode = FLASH_OFF;
const unsigned long ALARM_FLASH_INTERVAL_MS = 65; // ≈ 7.7 Hz
/* The sweet spot for disruption: research found the most disruptive frequencies lie between 10 and 15 Hz
 Brain entrainment for relaxation: flickering light at 10 Hz produces alpha-frequency brainwaves
 40 Hz stimulation is proven to increase dopamine and serotonin
*/


// wave: period in ms, amplitude 500–1000
uint16_t sineWave(unsigned long periodMs) {
    float phase = (float)(millis() % periodMs) / periodMs;  // 0.0 – 1.0
    float s = (sinf(phase * TWO_PI) + 1.0f) * 0.5f;        // 0.0 – 1.0
    return (uint16_t)(500 + s * 500.0f);
}


// ============================================================
// ZERO ZONE  &  VELOCITY CONSTANT
// ============================================================
int   zeroRange     = 0;
float velocityConst = 68.0f;

// ============================================================
// MODE
// ============================================================
enum Mode { DUNKEL, HELL, LUX, OFF };
Mode   mode    = DUNKEL;
String modeStr = "DUNKEL";


// ============================================================
// VEML7700
// ============================================================


#define VEML_ADDR 0x10
#define ALS_REG   0x04

// Sensor integration / gain modes
enum VemlMode { VEML_FAST, VEML_NORMAL, VEML_HIGH, VEML_DSUN };
VemlMode vemlMode = VEML_NORMAL;

// Non-blocking read state machine
enum VemlState { VEML_IDLE, VEML_WAITING, VEML_STALE };
VemlState     vemlState    = VEML_IDLE;
unsigned long vemlWaitEnd  = 0;
unsigned long lastLuxUpdate = 0;


void initVEML7700(VemlMode m)
{
    uint16_t conf = 0x0000;

    switch (m) {
        case VEML_FAST:
            conf = 0x0B00; // gain x2 + 25ms
            break;

        case VEML_NORMAL:
            conf = 0x0000; // gain x1 + 100ms
            break;

        case VEML_HIGH:
            conf = 0x00C0; // gain x1 + 800ms
            break;

        case VEML_DSUN:
            conf = 0x1300; // gain x1/8 + 25ms
            break;
    }

    Wire.beginTransmission(VEML_ADDR);
    Wire.write(0x00);
    Wire.write(conf & 0xFF);         // LSB
    Wire.write((conf >> 8) & 0xFF);  // MSB
    Wire.endTransmission();

    vemlMode = m;
}

void initVEML7700A(VemlMode m) {
    uint8_t lsb = 0x00;
    uint8_t msb = 0x00;
    switch (m) {
        case VEML_FAST:   msb = 0x0B; break;  // 25 ms, gain ×2
        case VEML_NORMAL: msb = 0x00; break;  // 100 ms, gain ×1
        case VEML_HIGH:   lsb = 0xC0; break;  // 800 ms, gain ×1  (low-light)
        case VEML_DSUN:   msb = 0x13; break;  // 25 ms, gain ×1/8 (direct sun)  
    }
    Wire.beginTransmission(VEML_ADDR);
    Wire.write(0x00);
    Wire.write(lsb);
    Wire.write(msb);
    Wire.endTransmission();
    vemlMode = m;
}


bool readVemlRaw(uint16_t &out) {
    Wire.beginTransmission(VEML_ADDR);
    Wire.write(ALS_REG);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(VEML_ADDR, 2) != 2) return false;
    uint8_t lo = (uint8_t)Wire.read();
    uint8_t hi = (uint8_t)Wire.read();
    out = ((uint16_t)hi << 8) | lo;
    return true;
}



// Lux conversion factors per mode

// new revision Mar-2025 https://www.vishay.com/docs/84323/designingveml7700.pdf?utm_source=chatgpt.com
float vemlFactor() {
    switch (vemlMode) {
        case VEML_FAST:   return 0.1344f;   // 25 ms, gain ×2
        case VEML_NORMAL: return 0.0672f;   // 100 ms, gain ×1
        case VEML_HIGH:   return 0.0084f;   // 800 ms, gain ×1
        case VEML_DSUN:   return 2.1504f;   // 25 ms, gain ×1/8
    }
    return 0.0672f;
}



// Minimum settling times per mode (ms)
uint16_t vemlSettleMs() {
    switch (vemlMode) {
        case VEML_FAST:   return 30;
        case VEML_NORMAL: return 150;
        case VEML_HIGH:   return 1600;//810;
        case VEML_DSUN:   return 60;
    }
    return 150;
}




// ============================================================
// NON-BLOCKING VEML READ
// Call every loop. Returns true when a fresh lux value is ready.
// ============================================================


float correctLux(float lux)
{
    return (2e-15f * pow(lux,4)) +
           (1e-12f * pow(lux,3)) -
           (9e-5f  * lux * lux) +
           (1.0179f * lux);
}


// ============================================================
// LUTs
// ============================================================

//float    d_lux[20]; uint16_t d_pwm[20]; int D_SIZE = 12;
//float    h_lux[20]; uint16_t h_pwm[20]; int H_SIZE = 12;


float    d_lux[20] = {0,0.01,0.03,0.05,0.1,0.25,0.5,1,5,20,50,500};
uint16_t d_pwm[20] = {10,12,15,20,30,45,65,90,130,170,200,500};
int      D_SIZE  = 12;

float    h_lux[20] = {0,0.5,1,2,5,10,25,50,100,200,350,500};
uint16_t h_pwm[20] = {100,120,140,180,250,350,550,800,1100,1500,1800,2000};
int      H_SIZE  = 12;

uint16_t interp(float x, const float *lx, const uint16_t *pw, int size) {
    if (x <= lx[0])        return pw[0];
    if (x >= lx[size - 1]) return pw[size - 1];
    for (int i = 0; i < size - 1; i++) {
        if (x < lx[i + 1]) {
            float t = (x - lx[i]) / (lx[i + 1] - lx[i]);
            return (uint16_t)(pw[i] + t * (pw[i + 1] - pw[i]));
        }
    }
    return pw[size - 1];
}







// ============================================================
// PWM STATE
// ============================================================
uint16_t currentPWM = 0;

void setPWM(uint16_t v) {
    currentPWM = v;
    ledcWrite(PWM_PIN, v);
}

float luxNow      = 0;
float luxFiltered = 0;
float luxHellFiltered = 0; // measured only in darkness

bool updateLux() {
    uint32_t now = millis();

    if (vemlState == VEML_IDLE || vemlState == VEML_STALE) {
        // Kick off a read; sensor already integrating continuously
        uint16_t raw ;
        if (readVemlRaw(raw)){
             lastLuxUpdate = now;
             vemlState = VEML_IDLE; // stale recover
        }
        else {
            if (millis() - lastLuxUpdate > 2000) {
                vemlState    = VEML_STALE;
                luxNow       = -1;
                luxFiltered  = 10;
                return false;
            }
            initVEML7700(VEML_DSUN);
            vemlState   = VEML_WAITING;
            vemlWaitEnd = now + vemlSettleMs();
            return false;
        }

        float lux = raw * vemlFactor();


        // ---- Auto-range ----


        if (raw < 200 && vemlMode != VEML_HIGH  ) {
            initVEML7700(VEML_HIGH);
            vemlState   = VEML_WAITING;
            vemlWaitEnd = now + vemlSettleMs();
            return false;
        }
        if ((raw > 1000 && vemlMode == VEML_HIGH) ||
            (raw < 1000 && vemlMode == VEML_DSUN)) {
            initVEML7700(VEML_NORMAL);
            vemlState   = VEML_WAITING;
            vemlWaitEnd = now + vemlSettleMs();
            return false;
        }
        if (raw > 30000 && vemlMode != VEML_DSUN  ) {
            initVEML7700(VEML_DSUN);
            vemlState   = VEML_WAITING;
            vemlWaitEnd = now + vemlSettleMs();
            return false;
        }

        luxNow      =  lux > 1000? correctLux(lux) : lux;
        luxFiltered = luxFiltered * 0.9f + luxNow * 0.1f;
        if (mode == DUNKEL) luxHellFiltered = luxFiltered;

        return true;
    }

    if (vemlState == VEML_WAITING) {
        if (now >= vemlWaitEnd) {
            vemlState = VEML_IDLE;
        }
        return false;
    }
    return false;
}



void handleLuxRead() {
    char buf[32];
    snprintf(buf, sizeof(buf), "{\"lux\":%.4f}", luxNow);
    server.send(200, "application/json", buf);
}



// ============================================================
// PRESENCE / MOTION (LD2420)
// ============================================================
enum MotionState { M_NONE, M_ENTERING, M_INSIDE, M_LEAVING, M_OUTSIDE, M_STALE };

// Defined immediately after enum to avoid Arduino IDE forward-decl bug
String motionToStr(int m) {
    switch (m) {
        case M_ENTERING: return "ENTERING";
        case M_INSIDE:   return "INSIDE";
        case M_LEAVING:  return "LEAVING";
        case M_OUTSIDE:  return "OUTSIDE";
        case M_STALE:    return "STALE";
        default:         return "NONE";
    }
}

MotionState motionState = M_OUTSIDE;

String presenceStr   = "NONE";
int    rangeValue    = -1;
float  rangeFiltered = -1;

const int DEAD_ZONE = 10;

unsigned long lastLDUpdate   = 0;
const unsigned long STALE_MS = 1500;

float         lastFilteredLocal = -1;
unsigned long lastMotionTime    = 0;

// ============================================================
// FADE ENGINE
// ============================================================
// Fades currentPWM toward a target over a given duration (ms).
// Call applyFade() every loop; start a fade with startFade().

struct Fade {
    uint16_t startPWM;
    uint16_t targetPWM;
    unsigned long startMs;
    unsigned long durationMs;
    bool active;
} fade = {0, 0, 0, 0, false};

void startFade(uint16_t target, unsigned long durationMs) {
    fade.startPWM   = currentPWM;
    fade.targetPWM  = target;
    fade.startMs    = millis();
    fade.durationMs = durationMs;
    fade.active     = true;
}

// Returns true while fade is running
bool applyFade() {
    if (!fade.active) return false;
    unsigned long elapsed = millis() - fade.startMs;
    if (elapsed >= fade.durationMs) {
        setPWM(fade.targetPWM);
        fade.active = false;
        return false;
    }
    float    t = (float)elapsed / fade.durationMs;
    uint16_t v = fade.startPWM + t * ((int32_t)fade.targetPWM - fade.startPWM);
    setPWM(v);
    return true;
}


unsigned long calcFadeDuration(float zoneCm) {
    if (velocityConst <= 0.0f) return 10000UL;
    unsigned long ms = (unsigned long)((fabsf(zoneCm) / velocityConst) * 1000.0f);
    return constrain(ms, 500UL, 30000UL);
}




// ============================================================
// MOTION → LIGHT LOGIC
// ============================================================
MotionState lastMotionState = M_NONE;

void applyMotionToLight() {
    if (motionState == lastMotionState) return;   // no change
    lastMotionState = motionState;

    uint16_t dunkelPWM = interp(luxHellFiltered, d_lux, d_pwm, D_SIZE);
    uint16_t hellPWM   = interp(luxHellFiltered, h_lux, h_pwm, H_SIZE);

    switch (motionState) {
        case M_ENTERING:
            startFade(hellPWM, calcFadeDuration(outRange - inRange));
            break;
        case M_INSIDE:
            startFade(hellPWM, calcFadeDuration(inRange));
            break;
        case M_LEAVING:
            startFade(dunkelPWM, calcFadeDuration(outRange - inRange));
            break;
        case M_NONE:
            fade.active = false;         // hold current level
            break;
        case M_OUTSIDE:
        case M_STALE:
            // complete fade down to dunkel if not already heading there
            if (!fade.active || fade.targetPWM != dunkelPWM) {
                startFade(dunkelPWM, 10000);
            }
            break;
    }
}

// ============================================================
// LD2420 PARSER
// ============================================================
String ldBuffer = "";

void parseLDLine(String line) {
    line.trim();
    if (line.length() == 0) return;

    if (line.indexOf("OFF") >= 0) {
        presenceStr       = "NONE";
        rangeValue        = -1;
        rangeFiltered     = -1;
        lastFilteredLocal = -1;
        motionState       = M_OUTSIDE;
        lastLDUpdate      = millis();
        return;
    }

    if (line.indexOf("ON") >= 0) {
        presenceStr  = "YES";
        lastLDUpdate = millis();
    }

    int idx = line.indexOf("Range");
    if (idx >= 0) {
        String num = "";
        for (int i = idx + 5; i < (int)line.length(); i++) {
            if (isDigit(line[i]))      num += line[i];
            else if (num.length() > 0) break;
        }
        if (num.length() == 0) return;

        rangeValue = num.toInt();

        if (rangeFiltered < 0) rangeFiltered = rangeValue;
        else rangeFiltered = rangeFiltered * 0.4f + rangeValue * 0.6f;

        // ---- Direction detection ----
        if (lastFilteredLocal < 0) {
            lastFilteredLocal = rangeFiltered;
        } else {
            float diff = rangeFiltered - lastFilteredLocal;
            if (diff > DEAD_ZONE && rangeFiltered >= inRange) {
                motionState       = M_LEAVING;
                lastFilteredLocal = rangeFiltered;
                lastMotionTime    = millis();
            } else if (diff < -DEAD_ZONE && rangeFiltered >= inRange) {
                motionState       = M_ENTERING;
                lastFilteredLocal = rangeFiltered;
                lastMotionTime    = millis();
            }
        }

        // No movement for 1 s → NONE (hold level)
        if (millis() - lastMotionTime > 1000) {
            motionState       = M_NONE;
            lastMotionTime    = millis();
            lastFilteredLocal = rangeFiltered;
        }

        // Far edge → treat as outside
        if (rangeFiltered >= outRange) {
            presenceStr       = "NONE";
            motionState       = M_OUTSIDE;
            lastFilteredLocal = -1;
            lastMotionTime    = millis();
        }

        // Very close → INSIDE
        if (rangeFiltered <= inRange) {
            motionState       = M_INSIDE;
            presenceStr       = "YES";
            lastMotionTime    = millis();
            lastFilteredLocal = rangeFiltered;
        }

        // Zero zone – sensor artifact / too close; ignore as real presence
        if (zeroRange > 0 && rangeFiltered <= zeroRange) {
            presenceStr       = "NONE";
            motionState       = M_OUTSIDE;
            lastFilteredLocal = -1;
            lastMotionTime    = millis();
        }

        lastLDUpdate = millis();
    }
}

void readLD2420() {
    int count = 0;
    while (ldSerial.available() && count < 64) {
        char c = ldSerial.read();
        if (c == '\n') {
            parseLDLine(ldBuffer);
            ldBuffer = "";
        } else if (c != '\r') {
            ldBuffer += c;
        }
        count++;
    }
}

void applyStaleRule() { //LD24 Sensor
    if (millis() - lastLDUpdate > STALE_MS) {
        presenceStr       = "STALE";
        rangeValue        = -1;
        rangeFiltered     = -1;
        lastFilteredLocal = -1;
        motionState       = M_STALE;
        lastMotionTime    = millis();
        ldBuffer          = ""; // dicscard partial frame
    }
}


void handleWifiPost() {
    // TODO: parse {"ssid":"…","pass":"…"}, save prefs, ESP.restart()
    server.send(200, "application/json", "{}");
}

void handleAlarmGet() {
    char buf[256];
    snprintf(buf, sizeof(buf),
        "{\"enabled\":%s,\"armed\":%s,\"tripped\":%s,\"log\":\"%s\"}",
        alarmEnabled ? "true" : "false",
        alarmArmed   ? "true" : "false",
        alarmTripped ? "true" : "false",
        alarmLog.c_str());
    server.send(200, "application/json", buf);
}

void handleAlarmPost() {
    // TODO: parse {"action":"arm"|"disarm"|"clear"|"setpin","pin":"…"}
    server.send(200, "application/json", "{}");
}

// ============================================================
// AP FALLBACK (hotel / captive-portal mode)
// ============================================================
void startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("RAUMLICHT-SETUP");
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    apMode = true;
    DBGLN("AP mode: " + WiFi.softAPIP().toString());
}
// ============================================================
// ALARM ENGINE
// ============================================================
void checkAlarm() {
    if (!alarmArmed || !alarmEnabled) return;
    if (motionState == M_ENTERING || motionState == M_INSIDE) {
        if (!alarmTripped) {
            alarmTripped = true;
            alarmLog += String(millis()) + "\n";  // TODO: swap for RTC timestamp
        }
    }
}

// Call every loop; owns PWM while alarm is tripped
void alarmFlash() {
    if (!alarmTripped || !alarmEnabled) return;
    if (millis() - alarmFlashMs >= ALARM_FLASH_INTERVAL_MS) {
        alarmFlashMs = millis();
        alarmFlashOn = !alarmFlashOn;
        ledcWrite(PWM_PIN, alarmFlashOn ? 4095 : 0);
    }
}






// ============================================================
// LUX → COLOR HELPER
// ============================================================
CRGB luxToColor(float lux) {
    // 0–10: deep blue, 10–80: cyan→white, 80–180: warm white→orange, 180–250: orange→red
    lux = constrain(lux, 0, 250);
    if (lux < 5)  return blend(CRGB::MidnightBlue, CRGB::Cyan,          lux / 5.0f * 255);
    if (lux < 80)  return blend(CRGB::Cyan,          CRGB(255, 255, 200), (lux - 5)  / 75.0f  * 255);
    if (lux < 180) return blend(CRGB(255, 255, 200), CRGB::Orange,        (lux - 80)  / 100.0f * 255);
                   return blend(CRGB::Orange,         CRGB::Red,           (lux - 180) / 70.0f  * 255);
}

// ============================================================
// BUTTON
// ============================================================
bool          lastButton     = LOW;
bool          dimDir         = HIGH;
unsigned long pressStart     = 0;
bool          longHandled    = false;
float         targetLux      = 50;
unsigned long lastLuxRampMs  = 0;



struct Config {
    int inRange;
    int outRange;
    int zeroRange;
    float velocityConst;
    float targetLux;
    float    dLux[20];
    uint16_t dPwm[20];
    int      dSize;
    float    hLux[20];
    uint16_t hPwm[20];
    int      hSize;
} cfg;


void loadConfig() {
    cfg.inRange       = prefs.getInt  ("inRange",       50);
    cfg.outRange      = prefs.getInt  ("outRange",      350);
    cfg.zeroRange     = prefs.getInt  ("zeroRange",     0);
    cfg.velocityConst = prefs.getFloat("velocityConst", 68.0f);
    cfg.targetLux     = prefs.getFloat("targetLux",     50.0f);

    // LUT D — fall back to hardcoded defaults if never saved
    cfg.dSize = prefs.getInt("dSize", 0);
    if (cfg.dSize == 0 || prefs.getBytes("dLux", cfg.dLux, cfg.dSize * sizeof(float)) == 0) {
        float    defLux[] = {0,0.01,0.03,0.05,0.1,0.25,0.5,1,5,20,50,500};
        uint16_t defPwm[] = {10,12,15,20,30,45,65,90,130,170,200,500};
        cfg.dSize = 12;
        memcpy(cfg.dLux, defLux, cfg.dSize * sizeof(float));
        memcpy(cfg.dPwm, defPwm, cfg.dSize * sizeof(uint16_t));
    } else {
        prefs.getBytes("dPwm", cfg.dPwm, cfg.dSize * sizeof(uint16_t));
    }

    // LUT H — fall back to hardcoded defaults if never saved
    cfg.hSize = prefs.getInt("hSize", 0);
    if (cfg.hSize == 0 || prefs.getBytes("hLux", cfg.hLux, cfg.hSize * sizeof(float)) == 0) {
        float    defLux[] = {0,0.5,1,2,5,10,25,50,100,200,350,500};
        uint16_t defPwm[] = {100,120,140,180,250,350,550,800,1100,1500,1800,2000};
        cfg.hSize = 12;
        memcpy(cfg.hLux, defLux, cfg.hSize * sizeof(float));
        memcpy(cfg.hPwm, defPwm, cfg.hSize * sizeof(uint16_t));
    } else {
        prefs.getBytes("hPwm", cfg.hPwm, cfg.hSize * sizeof(uint16_t));
    }
}


void saveConfig() {
    prefs.putInt  ("inRange",       cfg.inRange);
    prefs.putInt  ("outRange",      cfg.outRange);
    prefs.putInt  ("zeroRange",     cfg.zeroRange);
    prefs.putFloat("velocityConst", cfg.velocityConst);
    prefs.putFloat("targetLux",     cfg.targetLux);
    prefs.putInt  ("dSize",         cfg.dSize);
    prefs.putBytes("dLux",          cfg.dLux, cfg.dSize * sizeof(float));
    prefs.putBytes("dPwm",          cfg.dPwm, cfg.dSize * sizeof(uint16_t));
    prefs.putInt  ("hSize",         cfg.hSize);
    prefs.putBytes("hLux",          cfg.hLux, cfg.hSize * sizeof(float));
    prefs.putBytes("hPwm",          cfg.hPwm, cfg.hSize * sizeof(uint16_t));
}

void syncConfigToRuntime() {
    inRange       = cfg.inRange;
    outRange      = cfg.outRange;
    zeroRange     = cfg.zeroRange;
    velocityConst = cfg.velocityConst;
    targetLux     = cfg.targetLux;
    D_SIZE        = cfg.dSize;
    memcpy(d_lux, cfg.dLux, D_SIZE * sizeof(float));
    memcpy(d_pwm, cfg.dPwm, D_SIZE * sizeof(uint16_t));
    H_SIZE        = cfg.hSize;
    memcpy(h_lux, cfg.hLux, H_SIZE * sizeof(float));
    memcpy(h_pwm, cfg.hPwm, H_SIZE * sizeof(uint16_t));
}





void handleButton() {
    bool b = digitalRead(BUTTON_PIN);

    // Press start
    if (lastButton == LOW && b == HIGH) {
        pressStart  = millis();
        longHandled = false;
    }

    // Long press: enter LUX adjust
    if (b == HIGH && !longHandled) {
        if (millis() - pressStart > 1000) {
            longHandled = true;
            mode        = LUX;
            modeStr     = "LUX";
            pressStart  = millis();
        }
    }

    // Long press held: ramp targetLux (non-blocking)
    if (b == HIGH && longHandled) {
        unsigned long interval = (luxFiltered < 5) ? 50 : 15;
        if (millis() - lastLuxRampMs >= interval) {
            lastLuxRampMs = millis();
            float step = (luxFiltered < 5) ? 0.01f : 1.0f;
            if (dimDir == HIGH && targetLux < 250) {
                targetLux += step;
                leds[0] = luxToColor(targetLux);
                leds[0].nscale8(map8(cubicwave8(beat8(120)),  60, 255));
            }
            if (dimDir == LOW && targetLux > 0.01f) {
                targetLux -= step;
                leds[0] = luxToColor(targetLux);
                leds[0].nscale8(map8(cubicwave8(beat8(60)),  60, 255));
            }
            // Boundary hint: flash white at the extremes
            if (targetLux >= 249 || targetLux <= 0.02f) leds[0] = CRGB::White;
        }
    }

    // Release
    if (lastButton == HIGH && b == LOW) {
        if (!longHandled) {
            mode = (Mode)((mode + 1) % 3);
            if (mode == DUNKEL) modeStr = "DUNKEL";
            if (mode == HELL)   modeStr = "HELL";
            if (mode == LUX)  { leds[0] = CRGB::Cyan; modeStr = "LUX"; }
        }
        if (longHandled) {
            dimDir      = !dimDir;
            longHandled = false;
        }
        leds[0] = luxToColor(targetLux); // hold the color that reflects current target
    }

    lastButton = b;
}



void handleRoot()   { 
    server.send_P(200, "text/html; charset=UTF-8", PAGE); 
    }

void handleSrc() {
    server.send_P(200, "text/html; charset=UTF-8", SRC);
}

void handleConf() {
    server.send_P(200, "text/html; charset=UTF-8", CONF);
}

void handleState() {
    char buf[200];
    snprintf(buf, sizeof(buf),
        "{\"mode\":\"%s\",\"lux\":%.3f,\"target\":%.2f,\"pwm\":%u,"
        "\"presence\":\"%s\",\"motion\":\"%s\",\"range\":%d}",
        modeStr.c_str(), luxNow, targetLux, currentPWM,
        presenceStr.c_str(), motionToStr(motionState).c_str(),
        (int)rangeFiltered);
    server.send(200, "application/json", buf);
}

void handleMode() {
    if (server.hasArg("m")) {
        modeStr = server.arg("m");
        if (modeStr == "DUNKEL") mode = DUNKEL;
        if (modeStr == "HELL")   mode = HELL;
        if (modeStr == "LUX")    mode = LUX;
    }
    server.send(200, "text/plain", "OK");
}

void handleTarget() {
    if (server.hasArg("v")) {
        targetLux = server.arg("v").toFloat();
        mode      = LUX;
        modeStr   = "LUX";
        leds[0] = CRGB::Cyan; // LUX color
        // save to NVS, but throttled — NVS has limited write cycles
        static unsigned long lastSave = 0;
        if (millis() - lastSave > 15000) {
            prefs.putFloat("target_lux", targetLux);
            lastSave = millis();
        }
    }
    server.send(200, "text/plain", "OK");
}



void handleConfigGet() {
    StaticJsonDocument<2048> doc;
    doc["inRange"]       = inRange;
    doc["outRange"]      = outRange;
    doc["zeroRange"]     = zeroRange;
    doc["velocityConst"] = velocityConst;
    doc["targetLux"]     = targetLux;

    JsonObject lutD = doc.createNestedObject("lut_d");
    JsonArray dLuxArr = lutD.createNestedArray("lux");
    JsonArray dPwmArr = lutD.createNestedArray("pwm");
    for (int i = 0; i < D_SIZE; i++) { dLuxArr.add(d_lux[i]); dPwmArr.add(d_pwm[i]); }

    JsonObject lutH = doc.createNestedObject("lut_h");
    JsonArray hLuxArr = lutH.createNestedArray("lux");
    JsonArray hPwmArr = lutH.createNestedArray("pwm");
    for (int i = 0; i < H_SIZE; i++) { hLuxArr.add(h_lux[i]); hPwmArr.add(h_pwm[i]); }

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

void handleConfigPost() {
    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));
    if (err) { server.send(400, "application/json", "{\"error\":\"bad json\"}"); return; }

    if (doc.containsKey("inRange"))       cfg.inRange       = doc["inRange"];
    if (doc.containsKey("outRange"))      cfg.outRange      = doc["outRange"];
    if (doc.containsKey("zeroRange"))     cfg.zeroRange     = doc["zeroRange"];
    if (doc.containsKey("velocityConst")) cfg.velocityConst = doc["velocityConst"];
    if (doc.containsKey("targetLux"))     cfg.targetLux     = doc["targetLux"];

    if (doc.containsKey("lut_d")) {
        JsonArray lx = doc["lut_d"]["lux"];
        JsonArray pw = doc["lut_d"]["pwm"];
        int n = min((int)lx.size(), 20);
        cfg.dSize = n;
        for (int i = 0; i < n; i++) { cfg.dLux[i] = lx[i]; cfg.dPwm[i] = pw[i]; }
    }
    if (doc.containsKey("lut_h")) {
        JsonArray lx = doc["lut_h"]["lux"];
        JsonArray pw = doc["lut_h"]["pwm"];
        int n = min((int)lx.size(), 20);
        cfg.hSize = n;
        for (int i = 0; i < n; i++) { cfg.hLux[i] = lx[i]; cfg.hPwm[i] = pw[i]; }
    }

    saveConfig();
    syncConfigToRuntime();
    server.send(200, "application/json", "{}");
}




void handleBoot() {
    char buf[512];

    snprintf(buf, sizeof(buf),
        "{"
        "\"config\":{"
            "\"in_range\":%d,"
            "\"out_range\":%d,"
            "\"zero_range\":%d,"
            "\"velocity_const\":%.2f,"
            "\"target_lux\":%.2f"
        "},"
        "\"state\":{"
            "\"mode\":\"%s\","
            "\"lux\":%.2f,"
            "\"pwm\":%u,"
            "\"range\":%d,"
            "\"presence\":\"%s\","
            "\"motion\":\"%s\""
        "}"
        "}",
        inRange, outRange, zeroRange, velocityConst, targetLux,
        modeStr.c_str(),
        luxFiltered,
        currentPWM,
        (int)rangeFiltered,
        presenceStr.c_str(),
        motionToStr(motionState).c_str()
    );

    server.send(200, "application/json", buf);
}

// ============================================================
// SETUP
// ============================================================

void setup() {
    prefs.begin("config", false);
    loadConfig();
    syncConfigToRuntime();

#ifdef DEBUG
    Serial.begin(115200);
#endif

    // alarm + wifi — not part of cfg, loaded directly
    alarmEnabled = prefs.getBool  ("alarmEnabled", false);
    alarmPin     = prefs.getString("alarmPin",     "0000");
    prefs.getString("wifiSsid", wifiSSID, sizeof(wifiSSID));
    prefs.getString("wifiPass", wifiPASS, sizeof(wifiPASS));

    if (strlen(wifiSSID) == 0) {
        strlcpy(wifiSSID, WIFI_SSID, sizeof(wifiSSID));
        strlcpy(wifiPASS, WIFI_PASS, sizeof(wifiPASS));
    }

    DBGLN("loaded targetLux: " + String(targetLux));                                  // DEBUG

    pinMode(BUTTON_PIN, INPUT);
    Wire.begin();
    initVEML7700(VEML_NORMAL);

    ldSerial.begin(115200, SERIAL_8N1, LD_RX, LD_TX);
    ldBuffer.reserve(128);

    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RES);
    setPWM(0);

    WiFi.begin(wifiSSID, wifiPASS);
    WiFi.setSleep(true);


    while (WiFi.status() != WL_CONNECTED) {
        if (millis() > 15000) { startAP(); break; }
        delay(300);
        Serial.print(".");
    }


#ifdef DEBUG
    DBGLN("\nWiFi OK");
    DBGLN(WiFi.localIP());
#endif

    // All routes registered together before server.begin()

    server.on("/",       handleRoot);
    server.on("/boot", handleBoot);    
    server.on("/SRC",    handleSrc);
    server.on("/CONFIG", handleConf);   
    server.on("/state",  handleState);
    server.on("/mode",   handleMode);
    server.on("/target", handleTarget);
    server.on("/config/get", HTTP_GET,  handleConfigGet);
    server.on("/config/set", HTTP_POST, handleConfigPost);
    server.on("/wifi",   HTTP_POST, handleWifiPost);
    server.on("/alarm",  HTTP_GET,  handleAlarmGet);
    server.on("/alarm",  HTTP_POST, handleAlarmPost);
    server.on("/lux/read", HTTP_GET, handleLuxRead);

    server.begin();



    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
           .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(20); // Keep low — onboard LED is bright

    // setup end
    leds[0] = CRGB::Blue;
    FastLED.show();



}

// ============================================================
// LOOP
// ============================================================
void loop() {
    server.handleClient();
    if (apMode) dnsServer.processNextRequest();
    handleButton();
    readLD2420();
    applyStaleRule();//LD24
    checkAlarm();
    FastLED.show();
    if (alarmTripped && alarmEnabled) { alarmFlash(); return; }  // alarm owns PWM
    
    bool freshLux = updateLux(); //-› luxNow luxFiltered + luxHellFiltered

    // ---- Motion → light (HELL mode only) ----
    if (mode == HELL) {
        applyMotionToLight();
        applyFade();
        leds[0] = CRGB::Green;
        leds[0].nscale8(beatsin8(30, 60, 255));

         
    }

    // ---- Mode-specific PWM ----
    if (mode == DUNKEL) {
        if (freshLux && !fade.active) {
            setPWM(interp(luxFiltered, d_lux, d_pwm, D_SIZE));
        }
        leds[0] = CRGB::Blue;
        leds[0].nscale8(beatsin8(10, 60, 255));        
    }

    else if (mode == HELL) {
        static unsigned long hellMeasureLast = 0;
        static bool          hellMeasuring   = false;

        if (!hellMeasuring && millis() - hellMeasureLast > 30000) {
            // Dim LED so it doesn't contaminate the ambient reading
            setPWM(currentPWM / 3);  // direct hw write, currentPWM unchanged
            initVEML7700(VEML_FAST);
            vemlState     = VEML_WAITING;
            vemlWaitEnd   = millis() + vemlSettleMs();
            hellMeasuring = true;
            hellMeasureLast = millis();
        }

        if (hellMeasuring && freshLux) {
            hellMeasuring = false;
            lastMotionState = M_NONE; // force applyMotionToLight() set fade.active=false ↦ do nothing
            luxHellFiltered = luxHellFiltered * 0.9f + luxNow * 0.1f; // this is used by applyMotionToLight() 
        
        }
        if( currentPWM <= interp(luxHellFiltered, d_lux, d_pwm, D_SIZE) && freshLux){
            luxHellFiltered = luxHellFiltered * 0.9f + luxNow * 0.1f; // this is used by applyMotionToLight() 
            hellMeasureLast = millis();
        }
    }


    else if (mode == LUX) {

        if (freshLux && vemlState != VEML_STALE) {
            float err = (targetLux - luxFiltered) * (luxFiltered < 3 ? 50.0f : 5.0f);
            int32_t next = (int32_t)currentPWM + (int32_t)constrain(err,
                            -(float)currentPWM, 4095.0f - currentPWM);
            uint16_t floor = interp(luxFiltered, d_lux, d_pwm, D_SIZE);
            setPWM((uint16_t)constrain(next, (int32_t)floor, 4095));
        }
        if (vemlState == VEML_STALE) {
            setPWM((uint16_t)map(targetLux, 0, 250, 0, 4095));
        }
    }
    else if (mode == OFF) setPWM(0);
}

//END







`;
// ─────────────────────────────────────────────────────────

const KW  = ['void','return','while','if','else','for','const','true','false',
              'nullptr','new','delete','class','struct','public','private',
              'protected','namespace','using','auto','int','char','bool',
              'float','double','unsigned','long','short','static','inline','extern'];
const TYP = ['ESP8266WebServer','ESP32WebServer','WebServer','WiFiClient',
             'String','IPAddress','Serial','WiFi','server','PROGMEM'];

function esc(s){return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;')}

function tok(line){
  if(line.trim().startsWith('//')) return '<span class="cm">'+esc(line)+'</span>';
  if(line.trim().startsWith('#'))  return '<span class="pp">'+esc(line)+'</span>';
  let r='',i=0;
  while(i<line.length){
    const c=line[i];
    if(c==='"'||c==="'"){
      let j=i+1;
      while(j<line.length&&line[j]!==c){if(line[j]==='\\')j++;j++;}
      r+='<span class="st">'+esc(line.slice(i,j+1))+'</span>';
      i=j+1;continue;
    }
    const ahead=line.slice(i);
    let hit=false;
    for(const k of KW){
      if(new RegExp('^'+k+'(?=[^a-zA-Z0-9_]|$)').test(ahead)){
        r+='<span class="kw">'+k+'</span>';i+=k.length;hit=true;break;
      }
    }
    if(hit)continue;
    for(const t of TYP){
      if(new RegExp('^'+t+'(?=[^a-zA-Z0-9_]|$)').test(ahead)){
        r+='<span class="tp">'+t+'</span>';i+=t.length;hit=true;break;
      }
    }
    if(hit)continue;
    const fm=ahead.match(/^([a-zA-Z_]\w*)(?=\s*\()/);
    if(fm){r+='<span class="fn">'+fm[1]+'</span>';i+=fm[1].length;continue;}
    const nm=ahead.match(/^[0-9]+/);
    if(nm){r+='<span class="nm">'+nm[0]+'</span>';i+=nm[0].length;continue;}
    r+=esc(c);i++;
  }
  return r;
}

let rawMode=false;

function render(code){
  const lines=code.split('\n');
  const tbl=document.getElementById('codeTable');
  tbl.innerHTML='';
  lines.forEach((l,idx)=>{
    const tr=document.createElement('tr');
    const td1=document.createElement('td');td1.className='ln';td1.textContent=idx+1;
    const td2=document.createElement('td');td2.className='cd';td2.innerHTML=tok(l)||' ';
    tr.appendChild(td1);tr.appendChild(td2);tbl.appendChild(tr);
  });
  const fns=(code.match(/\bvoid\s+\w+\s*\(/g)||[]).length;
  document.getElementById('stats').innerHTML=
    '<span><span class="sv">'+lines.length+'</span> lines</span>'+
    '<span><span class="sv">'+code.length+'</span> chars</span>'+
    '<span><span class="sv">'+fns+'</span> functions</span>'+
    '<span style="margin-left:auto">C++ &middot; ESP32 UBÄR</span>';
  document.getElementById('ta').value=code;
}

function toggleRaw(){
  rawMode=!rawMode;
  const btn=document.getElementById('rawBtn');
  const wrap=document.getElementById('codeWrap');
  if(rawMode){
    wrap.innerHTML='<pre style="padding:16px 20px;font-size:13px;line-height:1.7;color:var(--text);white-space:pre-wrap;word-break:break-all;">'+esc(CODE)+'</pre>';
    btn.style.color='#3fb950';btn.style.borderColor='#3fb950';
    btn.textContent='&#10005; highlighted';
  } else {
    wrap.innerHTML='<table id="codeTable"></table>';
    render(CODE);
    btn.style.color='';btn.style.borderColor='';
    btn.innerHTML='&#60;/&#62; raw';
  }
}

function copyCode(){
  const btn=document.getElementById('copyBtn');
  try{navigator.clipboard.writeText(CODE);}
  catch(e){const ta=document.getElementById('ta');ta.select();document.execCommand('copy');}
  btn.innerHTML='&#10003; copied';btn.classList.add('ok');
  setTimeout(()=>{btn.innerHTML='&#128203; copy';btn.classList.remove('ok');},2000);
}

render(CODE.trim());
</script>
</body>
</html>
)SRCEOF";