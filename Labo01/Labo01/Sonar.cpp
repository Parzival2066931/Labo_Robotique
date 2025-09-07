#include <Arduino.h>
#include "Sonar.hpp"

Sonar::Sonar(int TRIGGER_PIN, int ECHO_PIN)
  : _hc(TRIGGER_PIN, ECHO_PIN) {
  setup();
}
void Sonar::setup() {


  setMinDist(10);
  setMaxDist(50);
  Serial.println("Setup completed for [Sonar]");
}
void Sonar::setMinDist(int dist) {
  _minDist = dist;
}
void Sonar::setMaxDist(int dist) {
  _maxDist = dist;
}
int Sonar::getDist() {
  static int lastDist = 0;

  if (_currentTime - _lastUpdate < _rate) { return lastDist; }
  

  _dist = _hc.dist();
  _lastUpdate = _currentTime;

  if (_dist > 0) {
    lastDist = _dist;
  } else {
    _dist = lastDist;
  }
  return constrain(_dist, _minDist, _maxDist);
}
void Sonar::_printDist() const {
  if (_currentTime - _lastUpdate < _rate) { return; }
  Serial.print("Distance: ");
  Serial.println(getDist());
}
void Sonar::update() {
  _currentTime = millis();
  _printDist();
  
}