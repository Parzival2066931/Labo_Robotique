#include <Arduino.h>
#include "MeAnneau.h"

Anneau::Anneau()
  : _led(0, LEDNUM) {}

void Anneau::Setup() {
  _led.setpin(LED_PIN);
  Serial.println("Setup completed for [Anneau]");
}

void Anneau::SetColor(int firstLed, int lastLed, int r, int g, int b) {
  for (int i = 0; i < LEDNUM; i++) {
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
  SetColor(0, LEDNUM, r, g, b);
  _led.show();
}

void Anneau::partLeds(int r, int g, int b) {
  SetColor(_firstLed, _lastLed, r, g, b);
  _led.show();
}

void Anneau::oneLed(int i, int r, int g, int b) {
  _led.setColor(i, r, g, b);
  _led.show();
}

void Anneau::trailLed(int r, int g, int b, bool sensHoraire) {
  static short idx = 1;



  SetColor(0, LEDNUM, 0, 0, 0);
  _led.setColor(idx, r, g, b);
  _led.show();

  if (sensHoraire) {
    idx = idx == LEDNUM ? 1 : idx + 1;
  } else {
    idx = (idx <= 1) ? LEDNUM - 1 : idx - 1;
  }
}

void Anneau::SetFirstLed(int led) {
  _firstLed = led;
}

void Anneau::SetLastLed(int led) {
  _lastLed = led;
}

void Anneau::RainbowRing() {
  static float j;
  static float f;
  static float k;

  for (uint8_t t = 0; t < LEDNUM; t++) {
    uint8_t red = 8 * (1 + sin(t / 2.0 + j / 4.0));
    uint8_t green = 8 * (1 + sin(t / 1.0 + f / 9.0 + 2.1));
    uint8_t blue = 8 * (1 + sin(t / 3.0 + k / 14.0 + 4.2));
    _led.setColorAt(t, red, green, blue);
  }
  _led.show();

  j += random(1, 6) / 6.0;
  f += random(1, 6) / 6.0;
  k += random(1, 6) / 6.0;
}