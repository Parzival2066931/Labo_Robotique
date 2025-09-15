#pragma once
#include "MeConduire.h"
#include "MeSonar.h"
#include "MeAnneau.h"



#pragma once

enum RobotState {NORMAL, RALENTI, DANGER, RONDE};

class Patrouille {
  private:
    int _dist;
    int _minDist;
    int _dangerDist;
    int _slowDist;

    int _backwardDelay;
    int _stopDelay;
    int _printDelay;
    int _rondeDelay;
    int _blinkDelay;
    int _rondeStateDelay;
    
    unsigned long _currentTime;
    unsigned long _lastStop;
    unsigned long _lastBackward;
    unsigned long _lastRonde;
    unsigned long _lastBlink;
    unsigned long _lastSuccess;

    bool _blinkState;
    bool _hasTurned;


    int _turnAngle;
    int _turnSpeed;
    int _normalSpeed;
    int _slowSpeed;
    int _maxSpeed;
    
    RobotState _state;
    Conduire _conduit;
    Anneau _anneau;
    Sonar _sonar;

    void _normalState();
    void _slowState();
    void _dangerState();
    void _rondeState();

    void _printTask();


  public:
    Patrouille();

    void setBackwardDelay(int delay);
    void setStopDelay(int delay);
    void setPrintDelay(int delay);
    void setTurnAngle(int angle);
    void setNormalSpeed(int speed);
    void setSlowSpeed(int speed);
    void Update();
    void Setup();
};