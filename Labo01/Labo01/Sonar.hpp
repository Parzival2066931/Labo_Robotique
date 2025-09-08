#include <HCSR04.h>

#pragma once

class Sonar {
  private:
    void _printDist() const;
    unsigned long _currentTime;
    unsigned long _lastUpdate = 0;
    int _rate = 100;
    int _dist;
    int _minDist;
    int _maxDist;
    HCSR04 _hc;

  public:
    Sonar(int TRIGGER_PIN, int ECHO_PIN);
    void setMinDist(int dist);
    void setMaxDist(int dist);
    int getDist();
    void update();
    void setup();
};
