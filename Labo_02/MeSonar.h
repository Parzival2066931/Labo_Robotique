#pragma once
#include <MeAuriga.h>


class Sonar {
private:
  void _printDist() const;
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
  void setMinDist(int dist);
  void setMaxDist(int dist);
  void setPrintDelay(int delay);
  int getDist();
  int getMinDist() const;
  int getMaxDist() const;
  void update();
  void Setup();
};