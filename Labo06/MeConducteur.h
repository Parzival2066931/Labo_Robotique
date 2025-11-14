#pragma once
#include <MeAuriga.h>
#include "PIDControlleur.h"
#include "LyneTracker.h"


#define GYRO_ADRESS 0x69

#define PULSE 9
#define RATIO 39.267
#define DIAMETER 6.5



enum ConducteurState {
  FORWARD,
  BACKWARD,
  STOP,
  RTURNING,
  LTURNING,
  FOLLOW,
  CALIBRATE
};

enum DriveState {
  DISTANCE,
  FREE,
};

enum FollowState {
  ON_LINE,
  TURN_RIGHT,
  TURN_LEFT,
  NO_LINE
  //INTERSECTION
};


class Conducteur {
private:

  MeGyro _gyro;
  Tracker _tracker;
  MeEncoderOnBoard _encoderRight;
  MeEncoderOnBoard _encoderLeft;
  PIDControlleur _gyroPid;
  PIDControlleur _trackerPid;

  ConducteurState _cState;
  DriveState _dState;
  FollowState _fState;

  double _kp = 1;
  double _ki = 1;
  double _kd = 1;

  float _circonference;
  float _distance;
  int _oneRot;
  int _minSpeed;
  int _maxSpeed;
  int _speed;
  int _turnSpeed;
  int _angle;

  bool _firstTime;
  bool _fSubFirstTime;
  bool _turnSuccess;

  void _Stop();
  void _TurnRight();
  void _TurnRightTo();
  void _DriveTo();
  void _Drive();
  void _TurnLeft();
  void _TurnLeftTo();

  void _FollowLine();
  void _Calibrate_IR();
  bool _rotateTo(double targetAngle);
  void _noLineState();
  void _onLineState();
  void _onRightAngle();
  void _onIntersection();

  void _isr_process_encoder1(void);
  void _isr_process_encoder2(void);

public:

  Conducteur();

  void Setup();
  void Update();
  void SetMaxSpeed(int speed);
  void SetMinSpeed(int speed);
  void SetAngle(int angle);
  void SetSpeed(int speed);
  void SetTurnSpeed(int speed);
  void SetState(ConducteurState state);
  void SetFState(FollowState state);
  void SetGyroPID(double p, double i, double d);
  void SetTrackerPID(double p, double i, double d);
  void SetDistance(float distance);
  void SetDriveMode(DriveState state);
  int GetPinRight() const;
  int GetPinLeft() const;
  static void GetIsrRight();
  static void GetIsrLeft();
  int GetSpeed() const;
  ConducteurState GetState() const;
  bool GetTurnState() const;
  long GetDistToGo() const;
  float Conducteur::GetDistanceTraveled();
  void Conducteur::DebugPrint();
};