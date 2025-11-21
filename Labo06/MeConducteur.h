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
  NO_LINE,
  INTERSECTION
};

enum IntersectionState {
  I_NONE,
  I_TURN_90,
  I_CHECK_1,
  I_TURN_180,
  I_CHECK_2
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
  IntersectionState _iState;



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
  int _limitDist;

  bool _delayRunning;
  int _afterFollowDelay;
  unsigned long _currentTime;

  bool _firstTime;
  bool _fSubFirstTime;
  bool _turnSuccess;
  bool _deliveryDone;

  void _Stop();
  void _TurnRight();
  void _TurnRightTo();
  void _DriveTo();
  void _Drive();
  void _TurnLeft();
  void _TurnLeftTo();

  void _FollowLine(int dist);
  void _Calibrate_IR();
  bool _rotateTo(double targetAngle);
  void _noLineState();
  void _onLineState();
  void _onRightAngle();
  void _onIntersection(int dist);

  void _iTurnState(int angle, IntersectionState nextState);
  void _iCheck1State(int dist);
  void _iCheck2State(int dist);

  void _isr_process_encoder1(void);
  void _isr_process_encoder2(void);

public:

  Conducteur();

  void Setup();
  void Update(int obstacleDist = 0);

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
  float GetDistanceTraveled();

  void DebugPrint();

  bool IsDeliveryDone() const { return _deliveryDone; }
};