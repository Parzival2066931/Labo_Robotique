#include <Arduino.h>
#include "MeAuriga.h"
#include "MeConduire.h"

Conduire::Conduire()
  : _gyro(0, GYRO_ADRESS) {}

void Conduire::Setup() {
  _gyro.begin();

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

  Serial.println("Setup completed for [Conduire]");
}

int Conduire::GetSpeed() const {
  return _speed;
}
void Conduire::Stop() {
  analogWrite(_m1_pwm, 0);
  analogWrite(_m2_pwm, 0);
}
void Conduire::Forward(int speed) {
  digitalWrite(_m1_in2, LOW);
  digitalWrite(_m1_in1, HIGH);
  analogWrite(_m1_pwm, speed);

  digitalWrite(_m2_in2, HIGH);
  digitalWrite(_m2_in1, LOW);
  analogWrite(_m2_pwm, speed);
}

void Conduire::Backward(int speed) {
  digitalWrite(_m1_in2, HIGH);
  digitalWrite(_m1_in1, LOW);
  analogWrite(_m1_pwm, speed);

  digitalWrite(_m2_in2, LOW);
  digitalWrite(_m2_in1, HIGH);
  analogWrite(_m2_pwm, speed);
}

void Conduire::TurnRight(int angle, int speed) {
  _gyro.update();
  float currentAngle;
  float startAngle = _gyro.getAngle(3);
  float target = startAngle + angle;
  if (target > 180) target -= 360;

  digitalWrite(_m1_in1, HIGH);
  digitalWrite(_m1_in2, LOW);
  analogWrite(_m1_pwm, speed);

  digitalWrite(_m2_in1, LOW);
  digitalWrite(_m2_in2, HIGH);
  analogWrite(_m2_pwm, speed);

  do {
    _gyro.update();
    currentAngle = _gyro.getAngle(3);
  } while(fabs(currentAngle - target) >= 2);
  Stop();
}

void Conduire::TurnLeft(int angle, int speed) {
  _gyro.update();
  float currentAngle;
  float startAngle = _gyro.getAngle(3);
  float target = startAngle - angle;
  if (target < -180) target += 360;

  
  digitalWrite(_m1_in1, LOW);
  digitalWrite(_m1_in2, HIGH);
  analogWrite(_m1_pwm, speed);
  
  digitalWrite(_m2_in1, HIGH);
  digitalWrite(_m2_in2, LOW);
  analogWrite(_m2_pwm, speed);

  do {
    _gyro.update();
    currentAngle = _gyro.getAngle(3);
  } while(fabs(currentAngle - target) >= 2);
  Stop();
}