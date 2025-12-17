#pragma once
#include <MeAuriga.h>


class Sonar {
private:
  
  unsigned long _currentTime;
  unsigned long _lastUpdate;
  int _lastDist;
  int _dist;
  int _minDist;
  int _maxDist;
  int _printDelay;
  MeUltrasonicSensor _sensor;

public:
  Sonar();
  void printDist() const;
  void SetMinDist(int dist);
  void SetMaxDist(int dist);
  void SetPrintDelay(int delay);
  int GetDist();
  int GetMinDist() const;
  int GetMaxDist() const;
  void Update();
  void Setup();
};