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
    _gyroPid(_kp, _ki, _kd),
    _trackerPid(_kp, _ki, _kd) {
  _conducteur = this;
}

void Conducteur::Setup() {
  _gyro.begin();
  _tracker.Setup();

  _cState = STOP;
  _dState = FREE;
  _fState = ON_LINE;
  _iState = I_NONE;

  _firstTime = true;
  _fSubFirstTime = true;
  _deliveryDone = false;

  _afterFollowDelay = 300;
  _delayRunning = false;
  _limitDist = 40;

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

void Conducteur::Update(int obstacleDist = 0) {
  _currentTime = millis();

  _encoderRight.loop();
  _encoderLeft.loop();

  _tracker.Update();

  // DebugPrint();  

  switch (_cState) {
    case CALIBRATE:
      SetAngle(357);
      _Calibrate_IR();
      break;
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
    case FOLLOW:
      _FollowLine(obstacleDist);
      break;
  }
}

int Conducteur::GetSpeed() const {
  return _speed;
}

ConducteurState Conducteur::GetState() const {
  return _cState;
}

FollowState Conducteur::GetFState() const {
    return _fState;
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
    _gyroPid.ResetValues();
    _gyroPid.SetTarget(_gyro.getAngleZ());
  }

  _gyro.update();
  _gyroPid.SetValue(_gyro.getAngleZ());
  _gyroPid.Update();
  double correction = _gyroPid.GetCorrection();

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
    _gyroPid.ResetValues();
    _gyroPid.SetTarget(_gyro.getAngleZ());
  }

  _gyro.update();
  _gyroPid.SetValue(_gyro.getAngleZ());
  _gyroPid.Update();
  double correction = _gyroPid.GetCorrection();

  int baseRight = forward ? -_speed : _speed;
  int baseLeft = forward ? _speed : -_speed;

  _encoderRight.setMotorPwm(baseRight + correction);
  _encoderLeft.setMotorPwm(baseLeft + correction);
}



void Conducteur::_FollowLine(int dist) {

  switch(_fState) {
    case ON_LINE:
      _onLineState();
      break;
    case TURN_RIGHT:
    case TURN_LEFT:
      _onRightAngle();
      break;
    case NO_LINE:
      _noLineState();
      break;
    case INTERSECTION:
      _onIntersection(dist);
      break;
  }
}

void Conducteur::_Calibrate_IR() {
  static float startAngle = 0;
  static float target = 0;
  float currentAngle = 0;
  float deltaAngle = 0;
  bool transition = false;

  if (_firstTime) {
    _firstTime = false;

    _tracker.ResetValues();
    _gyro.update();
    startAngle = _gyro.getAngleZ();
    target = startAngle - _angle;
  }

  _encoderRight.setMotorPwm(-_turnSpeed);
  _encoderLeft.setMotorPwm(-_turnSpeed);

  _gyro.update();
  _tracker.Calibrate_IR();

  currentAngle = _gyro.getAngleZ();
  deltaAngle = currentAngle - target;
  if (deltaAngle > 180) deltaAngle -= 360;
  if (deltaAngle < -180) deltaAngle += 360;

  transition = fabs(deltaAngle) <= 2;

  if (transition) {
    _Stop();
    _cState = FOLLOW;
  }
}

void Conducteur::_noLineState() {
  static double targetAngle = 0;

  if(_fSubFirstTime) {
    _fSubFirstTime = false;

    _Stop();
    SetAngle(180);

    _gyro.update();
    double startAngle = _gyro.getAngleZ();
    targetAngle = startAngle + _angle;
  }

  bool transition = _rotateTo(targetAngle);
  if(transition) {
    _Stop();
    SetFState(ON_LINE);
  }
}

void Conducteur::_onLineState() {
    static unsigned long lastSeenLine = 0;

    if (_fSubFirstTime) {
        _fSubFirstTime = false;
        _trackerPid.ResetValues();
        _trackerPid.SetTarget(0);
        Serial.println("Entrée dans l'état ON_LINE");
    }

    _tracker.Update();

    // PID suivi de ligne
    _trackerPid.SetValue(_tracker.GetLinePosition());
    _trackerPid.Update();
    double correction = _trackerPid.GetCorrection();

    _encoderRight.setMotorPwm(-_speed + correction);
    _encoderLeft.setMotorPwm(_speed + correction);

    bool rawNoLine = _tracker.IsNoLine();
    bool rightTransition = _tracker.IsRightAngleRight();
    bool leftTransition  = _tracker.IsRightAngleLeft();
    bool intersectionTransition = _tracker.IsIntersection();

    if (!rawNoLine) {
        lastSeenLine = _currentTime;
    }
    bool noLineTransition = rawNoLine && (_currentTime - lastSeenLine > 100);


    if (intersectionTransition) {
      Serial.println("Entrée dans l'état INTERSECTION");
      _iState = I_TURN_90;
      SetFState(INTERSECTION);
      return;
    }

    if (noLineTransition) {
      SetFState(NO_LINE);
      return;
    }

    if (rightTransition) {
      SetFState(TURN_RIGHT);
      return;
    }

    if (leftTransition) {
      SetFState(TURN_LEFT);
      return;
    }
}


void Conducteur::_onRightAngle() {
    static double targetAngle = 0;
    static unsigned long lastTurn = 0;

    if (_fSubFirstTime) {
        _fSubFirstTime = false;

        _delayRunning = true;
        lastTurn = _currentTime;

        int angle = (_fState == TURN_LEFT) ? -90 : 90;
        SetAngle(angle);

        _gyro.update();
        double startAngle = _gyro.getAngleZ();
        targetAngle = startAngle + _angle;

        Serial.println("Je TOURNE");
    }

    if (_delayRunning) {
        if (_currentTime - lastTurn < _afterFollowDelay) {
            _encoderRight.setMotorPwm(-_speed);
            _encoderLeft.setMotorPwm(_speed);
            return;
        }

        _delayRunning = false;
        _Stop();
    }

    bool transition = _rotateTo(targetAngle);
    if (transition) {
        _Stop();
        SetFState(ON_LINE);
    }
}

void Conducteur::_onIntersection(int dist) {
  switch (_iState) {

    case I_TURN_90:
      Serial.println("Je tourne à gauche");
      _iFollowTurn();
      break;

    case I_CHECK_1:
      _iCheck1State(dist);
      break;

    case I_TURN_180:
      _iDetectionTurn();
      break;

    case I_CHECK_2:
      _iCheck2State(dist);
      break;

    case I_NONE:
    default:
      SetFState(ON_LINE);
      break;
  }
}


void Conducteur::_iFollowTurn() {
  static double targetAngle = 0;
  static bool firstTime = true;
  static unsigned long lastTurn = 0;

  if (firstTime) {
    firstTime = false;

    _delayRunning = true;
    lastTurn = _currentTime;

    SetAngle(-88);
    _gyro.update();
    double start = _gyro.getAngleZ();
    targetAngle = start + _angle;
  }

  if (_delayRunning) {
      if (_currentTime - lastTurn < _afterFollowDelay) {
          _encoderRight.setMotorPwm(-_speed);
          _encoderLeft.setMotorPwm(_speed);
          return;
      }

      _delayRunning = false;
      _Stop();
  }

  if (!_rotateTo(targetAngle)) return;

  _Stop();
  firstTime = true;
  _iState = I_CHECK_1;
}

void Conducteur::_iDetectionTurn() {
  static double targetAngle = 0;
  static bool firstTime = true;

  if (firstTime) {
    firstTime = false;

    SetAngle(180);
    _gyro.update();
    double start = _gyro.getAngleZ();
    targetAngle = start + _angle;
  }

  if (!_rotateTo(targetAngle)) return;

  _Stop();
  firstTime = true;
  _iState = I_CHECK_2;
}

void Conducteur::_iCheck1State(int dist) {
    if (dist < _limitDist) {
      _iState = I_TURN_180;
      return;
    }
    _iState = I_NONE;
    SetFState(ON_LINE);
}

void Conducteur::_iCheck2State(int dist) {
    if (dist < _limitDist) _deliveryDone = true;
    _iState = I_NONE;
}




void Conducteur::_TurnRight() {
  _encoderRight.setMotorPwm(_turnSpeed);
  _encoderLeft.setMotorPwm(_turnSpeed);
}

void Conducteur::_TurnRightTo() {
    static double targetAngle = 0;

    if (firstTime) {
        _firstTime = false;
        _turnSuccess = false;

        _gyro.update();
        double start = _gyro.getAngleZ();

        SetAngle(_angle);               
        targetAngle = start + _angle;
    }

    if (!_rotateTo(targetAngle)) return;

    _turnSuccess = true;
    _cState = STOP;

    _gyroPid.ResetValues();
    _gyroPid.SetTarget(_gyro.getAngleZ());
}

void Conducteur::_TurnLeft() {
  _encoderRight.setMotorPwm(-_turnSpeed);
  _encoderLeft.setMotorPwm(-_turnSpeed);
}

void Conducteur::_TurnLeftTo() {
    static double targetAngle = 0;

    if (firstTime) {
      _firstTime = false;
      _turnSuccess = false;

      _gyro.update();
      double start = _gyro.getAngleZ();

      targetAngle = start - _angle;
    }

    if (!_rotateTo(targetAngle)) return;

    _turnSuccess = true;
    _cState = STOP;

    _gyroPid.ResetValues();
    _gyroPid.SetTarget(_gyro.getAngleZ());
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

  if (_cState == state) return;

  if (state == LTURNING || state == RTURNING) {
    _turnSuccess = false;
  }
  _cState = state;
  _firstTime = true;

  if (state == FORWARD || state == BACKWARD) {
    _gyro.update();
    _gyroPid.ResetValues();
    _gyroPid.SetTarget(_gyro.getAngleZ());
  }
}

void Conducteur::SetFState(FollowState state) {

  if (_fState == state) return;

  _fState = state;
  _fSubFirstTime = true;
}

void Conducteur::SetDriveMode(DriveState state) {
  _dState = state;
}

void Conducteur::SetSpeed(int speed) {
  _speed = constrain(speed, _minSpeed, _maxSpeed);
}

void Conducteur::SetTurnSpeed(int speed) {
  _turnSpeed = speed;
}

void Conducteur::SetGyroPID(double p, double i, double d) {
  _kp = p;
  _ki = i;
  _kd = d;
  _gyroPid.SetPID(p, i, d);
}

void Conducteur::SetTrackerPID(double p, double i, double d) {
  _kp = p;
  _ki = i;
  _kd = d;
  _trackerPid.SetPID(p, i, d);
}

void Conducteur::SetDistance(float distance) {
  _distance = distance;
}


//Tracker

void Conducteur::DebugPrint() {
  _tracker.DebugPrint();
}

bool Conducteur::_rotateTo(double targetAngle) {
    _gyro.update();
    float current = _gyro.getAngleZ();

    float delta = current - targetAngle;
    if (delta > 180) delta -= 360;
    if (delta < -180) delta += 360;

    bool reached = (fabs(delta) <= 2);

    if (!reached) {
      int speed = (delta > 0) ? -_turnSpeed : _turnSpeed;
      _encoderRight.setMotorPwm(speed);
      _encoderLeft.setMotorPwm(speed);
    }

    return reached;
}

bool Conducteur::IsDeliveryDone() const { return _deliveryDone; }

bool Conducteur::IsIntersection() const { return _tracker.IsIntersection(); }