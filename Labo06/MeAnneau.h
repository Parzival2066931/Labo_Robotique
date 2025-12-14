#pragma once
#include <MeAuriga.h>


#define ALL_LEDS 0
#define LEDNUM  12 
#define LED_PIN 44

class Anneau {
  private:
    MeRGBLed _led;
    int _firstLed;
    int _lastLed; //DA: 2066931
    
  public:
    Anneau();
    void Setup();
    void fullLeds(int r, int g, int b);
    void halfLeds(int r, int g, int b); // n'etteint pas les autres leds
    void partLeds(int r, int g, int b); // éteint les autres leds
    void offLeds();
    void oneLed(int i, int r, int g, int b);
    void SetColor(int firstLed, int lastLed, int r, int g, int b);
    void SetPartColor(int firstLed, int lastLed, int r, int g, int b);
    void SetFirstLed(int led);
    void SetLastLed(int led);
    void trailLed(int r, int g, int b, bool sensHoraire);

    void RainbowRing();
};