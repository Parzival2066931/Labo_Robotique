#include <Arduino.h>
#include "MeAnneau.h"

Anneau::Anneau(): _led(0, LEDNUM) {}

void Anneau::Setup() {
  _led.setpin(LED_PIN);
  Serial.println("Setup completed for [Anneau]");
}

void Anneau::setColor(int firstLed, int lastLed, int r, int g, int b) {
  for (int i = 1; i <= LEDNUM; i++) {
    bool inRange = false;

    if (firstLed <= lastLed) {

      inRange = (i >= firstLed && i <= lastLed);
    } else {
      
      inRange = (i >= firstLed || i <= lastLed);
    }

    if (inRange) {

      _led.setColorAt(i, r, g, b);
    } else {
      
      _led.setColorAt(i, 0, 0, 0);
    }
  }
}

void Anneau::fullLeds(int r, int g, int b) {
  setColor(0, LEDNUM, r, g, b);
  _led.show();
}

void Anneau::halfLeds(int r, int g, int b) {
  setColor(_firstLed, _lastLed, r, g, b);
  _led.show();
}

void Anneau::setFirstLed(int led) {
  _firstLed = led;
}

void Anneau::setLastLed(int led) {
  _lastLed = led;
}