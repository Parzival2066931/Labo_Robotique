#include <Arduino.h>
#include "SSD1306.hpp"

ScreenSSD::ScreenSSD(int sw, int sh, int SCREEN_ADDRESS, int reset)
  : _screen(sw, sh, &Wire, reset) {
  _address = SCREEN_ADDRESS;
}
void ScreenSSD::setup() {

  if (!_screen.begin(SSD1306_SWITCHCAPVCC, _address)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  _screen.display();
  delay(2000);
  _screen.clearDisplay();
  _drawHouse();
  _writeName();
  
}
void ScreenSSD::_drawHouse() {

  _screen.fillRect(5, 44, 20, 20, SSD1306_WHITE);
  _screen.fillTriangle(0, 44, 30, 44, 15, 30, SSD1306_WHITE);
  _screen.fillRect(10, 52, 10, 15, SSD1306_INVERSE);
  _screen.fillCircle(18, 59, 1, SSD1306_INVERSE);

  _screen.display();
}
void ScreenSSD::_writeName() {
  _screen.setTextSize(2);
  _screen.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  _screen.setCursor(0,0);
  _screen.println(F("Beland"));
  _screen.display();
}
void ScreenSSD::_sunUpdate(int height) {

  _screen.fillRect(70, 0, 64, 128, SSD1306_BLACK);
  _screen.fillCircle(108, height, 10, SSD1306_WHITE);
  _screen.display();
}
void ScreenSSD::update(int dist) {
  _currentTime = millis();
  

  if (_currentTime - _lastUpdate < _rate) { return; }
  _lastUpdate = _currentTime;

  
  _sunUpdate(_getHeight(dist));
}
int ScreenSSD::_getHeight(int dist) {
  const int minDist = 10;
  const int maxDist = 50;
  static int lastDist = 0;
  
  return map(dist, minDist, maxDist, minHeight, maxHeight);
}