
/****************************************************
 LIGHT CONTROLLER
 ESP32-S3 (core3.x) + VEML7700 + Web UI (Dark Mode)

 FEATURES:
 - DUNKEL / HELL / LUX modes
 - LUT-based PWM control
 - 12-bit LED PWM
 - button: short = mode, long = adjust
 - Web UI (dark polished tablet interface)
 - live lux graph + slider
 - reload restores state
****************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "config.h"
#include "webpage.h"

// -----------------------------
// WEB SERVER
// -----------------------------
WebServer server(80);

// -----------------------------
// STATE
// -----------------------------
enum Mode { DUNKEL, HELL, LUX };
Mode mode = DUNKEL;

String modeStr = "DUNKEL";
float luxNow = 0;
float luxFiltered = 0;
float targetLux = 50;
bool highSense = false;
bool dSun = false;

unsigned long measureLast = 0;
uint16_t currentPWM = 0;  
uint16_t lastPWM = 0;
// -----------------------------
// PWM (ESP32-C3 v3.x)
// -----------------------------

const int PWM_PIN = 21;
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;
const int PWM_RES = 12;

// -----------------------------
// BUTTON
// -----------------------------
#define BUTTON_PIN 4
bool lastButton = LOW ;
bool dim = HIGH;
unsigned long pressStart = 0;

bool longHandled = false;

#define VEML_ADDR 0x10
#define ALS_REG 0x04


void initVEML7700(uint8_t sense) {
    Wire.beginTransmission(VEML_ADDR);
    Wire.write(0x00);        // ALS config register
    Wire.write(0x00);        // LSB
    if (sense==2) Wire.write(0x0C);        // MSB (800 ms, gain x1)
    else if (sense == 1)     Wire.write(0x00);// MSB: IT=100ms, gain=1
    else if (sense == 0)     Wire.write(0xC0);        // MSB: IT=25ms, gain=1/8
    else if (sense == 3)     Wire.write(0x08); // 25 ms, gain x2 
    Wire.endTransmission();
}

float readLuxFast(){  // read fast 25ms gain ×2 sense 3

    Wire.beginTransmission(VEML_ADDR);    
    Wire.write(ALS_REG);
    Wire.endTransmission(false); // repeated start

    Wire.requestFrom(VEML_ADDR, 2);
    if (Wire.available() < 2) return -1;

    uint16_t raw = Wire.read();
    raw |= (Wire.read() << 8);
 
    return raw * 0.1152; // 25ms gain 2×
    
        
}


float readLux() {

    Wire.beginTransmission(VEML_ADDR);
    Wire.write(ALS_REG);
    Wire.endTransmission(false); // repeated start

    Wire.requestFrom(VEML_ADDR, 2);
    if (Wire.available() < 2) return -1;

    uint16_t raw = Wire.read();
    raw |= (Wire.read() << 8);

    float factor = highSense ? 0.0072 : (dSun ? 1.8432 : 0.0576);

    if (raw < 100 && !highSense) {
        highSense = true;
        dSun = false;
        initVEML7700(2);
        delay(800);
    }
        else if ((raw > 1000 && highSense) || (raw < 1000 && dSun) ){
         highSense = false;
         dSun = false; 
         initVEML7700(1);         
         delay (100);  
        }

        else if (raw > 50000){ //direct sun fallback
            highSense = false;
            initVEML7700(0); 
            dSun=true;
            delay (25);
        } 
    if(highSense)delay(250); 
    return raw * factor;
    
        
}

// -----------------------------
// LUTS look up table
// -----------------------------

// DUNKEL (0–50 lux → 10–200 PWM)
const float d_lux[] = {0,0.01,0.03,0.05,0.1,0.25,0.5,1,5,20,50};
const uint16_t d_pwm[] = {10,12,15,20,30,45,65,90,130,170,200};
const int D_SIZE = 11;

// HELL (0–500 lux → 100–2000 PWM)
const float h_lux[] = {0,0.5,1,2,5,10,25,50,100,200,350,500};
const uint16_t h_pwm[] = {100,120,140,180,250,350,550,800,1100,1500,1800,2000};
const int H_SIZE = 12;

// -----------------------------
// INTERPOLATION
// -----------------------------
uint16_t interp(float x, const float *lx, const uint16_t *pw, int size) {
    if (x <= lx[0]) return pw[0];
    if (x >= lx[size-1]) return pw[size-1];

    for (int i=0;i<size-1;i++) {
        if (x < lx[i+1]) {
            float t = (x-lx[i])/(lx[i+1]-lx[i]);
            return pw[i] + t*(pw[i+1]-pw[i]);
        }
    }
    return pw[size-1];
}





// -----------------------------
// PWM OUTPUT
// -----------------------------
void setPWM(uint16_t v) {
    ledcWrite(PWM_PIN, v);
}

// -----------------------------
// BUTTON LOGIC
// -----------------------------
void handleButton() {
    bool b = digitalRead(BUTTON_PIN);


    if (lastButton == LOW && b == HIGH) {
        pressStart = millis();
        longHandled = false;
    }

    if (b == HIGH && !longHandled) {
        if (millis() - pressStart > 1000) {
            longHandled = true;
            if(dim == HIGH)targetLux += 0.10;
            if(dim == LOW)targetLux -= 0.10;

            mode = LUX;
            modeStr = "LUX";
            pressStart = millis();
            
        }
    }
    //dimming
    if (b == HIGH && longHandled){
            if(dim == HIGH && targetLux < 250)targetLux += 0.1;
            if(dim == LOW  && targetLux > 0)targetLux -= 0.1;
            delay (luxFiltered<5?50:5);
    }
   // button let go
    if (lastButton == HIGH && b == LOW) {
        if (!longHandled) {
            mode = (Mode)((mode + 1) % 3);

            if (mode == DUNKEL) modeStr = "DUNKEL";
            if (mode == HELL) modeStr = "HELL";
            if (mode == LUX) modeStr = "LUX";
        }
        if (longHandled){
            dim =! dim;
            longHandled = false;
        } 
    }

    lastButton = b;
}



// -----------------------------
// WEB HANDLERS
// -----------------------------
void handleRoot() {
    server.send_P(200,"text/html",PAGE);
}

void handleState() {
    String j="{";
    j+="\"mode\":\""+modeStr+"\",";
    j+="\"lux\":"+String(luxNow)+",";
    j+="\"target\":"+String(targetLux)+",";
    j+="\"pwm\":"+String(currentPWM);
    j+="}";
    server.send(200,"application/json",j);
}

void handleMode() {
    if(server.hasArg("m")) {
        modeStr=server.arg("m");
        if(modeStr=="DUNKEL") mode=DUNKEL;
        if(modeStr=="HELL") mode=HELL;
        if(modeStr=="LUX") mode=LUX;
    }
    server.send(200,"text/plain","OK");
}

void handleTarget() {
    if(server.hasArg("v")) {
        targetLux=server.arg("v").toFloat();
        mode=LUX;
        modeStr="LUX";
    }
    server.send(200,"text/plain","OK");
}

// -----------------------------
// SETUP WEB
// -----------------------------
void setupWeb() {
    server.on("/",handleRoot);
    server.on("/state",handleState);
    server.on("/mode",handleMode);
    server.on("/target",handleTarget);
    server.begin();
}

// -----------------------------
// SETUP
// -----------------------------
void setup() {
    //Serial.begin(115200);
    pinMode(BUTTON_PIN,INPUT);
    Wire.begin();
    initVEML7700(highSense?2:1); 

    ledcAttach(PWM_PIN, PWM_FREQ, PWM_RES);

    WiFi.begin(WIFI_SSID,WIFI_PASS);

    setupWeb();
}

// -----------------------------
// LOOP
// -----------------------------
void loop() {

    
    lastPWM = currentPWM * 0.5 + lastPWM * 0.5;
    server.handleClient();
    handleButton();
 
 
    if(mode==DUNKEL){
        
         luxNow = readLux();
         luxFiltered = luxFiltered*0.9 + luxNow*0.1;

        currentPWM=interp(luxFiltered,d_lux,d_pwm,D_SIZE);
        delay(50);
    }

    else if(mode==HELL){
        delay(100);
        luxNow = readLux(); // webupdate only
        longHandled = false;
        if (millis() - measureLast > 30000 ) {
            setPWM(currentPWM/3);
            initVEML7700(3); // set it 25ms gain 2×            
            delay(30);
            luxFiltered=readLuxFast(); // read it 25ms gain 2×
            initVEML7700(highSense?2:(dSun?0:1)); // set back
            measureLast = millis();
        }
        currentPWM=interp(luxFiltered,h_lux,h_pwm,H_SIZE);
    }

    else if(mode==LUX){
        
        luxNow = readLux();
        luxFiltered = luxFiltered*0.9 + luxNow*0.1;

        float err = (targetLux - luxFiltered) * (luxFiltered<3?50:5);
        err = constrain(err, -currentPWM, 4095 - currentPWM);
        currentPWM += err;
        //if(currentPWM + err >= 0) currentPWM += err; // zero underflow  protect
        
        currentPWM = constrain(currentPWM, interp(luxFiltered,d_lux,d_pwm,D_SIZE), 4095);
        delay(50);
    }

if (abs(lastPWM - currentPWM)>100){
    setPWM(lastPWM*0.9+currentPWM*0.1);
    delay(100);
}
else
     setPWM(currentPWM);

 delay(10); 
}