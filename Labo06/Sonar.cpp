#include <Arduino.h>
#include "MeSonar.h"

Sonar::Sonar()
  : _sensor(PORT_10) {}

void Sonar::Setup() {
  _lastUpdate = 0;
  _lastDist = 0;

  _minDist = 0;
  _maxDist = 400;

  Serial.println("Setup completed for [Sonar]");
}

void Sonar::SetMinDist(int dist) {
  _minDist = dist;
}

void Sonar::SetMaxDist(int dist) {
  _maxDist = dist;
}

void Sonar::SetPrintDelay(int delay) {
  _printDelay = delay;
}

int Sonar::GetDist() { return _dist; }
int Sonar::GetMinDist() const { return _minDist; }
int Sonar::GetMaxDist() const { return _maxDist; }

void Sonar::printDist() const {
  
  Serial.print("Distance: ");
  Serial.print(_dist);
  Serial.println(" cm");
}

void Sonar::Update() {
  _currentTime = millis();
  if (_currentTime - _lastUpdate < _printDelay) { return; }
  
  _dist = _sensor.distanceCm();

  if (_dist > 0) {
    _lastDist = _dist;
  } else {
    _dist = _lastDist;
  }

  constrain(_dist, _minDist, _maxDist);
  
  _lastUpdate = _currentTime;
}