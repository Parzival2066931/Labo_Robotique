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
    void halfLeds(int r, int g, int b);
    void setColor(int firstLed, int lastLed, int r, int g, int b);
    void setFirstLed(int led);
    void setLastLed(int led);
};