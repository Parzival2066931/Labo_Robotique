#include <Arduino.h>
#include "MeAuriga.h"
#include "MeConducteur.h"

Conducteur::Conducteur()
  : _gyro(0, GYRO_ADRESS) {}

void Conducteur::Setup() {
  _gyro.begin();

  _state = STOP;

  _m1_pwm = 11;
  _m1_in1 = 48;
  _m1_in2 = 49;

  _m2_pwm = 10;
  _m2_in1 = 46;
  _m2_in2 = 47;

  pinMode(_m1_pwm, OUTPUT);
  pinMode(_m1_in1, OUTPUT);
  pinMode(_m1_in2, OUTPUT);
  pinMode(_m2_pwm, OUTPUT);
  pinMode(_m2_in1, OUTPUT);
  pinMode(_m2_in2, OUTPUT);

  Serial.println("Setup completed for [Conducteur]");
}

void Conducteur::Update() {

  switch(_state) {
    case FORWARD:
      _Forward();
      break;
    case BACKWARD:
      _Backward();
      break;
    case STOP:
      _Stop();
      break;
    case RTURNING:
      _TurnRight();
      break;
    case LTURNING:
      _TurnLeft();
      break;
  }
}

int Conducteur::GetSpeed() const {
  return _speed;
}

bool Conducteur::GetTurnState() const {
  return _turnSuccess;
}
void Conducteur::_Stop() {
  analogWrite(_m1_pwm, 0);
  analogWrite(_m2_pwm, 0);
}
void Conducteur::_Forward() {
  digitalWrite(_m1_in2, LOW);
  digitalWrite(_m1_in1, HIGH);
  analogWrite(_m1_pwm, _speed);

  digitalWrite(_m2_in2, HIGH);
  digitalWrite(_m2_in1, LOW);
  analogWrite(_m2_pwm, _speed);
}

void Conducteur::_Backward() {
  digitalWrite(_m1_in2, HIGH);
  digitalWrite(_m1_in1, LOW);
  analogWrite(_m1_pwm, _speed);

  digitalWrite(_m2_in2, LOW);
  digitalWrite(_m2_in1, HIGH);
  analogWrite(_m2_pwm, _speed);
}

void Conducteur::_TurnRight() {
  static bool firstTime = true;
  static float startAngle = 0;
  static float target = 0;
  float currentAngle = 0;
  bool transition = false;

  if(firstTime) {
    firstTime = false;
    _turnSuccess = false;
    _gyro.update();
    startAngle = _gyro.getAngle(3);
    target = startAngle + _angle;
  }

  if (target > 180) target -= 360;
  if (target < -180) target += 360;

  digitalWrite(_m1_in1, HIGH);
  digitalWrite(_m1_in2, LOW);
  analogWrite(_m1_pwm, _turnSpeed);

  digitalWrite(_m2_in1, LOW);
  digitalWrite(_m2_in2, HIGH);
  analogWrite(_m2_pwm, _turnSpeed);

  _gyro.update();
  currentAngle = _gyro.getAngle(3);

  transition = fabs(currentAngle - target) <= 2;
  
  if(transition) {
    firstTime = true;
    _turnSuccess = true;
    _state = STOP;
  }
}

void Conducteur::_TurnLeft() {
  static bool firstTime = true;
  static float startAngle = 0;
  static float target = 0;
  float currentAngle;
  bool transition = false;

  if(firstTime) {
    firstTime = false;
    _turnSuccess = false;

    _gyro.update();
    startAngle = _gyro.getAngle(3);
    target = startAngle - _angle;
  }
  
  if (target > 180) target -= 360;
  if (target < -180) target += 360;
  
  digitalWrite(_m1_in1, HIGH);
  digitalWrite(_m1_in2, LOW);
  analogWrite(_m1_pwm, _turnSpeed);
  
  digitalWrite(_m2_in1, HIGH);
  digitalWrite(_m2_in2, LOW);
  analogWrite(_m2_pwm, _turnSpeed);

  _gyro.update();
  currentAngle = _gyro.getAngle(3);

  transition = fabs(currentAngle - target) <= 2;
  
  if(transition) {
    firstTime = true;
    _turnSuccess = true;
    _state = STOP;
  }
}

void Conducteur::SetAngle(int angle) {
  _angle = angle;
}

void Conducteur::SetMinSpeed(int speed) {
  _minSpeed = speed;
}

void Conducteur::SetMaxSpeed(int speed) {
  _maxSpeed = speed;
}

void Conducteur::SetState(ConducteurState state) {
  _state = state;
}

void Conducteur::SetSpeed(int speed) {
  _speed = speed;
}

void Conducteur::SetTurnSpeed(int speed) {
  _turnSpeed = speed;
}