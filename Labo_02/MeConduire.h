#pragma once
#include <MeAuriga.h>


#define GYRO_ADRESS 0x69


class Conduire {
  private:

    MeGyro _gyro;

    int _minSpeed;
    int _maxSpeed;
    int _speed;
    int _turnSpeed;

    //Motor Left
    int _m1_pwm;
    int _m1_in1; // M1 ENA
    int _m1_in2; // M1 ENB

    //Motor Right
    int _m2_pwm;
    int _m2_in1; // M2 ENA
    int _m2_in2; // M2 ENB
    
  public:

    Conduire();
    void Stop();
    void TurnRight(int angle, int speed);
    void Forward(int speed);
    void Backward(int speed);
    void TurnLeft(int angle, int speed);
    void UTurn(int speed);
    void Setup();
    void SetMaxSpeed(int speed);
    void SetMinSpeed(int speed);
    int GetSpeed() const;
};