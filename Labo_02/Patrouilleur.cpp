#include <Arduino.h>
#include "Patrouilleur.h"


Patrouille::Patrouille():  _sonar(), _conduit(), _anneau() {}

void Patrouille::Setup() {
  _sonar.Setup();
  _conduit.Setup();
  _anneau.Setup();

  _minDist = 0;
  _dangerDist = 40;
  _slowDist = 80;
  _backwardDelay = 1000;
  _stopDelay = 2000;
  _printDelay = 250;
  _turnAngle = 90;
  _state = NORMAL;

  _sonar.setPrintDelay(_printDelay);
}

void Patrouille::setBackwardDelay(int delay) {
  _backwardDelay = delay;
}
void Patrouille::setStopDelay(int delay) {
  _stopDelay = delay;
}

void Patrouille::setPrintDelay(int delay) {
  _printDelay = delay;
  _sonar.setPrintDelay(_printDelay);
}

void Patrouille::setTurnAngle(int angle) {
  _turnAngle = angle;
}

void Patrouille::Update() {
  _currentTime = millis();

  _sonar.update();
  _dist = _sonar.getDist();

  switch(_state) {
    //...
  }
}



