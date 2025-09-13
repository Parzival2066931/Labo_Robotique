#include <Arduino.h>
#include "MeSonar.h"

Sonar::Sonar()
  : _sensor(PORT_10) {}

void Sonar::Setup() {
  _lastUpdate = 0;
  _lastDist = 0;
  

  Serial.println("Setup completed for [Sonar]");
}

void Sonar::setMinDist(int dist) {
  _minDist = dist;
}

void Sonar::setMaxDist(int dist) {
  _maxDist = dist;
}

void Sonar::setPrintDelay(int delay) {
  _printDelay = delay;
}

int Sonar::getDist() { return _dist; }
int Sonar::getMinDist() const { return _minDist; }
int Sonar::getMaxDist() const { return _maxDist; }

void Sonar::_printDist() const {
  
  Serial.print("Distance: ");
  Serial.println(_dist);
}

void Sonar::update() {
  _currentTime = millis();
  if (_currentTime - _lastUpdate < _printDelay) { return; }
  
  _dist = _sensor.distanceCm();

  if (_dist > 0) {
    _lastDist = _dist;
  } else {
    _dist = _lastDist;
  }

  constrain(_dist, _minDist, _maxDist);

  _printDist();
  
  _lastUpdate = _currentTime;
}