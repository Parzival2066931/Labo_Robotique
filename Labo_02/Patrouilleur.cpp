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
  _rondeDelay = 10000;
  _turnAngle = 90;
  _turnSpeed = 100;

  _state = NORMAL;

  _maxSpeed = 255;
  _normalSpeed = _maxSpeed * 0.7;
  _slowSpeed = _maxSpeed * 0.5;

  _sonar.setPrintDelay(_printDelay);

  Serial.println("Setup completed for [Patrouille]");
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

void Patrouille::setNormalSpeed(int speed) {
  _normalSpeed = speed;
}

void Patrouille::setSlowSpeed(int speed) {
  _slowSpeed = speed;
}

void Patrouille::_normalState() {
  static bool firstTime = true;

  static bool transition = _dist < _slowDist;
  static bool transitionRonde = _dist > _slowDist;
  


  if(firstTime) {
    firstTime = false;
    _lastRonde = _currentTime;
  }

  _anneau.setFirstLed(4);
  _anneau.setLastLed(10);  
  _anneau.halfLeds(0, 10, 0);

  _conduit.Forward(_normalSpeed);

  if(transition) {
    firstTime = true;
    _state = RALENTI;
  }

  if(_currentTime - _lastRonde < _rondeDelay) return;

  if(transitionRonde) {
    firstTime = true;
    _state = RONDE;
  }
}

void Patrouille::_slowState() {
  static bool upTransition = _dist > _slowDist;
  static bool downTransition = _dist < _dangerDist;

  _anneau.setFirstLed(10);
  _anneau.setLastLed(4);  
  _anneau.halfLeds(0, 0, 10);
  
  _conduit.Forward(_slowSpeed);

  if(upTransition) _state = NORMAL;
  if(downTransition) _state = DANGER;
}

void Patrouille::_dangerState() {
  static bool firstTime = true;
  static bool transition = _dist > _dangerDist;

  if(firstTime) {
    firstTime = false;

    _lastStop = _currentTime;
    _lastBackward = _currentTime;
  }

  _anneau.fullLeds(10, 0, 0);
  
  _conduit.Stop();
  if (_currentTime - _lastStop < _stopDelay) return;
  
  _conduit.Backward(_slowSpeed);
  if(_currentTime - _lastBackward < _backwardDelay) return;
  
  _conduit.TurnLeft(_turnAngle, _turnSpeed);

  if(transition) {
    firstTime = true;
    _state = RALENTI;
  }
}

void Patrouille::Update() {
  _currentTime = millis();

  _sonar.update();
  _dist = _sonar.getDist();

  switch(_state) {
    case NORMAL:
      _normalState();
      break;
    case RALENTI:
      _slowState();
      break;
    case DANGER:
      _dangerState();
      break;
    case RONDE:
      _rondeState();
      break;
  }
}