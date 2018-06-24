
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "WROVER_KIT_LCD.h"
#include "math.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Define pins for button, relay and temp sensor
const int BUTTON = 2;
const int RELAY = 14;
const int TEMP_SENSOR = 4;

// Define update frequency for LCD, Temp and Button polling/refresh
const int lcdUpdateRate = 500;
const int tempUpdateRate = 500;
const int buttonUpdateRate = 50;

// Define min and max for temp setting.
const float maxTemp = 90;
const float minTemp = 40;

// Step side for temp increase
const float stepSize = 5;

unsigned long lastTempMillis = 0;
unsigned long lastLcdMillis = 0;
unsigned long lastButtonMillis = 0;


bool socketOn = true;
float currentTemp = 0;
float targetTemp = 40;

OneWire oneWire(TEMP_SENSOR);
DallasTemperature sensors(&oneWire);
WROVER_KIT_LCD tft;

void setup() {
  Serial.begin(115200);
  sensors.begin();

  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(RELAY, OUTPUT);

  // Make sure relay is LOW.
  digitalWrite(RELAY, LOW);

  tft.begin();
  tft.setRotation(1);
}



void loop(void) {
  unsigned long currentMillis = millis();

  if (currentMillis - lastTempMillis > tempUpdateRate){
    lastTempMillis = currentMillis;
    
    currentTemp = readTemp();
    Serial.printf("[%d] Current Temp: %.2f*c\n", currentMillis, currentTemp);

    if(targetTemp > currentTemp){
      setSocket(true);
    }else{
      setSocket(false);
    }
    
  }

  if (currentMillis - lastLcdMillis > lcdUpdateRate) {
    lastLcdMillis = currentMillis;
    
    showTemp(currentTemp);
  }

  if (currentMillis - lastButtonMillis > buttonUpdateRate) {
    lastButtonMillis = currentMillis;

    if (digitalRead(BUTTON) == LOW) { // Check if button has been pressed
      if (targetTemp >= maxTemp) {
        targetTemp = minTemp;
      } else {
        targetTemp += 5;
      }

      Serial.println("SW2_BTN LOW");
      showTemp(readTemp());
    }
  }

}

float readTemp() {
  sensors.requestTemperatures(); 
  return sensors.getTempCByIndex(0);
}

void setSocket(bool on){
  
  if(on == socketOn){
    Serial.printf("Socket already %s.\n", on ? "on" : "off");
  }else{
    Serial.printf("Turning socket : %s\n", on ? "on" : "off");
    Serial.println("SETTING RELAY HIGH!");
    digitalWrite(RELAY, HIGH);
    Serial.println("SLEEPING FOR 250ms!");
    delay(250);
    Serial.println("SETTING RELAY LOW!");
    digitalWrite(RELAY, LOW);
  }

  socketOn = on;
}

void showTemp(float temp) {
  tft.fillScreen(WROVER_BLACK);

  tft.setTextColor(WROVER_RED);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.println("ESPide32");


  tft.setTextColor(WROVER_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0, 25);
  tft.printf("Target Temp:  %.2f *c (%.2f *c diff)\n", targetTemp, temp - targetTemp);

  int colour = WROVER_GREEN;

  if (targetTemp > temp) {
    colour = WROVER_BLUE;
  } else if (targetTemp < temp) {
    colour = WROVER_RED;
  }

  tft.setTextColor(colour);
  tft.setTextSize(5);

  tft.setCursor(0, 50);
  tft.printf("%.2f *c", temp);
}
