#include <Arduino.h>
#include "PIDControlleur.h"


PIDControlleur::PIDControlleur(double p, double i, double d) {
  _kp = p;
  _ki = i;
  _kd = d;

  _integral = 0;
  _error = 0;
  _lastError = 0;
  _diff = 0;
  _prop = 0;
  _correction = 0;
  _newValue = 0;
}

double PIDControlleur::_calculatePid() {
  
  _error = _targetValue - _currentValue;

  _prop = _kp * _error;
  _integral += _ki * _error * _dt;    
  _integral = constrain(_integral, -255, 255);
  _diff = _kd * (_error - _lastError) / _dt; 

  

  _correction = _prop + _integral + _diff;

  _lastError = _error;
  return _correction;
}

void PIDControlleur::Update() {
  _currentTime = millis();
  _dt = (_currentTime - _lastTime) / 1000.0;
  _lastTime = _currentTime;

  _newValue = _calculatePid();
}

void PIDControlleur::Setup() {
  _lastTime = millis();
}

void PIDControlleur::SetTarget(double value) {
  _targetValue = value;
}

void PIDControlleur::SetValue(double value) {
  _currentValue = value;
}

void PIDControlleur::SetPID(double p, double i, double d) {
  _kp = p;
  _ki = i;
  _kd = d;
}

double PIDControlleur::GetCorrection() const {
  return _newValue;
}

void PIDControlleur::ResetValues() {
  _integral = 0;
  _error = 0;
  _lastError = 0;
  _diff = 0;
  _prop = 0;
  _correction = 0;
  _newValue = 0;

  _lastTime = millis();
}