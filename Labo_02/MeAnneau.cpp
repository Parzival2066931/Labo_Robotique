#include <Arduino.h>
#include "MeAnneau.h"

Anneau::Anneau(): _led(0, LEDNUM) {}

void Anneau::Setup() {
  _led.setpin(LED_PIN);

  _firstLed = 4;
  _lastLed = 10;
}

void Anneau::setColor(int firstLed, int lastLed, int r, int g, int b) {
  for( int i = 0; i < LEDNUM; i++) {

    if(i >= firstLed && i <= lastLed) {
      _led.setColorAt(i, r, g, b);
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



