#pragma once

class PIDControlleur {
private:

  double _kp;
  double _ki;
  double _kd;

  double _integral;
  double _error;
  double _lastError;
  double _diff;
  double _prop;
  double _correction;

  double _currentValue;
  double _targetValue;
  double _newValue;

  unsigned long _currentTime;
  unsigned long _lastTime;
  double _dt;

  double _calculatePid();

public:
  PIDControlleur(double p, double i, double d);
  void ResetValues();
  void Setup();
  void Update();
  void SetTarget(double value);
  void SetValue(double value);
  void SetPID(double p, double i, double d);
  double GetCorrection() const;
};