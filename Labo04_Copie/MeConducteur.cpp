#include <Arduino.h>
#include "MeConducteur.h"

static Conducteur *_conducteur = nullptr;

void Conducteur::_isr_process_encoder1() {
  if (digitalRead(_encoderRight.getPortB()) == 0) {
    _encoderRight.pulsePosMinus();
  } else {
    _encoderRight.pulsePosPlus();
    ;
  }
}

void Conducteur::_isr_process_encoder2() {
  if (digitalRead(_encoderLeft.getPortB()) == 0) {
    _encoderLeft.pulsePosMinus();
  } else {
    _encoderLeft.pulsePosPlus();
    ;
  }
}

void Conducteur::GetIsrRight() {
  if (_conducteur != nullptr) _conducteur->_isr_process_encoder1();
}

void Conducteur::GetIsrLeft() {
  if (_conducteur != nullptr) _conducteur->_isr_process_encoder2();
}



Conducteur::Conducteur()
  : _gyro(0, GYRO_ADRESS),
    _encoderRight(SLOT1),
    _encoderLeft(SLOT2),
    _pid(_kp, _ki, _kd) {
  _conducteur = this;
}

void Conducteur::Setup() {
  _gyro.begin();

  _cState = STOP;
  _dState = FREE;

  _firstTime = true;

  //encodeurs

  //Set PWM 8KHz
  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(CS11) | _BV(WGM12);

  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS21);

  _encoderRight.setPulse(PULSE);
  _encoderLeft.setPulse(PULSE);
  _encoderRight.setRatio(RATIO);
  _encoderLeft.setRatio(RATIO);
  _encoderRight.setPosPid(1.8, 0, 1.2);
  _encoderLeft.setPosPid(1.8, 0, 1.2);
  _encoderRight.setSpeedPid(0.18, 0, 0);
  _encoderLeft.setSpeedPid(0.18, 0, 0);

  _oneRot = PULSE * RATIO;
  _circonference = DIAMETER * 3.1416;

  Serial.println("Setup completed for [Conducteur]");
}

void Conducteur::Update() {


  _encoderRight.loop();
  _encoderLeft.loop();

  switch (_cState) {
    case FORWARD:
      if (_dState == DISTANCE) _DriveTo();
      else _Drive();
      break;
    case BACKWARD:
      if (_dState == DISTANCE) _DriveTo();
      else _Drive();
      break;
    case STOP:
      _Stop();
      break;
    case RTURNING:
      if (_dState == DISTANCE) _TurnRightTo();
      else _TurnRight();
      break;
    case LTURNING:
      if (_dState == DISTANCE) _TurnLeftTo();
      else _TurnLeft();
      break;
  }
}



int Conducteur::GetSpeed() const {
  return _speed;
}

ConducteurState Conducteur::GetState() const {
  return _cState;
}

bool Conducteur::GetTurnState() const {
  return _turnSuccess;
}

int Conducteur::GetPinRight() const {
  return _encoderRight.getIntNum();
}

int Conducteur::GetPinLeft() const {
  return _encoderLeft.getIntNum();
}

long Conducteur::GetDistToGo() const {
  return _encoderRight.distanceToGo();
}

float Conducteur::GetDistanceTraveled() {
  long pulsesRight = labs(_encoderRight.getPulsePos());
  long pulsesLeft = labs(_encoderLeft.getPulsePos());

  float distanceRight = pulsesRight / _oneRot * _circonference;
  float distanceLeft = pulsesLeft / _oneRot * _circonference;

  return max(distanceRight, distanceLeft);
}



void Conducteur::_Stop() {
  _encoderRight.setMotorPwm(0);
  _encoderLeft.setMotorPwm(0);
}

void Conducteur::_DriveTo() {
  bool forward = (_cState == FORWARD);
  static long targetPulse = 0;

  if (_firstTime) {
    _firstTime = false;

    _encoderRight.setPulsePos(0);
    _encoderLeft.setPulsePos(0);

    targetPulse = (_distance / _circonference * _oneRot);

    _gyro.update();
    _pid.ResetValues();
    _pid.SetTarget(_gyro.getAngleZ());
  }

  _gyro.update();
  _pid.SetValue(_gyro.getAngleZ());
  _pid.Update();
  double correction = _pid.GetCorrection();

  int baseRight = forward ? -_speed : _speed;
  int baseLeft = forward ? _speed : -_speed;

  _encoderRight.setMotorPwm(baseRight + correction);
  _encoderLeft.setMotorPwm(baseLeft + correction);

  long rightPulse = labs(_encoderRight.getPulsePos());
  long leftPulse = labs(_encoderLeft.getPulsePos());
  long currentPulse = min(rightPulse, leftPulse);

  if (currentPulse >= targetPulse) {
    SetState(STOP);
  }
}

void Conducteur::_Drive() {
  bool forward = (_cState == FORWARD);


  if (_firstTime) {
    _firstTime = false;

    _gyro.update();
    _pid.ResetValues();
    _pid.SetTarget(_gyro.getAngleZ());
  }

  _gyro.update();
  _pid.SetValue(_gyro.getAngleZ());
  _pid.Update();
  double correction = _pid.GetCorrection();

  int baseRight = forward ? -_speed : _speed;
  int baseLeft = forward ? _speed : -_speed;

  _encoderRight.setMotorPwm(baseRight + correction);
  _encoderLeft.setMotorPwm(baseLeft + correction);
}

void Conducteur::_TurnRight() {
  _encoderRight.setMotorPwm(_turnSpeed);
  _encoderLeft.setMotorPwm(_turnSpeed);
}

void Conducteur::_TurnRightTo() {
  static float startAngle = 0;
  static float target = 0;
  float currentAngle = 0;
  float deltaAngle = 0;
  bool transition = false;


  if (_firstTime) {
    _firstTime = false;
    _turnSuccess = false;
    _gyro.update();
    startAngle = _gyro.getAngleZ();
    target = startAngle + _angle;
  }

  _encoderRight.setMotorPwm(_turnSpeed);
  _encoderLeft.setMotorPwm(_turnSpeed);

  _gyro.update();
  currentAngle = _gyro.getAngleZ();

  deltaAngle = currentAngle - target;
  if (deltaAngle > 180) deltaAngle -= 360;
  if (deltaAngle < -180) deltaAngle += 360;

  transition = fabs(deltaAngle) <= 2;

  if (transition) {
    _turnSuccess = true;
    _cState = STOP;

    _pid.ResetValues();
    _pid.SetTarget(_gyro.getAngleZ());
  }
}

void Conducteur::_TurnLeft() {
  _encoderRight.setMotorPwm(-_turnSpeed);
  _encoderLeft.setMotorPwm(-_turnSpeed);
}

void Conducteur::_TurnLeftTo() {
  static float startAngle = 0;
  static float target = 0;
  float currentAngle = 0;
  float deltaAngle = 0;
  bool transition = false;

  if (_firstTime) {
    _firstTime = false;
    _turnSuccess = false;

    _gyro.update();
    startAngle = _gyro.getAngleZ();
    target = startAngle - _angle;
  }

  _encoderRight.setMotorPwm(-_turnSpeed);
  _encoderLeft.setMotorPwm(-_turnSpeed);

  _gyro.update();
  currentAngle = _gyro.getAngleZ();

  deltaAngle = currentAngle - target;
  if (deltaAngle > 180) deltaAngle -= 360;
  if (deltaAngle < -180) deltaAngle += 360;

  transition = fabs(deltaAngle) <= 2;

  if (transition) {
    _turnSuccess = true;
    _cState = STOP;

    _pid.ResetValues();
    _pid.SetTarget(_gyro.getAngleZ());
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
  static ConducteurState lastState = STOP;

  if (_cState == state) return;

  if (state == LTURNING || state == RTURNING) {
    _turnSuccess = false;
  }
  _cState = state;
  lastState = _cState;

  _firstTime = true;

  if (state == FORWARD || state == BACKWARD) {
    _gyro.update();
    _pid.ResetValues();
    _pid.SetTarget(_gyro.getAngleZ());
  }
}

void Conducteur::SetDriveMode(DriveState state) {
  _dState = state;
}

void Conducteur::SetSpeed(int speed) {
  if (speed >= _minSpeed && speed <= _maxSpeed) _speed = speed;
}

void Conducteur::SetTurnSpeed(int speed) {
  _turnSpeed = speed;
}

void Conducteur::SetPID(double p, double i, double d) {
  _kp = p;
  _ki = i;
  _kd = d;
  _pid.SetPID(p, i, d);
}

void Conducteur::SetDistance(float distance) {
  _distance = distance;
}
