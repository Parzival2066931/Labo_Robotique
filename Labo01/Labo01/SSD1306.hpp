#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

 


class ScreenSSD {
  private:
    int _address;
    Adafruit_SSD1306 _screen;
    unsigned long _currentTime;
    unsigned long _lastUpdate = 0;
    int _rate = 100;
    int minHeight = 5;
    int maxHeight = 59;

    int _getHeight(int dist);
    void _sunUpdate(int dist);
    void _drawHouse();
    void _writeName();



  public:
    ScreenSSD(int sw, int sh, int address, int reset);
    void setup();
    void update(int dist);
    
};