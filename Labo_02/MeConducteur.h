#pragma once
#include <MeAuriga.h>


#define GYRO_ADRESS 0x69

enum ConducteurState {
  FORWARD,
  BACKWARD,
  STOP,
  RTURNING,
  LTURNING,
};

class Conducteur {
  private:

    MeGyro _gyro;
    ConducteurState _state;

    int _minSpeed;
    int _maxSpeed;
    int _speed;
    int _turnSpeed;
    int _angle;
    bool _turnSuccess;

    //Motor Left
    int _m1_pwm;
    int _m1_in1; // M1 ENA
    int _m1_in2; // M1 ENB

    //Motor Right
    int _m2_pwm;
    int _m2_in1; // M2 ENA
    int _m2_in2; // M2 ENB

    // unsigned long _currentTime;

    void _Stop();
    void _TurnRight();
    void _Forward();
    void _Backward();
    void _TurnLeft();
    
  public:

    Conducteur();
    
    void UTurn();
    void Setup();
    void Update();
    void SetMaxSpeed(int speed);
    void SetMinSpeed(int speed);
    void SetAngle(int angle);
    void SetSpeed(int speed);
    void SetTurnSpeed(int speed);
    void SetState(ConducteurState state);
    int GetSpeed() const;
    bool GetTurnState() const;
};