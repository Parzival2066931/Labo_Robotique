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
  _rondeStateDelay = 2000;
  _blinkDelay = 400;
  _turnAngle = 90;
  _turnSpeed = 100;

  _blinkState = false;

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

void Patrouille::_rondeState() {
  static bool firstTime = true;

  if(firstTime) {
    firstTime = false;
    _lastBlink = _currentTime;
    _lastSuccess = _currentTime;
    Serial.println("Entering in: RONDE");
  }

  if(_currentTime - _lastBlink < _blinkDelay) return;
  _blinkState = !_blinkState;

  if(_blinkState) {
    _anneau.fullLeds(0, 100, 0);
  }
  else {
    _anneau.fullLeds(0, 0, 0);
  }

  if(_currentTime - _lastSuccess < _rondeStateDelay) return;
  _state = NORMAL;
}

void Patrouille::_normalState() {
  static bool firstTime = true;

  bool transition = _dist < _slowDist;
  bool transitionRonde = _dist > _slowDist;

  if(firstTime) {
    firstTime = false;
    _lastRonde = _currentTime;
    Serial.println("Entering in: NORMAL");
  }

  _anneau.setFirstLed(5);
  _anneau.setLastLed(11);  
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
  static bool firstTime = true;

  bool upTransition = _dist > _slowDist;
  bool downTransition = _dist < _dangerDist;

  if(firstTime) {
    firstTime = false;
    Serial.println("Entering in: RALENTI");

  }


  _anneau.setFirstLed(11);
  _anneau.setLastLed(5);  
  _anneau.halfLeds(0, 0, 10);
  
  _conduit.Forward(_slowSpeed);

  if(upTransition) _state = NORMAL;
  if(downTransition) _state = DANGER;
}

void Patrouille::_dangerState() {
  static bool firstTime = true;
  bool transition = _dist > _dangerDist;

  if(firstTime) {
    firstTime = false;

    _lastStop = _currentTime;
    _lastBackward = 0;
    _hasTurned = false;
    Serial.println("Entering in: DANGER");
    _conduit.Stop();
    _anneau.fullLeds(10, 0, 0);
  }
  
  if (_currentTime - _lastStop < _stopDelay) return;
  Serial.println("Je recule");
  if (_lastBackward == 0) {
    _lastBackward = _currentTime;
    _conduit.Backward(_slowSpeed);
    return;
  }
  if(_currentTime - _lastBackward < _backwardDelay) return;
    _conduit.Stop();
  if(!_hasTurned) {
    Serial.println("Je tourne");
    _conduit.TurnLeft(_turnAngle, _turnSpeed);
    _hasTurned = true;
  }

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