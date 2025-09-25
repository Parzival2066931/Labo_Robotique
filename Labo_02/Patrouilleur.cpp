#include <Arduino.h>
#include "Patrouilleur.h"


Patrouille::Patrouille()
  : _sonar(), _conducteur(), _anneau() {}
void Patrouille::Setup() {
  _sonar.Setup();
  _conducteur.Setup();
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

  _maxSpeed = 255;
  _normalSpeed = _maxSpeed * 0.7;
  _slowSpeed = _maxSpeed * 0.5;

  _state = NORMAL;

  _conducteur.SetState(STOP);
  _conducteur.SetAngle(_turnAngle);
  _conducteur.SetTurnSpeed(_normalSpeed);

  _sonar.SetPrintDelay(_printDelay);

  Serial.println("Setup completed for [Patrouille]");
}

void Patrouille::SetBackwardDelay(int delay) {
  _backwardDelay = delay;
}

void Patrouille::SetStopDelay(int delay) {
  _stopDelay = delay;
}

void Patrouille::SetPrintDelay(int delay) {
  _printDelay = delay;
  _sonar.SetPrintDelay(_printDelay);
}

void Patrouille::SetTurnAngle(int angle) {
  _turnAngle = angle;
}

void Patrouille::SetNormalSpeed(int speed) {
  _normalSpeed = speed;
}

void Patrouille::SetSlowSpeed(int speed) {
  _slowSpeed = speed;
}

void Patrouille::_rondeState() {
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;
    _lastBlink = _currentTime;
    _lastSuccess = _currentTime;
    _conducteur.SetState(STOP);
    Serial.println("Entering in: RONDE");
  }

  if (_currentTime - _lastBlink < _blinkDelay) return;
  _blinkState = !_blinkState;
  _lastBlink = _currentTime;

  if (_blinkState) {
    _anneau.fullLeds(0, 100, 0);
  } else {
    _anneau.fullLeds(0, 0, 0);
  }

  if (_currentTime - _lastSuccess < _rondeStateDelay) return;
  
  firstTime = true;
  _state = NORMAL;
}

void Patrouille::_normalState() {
  static bool firstTime = true;

  bool transition = _dist < _slowDist;
  bool transitionRonde = _dist > _slowDist;

  if (firstTime) {
    firstTime = false;

    _lastRonde = _currentTime;
    Serial.println("Entering in: NORMAL");
  }

  _anneau.SetFirstLed(5);
  _anneau.SetLastLed(11);
  _anneau.halfLeds(0, 10, 0);

  _conducteur.SetSpeed(_normalSpeed);
  _conducteur.SetState(FORWARD);

  if (transition) {
    firstTime = true;
    _state = RALENTI;
  }

  if (_currentTime - _lastRonde < _rondeDelay) return;

  if (transitionRonde) {
    firstTime = true;
    _state = RONDE;
  }
}

void Patrouille::_slowState() {
  static bool firstTime = true;

  bool upTransition = _dist > _slowDist;
  bool downTransition = _dist < _dangerDist;

  if (firstTime) {
    firstTime = false;
    Serial.println("Entering in: RALENTI");
  }

  _anneau.SetFirstLed(11);
  _anneau.SetLastLed(5);
  _anneau.halfLeds(0, 0, 10);

  _conducteur.SetSpeed(_slowSpeed);
  _conducteur.SetState(FORWARD);

  if (upTransition) _state = NORMAL;
  if (downTransition) _state = DANGER;
}

void Patrouille::_dangerState() {
  static bool firstTime = true;
  bool transition = _dist > _dangerDist;
  bool hasTurned = _conducteur.GetTurnState();

  if (firstTime) {
    firstTime = false;

    _lastStop = _currentTime;
    _lastBackward = 0;
    _hasTurned = false;
    Serial.println("Entering in: DANGER");
    _conducteur.SetState(STOP);
    _anneau.fullLeds(10, 0, 0);
  }

  if (_currentTime - _lastStop < _stopDelay) return;

  if (_lastBackward == 0) {
    _lastBackward = _currentTime;
    _conducteur.SetSpeed(_slowSpeed);
    _conducteur.SetState(BACKWARD);
    return;
  }

  if (_currentTime - _lastBackward < _backwardDelay) return;

  
  _conducteur.SetState(LTURNING);

  if (!hasTurned) return;

  _conducteur.SetState(STOP);

  if (transition) {
    firstTime = true;
    _state = RALENTI;
  }
}

void Patrouille::Update() {
  _currentTime = millis();

  _sonar.Update();
  _dist = _sonar.GetDist();

  switch (_state) {
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
  _conducteur.Update();

}

void Patrouille::PrintTask() {
  _sonar.printDist();
}